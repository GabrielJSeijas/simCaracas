#include "graph.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

void graph_init(CityGraph *graph) {
    graph->zones = malloc(sizeof(Zone) * 10);  // Capacidad inicial
    graph->count = 0;
    graph->capacity = 10;
    graph->total_employed = 0;
    graph->total_unemployed = 0;
    pthread_rwlock_init(&graph->rwlock, NULL);

}

// Libera todos los recursos del grafo
void graph_destroy(CityGraph *graph) {
    pthread_rwlock_wrlock(&graph->rwlock);
    
    for (int i = 0; i < graph->count; i++) {
        pthread_mutex_destroy(&graph->zones[i].lock);
    }
    free(graph->zones);
    graph->zones = NULL;
    graph->count = graph->capacity = 0;
    
    pthread_rwlock_unlock(&graph->rwlock);
    pthread_rwlock_destroy(&graph->rwlock);
}

// Añade una nueva zona al grafo
Zone* graph_add_zone(CityGraph *graph, const char *code, int level, bool is_source) {
    if (strlen(code) != 3) return NULL;
    
    pthread_rwlock_wrlock(&graph->rwlock);
    
    // Verificar si ya existe una zona con este código
    for (int i = 0; i < graph->count; i++) {
        if (strcmp(graph->zones[i].code, code) == 0) {
            pthread_rwlock_unlock(&graph->rwlock);
            return NULL;
        }
    }
    
    // Redimensionar si es necesario
    if (graph->count >= graph->capacity) {
        graph->capacity *= 2;
        graph->zones = realloc(graph->zones, sizeof(Zone) * graph->capacity);
    }
    
    // Inicializar la nueva zona
    Zone *new_zone = &graph->zones[graph->count];
    strncpy(new_zone->code, code, 3);
    new_zone->code[3] = '\0';
    new_zone->level = level;
    new_zone->points = 0;
    new_zone->is_source = is_source;
    
    if (is_source) {
        new_zone->available = (1 << level) / 2; // Residentes iniciales (nivel 1 = 2 personas)
    } else {
        new_zone->available = (1 << level);     // Puestos disponibles
    }
    
    // Inicializar conexiones
    new_zone->north = new_zone->south = new_zone->east = new_zone->west = NULL;
    new_zone->north_capacity = new_zone->south_capacity = 
    new_zone->east_capacity = new_zone->west_capacity = 0;
    new_zone->north_vehicles = new_zone->south_vehicles = 
    new_zone->east_vehicles = new_zone->west_vehicles = 0;
    
    pthread_mutex_init(&new_zone->lock, NULL);
    
    graph->count++;
    pthread_rwlock_unlock(&graph->rwlock);
    
    return new_zone;
}

// Conecta dos zonas con una arteria vial
bool graph_connect_zones(CityGraph *graph, Zone *zone1, Zone *zone2, Direction dir, int initial_capacity) {
    if (!zone1 || !zone2 || dir == NONE) return false;
    
    Direction opp_dir = opposite_direction(dir);
    
    pthread_rwlock_wrlock(&graph->rwlock);
    pthread_mutex_lock(&zone1->lock);
    pthread_mutex_lock(&zone2->lock);
    
    // Verificar que la conexión no exista ya
    switch (dir) {
        case NORTH:
            if (zone1->north != NULL || zone2->south != NULL) goto error;
            zone1->north = zone2;
            zone1->north_capacity = initial_capacity;
            zone2->south = zone1;
            zone2->south_capacity = initial_capacity;
            break;
        case SOUTH:
            if (zone1->south != NULL || zone2->north != NULL) goto error;
            zone1->south = zone2;
            zone1->south_capacity = initial_capacity;
            zone2->north = zone1;
            zone2->north_capacity = initial_capacity;
            break;
        case EAST:
            if (zone1->east != NULL || zone2->west != NULL) goto error;
            zone1->east = zone2;
            zone1->east_capacity = initial_capacity;
            zone2->west = zone1;
            zone2->west_capacity = initial_capacity;
            break;
        case WEST:
            if (zone1->west != NULL || zone2->east != NULL) goto error;
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
}

// Encuentra una zona por su código
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
}

// Imprime el grafo (para depuración)
void graph_print(const CityGraph *graph) {
    pthread_rwlock_rdlock(&graph->rwlock);
    
    printf("Ciudad con %d zonas:\n", graph->count);
    for (int i = 0; i < graph->count; i++) {
        const Zone *z = &graph->zones[i];
        pthread_mutex_lock(&z->lock);
        
        printf("[%s] Nivel %d %s | Puntos: %d | ",
               z->code, z->level, 
               z->is_source ? "Fuente" : "Sumidero",
               z->points);
        
        if (z->is_source) {
            printf("Desempleados: %d\n", z->available);
        } else {
            printf("Puestos libres: %d\n", z->available);
        }
        
        printf("  Conexiones: N(%s:%d/%d) S(%s:%d/%d) E(%s:%d/%d) W(%s:%d/%d)\n",
               z->north ? z->north->code : "--",
               z->north_vehicles, z->north_capacity,
               z->south ? z->south->code : "--",
               z->south_vehicles, z->south_capacity,
               z->east ? z->east->code : "--",
               z->east_vehicles, z->east_capacity,
               z->west ? z->west->code : "--",
               z->west_vehicles, z->west_capacity);
        
        pthread_mutex_unlock(&z->lock);
    }
    
    pthread_rwlock_unlock(&graph->rwlock);
}

// Devuelve la dirección opuesta
Direction opposite_direction(Direction dir) {
    switch (dir) {
        case NORTH: return SOUTH;
        case SOUTH: return NORTH;
        case EAST: return WEST;
        case WEST: return EAST;
        default: return NONE;
    }
}