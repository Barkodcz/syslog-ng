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

#ifndef MQTT_SOURCE_H_INCLUDE
#define MQTT_SOURCE_H_INCLUDE

#include "mqtt-options.h"
#include "logthrsource/logthrfetcherdrv.h"
#include "atomic.h"
#include <MQTTClient.h>

typedef struct _MQTTSourceDriver MQTTSourceDriver;

struct _MQTTSourceDriver
{
  LogThreadedFetcherDriver super;
  MQTTOptions option;
  MQTTClient client;
  gchar **topics;
  guint topics_len;
  gchar *topic_in_one;
};


LogDriver *mqtt_sd_new(GlobalConfig *cfg);
MQTTOptions *mqtt_sd_get_options(LogDriver *s);
void mqtt_sd_set_topics(LogDriver *s, GList *list);

#endif /* MQTT_SOURCE_H_INCLUDE */
