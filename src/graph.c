// src/graph.c

#include "graph.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/**
 * Inicializa un CityGraph vacío.
 * Reserva espacio para 10 zonas inicialmente y
 * deja estadísticas en cero.
 */
void graph_init(CityGraph *graph) {
    graph->zones           = malloc(sizeof(Zone) * 10);
    assert(graph->zones != NULL);
    graph->count           = 0;
    graph->capacity        = 10;
    graph->total_employed  = 0;
    graph->total_unemployed = 0;
    pthread_rwlock_init(&graph->rwlock, NULL);
}  // :contentReference[oaicite:0]{index=0}

/**
 * Libera todos los recursos asociados al grafo:
 * destruye mutex de cada zona, libera el array y
 * destruye el rwlock global.
 */
void graph_destroy(CityGraph *graph) {
    pthread_rwlock_wrlock(&graph->rwlock);
    for (int i = 0; i < graph->count; i++) {
        pthread_mutex_destroy(&graph->zones[i].lock);
    }
    free(graph->zones);
    graph->zones   = NULL;
    graph->count   = 0;
    graph->capacity = 0;
    pthread_rwlock_unlock(&graph->rwlock);
    pthread_rwlock_destroy(&graph->rwlock);
}  // :contentReference[oaicite:1]{index=1}

/**
 * Añade una nueva zona con código, nivel y tipo (fuente o sumidero).
 * Si ya existe una zona con ese código, devuelve NULL.
 */
Zone* graph_add_zone(CityGraph *graph, const char *code, int level, bool is_source) {
    if (strlen(code) != 3) return NULL;

    pthread_rwlock_wrlock(&graph->rwlock);
    // Verificar duplicado
    for (int i = 0; i < graph->count; i++) {
        if (strcmp(graph->zones[i].code, code) == 0) {
            pthread_rwlock_unlock(&graph->rwlock);
            return NULL;
        }
    }
    // Redimensionar si hace falta
    if (graph->count >= graph->capacity) {
        graph->capacity *= 2;
        graph->zones = realloc(graph->zones, sizeof(Zone) * graph->capacity);
        assert(graph->zones != NULL);
    }
    // Inicializar la nueva zona
    Zone *z = &graph->zones[graph->count++];
    strncpy(z->code, code, 3);
    z->code[3] = '\0';
    z->level   = level;
    z->points  = 0;
    z->is_source = is_source;
    if (is_source) {
        z->available = (1 << level) / 2;  // desempleados iniciales
    } else {
        z->available = (1 << level);      // puestos libres iniciales
    }
    // Sin conexiones al inicio
    z->north = z->south = z->east = z->west = NULL;
    z->north_capacity = z->south_capacity = z->east_capacity = z->west_capacity = 0;
    z->north_vehicles = z->south_vehicles = z->east_vehicles   = z->west_vehicles   = 0;
    pthread_mutex_init(&z->lock, NULL);
    pthread_rwlock_unlock(&graph->rwlock);
    return z;
}  // :contentReference[oaicite:2]{index=2}

/**
 * Conecta dos zonas bidireccionalmente en la dirección dir
 * con capacidad inicial especificada.
 */
bool graph_connect_zones(CityGraph *graph,
                         Zone *zone1,
                         Zone *zone2,
                         Direction dir,
                         int initial_capacity)
{
    if (!zone1 || !zone2 || dir == NONE) return false;

    pthread_rwlock_wrlock(&graph->rwlock);
    pthread_mutex_lock(&zone1->lock);
    pthread_mutex_lock(&zone2->lock);

    switch (dir) {
        case NORTH:
            if (zone1->north || zone2->south) goto error;
            zone1->north = zone2;
            zone1->north_capacity = initial_capacity;
            zone2->south = zone1;
            zone2->south_capacity = initial_capacity;
            break;
        case SOUTH:
            if (zone1->south || zone2->north) goto error;
            zone1->south = zone2;
            zone1->south_capacity = initial_capacity;
            zone2->north = zone1;
            zone2->north_capacity = initial_capacity;
            break;
        case EAST:
            if (zone1->east || zone2->west) goto error;
            zone1->east = zone2;
            zone1->east_capacity = initial_capacity;
            zone2->west = zone1;
            zone2->west_capacity = initial_capacity;
            break;
        case WEST:
            if (zone1->west || zone2->east) goto error;
            zone1->west = zone2;
            zone1->west_capacity = initial_capacity;
            zone2->east = zone1;
            zone2->east_capacity = initial_capacity;
            break;
        default:
            goto error;
    }

    pthread_mutex_unlock(&zone2->lock);
    pthread_mutex_unlock(&zone1->lock);
    pthread_rwlock_unlock(&graph->rwlock);
    return true;

error:
    pthread_mutex_unlock(&zone2->lock);
    pthread_mutex_unlock(&zone1->lock);
    pthread_rwlock_unlock(&graph->rwlock);
    return false;
}  // :contentReference[oaicite:3]{index=3}

