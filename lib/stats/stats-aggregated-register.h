/*
 * Copyright (c) 2021 One Identity
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#ifndef STATS_AGGREGATED_REGISTER_H
#define STATS_AGGREGATED_REGISTER_H

#include "stats/stats-aggregated.h"
#include "stats/stats-registry.h"
#include "syslog-ng.h"

void stats_aggregated_lock(void);
void stats_aggregated_unlock(void);

void stats_aggregated_registry_init(void);
void stats_aggregated_registry_deinit(void);

void stats_register_aggregated_maximum(gint level, guint16 component, const gchar *id, const gchar *instance,
                                       StatsAggregatedItem **s);
void stats_unregister_aggregated_maximum(StatsAggregatedItem *s);

void stats_register_aggregated_average(gint level, guint16 component, const gchar *id, const gchar *instance,
                                       StatsAggregatedItem **s);
void stats_unregister_aggregated_average(StatsAggregatedItem *s);

void stats_register_aggregated_eps(gint level, guint16 component, const gchar *id, const gchar *instance,
                                   StatsCounterItem *counter, StatsAggregatedItem **s);
void stats_unregister_aggregated_eps(StatsAggregatedItem *s);


#endif /* STATS_AGGREGATED_REGISTER_H */