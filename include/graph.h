#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <pthread.h>

#define MAX_ZONES 100  // Máximo temporal, puedes usar realloc después para hacerlo dinámico
#define ZONE_CODE_LEN 4 // 3 letras + null terminator

// Direcciones para conexiones entre zonas
typedef enum {
    NORTH,
    SOUTH,
    EAST,
    WEST,
    NONE
} Direction;

// Estructura que representa una zona (nodo del grafo)
typedef struct Zone {
    char code[ZONE_CODE_LEN];  // Código de 3 letras
    int level;                 // Nivel de la zona (potencia de 2)
    int points;                // Puntos acumulados
    bool is_source;           // true = fuente (hogares), false = sumidero (trabajos)
    int available;             // Puestos disponibles (sumidero) o desempleados (fuente)
    
    // Conexiones a otras zonas
    struct Zone *north;
    struct Zone *south;
    struct Zone *east;
    struct Zone *west;
    
    // Capacidades de las arterias viales
    int north_capacity;
    int south_capacity;
    int east_capacity;
    int west_capacity;
    
    // Vehículos actuales en cada arteria (para tráfico)
    int north_vehicles;
    int south_vehicles;
    int east_vehicles;
    int west_vehicles;
    
    pthread_mutex_t lock;  // Mutex para sincronización de hilos
} Zone;

// Estructura principal que contiene todo el grafo de la ciudad
typedef struct {
    Zone *zones;          // Array dinámico de zonas
    int count;            // Número actual de zonas
    int capacity;         // Capacidad actual del array
    
    // Estadísticas globales
    int total_employed;
    int total_unemployed;
    
    pthread_rwlock_t rwlock;  // Lock de lectura/escritura para el grafo completo
} CityGraph;

// Prototipos de funciones
void graph_init(CityGraph *graph);
void graph_destroy(CityGraph *graph);
Zone* graph_add_zone(CityGraph *graph, const char *code, int level, bool is_source);
bool graph_connect_zones(CityGraph *graph, Zone *zone1, Zone *zone2, Direction dir, int initial_capacity);
Zone* graph_find_zone(CityGraph *graph, const char *code);
void graph_print(const CityGraph *graph);
Direction opposite_direction(Direction dir);

#endif // GRAPH_H