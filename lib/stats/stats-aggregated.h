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

#ifndef STATS_AGGREGATED_H
#define STATS_AGGREGATED_H

#include "stats/stats-counter.h"
#include "syslog-ng.h"
#include "stats-cluster.h"

#define HOUR_IN_SEC 3600 /* 60*60 */
#define DAY_IN_SEC 86400 /* 60*60*24 */

typedef struct _StatsAggregatedItem StatsAggregatedItem;

struct _StatsAggregatedItem
{
  void (*feed_input)(StatsAggregatedItem *self, gsize value);
  void (*update)(StatsAggregatedItem *self, time_t *now);
  void (*free)(StatsAggregatedItem *self);

  gssize use_count;
  StatsClusterKey key;
};

gboolean stats_aggregated_is_orphaned(StatsAggregatedItem *self);

void stats_aggregated_track_counter(StatsAggregatedItem *self);
void stats_aggregated_untrack_counter(StatsAggregatedItem *self);

void stats_aggregated_feed_input(StatsAggregatedItem *self, gsize value);
void stats_aggregated_update(StatsAggregatedItem *self, time_t *now);
void stats_aggregated_free(StatsAggregatedItem *self);

void stats_aggregated_item_init(StatsAggregatedItem *self);

void stats_aggregated_maximum_create_stats_key(StatsClusterKey *key, guint16 component, const gchar *id,
                                               const gchar *instance);
void stats_init_aggregated_maximum(gint level, guint16 component, const gchar *id, const gchar *instance,
                                   StatsAggregatedItem **s);
void stats_deinit_aggregated_maximum(StatsAggregatedItem *s);

void stats_aggregated_average_create_stats_key(StatsClusterKey *key, guint16 component, const gchar *id,
                                               const gchar *instance);
void stats_init_aggregated_average(gint level, guint16 component, const gchar *id, const gchar *instance,
                                   StatsAggregatedItem **s);
void stats_deinit_aggregated_average(StatsAggregatedItem *s);

void stats_aggregated_eps_create_stats_key(StatsClusterKey *key, guint16 component, const gchar *id,
                                           const gchar *instance);
void stats_init_aggregated_eps(gint level, guint16 component, const gchar *id, const gchar *instance,
                               StatsCounterItem *counter, StatsAggregatedItem **s);
void stats_deinit_aggregated_eps(StatsAggregatedItem *s);

#endif /* STATS_AGGREGATED_H */