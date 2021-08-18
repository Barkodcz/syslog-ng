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

#include "stats/stats-aggregated-register.h"
#include "stats/stats-registry.h"
#include "stats/stats-query.h"
#include "cfg.h"


typedef struct _StatsClusterContainer
{
  GHashTable *clusters;
} StatsAggregatedClusterContainer;

static StatsAggregatedClusterContainer stats_cluster_container;

static GStaticMutex stats_aggregated_mutex = G_STATIC_MUTEX_INIT;
static gboolean stats_aggregated_locked;


void
stats_aggregated_lock(void)
{
  g_static_mutex_lock(&stats_aggregated_mutex);
  stats_aggregated_locked = TRUE;
}

void
stats_aggregated_unlock(void)
{
  stats_aggregated_locked = FALSE;
  g_static_mutex_unlock(&stats_aggregated_mutex);
}

static guint
_stats_cluster_key_hash(const StatsClusterKey *self)
{
  return g_str_hash(self->id) + g_str_hash(self->instance) + self->component;
}

void
stats_aggregated_registry_init(void)
{
  stats_cluster_container.clusters = g_hash_table_new_full((GHashFunc) _stats_cluster_key_hash,
                                                           (GEqualFunc) stats_cluster_key_equal, NULL, NULL);
  _init_timer();
  g_static_mutex_init(&stats_aggregated_mutex);
}

void
stats_aggregated_registry_deinit(void)
{
  g_hash_table_destroy(stats_cluster_container.clusters);
  stats_cluster_container.clusters = NULL;
  g_static_mutex_free(&stats_aggregated_mutex);
}

static void
_insert_to_table(StatsAggregatedItem *value)
{
  g_hash_table_insert(stats_cluster_container.clusters, &value->key, value);
}

static gboolean
_is_in_table(StatsClusterKey *sc_key)
{
  return g_hash_table_lookup(stats_cluster_container.clusters, sc_key) != NULL;
}

static void
_remove_from_table(StatsClusterKey *sc_key)
{
  g_hash_table_remove(stats_cluster_container.clusters, sc_key);
}

static StatsAggregatedItem *
_get_from_table(StatsClusterKey *sc_key)
{
  return g_hash_table_lookup(stats_cluster_container.clusters, sc_key);
}

static void
_unregister(StatsAggregatedItem *s)
{
  if(s == NULL)
    return;

  stats_aggregated_untrack_counter(s);

  if (stats_aggregated_is_orphaned(s) && _is_in_table(&s->key))
    {
      _remove_from_table(&s->key);
      stats_aggregated_free(s);
    }
}

void
stats_register_aggregated_maximum(gint level, guint16 component, const gchar *id, const gchar *instance,
                                  StatsAggregatedItem **s)
{
  g_assert(stats_aggregated_locked);
  StatsClusterKey sc_key;
  stats_aggregated_maximum_create_stats_key(&sc_key, component, id, instance);

  if (!_is_in_table(&sc_key))
    {
      stats_init_aggregated_maximum(level, component, id, instance, s);
      _insert_to_table(*s);
    }
  else
    {
      *s = _get_from_table(&sc_key);
    }

  stats_aggregated_track_counter(*s);
}

void
stats_unregister_aggregated_maximum(StatsAggregatedItem *s)
{
  g_assert(stats_aggregated_locked);
  _unregister(s);
}

void
stats_register_aggregated_average(gint level, guint16 component, const gchar *id, const gchar *instance,
                                  StatsAggregatedItem **s)
{
  g_assert(stats_aggregated_locked);
  StatsClusterKey sc_key;
  stats_aggregated_average_create_stats_key(&sc_key, component, id, instance);

  if (!_is_in_table(&sc_key))
    {
      stats_init_aggregated_average(level, component, id, instance, s);
      _insert_to_table(*s);
    }
  else
    {
      *s = _get_from_table(&sc_key);
    }

  stats_aggregated_track_counter(*s);
}

void
stats_unregister_aggregated_average(StatsAggregatedItem *s)
{
  g_assert(stats_aggregated_locked);
  _unregister(s);
}

void
stats_register_aggregated_eps(gint level, guint16 component, const gchar *id, const gchar *instance,
                              StatsCounterItem *counter, StatsAggregatedItem **s)
{
  g_assert(stats_aggregated_locked);
  StatsClusterKey sc_key;
  stats_aggregated_eps_create_stats_key(&sc_key, component, id, instance);

  if (!_is_in_table(&sc_key))
    {
      stats_init_aggregated_eps(level, component, id, instance, counter, s);
      _insert_to_table(*s);
    }
  else
    {
      *s = _get_from_table(&sc_key);
    }

  stats_aggregated_track_counter(*s);
}

void
stats_unregister_aggregated_eps(StatsAggregatedItem *s)
{
  g_assert(stats_aggregated_locked);
  _unregister(s);
}

