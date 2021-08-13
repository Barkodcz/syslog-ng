#ifndef STATS_LARGEST_H
#define STATS_LARGEST_H

#include "stats/stats-counter.h"

typedef struct
{
  StatsCounterItem *largest;
} StatsLargestItem;


static inline void
stats_register_largest(gint level, const StatsClusterKey *sc_key, gint type, StatsLargestItem *self)
{
  stats_register_counter(level, sc_key, type, &self->largest);
}

static inline void
stats_unregister_largest(const StatsClusterKey *sc_key, gint type, StatsLargestItem *self)
{
  stats_unregister_counter(sc_key, type, &self->largest);
}

static inline void
stats_largest_free(StatsLargestItem *self)
{
  stats_counter_free(self->largest);
}

/* NOTE: this is _not_ atomic and doesn't have to be as sets would race anyway */
static inline gsize
stats_largest_get(StatsLargestItem *self)
{
  return stats_counter_get(self->largest);
}

static inline gchar *
stats_largest_get_name(StatsLargestItem *self)
{
  if (self && self->largest)
    return self->largest->name;
  return NULL;
}

static inline void
stats_largest_set(StatsLargestItem *self, gsize value)
{
  if (self && self->largest)
    {
      g_assert(!stats_counter_read_only(self->largest));
      stats_counter_set(self->largest, value);
    }
}

static inline void
stats_largest_add(StatsLargestItem *self, gsize add)
{
  if (self && self->largest)
    {
      g_assert(!stats_counter_read_only(self->largest));
      if (stats_largest_get(self) < add)
        stats_largest_set(self, add);
    }
}

#endif /* STATS_LARGEST_H */