#include "zone.h"
#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

// Crear un nuevo nodo para la lista ordenada
ZoneListNode* create_zone_list_node(Zone *zone, int priority) {
    ZoneListNode *node = malloc(sizeof(ZoneListNode));
    if (!node) return NULL;
    
    node->zone = zone;
    node->priority = priority;
    node->next = NULL;
    return node;
}

// Insertar en la lista manteniendo el orden descendente por prioridad
void insert_sorted(ZoneListNode **head, Zone *zone, int priority) {
    ZoneListNode *new_node = create_zone_list_node(zone, priority);
    if (!new_node) return;
    
    pthread_mutex_lock(&zone->lock);
    int available = zone->available;
    pthread_mutex_unlock(&zone->lock);
    
    // Lista vacía o nuevo elemento tiene mayor prioridad que la cabeza
    if (*head == NULL || priority > (*head)->priority) {
        new_node->next = *head;
        *head = new_node;
    } else {
        ZoneListNode *current = *head;
        while (current->next != NULL && current->next->priority >= priority) {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}

// Extraer el elemento con mayor prioridad
Zone* pop_highest_priority(ZoneListNode **head) {
    if (*head == NULL) return NULL;
    
    ZoneListNode *temp = *head;
    Zone *result = temp->zone;
    *head = (*head)->next;
    free(temp);
    return result;
}

// Liberar memoria de la lista
void free_zone_list(ZoneListNode *head) {
    while (head != NULL) {
        ZoneListNode *temp = head;
        head = head->next;
        free(temp);
    }
}

// BFS modificado para usar listas ordenadas
void zone_bfs_search(Zone *start, bool find_sink, CityGraph *graph) {
    ZoneListNode *priority_queue = NULL;
    bool *visited = calloc(graph->count, sizeof(bool));
    
    // Usamos la disponibilidad como prioridad
    int start_priority;
    pthread_mutex_lock(&start->lock);
    start_priority = start->available;
    pthread_mutex_unlock(&start->lock);
    
    insert_sorted(&priority_queue, start, start_priority);
    visited[start - graph->zones] = true;
    
    while (priority_queue != NULL) {
        Zone *current = pop_highest_priority(&priority_queue);
        
        // Verificar si es el tipo de nodo que buscamos
        bool is_match = false;
        pthread_mutex_lock(&current->lock);
        
        if ((find_sink && !current->is_source) || 
            (!find_sink && current->is_source && current->available > 0)) {
            is_match = true;
        }
        
        int current_available = current->available;
        pthread_mutex_unlock(&current->lock);
        
        if (is_match) {
            assign_residents_to_jobs(start, current, graph);
            free_zone_list(priority_queue);
            free(visited);
            return;
        }
        
        // Explorar vecinos y agregarlos a la cola de prioridad
        Zone *neighbors[] = {current->north, current->south, current->east, current->west};
        for (int i = 0; i < 4; i++) {
            if (neighbors[i] && !visited[neighbors[i] - graph->zones]) {
                visited[neighbors[i] - graph->zones] = true;
                
                // Calcular prioridad según disponibilidad
                pthread_mutex_lock(&neighbors[i]->lock);
                int priority = neighbors[i]->available;
                pthread_mutex_unlock(&neighbors[i]->lock);
                
                insert_sorted(&priority_queue, neighbors[i], priority);
            }
        }
    }
    
    free(visited);
    free_zone_list(priority_queue);
}

// Asignación de residentes a trabajos (similar pero con mejor manejo de prioridades)
void assign_residents_to_jobs(Zone *source, Zone *sink, CityGraph *graph) {
    pthread_mutex_lock(&source->lock);
    pthread_mutex_lock(&sink->lock);
    
    int to_assign = source->available;
    if (sink->available < to_assign) {
        to_assign = sink->available;
    }
    
    if (to_assign > 0) {
        source->available -= to_assign;
        sink->available -= to_assign;
        
        // Actualizar estadísticas globales
        pthread_rwlock_wrlock(&graph->rwlock);
        if (source->is_source) {
            graph->total_employed += to_assign;
            graph->total_unemployed -= to_assign;
        }
        pthread_rwlock_unlock(&graph->rwlock);
        
        // Registrar la ruta para el tráfico (implementar según necesidad)
        printf("Asignados %d residentes de %s a %s\n", 
               to_assign, source->code, sink->code);
    }
    
    pthread_mutex_unlock(&sink->lock);
    pthread_mutex_unlock(&source->lock);
}

// Actualización de puntos (ahora considera prioridades)
void update_zone_points(CityGraph *graph, bool is_morning) {
    pthread_rwlock_wrlock(&graph->rwlock);
    
    for (int i = 0; i < graph->count; i++) {
        pthread_mutex_lock(&graph->zones[i].lock);
        
        if (is_morning && !graph->zones[i].is_source) {
            // Sumideros ganan puntos por puestos ocupados
            int occupied = (1 << graph->zones[i].level) - graph->zones[i].available;
            graph->zones[i].points += occupied;
            
            // Verificar si sube de nivel
            int required = (1 << graph->zones[i].level) * (1 << graph->zones[i].level);
            if (graph->zones[i].points >= required && 
                graph->zones[i].level < graph->max_zone_level) {
                graph->zones[i].level++;
                graph->zones[i].available = (1 << graph->zones[i].level);
                printf("¡%s subió a nivel %d!\n", graph->zones[i].code, graph->zones[i].level);
            }
        } 
        else if (!is_morning && graph->zones[i].is_source) {
            // Fuentes ganan puntos por residentes empleados
            int employed = (1 << graph->zones[i].level) / 2 - graph->zones[i].available;
            graph->zones[i].points += employed;
            
            // Verificar si sube de nivel
            int required = (1 << graph->zones[i].level) * (1 << graph->zones[i].level);
            if (graph->zones[i].points >= required && 
                graph->zones[i].level < graph->max_zone_level) {
                graph->zones[i].level++;
                graph->zones[i].available = (1 << graph->zones[i].level) / 2;
                printf("¡%s subió a nivel %d!\n", graph->zones[i].code, graph->zones[i].level);
            }
        }
        
        pthread_mutex_unlock(&graph->zones[i].lock);
    }
    
    pthread_rwlock_unlock(&graph->rwlock);
}