#include "stats-eps.h"
#include "timeutils/cache.h"
#include "stats/stats-registry.h"

#include <math.h>

static void _update(void *cookie);

static void
_stop_timer(StatsEPSItem *self)
{
  if (iv_timer_registered(&self->timer))
    iv_timer_unregister(&self->timer);
}

static void
_start_timer(StatsEPSItem *self)
{
  iv_validate_now();
  self->timer.expires = iv_now;
  self->timer.expires.tv_sec += SAMPLING_FREQUENCY;

  if (!iv_timer_registered(&self->timer))
    iv_timer_register(&self->timer);
}

static void
_init_timer(StatsEPSItem *self)
{
  IV_TIMER_INIT(&self->timer);
  self->timer.cookie = self;
  self->timer.handler = _update;
}

static void
_deinit_timer(StatsEPSItem *self)
{
  _stop_timer(self);
}


static inline gdouble
_calc_sec_between_time(time_t *start, time_t *end)
{
  return fabs(difftime(*start, *end));
}

static inline gboolean
_is_less_an_hour(time_t *start, time_t *now)
{
  return _calc_sec_between_time(start, now) <= HOUR_IN_SEC;
}

static inline gboolean
_is_less_a_day(time_t *start, time_t *now)
{
  return _calc_sec_between_time(start, now) <= DAY_IN_SEC;
}

static void
add_counts(StatsEPSItem *self, gsize count)
{
  gsize diff = count - self->last_message_count;
  self->last_message_count = count;

  self->sum_count_hour += diff;
  self->sum_count_day += diff;

  time_t now = cached_g_current_time_sec();
  gsize elipsed_time = (gsize)_calc_sec_between_time(&self->init_time, &now);
  
  if(elipsed_time == 0)
    return;

  gsize elipsed_time_since_last = (gsize)_calc_sec_between_time(&self->last_add_time, &now);
  
  if(!_is_less_an_hour(&self->init_time, &now))
    self->sum_count_hour -= self->avg_count_hour * elipsed_time_since_last;

  if(!_is_less_a_day(&self->init_time, &now))
    self->sum_count_day -= self->avg_count_day * elipsed_time_since_last;

  self->last_add_time = now;
}

static void
get_EPS(StatsEPSItem *item, gsize msg_count, gsize *one_hour, gsize *one_day, gsize *from_start)
{
  time_t now = cached_g_current_time_sec();
  gsize elipsed_time = (gsize)_calc_sec_between_time(&item->init_time, &now);

  if(elipsed_time == 0)
    elipsed_time = 1;

  *from_start = msg_count / elipsed_time;

  *one_hour = _is_less_an_hour(&item->init_time, &now) ?
              (*from_start)
              :
              (item->sum_count_hour / HOUR_IN_SEC);
  item->avg_count_hour = *one_hour;

  *one_day = _is_less_a_day(&item->init_time, &now) ?
             (*from_start)
             :
             (item->sum_count_day / DAY_IN_SEC);
  item->avg_count_day = *one_day;
}

static void
_update(void *cookie)
{
  StatsEPSItem *self = (StatsEPSItem *) cookie;
  gsize msg_count = stats_counter_get(self->message_count_stats);
  add_counts(self, msg_count);

  gsize one_hour, one_day, from_start;
  get_EPS(self, msg_count, &one_hour, &one_day, &from_start);

  stats_counter_set(self->eps_stats_day, one_hour);
  stats_counter_set(self->eps_stats_hour, one_day);
  stats_counter_set(self->eps_stats_start, from_start);

  _start_timer(self);
}

void
init_stats_eps_item(StatsEPSItem *self, StatsClusterKey *sc_key, StatsCounterItem *counter, gint level)
{
  self->sum_count_hour = 0;
  self->avg_count_hour = 0;
  self->sum_count_day = 0;
  self->avg_count_day = 0;
  self->init_time = cached_g_current_time_sec();
  self->last_add_time = cached_g_current_time_sec();

  self->message_count_stats = counter;
  stats_lock();
  stats_register_counter(level, sc_key, SC_TYPE_EPS_DAY, &self->eps_stats_day);
  stats_register_counter(level, sc_key, SC_TYPE_EPS_HOUR, &self->eps_stats_hour);
  stats_register_counter(level, sc_key, SC_TYPE_EPS_START, &self->eps_stats_start);
  stats_unlock();

  _init_timer(self);
  _start_timer(self);
}

void
deinit_stats_eps_item(StatsEPSItem *self, StatsClusterKey *sc_key)
{
  _deinit_timer(self);

  stats_lock();
  stats_unregister_counter(sc_key, SC_TYPE_EPS_DAY, &self->eps_stats_day);
  stats_unregister_counter(sc_key, SC_TYPE_EPS_HOUR, &self->eps_stats_hour);
  stats_unregister_counter(sc_key, SC_TYPE_EPS_START, &self->eps_stats_start);
  stats_unlock();
}
