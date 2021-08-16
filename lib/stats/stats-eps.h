/*
 * Copyright (c) 2002-2013 One Identity
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

#ifndef STATS_EPS_H
#define STATS_EPS_H

#include "stats/stats-aggregated.h"

typedef struct
{
  StatsCounterItem *counter;
  atomic_gssize average;
  atomic_gssize sum;
  atomic_gssize last_message_count;

  gssize duration; /* if the duration equals -1, thats mean, it count since syslog start */
} EPSLogic;

typedef struct
{
  StatsAggregatedItem super;
  time_t init_time;
  time_t last_add_time;


  StatsCounterItem *counter;
  atomic_gssize buf;

  EPSLogic hour;
  EPSLogic day;
  EPSLogic start;
} StatsAggregatedEPSItem;

static inline void
_set_buf(StatsAggregatedEPSItem *self, gsize set)
{
  atomic_gssize_set(&self->buf, set);
}

static inline void
_add_to_buf(StatsAggregatedEPSItem *self, gsize value)
{
  atomic_gssize_add(&self->buf, value);
}

static inline gsize
_get_buf(StatsAggregatedEPSItem *self)
{
  if (self->counter != NULL)
    return stats_counter_get(self->counter);
  return (gsize)atomic_gssize_get(&self->buf);
}

static inline void
_set_sum(EPSLogic *self, gsize set)
{
  atomic_gssize_set(&self->sum, set);
}

static inline void
_add_to_sum(EPSLogic *self, gsize value)
{
  atomic_gssize_add(&self->sum, value);
}

static inline gsize
_get_sum(EPSLogic *self)
{
  return (gsize)atomic_gssize_get(&self->sum);
}

static inline void
_set_average(EPSLogic *self, gsize set)
{
  atomic_gssize_set(&self->average, set);
}

static inline void
_add_to_average(EPSLogic *self, gsize value)
{
  atomic_gssize_add(&self->average, value);
}

static inline gsize
_get_average(EPSLogic *self)
{
  return (gsize)atomic_gssize_get(&self->average);
}

static inline void
_set_last_message_count(EPSLogic *self, gsize set)
{
  atomic_gssize_set(&self->last_message_count, set);
}

static inline void
_add_to_last_message_count(EPSLogic *self, gsize value)
{
  atomic_gssize_add(&self->last_message_count, value);
}

static inline gsize
_get_last_message_count(EPSLogic *self)
{
  return (gsize)atomic_gssize_get(&self->last_message_count);
}

#endif /* STATS_EPS_H */