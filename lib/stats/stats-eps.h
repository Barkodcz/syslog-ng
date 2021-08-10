#ifndef STATS_EPS_H
#define STATS_EPS_H

#include "syslog-ng.h"
#include "cfg.h"
#include "iv.h"


#define HOUR_IN_SEC 3600 /* 60*60 */
#define DAY_IN_SEC 86400 /* 60*60*24 */

#define SAMPLING_FREQUENCY 10 /* second */

typedef struct
{
  gsize sum_count_hour;
  gsize avg_count_hour;
  gsize sum_count_day;
  gsize avg_count_day;
  time_t init_time;
  time_t last_add_time;
  gsize last_message_count;

  struct iv_timer timer;
  StatsCounterItem *eps_stats_hour;
  StatsCounterItem *eps_stats_day;
  StatsCounterItem *eps_stats_start;
  StatsCounterItem *message_count_stats;
  StatsClusterKey *sc_key;
} StatsEPSItem;

void init_stats_eps_item(StatsEPSItem *item, StatsClusterKey *sc_key, StatsCounterItem *counter);
void deinit_stats_eps_item(StatsEPSItem *item, StatsClusterKey *sc_key);

#endif /* STATS_EPS_H */