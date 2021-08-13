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
#include "stats/stats-aggregated-register.h"
#include "stats/stats-cluster-single.h"

typedef struct
{
  StatsAggregatedItem super;
  StatsCounterItem *item;
} StatsAggregatedMaximum;


void
stats_aggregated_maximum_create_stats_key(StatsClusterKey *key, guint16 component, const gchar *id,
                                          const gchar *instance)
{
  stats_cluster_single_key_set_with_name(key, component, id, instance, "maximum_message_size");
}

static void
_new(StatsAggregatedItem **self)
{
  *self = (StatsAggregatedItem *) g_new0(StatsAggregatedMaximum, 1);
}
static void
_free(StatsAggregatedItem *self)
{
  g_free(self);
}

static void
_feed_input(StatsAggregatedItem *s, gsize value)
{
  StatsAggregatedMaximum *self = (StatsAggregatedMaximum *)s;
  if (stats_counter_get(self->item) < value)
    stats_counter_set(self->item, value);
}

static void
_set_virtual_function(StatsAggregatedMaximum *self )
{
  self->super.feed_input = _feed_input;
  self->super.free = stats_deinit_aggregated_maximum;
}

static void
stats_aggregated_maximum_init(StatsAggregatedMaximum *self)
{
  stats_aggregated_item_init(&self->super);
  _set_virtual_function(self);
}

static void
_regist_counter( StatsAggregatedMaximum *self, gint level, guint16 component, const gchar *id, const gchar *instance)
{
  stats_aggregated_maximum_create_stats_key(&self->super.key, component, id, instance);
  stats_lock();
  stats_register_counter(level, &self->super.key, SC_TYPE_SINGLE_VALUE, &self->item);
  stats_unlock();
}

void
stats_init_aggregated_maximum(gint level, guint16 component, const gchar *id, const gchar *instance,
                              StatsAggregatedItem **s)
{
  _new(s);
  StatsAggregatedMaximum *self = (StatsAggregatedMaximum *)(*s);
  stats_aggregated_maximum_init(self);
  _regist_counter(self, level, component, id, instance);
}

static void
_unregist_counter(StatsAggregatedMaximum *self)
{
  if(self->item)
    {
      stats_lock();
      stats_unregister_counter(&self->super.key, SC_TYPE_SINGLE_VALUE, &self->item);
      stats_unlock();
    }
}

void
stats_deinit_aggregated_maximum(StatsAggregatedItem *s)
{
  StatsAggregatedMaximum *self = (StatsAggregatedMaximum *)s;
  _unregist_counter(self);
  _free(s);
}
