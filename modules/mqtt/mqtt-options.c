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

#include "mqtt-options.h"
#include "syslog-ng.h"

#include <MQTTClient.h>
#include <string.h>
#include <stddef.h>


#define DEFAULT_ADDRESS "tcp://localhost:1883"
#define DEFAULT_KEEPALIVE 60
#define DEFAULT_QOS 0

void
mqtt_option_set_default_value(MQTTOptions *self)
{
  self->keepalive = DEFAULT_KEEPALIVE;
  self->qos = DEFAULT_QOS;

  self->ssl_version = MQTT_SSL_VERSION_DEFAULT;
  self->peer_verify = TRUE;
  self->use_system_cert_store = FALSE;
}

void
mqtt_option_free(MQTTOptions *self)
{
  g_free(self->address);

  g_free(self->username);
  g_free(self->password);
  g_free(self->http_proxy);

  g_free(self->ca_dir);
  g_free(self->ca_file);
  g_free(self->cert_file);
  g_free(self->key_file);
  g_free(self->ciphers);

  g_free(self->fallback_topic_name);
}

void
mqtt_option_set_fallback_topic(MQTTOptions *self, const gchar *fallback_topic)
{
  g_free(self->fallback_topic_name);
  self->fallback_topic_name = g_strdup(fallback_topic);
}

void
mqtt_option_set_keepalive(MQTTOptions *self, const gint keepalive)
{
  self->keepalive = keepalive;
}

void
mqtt_option_set_address(MQTTOptions *self, const gchar *address)
{
  g_free(self->address);
  self->address = g_strdup(address);
}

void
mqtt_option_set_qos (MQTTOptions *self, const gint qos)
{
  self->qos = qos;
}


void
mqtt_option_set_username(MQTTOptions *self, const gchar *username)
{
  g_free(self->username);
  self->username = g_strdup(username);
}

void
mqtt_option_set_password(MQTTOptions *self, const gchar *password)
{
  g_free(self->password);
  self->password = g_strdup(password);
}

void
mqtt_option_set_http_proxy(MQTTOptions *self, const gchar *http_proxy)
{
  g_free(self->http_proxy);
  self->http_proxy = g_strdup(http_proxy);
}


void
mqtt_option_set_ca_dir(MQTTOptions *self, const gchar *ca_dir)
{
  g_free(self->ca_dir);
  self->ca_dir = g_strdup(ca_dir);
}

void
mqtt_option_set_ca_file(MQTTOptions *self, const gchar *ca_file)
{
  g_free(self->ca_file);
  self->ca_file = g_strdup(ca_file);
}

void
mqtt_option_set_cert_file(MQTTOptions *self, const gchar *cert_file)
{
  g_free(self->cert_file);
  self->cert_file = g_strdup(cert_file);
}

void
mqtt_option_set_key_file(MQTTOptions *self, const gchar *key_file)
{
  g_free(self->key_file);
  self->key_file = g_strdup(key_file);
}

void
mqtt_option_set_cipher_suite(MQTTOptions *self, const gchar *ciphers)
{
  g_free(self->ciphers);
  self->ciphers = g_strdup(ciphers);
}

gboolean
mqtt_option_set_ssl_version(MQTTOptions *self, const gchar *value)
{
  if (strcasecmp(value, "default") == 0)
    self->ssl_version = MQTT_SSL_VERSION_DEFAULT;
  else if (strcasecmp(value, "tlsv1_0") == 0)
    self->ssl_version = MQTT_SSL_VERSION_TLS_1_0;
  else if (strcasecmp(value, "tlsv1_1") == 0)
    self->ssl_version = MQTT_SSL_VERSION_TLS_1_1;
  else if (strcasecmp(value, "tlsv1_2") == 0)
    self->ssl_version = MQTT_SSL_VERSION_TLS_1_2;
  else
    return FALSE;

  return TRUE;
}

void
mqtt_option_set_peer_verify(MQTTOptions *self, gboolean verify)
{
  self->peer_verify = verify;
}

void
mqtt_option_use_system_cert_store(MQTTOptions *self, gboolean use_system_cert_store)
{
  /* TODO: auto_detect_ca_file() from the HTTP module */
  self->use_system_cert_store = use_system_cert_store;
}


static gboolean
_validate_protocol(const gchar *address)
{
  const gchar *valid_type[] = {"tcp", "ssl", "ws", "wss"};
  gint i;

  for (i = 0; i < G_N_ELEMENTS(valid_type); ++i)
    if (strncmp(valid_type[i], address, strlen(valid_type[i])) == 0)
      return TRUE;

  return FALSE;
}

gboolean
mqtt_option_validate_address(const gchar *address)
{
  if (strstr(address, "://") == NULL)
    return FALSE;

  if (!_validate_protocol(address))
    return FALSE;

  return TRUE;
}


static MQTTClient_SSLOptions
_create_ssl_options(MQTTOptions *option, gpointer context, gint(*log_error)(const gchar *str, gsize len, gpointer u))
{
  MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;
  ssl_opts.trustStore = option->ca_file;
  ssl_opts.CApath = option->ca_dir;
  ssl_opts.keyStore = option->cert_file;
  ssl_opts.privateKey = option->key_file;
  ssl_opts.enabledCipherSuites = option->ciphers;
  ssl_opts.sslVersion = option->ssl_version;
  ssl_opts.enableServerCertAuth = option->peer_verify;
  ssl_opts.verify = option->peer_verify;
  ssl_opts.disableDefaultTrustStore = !option->use_system_cert_store;
  ssl_opts.ssl_error_cb = log_error;
  ssl_opts.ssl_error_context = context;

  return ssl_opts;
}

gboolean
mqtt_connect(MQTTClient *client, MQTTOptions *option, gpointer context, LogDriver *driver,
             gint(*log_error)(const gchar *str, gsize len, gpointer u))
{
  gint rc;

  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.keepAliveInterval = option->keepalive;
  conn_opts.cleansession = FALSE;
  conn_opts.username = option->username;
  conn_opts.password = option->password;

#if SYSLOG_NG_HAVE_PAHO_HTTP_PROXY
  if (option->http_proxy)
    {
      conn_opts.httpProxy = option->http_proxy;
      conn_opts.httpsProxy = option->http_proxy;
    }
#endif

  MQTTClient_SSLOptions ssl_opts = _create_ssl_options(option, context, log_error);
  conn_opts.ssl = &ssl_opts;

  if ((rc = MQTTClient_connect(*client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
      msg_error("Error connecting mqtt client",
                evt_tag_str("error code", MQTTClient_strerror(rc)),
                evt_tag_str("driver", driver->id),
                log_pipe_location_tag(&driver->super));
      return FALSE;
    }

  return TRUE;
}

gboolean
mqtt_create(MQTTClient *client, gchar *address, LogDriver *driver)
{
  gint rc;

  if ((rc = MQTTClient_create(client, address,
                              log_pipe_get_persist_name(&driver->super),
                              MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
      msg_error("Error creating mqtt client",
                evt_tag_str("address", address),
                evt_tag_str("error code", MQTTClient_strerror(rc)),
                evt_tag_str("driver", driver->id),
                log_pipe_location_tag(&driver->super));
      return FALSE;
    }
  return TRUE;
}

void
mqtt_destroy(MQTTClient *client)
{
  MQTTClient_destroy(client);
}
