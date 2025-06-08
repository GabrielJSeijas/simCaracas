#ifndef TRAFFIC_H
#define TRAFFIC_H

#include "graph.h"

typedef struct Caravan {
    Zone *current;
    Zone *destination;
    int vehicle_count;
    struct Caravan *next;
} Caravan;

// Funciones principales de tráfico
void init_traffic_system(CityGraph *graph);
void simulate_morning_traffic(CityGraph *graph, int day_ticks);
void simulate_evening_traffic(CityGraph *graph, int day_ticks);
void update_traffic_flows(CityGraph *graph);
void free_caravans(Caravan *caravan);

#endif // TRAFFIC_H