#ifndef ZONE_H
#define ZONE_H

#include "graph.h"

// Estructura para nodos de lista ordenada
typedef struct ZoneListNode {
    Zone *zone;
    int priority;  // Prioridad para ordenamiento (ej: puestos disponibles o residentes)
    struct ZoneListNode *next;
} ZoneListNode;

// Funciones de lista ordenada
ZoneListNode* create_zone_list_node(Zone *zone, int priority);
void insert_sorted(ZoneListNode **head, Zone *zone, int priority);
Zone* pop_highest_priority(ZoneListNode **head);
void free_zone_list(ZoneListNode *head);

// Funciones principales
void zone_bfs_search(Zone *start, bool find_sink, CityGraph *graph);
void assign_residents_to_jobs(Zone *source, Zone *sink, CityGraph *graph);
void update_zone_points(CityGraph *graph, bool is_morning);

#endif