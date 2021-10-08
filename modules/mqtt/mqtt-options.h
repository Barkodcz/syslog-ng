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

#ifndef MQTT_OPTIONS
#define MQTT_OPTIONS

#include "driver.h"
#include <glib.h>
#include <MQTTClient.h>

#define DISCONNECT_TIMEOUT 10000

typedef struct
{
  gchar *fallback_topic_name;
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

} MQTTOptions;

void mqtt_option_set_default_value(MQTTOptions *self);
void mqtt_option_free(MQTTOptions *self);

void mqtt_option_set_fallback_topic(MQTTOptions *o, const gchar *fallback_topic);
void mqtt_option_set_keepalive (MQTTOptions *o, const gint keepalive);
void mqtt_option_set_address(MQTTOptions *o, const gchar *address);
void mqtt_option_set_qos (MQTTOptions *o, const gint qos);
void mqtt_option_set_client_id(MQTTOptions *o, const gchar *client_id);

void mqtt_option_set_username(MQTTOptions *o, const gchar *username);
void mqtt_option_set_password(MQTTOptions *o, const gchar *password);
void mqtt_option_set_http_proxy(MQTTOptions *o, const gchar *http_proxy);

void mqtt_option_set_ca_dir(MQTTOptions *o, const gchar *ca_dir);
void mqtt_option_set_ca_file(MQTTOptions *o, const gchar *ca_file);
void mqtt_option_set_cert_file(MQTTOptions *o, const gchar *cert_file);
void mqtt_option_set_key_file(MQTTOptions *o, const gchar *key_file);
void mqtt_option_set_cipher_suite(MQTTOptions *o, const gchar *ciphers);
gboolean mqtt_option_set_ssl_version(MQTTOptions *o, const gchar *value);
void mqtt_option_set_peer_verify(MQTTOptions *o, gboolean verify);
void mqtt_option_use_system_cert_store(MQTTOptions *o, gboolean use_system_cert_store);

gboolean mqtt_option_validate_address(const gchar *address);

gboolean mqtt_create(MQTTClient *client, gchar *address, gchar *client_id);
void mqtt_destroy(MQTTClient *client);
gboolean mqtt_connect(MQTTClient *client, MQTTOptions *option, gpointer context,
                      gint(*log_error)(const gchar *str, gsize len, gpointer u));

#endif /* MQTT_OPTIONS */
