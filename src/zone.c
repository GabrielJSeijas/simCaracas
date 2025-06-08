// src/zone.c

#include "zone.h"      // ZoneListNode, zone_bfs_search, assign_residents_to_jobs, update_zone_points :contentReference[oaicite:0]{index=0}
#include "graph.h"     // CityGraph, Zone, pthread locks :contentReference[oaicite:1]{index=1}
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

/**
 * Crea un nodo para la lista de prioridad de zonas.
 */
ZoneListNode* create_zone_list_node(Zone *zone, int priority) {
    ZoneListNode *node = malloc(sizeof(ZoneListNode));
    if (!node) return NULL;
    node->zone     = zone;
    node->priority = priority;
    node->next     = NULL;
    return node;
}

/**
 * Inserta en la lista ordenada (descendente por prioridad).
 */
void insert_sorted(ZoneListNode **head, Zone *zone, int priority) {
    ZoneListNode *new_node = create_zone_list_node(zone, priority);
    if (!new_node) return;

    // Determinar prioridad real (disponibilidad actual)
    pthread_mutex_lock(&zone->lock);
    int avail = zone->available;
    pthread_mutex_unlock(&zone->lock);

    // Si la lista está vacía o el nuevo nodo supera a la cabeza
    if (*head == NULL || priority > (*head)->priority) {
        new_node->next = *head;
        *head = new_node;
    } else {
        ZoneListNode *cur = *head;
        while (cur->next && cur->next->priority >= priority) {
            cur = cur->next;
        }
        new_node->next = cur->next;
        cur->next      = new_node;
    }
}

/**
 * Extrae y devuelve la zona con mayor prioridad (cabeza de la lista).
 */
Zone* pop_highest_priority(ZoneListNode **head) {
    if (*head == NULL) return NULL;
    ZoneListNode *tmp = *head;
    Zone *res         = tmp->zone;
    *head             = tmp->next;
    free(tmp);
    return res;
}

/**
 * Libera todos los nodos de la lista.
 */
void free_zone_list(ZoneListNode *head) {
    while (head) {
        ZoneListNode *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/**
 * Realiza un BFS donde la "cola" es una lista ordenada por prioridad:
 * si find_sink==true busca un sumidero, y si find_sink==false busca una fuente
 * con residentes disponibles. Cuando lo encuentra, asigna residentes/empleo.
 */
void zone_bfs_search(Zone *start, bool find_sink, CityGraph *graph) {
    ZoneListNode *queue = NULL;
    bool *visited       = calloc(graph->count, sizeof(bool));
    if (!visited) return;

    // Prioridad inicial: disponibilidad de la zona origen
    pthread_mutex_lock(&start->lock);
    int start_prio = start->available;
    pthread_mutex_unlock(&start->lock);

    insert_sorted(&queue, start, start_prio);
    visited[start - graph->zones] = true;

    while (queue) {
        Zone *current = pop_highest_priority(&queue);

        // Verificar si cumple condición de match
        bool match = false;
        pthread_mutex_lock(&current->lock);
        if ((find_sink && !current->is_source) ||
            (!find_sink &&  current->is_source && current->available > 0)) {
            match = true;
        }
        pthread_mutex_unlock(&current->lock);

        if (match) {
            assign_residents_to_jobs(start, current, graph);
            free_zone_list(queue);
            free(visited);
            return;
        }

        // Explorar vecinos (N, S, E, W)
        Zone *nbrs[4] = { current->north,
                          current->south,
                          current->east,
                          current->west };
        for (int i = 0; i < 4; i++) {
            Zone *nbr = nbrs[i];
            if (nbr && !visited[nbr - graph->zones]) {
                visited[nbr - graph->zones] = true;
                // Calcular prioridad
                pthread_mutex_lock(&nbr->lock);
                int pr = nbr->available;
                pthread_mutex_unlock(&nbr->lock);
                insert_sorted(&queue, nbr, pr);
            }
        }
    }

    // No se encontró match
    free_zone_list(queue);
    free(visited);
}

/**
 * Asigna residentes de la zona fuente a la zona sumidero,
 * actualiza available y estadísticas globales.
 */
void assign_residents_to_jobs(Zone *source, Zone *sink, CityGraph *graph) {
    pthread_mutex_lock(&source->lock);
    pthread_mutex_lock(&sink->lock);

    int to_assign = source->available;
    if (sink->available < to_assign) {
        to_assign = sink->available;
    }

    if (to_assign > 0) {
        source->available -= to_assign;
        sink->available   -= to_assign;

        // Actualizar totales globales
        pthread_rwlock_wrlock(&graph->rwlock);
        if (source->is_source) {
            graph->total_employed   += to_assign;
            graph->total_unemployed -= to_assign;
        }
        pthread_rwlock_unlock(&graph->rwlock);

        printf("Asignados %d residentes de %s a %s\n",
               to_assign, source->code, sink->code);
    }

    pthread_mutex_unlock(&sink->lock);
    pthread_mutex_unlock(&source->lock);
}

/**
 * Actualiza los puntos de cada zona según la fase del día:
 * - Durante la mañana, los sumideros ganan puntos por puestos ocupados.
 * - Durante la tarde, las fuentes ganan puntos por residentes empleados.
 * Si alcanzan los puntos requeridos (capacity^2), suben de nivel.
 */
void update_zone_points(CityGraph *graph, bool is_morning) {
    pthread_rwlock_wrlock(&graph->rwlock);

    for (int i = 0; i < graph->count; i++) {
        Zone *z = &graph->zones[i];
        pthread_mutex_lock(&z->lock);

        int level    = z->level;
        int capacity = (1 << level);
        int required = capacity * capacity;

        if (is_morning && !z->is_source) {
            // Sumideros: puntos = puestos ocupados
            int occupied = capacity - z->available;
            z->points += occupied;
            if (z->points >= required) {
                z->level++;
                z->available = (1 << z->level);
                printf("¡%s subió a nivel %d!\n", z->code, z->level);
            }
        }
        else if (!is_morning && z->is_source) {
            // Fuentes: puntos = empleados en ese día
            int employed = (capacity / 2) - z->available;
            z->points += employed;
            if (z->points >= required) {
                z->level++;
                z->available = (1 << z->level) / 2;
                printf("¡%s subió a nivel %d!\n", z->code, z->level);
            }
        }

        pthread_mutex_unlock(&z->lock);
    }

    pthread_rwlock_unlock(&graph->rwlock);
}