/**
 * Expande (o crea) la arteria entre z1 y z2 a la nueva capacidad newcap.
 * Devuelve true si existía la conexión y se actualizó, false en caso contrario.
 */
bool graph_expand_road(CityGraph *city, Zone *z1, Zone *z2, int newcap) {
    if (z1->north == z2) {
        z1->north_capacity = newcap;
        z2->south_capacity = newcap;
        return true;
    }
    if (z1->south == z2) {
        z1->south_capacity = newcap;
        z2->north_capacity = newcap;
        return true;
    }
    if (z1->east == z2) {
        z1->east_capacity = newcap;
        z2->west_capacity = newcap;
        return true;
    }
    if (z1->west == z2) {
        z1->west_capacity = newcap;
        z2->east_capacity = newcap;
        return true;
    }
    return false;
}

/**
 * Busca una zona por su código.
 * Devuelve NULL si no la encuentra.
 */
Zone* graph_find_zone(CityGraph *graph, const char *code) {
    pthread_rwlock_rdlock(&graph->rwlock);
    for (int i = 0; i < graph->count; i++) {
        if (strcmp(graph->zones[i].code, code) == 0) {
            pthread_rwlock_unlock(&graph->rwlock);
            return &graph->zones[i];
        }
    }
    pthread_rwlock_unlock(&graph->rwlock);
    return NULL;
}  // :contentReference[oaicite:4]{index=4}

/**
 * Muestra el grafo completo para depuración,
 * imprimiendo cada zona y sus conexiones.
 */
void graph_print(const CityGraph *graph) {
    pthread_rwlock_rdlock(&graph->rwlock);
    printf("Ciudad con %d zonas:\n", graph->count);
    for (int i = 0; i < graph->count; i++) {
        const Zone *z = &graph->zones[i];
        pthread_mutex_lock((pthread_mutex_t*)&z->lock);

        printf("[%s] Nivel %d %s | Puntos: %d | ",
               z->code,
               z->level,
               z->is_source ? "Fuente" : "Sumidero",
               z->points);

        if (z->is_source) {
            printf("Desempleados: %d\n", z->available);
        } else {
            printf("Puestos libres: %d\n", z->available);
        }

        printf("  Conexiones: N(%s:%d/%d) S(%s:%d/%d) E(%s:%d/%d) W(%s:%d/%d)\n",
               z->north   ? z->north->code   : "--", z->north_vehicles, z->north_capacity,
               z->south   ? z->south->code   : "--", z->south_vehicles, z->south_capacity,
               z->east    ? z->east->code    : "--", z->east_vehicles,  z->east_capacity,
               z->west    ? z->west->code    : "--", z->west_vehicles,  z->west_capacity);

        pthread_mutex_unlock((pthread_mutex_t*)&z->lock);
    }
    pthread_rwlock_unlock(&graph->rwlock);
}  // :contentReference[oaicite:5]{index=5}

/**
 * Devuelve la dirección opuesta a dir.
 */
Direction opposite_direction(Direction dir) {
    switch (dir) {
        case NORTH: return SOUTH;
        case SOUTH: return NORTH;
        case EAST:  return WEST;
        case WEST:  return EAST;
        default:    return NONE;
    }
}  // :contentReference[oaicite:6]{index=6}
