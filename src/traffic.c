#include "traffic.h"
#include "graph.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define CAPACITY_THRESHOLD_1 0.5   // 50%
#define CAPACITY_THRESHOLD_2 1.0   // 100%
#define CAPACITY_THRESHOLD_3 1.5   // 150%

static Caravan *morning_caravans = NULL;
static Caravan *evening_caravans = NULL;
static pthread_mutex_t traffic_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_traffic_system(CityGraph *graph) {
    // Inicialización de estructuras de tráfico
    morning_caravans = NULL;
    evening_caravans = NULL;
}

void simulate_morning_traffic(CityGraph *graph, int day_ticks) {
    pthread_rwlock_rdlock(&graph->rwlock);
    
    // Crear caravanas para cada ruta de fuente a sumidero
    for (int i = 0; i < graph->count; i++) {
        if (graph->zones[i].is_source) {
            pthread_mutex_lock(&graph->zones[i].lock);
            
            // Asumimos que cada residente empleado tiene un vehículo
            int vehicles = (1 << graph->zones[i].level) / 2 - graph->zones[i].available;
            
            // Aquí deberías crear caravanas basadas en las rutas almacenadas
            // Esto es un ejemplo simplificado
            if (vehicles > 0 && graph->zones[i].east && !graph->zones[i].east->is_source) {
                Caravan *new_caravan = malloc(sizeof(Caravan));
                new_caravan->current = &graph->zones[i];
                new_caravan->destination = graph->zones[i].east;
                new_caravan->vehicle_count = vehicles;
                new_caravan->next = morning_caravans;
                morning_caravans = new_caravan;
            }
            
            pthread_mutex_unlock(&graph->zones[i].lock);
        }
    }
    
    pthread_rwlock_unlock(&graph->rwlock);
    
    // Simular cada tick del día
    for (int tick = 0; tick < day_ticks; tick++) {
        update_traffic_flows(graph);
        usleep(100000); // Pequeña pausa para simular tiempo real
    }
}

void simulate_evening_traffic(CityGraph *graph, int day_ticks) {
    // Similar a simulate_morning_traffic pero en dirección contraria
    // Implementación simplificada
    for (int tick = 0; tick < day_ticks; tick++) {
        update_traffic_flows(graph);
        usleep(100000);
    }
}

void update_traffic_flows(CityGraph *graph) {
    pthread_mutex_lock(&traffic_mutex);
    
    Caravan *current = morning_caravans;
    while (current != NULL) {
        // Mover vehículos entre zonas según capacidad
        // Implementación simplificada - esto debería usar las rutas reales
        if (current->current->east == current->destination) {
            int available_capacity = current->current->east_capacity - current->current->east_vehicles;
            if (available_capacity > 0) {
                int moving = (available_capacity > current->vehicle_count) ? 
                            current->vehicle_count : available_capacity;
                
                current->current->east_vehicles += moving;
                current->vehicle_count -= moving;
                
                if (current->vehicle_count == 0) {
                    // Caravana llegó a su destino
                    Caravan *to_free = current;
                    current = current->next;
                    free(to_free);
                    continue;
                }
            }
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(&traffic_mutex);
}

void free_caravans(Caravan *caravan) {
    while (caravan != NULL) {
        Caravan *next = caravan->next;
        free(caravan);
        caravan = next;
    }
}