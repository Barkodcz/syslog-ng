#ifndef STATS_AVG_H
#define STATS_AVG_H

#include "stats/stats-counter.h"

typedef struct
{
  StatsCounterItem *average;
  atomic_gssize count;
  atomic_gssize sum;
} StatsAverageItem;

static inline void
_stats_average_update(StatsAverageItem *self)
{
  stats_counter_set(self->average, 
                   (atomic_gssize_get_unsigned (&self->sum) / atomic_gssize_get_unsigned(&self->count)));
}

static inline void
stats_average_add_to_sum(StatsAverageItem *self, gssize add)
{
  atomic_gssize_add(&self->sum, add);
}

static inline void
stats_average_add_to_count(StatsAverageItem *self, gssize add)
{
  atomic_gssize_add(&self->count, add);
}

static inline void
stats_average_sub_from_sum(StatsAverageItem *self, gssize sub)
{
  atomic_gssize_sub(&self->sum, sub);
}

static inline void
stats_average_sub_from_count(StatsAverageItem *self, gssize sub)
{
  atomic_gssize_sub(&self->count, sub);
}

static inline void
stats_average_inc_to_sum(StatsAverageItem *self)
{
  atomic_gssize_inc(&self->sum);
}

static inline void
stats_average_inc_to_count(StatsAverageItem *self)
{
  atomic_gssize_inc(&self->count);
}

static inline void
stats_average_dec_from_sum(StatsAverageItem *self)
{
  atomic_gssize_dec(&self->sum);
}

static inline void
stats_average_dec_from_count(StatsAverageItem *self)
{
  atomic_gssize_dec(&self->count);
}

static inline void
stats_average_add(StatsAverageItem *self, gssize add)
{
  if (self && self->average)
    {
      g_assert(!stats_counter_read_only(self->average));
      stats_average_inc_to_count(self);
      stats_average_add_to_sum(self, add);
      _stats_average_update(self);
    }
}

static inline void
stats_average_sub(StatsAverageItem *self, gssize sub)
{
  if (self && self->average)
    {
      g_assert(!stats_counter_read_only(self->average));
      stats_average_dec_from_count(self);
      stats_average_sub_from_sum(self, sub);
      _stats_average_update(self);
    }
}

static inline void
stats_average_inc(StatsAverageItem *self)
{
  stats_average_add(self, 1);
}

static inline void
stats_average_dec(StatsAverageItem *self)
{
  stats_average_sub(self, 1);
}

/* NOTE: this is _not_ atomic and doesn't have to be as sets would race anyway */
static inline gsize
stats_average_get(StatsAverageItem *self)
{
  return stats_counter_get(self->average);
}

static inline gchar *
stats_average_get_name(StatsAverageItem *self)
{
  if (self && self->average)
    return self->average->name;
  return NULL;
}

static inline void
stats_register_average(gint level, const StatsClusterKey *sc_key, gint type, StatsAverageItem *self)
{
  stats_register_counter(level, sc_key, type, &self->average);
  atomic_gssize_set(&self->count, 0);
  atomic_gssize_set(&self->sum, 0);
}

static inline void
stats_unregister_average(const StatsClusterKey *sc_key, gint type, StatsAverageItem *self)
{
  stats_unregister_counter(sc_key, type, &self->average);
}

static inline void
stats_average_free(StatsAverageItem *self)
{
  stats_counter_free(self->average);
}


#endif /* STATS_AVG_H */