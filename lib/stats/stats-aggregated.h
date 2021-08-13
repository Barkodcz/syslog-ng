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

#endif /* STATS_AGGREGATED_H */