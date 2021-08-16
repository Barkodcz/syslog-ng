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

#include "stats/stats-aggregated.h"
#include "stats/stats-registry.h"
#include "stats/stats-cluster-single.h"

typedef struct
{
  StatsAggregatedItem super;
  StatsCounterItem *item;
  atomic_gssize count;
  atomic_gssize sum;
} StatsAggregatedAverageItem;

static inline void
_inc_count(StatsAggregatedAverageItem *self)
{
  atomic_gssize_inc(&self->count);
}

static inline void
_add_sum(StatsAggregatedAverageItem *self, gsize value)
{
  atomic_gssize_add(&self->sum, value);
}

static inline gsize
_get_sum(StatsAggregatedAverageItem *self)
{
  return (gsize)atomic_gssize_get(&self->sum);
}

static inline gsize
_get_count(StatsAggregatedAverageItem *self)
{
  return (gsize)atomic_gssize_get(&self->count);
}

static void
_new(StatsAggregatedItem **self)
{
  *self = (StatsAggregatedItem *) g_new0(StatsAggregatedAverageItem, 1);
}

static void
_free(StatsAggregatedItem *self)
{
  g_free(self);
}

static void
_update(StatsAggregatedItem *s, time_t *now)
{
  StatsAggregatedAverageItem *self = (StatsAggregatedAverageItem *)s;
  stats_counter_set(self->item, (_get_sum(self) / _get_count(self)));
}

static void
_feed_input(StatsAggregatedItem *s, gsize value)
{
  StatsAggregatedAverageItem *self = (StatsAggregatedAverageItem *)s;

  if (self != NULL)
    {
      _inc_count(self);
      _add_sum(self, value);
      _update(s, NULL);
    }
}

static void
_set_virtual_function(StatsAggregatedAverageItem *self )
{
  self->super.feed_input = _feed_input;
  self->super.free = stats_deinit_aggregated_average;
}

static void
stats_aggregated_average_init(StatsAggregatedAverageItem *self)
{
  stats_aggregated_item_init(&self->super);
  _set_virtual_function(self);
}

void
stats_aggregated_average_create_stats_key(StatsClusterKey *key, guint16 component, const gchar *id,
                                          const gchar *instance)
{
  stats_cluster_single_key_set_with_name(key, component, id, instance, "average_message_size");
}

static void
_regist_counter(StatsAggregatedAverageItem *self, gint level, guint16 component, const gchar *id, const gchar *instance)
{
  stats_aggregated_average_create_stats_key(&self->super.key, component, id, instance);
  stats_lock();
  stats_register_counter(level, &self->super.key, SC_TYPE_SINGLE_VALUE, &self->item);
  stats_unlock();
}

void
stats_init_aggregated_average(gint level, guint16 component, const gchar *id, const gchar *instance,
                              StatsAggregatedItem **s)
{
  _new(s);
  StatsAggregatedAverageItem *self = (StatsAggregatedAverageItem *)(*s);
  stats_aggregated_average_init(self);
  _regist_counter(self, level, component, id, instance);
}

static void
_unregist_counter(StatsAggregatedAverageItem *self)
{
  if(self->item)
    {
      stats_lock();
      stats_unregister_counter(&self->super.key, SC_TYPE_SINGLE_VALUE, &self->item);
      stats_unlock();
    }
}

void
stats_deinit_aggregated_average(StatsAggregatedItem *s)
{
  StatsAggregatedAverageItem *self = (StatsAggregatedAverageItem *)s;
  _unregist_counter(self);
  _free(s);
}

