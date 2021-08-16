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
#include "stats/stats-eps.h"
#include <math.h>
#include "timeutils/cache.h"
#include "stats/stats-registry.h"
#include "stats/stats-cluster-single.h"

static void
_new(StatsAggregatedItem **self)
{
  *self = (StatsAggregatedItem *) g_new0(StatsAggregatedEPSItem, 1);
}
static void
_free(StatsAggregatedItem *self)
{
  g_free(self);
}

static inline gdouble
_calc_sec_between_time(time_t *start, time_t *end)
{
  return fabs(difftime(*start, *end));
}

static gboolean
_is_less_then_duration(StatsAggregatedEPSItem *self, EPSLogic *logic, time_t *now)
{
  if (logic->duration == -1)
    return TRUE;

  return _calc_sec_between_time(&self->init_time, now) <= logic->duration;
}

static void
_calc_sum(StatsAggregatedEPSItem *self, EPSLogic *logic, time_t *now)
{
  gsize diff = _get_buf(self) - _get_last_message_count(logic);
  _set_last_message_count(logic, _get_buf(self));

  if (!_is_less_then_duration(self, logic, now))
    {
      gsize elipsed_time_since_last = (gsize)_calc_sec_between_time(&self->last_add_time, now);
      diff -= _get_average(logic) * elipsed_time_since_last;
    }

  _add_to_sum(logic, diff);
  self->last_add_time = *now;
}

static void
_calc_average(StatsAggregatedEPSItem *self, EPSLogic *logic, time_t *now)
{
  gsize elipsed_time = (gsize)_calc_sec_between_time(&self->init_time, now);
  gsize to_divede = (_is_less_then_duration(self, logic, now)) ? elipsed_time : logic->duration;
  if (to_divede <= 0) to_divede = 1;

  _set_average(logic, (_get_sum(logic) / to_divede));
}

static void
_update_EPS_logic(StatsAggregatedEPSItem *self, EPSLogic *logic, time_t *now)
{
  _calc_sum(self, logic, now);
  _calc_average(self, logic, now);
  stats_counter_set(logic->counter, _get_average(logic));
}

static void
_update(StatsAggregatedItem *s, time_t *now)
{
  StatsAggregatedEPSItem *self = (StatsAggregatedEPSItem *)s;
  _update_EPS_logic(self, &self->hour, now);
  _update_EPS_logic(self, &self->day, now);
  _update_EPS_logic(self, &self->start, now);
}

static void
_feed_input(StatsAggregatedItem *s, gsize value)
{
  StatsAggregatedEPSItem *self = (StatsAggregatedEPSItem *)s;

  if (self != NULL)
    _add_to_buf(self, value);
}

static void
_set_virtual_function(StatsAggregatedEPSItem *self )
{
  self->super.feed_input = _feed_input;
  self->super.update = _update;
  self->super.free = stats_deinit_aggregated_eps;
}

void
stats_aggregated_eps_create_stats_key(StatsClusterKey *key, guint16 component, const gchar *id, const gchar *instance)
{
  stats_cluster_single_key_set_with_name(key, component, id, instance, "EPS");
}

void
stats_aggregated_eps_init(StatsAggregatedEPSItem *self)
{
  stats_aggregated_item_init(&self->super);
  _set_virtual_function(self);
}

static void
_regist_EPS_logic(EPSLogic *self, StatsClusterKey *sc_key, gint level, gint type, gssize duration)
{
  _set_average(self, 0);
  _set_sum(self, 0);
  _set_last_message_count(self, 0);
  self->duration = duration;
  if(!self->counter)
    stats_register_counter(level, sc_key, type, &self->counter);
}

static void
_regist_EPS_logics(StatsAggregatedEPSItem *self, gint level, guint16 component, const gchar *id, const gchar *instance)
{
  stats_lock();
  StatsClusterKey sc_key;
  stats_cluster_single_key_set_with_name(&sc_key, component, id, instance, "EPS_since_last_hour");
  _regist_EPS_logic(&self->hour, &sc_key, level, SC_TYPE_SINGLE_VALUE, HOUR_IN_SEC);

  stats_cluster_single_key_set_with_name(&sc_key, component, id, instance, "EPS_since_last_day");
  _regist_EPS_logic(&self->day, &sc_key, level, SC_TYPE_SINGLE_VALUE, DAY_IN_SEC);

  stats_cluster_single_key_set_with_name(&sc_key, component, id, instance, "EPS_since_begin");
  _regist_EPS_logic(&self->start, &sc_key, level, SC_TYPE_SINGLE_VALUE, -1);

  stats_aggregated_eps_create_stats_key(&self->super.key, component, id, instance);
  stats_unlock();
}

static void
_set_values(StatsAggregatedEPSItem *self, StatsCounterItem *counter)
{
  _set_buf(self, 0);
  self->init_time = cached_g_current_time_sec();
  self->counter = counter;
}

void
stats_init_aggregated_eps(gint level, guint16 component, const gchar *id, const gchar *instance,
                          StatsCounterItem *counter, StatsAggregatedItem **s)
{
  _new(s);
  StatsAggregatedEPSItem *self = (StatsAggregatedEPSItem *)(*s);
  stats_aggregated_eps_init(self);
  _set_values(self, counter);
  _regist_EPS_logics(self, level, component, id, instance);
}

static void
_unregist_EPS_logic(EPSLogic *self, StatsClusterKey *sc_key, gint type)
{
  stats_lock();
  stats_unregister_counter(sc_key, type, &self->counter);
  stats_unlock();
}

static void
_unregist_EPS_logics(StatsAggregatedEPSItem *self)
{
  StatsClusterKey sc_key;
  stats_cluster_single_key_set_with_name(&sc_key, self->super.key.component, self->super.key.id, self->super.key.instance,
                                         "EPS_since_last_hour");
  _unregist_EPS_logic(&self->hour, &sc_key, SC_TYPE_SINGLE_VALUE);

  stats_cluster_single_key_set_with_name(&sc_key, self->super.key.component, self->super.key.id, self->super.key.instance,
                                         "EPS_since_last_day");
  _unregist_EPS_logic(&self->day, &sc_key, SC_TYPE_SINGLE_VALUE);

  stats_cluster_single_key_set_with_name(&sc_key, self->super.key.component, self->super.key.id, self->super.key.instance,
                                         "EPS_since_begin");
  _unregist_EPS_logic(&self->start, &sc_key, SC_TYPE_SINGLE_VALUE);
}

void
stats_deinit_aggregated_eps(StatsAggregatedItem *s)
{
  StatsAggregatedEPSItem *self = (StatsAggregatedEPSItem *)s;
  _unregist_EPS_logics(self);
  _free(s);
}
