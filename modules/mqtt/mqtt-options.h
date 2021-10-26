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

#ifndef MQTT_CLIENT_OPTIONS_INCLUDED
#define MQTT_CLIENT_OPTIONS_INCLUDED

#include "driver.h"
#include <MQTTClient.h>

#define MQTT_DISCONNECT_TIMEOUT 10000

typedef struct _MQTTClientOptions MQTTClientOptions;

struct _MQTTClientOptions
{
  gint keepalive;
  gchar *address;
  gint qos;
  gchar *client_id;

  gchar *username;
  gchar *password;
  gchar *http_proxy;

  gchar *ca_dir;
  gchar *ca_file;
  gchar *cert_file;
  gchar *key_file;
  gchar *ciphers;
  gint ssl_version;
  gboolean peer_verify;
  gboolean use_system_cert_store;
  gpointer context;
  gint(*log_error) (const gchar *str, gsize len, gpointer u);
};

void mqtt_client_options_set_default_value(MQTTClientOptions *self);
void mqtt_client_options_destroy(MQTTClientOptions *self);

void mqtt_client_options_set_keepalive (MQTTClientOptions *o, const gint keepalive);
gboolean mqtt_client_options_set_address(MQTTClientOptions *o, const gchar *address);
void mqtt_client_options_set_qos (MQTTClientOptions *o, const gint qos);
void mqtt_client_options_set_client_id(MQTTClientOptions *o, const gchar *client_id);

void mqtt_client_options_set_username(MQTTClientOptions *o, const gchar *username);
void mqtt_client_options_set_password(MQTTClientOptions *o, const gchar *password);
void mqtt_client_options_set_http_proxy(MQTTClientOptions *o, const gchar *http_proxy);

void mqtt_client_options_set_ca_dir(MQTTClientOptions *o, const gchar *ca_dir);
void mqtt_client_options_set_ca_file(MQTTClientOptions *o, const gchar *ca_file);
void mqtt_client_options_set_cert_file(MQTTClientOptions *o, const gchar *cert_file);
void mqtt_client_options_set_key_file(MQTTClientOptions *o, const gchar *key_file);
void mqtt_client_options_set_cipher_suite(MQTTClientOptions *o, const gchar *ciphers);
gboolean mqtt_client_options_set_ssl_version(MQTTClientOptions *o, const gchar *value);
void mqtt_client_options_set_peer_verify(MQTTClientOptions *o, gboolean verify);
void mqtt_client_options_use_system_cert_store(MQTTClientOptions *o, gboolean use_system_cert_store);

void mqtt_client_options_set_context(MQTTClientOptions *o, gpointer context);
void mqtt_client_options_set_log_error_fn(MQTTClientOptions *o, gint(*log_error)(const gchar *str, gsize len,
                                          gpointer u));


void mqtt_client_options_set_context(MQTTClientOptions *o, gpointer context);
void mqtt_client_options_set_log_error_fn(MQTTClientOptions *o, gint(*log_error)(const gchar *str, gsize len,
                                          gpointer u));
gchar *mqtt_client_options_get_address(MQTTClientOptions *self);
gint mqtt_client_options_get_qos(MQTTClientOptions *self);
gchar *mqtt_client_options_get_client_id(MQTTClientOptions *self);

void mqtt_client_options_to_mqtt_connection_option(MQTTClientOptions *self, MQTTClient_connectOptions *conn_opts);

gboolean mqtt_client_options_checker(MQTTClientOptions *self);

#endif /* MQTT_CLIENT_OPTIONS_INCLUDED */
