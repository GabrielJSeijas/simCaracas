#ifndef PROCESS_H
#define PROCESS_H

#include "graph.h"

typedef struct {
    int day_ticks;
    float tick_duration;
    int max_zone_level;
} Config;

void create_child_processes(CityGraph *graph, Config config);
void zone_process(CityGraph *graph, int read_pipe, Config config);
void traffic_process(CityGraph *graph, int read_pipe, Config config);
void main_process_loop(CityGraph *graph, int zone_pipe, int traffic_pipe, Config config);

#endif