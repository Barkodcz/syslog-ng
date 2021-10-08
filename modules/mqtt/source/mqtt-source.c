/*
 * Copyright (c) 2021 One Identity
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "mqtt-source.h"
#include <MQTTClient.h>
#include "messages.h"

#define RECIVE_TIMEOUT 1000

const gchar *
_format_persist_name(const LogPipe *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *) s;
  static gchar stats_instance[1024];

  if (((LogPipe *)s)->persist_name)
    g_snprintf(stats_instance, sizeof(stats_instance), "%s", ((LogPipe *)s)->persist_name);
  else
    g_snprintf(stats_instance, sizeof(stats_instance), "mqtt,source,%s,%s", self->option.address,
               self->topic_in_one);

  return stats_instance;
}

static const gchar *
_format_stats_instance(LogThreadedSourceDriver *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *) s;
  LogPipe *p = &s->super.super.super;
  static gchar persist_name[1024];

  if (p->persist_name)
    g_snprintf(persist_name, sizeof(persist_name), "mqtt-source.%s", p->persist_name);
  else
    g_snprintf(persist_name, sizeof(persist_name), "mqtt-source.(%s,%s)", self->option.address,
               self->topic_in_one);

  return persist_name;
}

static LogMessage *
_create_message(MQTTSourceDriver *self, const gchar *message, gint length)
{
  LogMessage *msg = log_msg_new_empty();
  log_msg_set_value_by_name(msg, "MSG", message, length);

  return msg;
}

static gboolean
_client_init(MQTTSourceDriver *self)
{
  return mqtt_create(&self->client, self->option.address, self->option.client_id);
}

static gint
_log_ssl_errors(const gchar *str, gsize len, gpointer u)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *) u;

  msg_error("MQTT TLS error", evt_tag_printf("line", "%.*s", (gint) len, str),
            evt_tag_str("client_id", self->option.client_id));
  return TRUE;
}

static gboolean
_subscribe_topic(MQTTSourceDriver *self)
{
  gint *qoss = g_new0(gint, self->topics_len);
  for(int i = 0; i < self->topics_len; ++i)
    qoss[i] = self->option.qos;

  gint rc;
  gboolean result = TRUE;
  if ((rc = MQTTClient_subscribeMany(self->client, self->topics_len, self->topics, qoss)) != MQTTCLIENT_SUCCESS)
    {
      msg_error("Error while setting callbacks",
                evt_tag_str("topics", self->topic_in_one),
                evt_tag_str("error code", MQTTClient_strerror(rc)),
                evt_tag_str("client_id", self->option.client_id));
      result = FALSE;
    }

  g_free(qoss);
  return result;
}

static void
_thread_init(LogThreadedFetcherDriver *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *)s;
  _client_init(self);
}

static void
_thread_deinit(LogThreadedFetcherDriver *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *)s;

  mqtt_destroy(&self->client);
}

static gboolean
_connect(LogThreadedFetcherDriver *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *)s;

  if (!mqtt_connect(&self->client, &self->option, self, _log_ssl_errors))
    return FALSE;

  if (!_subscribe_topic(self))
    return FALSE;

  return TRUE;
}

static gboolean
_unsubscribe_topic(MQTTSourceDriver *self)
{
  gint rc;
  if ((rc = MQTTClient_unsubscribeMany(self->client, self->topics_len, self->topics)) != MQTTCLIENT_SUCCESS)
    {
      return FALSE;
    }

  return TRUE;
}

static void
_disconnect(LogThreadedFetcherDriver *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *)s;

  _unsubscribe_topic(self);

  MQTTClient_disconnect(self->client, DISCONNECT_TIMEOUT);
}

static LogThreadedFetchResult
_fetch(LogThreadedFetcherDriver *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *)s;
  ThreadedFetchResult result = THREADED_FETCH_ERROR;
  char *topicName = NULL;
  int topicLen;
  MQTTClient_message *message = NULL;
  LogMessage *msg = NULL;
  gint rc = MQTTClient_receive(self->client, &topicName, &topicLen, &message, RECIVE_TIMEOUT);

  if (message != NULL)
    {
      msg = _create_message(self, (gchar *)message->payload, message->payloadlen);
      result = THREADED_FETCH_SUCCESS;
      MQTTClient_freeMessage(&message);
      MQTTClient_free(topicName);
    }
  else
    result = THREADED_FETCH_TRY_AGAIN;

  if (rc != 0)
    result = THREADED_FETCH_NOT_CONNECTED;

  return (LogThreadedFetchResult)
  {
    result, msg
  };
}

static gboolean
_init(LogPipe *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *)s;

  if(!log_threaded_fetcher_driver_init_method(s))
    return FALSE;

  if (self->option.client_id == NULL)
    mqtt_option_set_client_id(&self->option, _format_persist_name(s));

  return TRUE;
}

static gboolean
_deinit(LogPipe *s)
{
  return log_threaded_fetcher_driver_deinit_method(s);
}

static void
_free_up_topics(MQTTSourceDriver *self)
{
  if (self->topics == NULL)
    return;

  for (int i = 0; i < self->topics_len; ++i)
    g_free(self->topics[i]);

  g_free(self->topics);
  g_free(self->topic_in_one);
}

static void
_free(LogPipe *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *)s;
  mqtt_option_free(&self->option);
  _free_up_topics(self);

  log_threaded_fetcher_driver_free_method(s);
}

LogDriver *
mqtt_sd_new(GlobalConfig *cfg)
{
  MQTTSourceDriver *self = g_new0(MQTTSourceDriver, 1);

  log_threaded_fetcher_driver_init_instance(&self->super, cfg);

  mqtt_option_set_default_value(&self->option);

  self->super.super.super.super.super.init = _init;
  self->super.super.super.super.super.deinit = _deinit;
  self->super.super.super.super.super.free_fn = _free;

  self->super.connect = _connect;
  self->super.disconnect = _disconnect;
  self->super.fetch = _fetch;
  self->super.thread_init = _thread_init;
  self->super.thread_deinit = _thread_deinit;

  self->super.super.super.super.super.generate_persist_name = _format_persist_name;
  self->super.super.format_stats_instance = _format_stats_instance;
  return &self->super.super.super.super;
}

MQTTOptions *
mqtt_sd_get_options(LogDriver *s)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *)s;

  return &self->option;
}

void
mqtt_sd_set_topics(LogDriver *s, GList *list)
{
  MQTTSourceDriver *self = (MQTTSourceDriver *)s;

  _free_up_topics(self);

  self->topics_len = g_list_length(list);
  self->topics = g_new0(gchar *, self->topics_len);
  int i = 0;
  GString *str = g_string_new("");

  for (GList *it = list; it != NULL; it = it->next)
    {
      self->topics[i++] = g_strdup(it->data);
      str = g_string_append(str, it->data);
    }

  self->topic_in_one = g_strdup(str->str);
  g_string_free(str, TRUE);
}
