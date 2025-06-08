#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>    // para pthread_mutex_lock, pthread_rwlock_rdlock, etc.

// Umbrales para indicadores de tráfico
#define CAPACITY_THRESHOLD_1 0.5f
#define CAPACITY_THRESHOLD_2 1.0f
#define CAPACITY_THRESHOLD_3 1.5f

bool is_valid_zone_code(const char *code) {
    if (strlen(code) != 3) return false;
    for (int i = 0; i < 3; i++) {
        if (!isalpha((unsigned char)code[i])) return false;
    }
    return true;
}

int calculate_required_points(int level) {
    int capacity = 1 << level; // 2^level
    return capacity * capacity;
}

void print_traffic_status(CityGraph *graph) {
    // Tomar lock de lectura para todo el grafo
    pthread_rwlock_rdlock(&graph->rwlock);

    printf("\nEstado del tráfico:\n");
    for (int i = 0; i < graph->count; i++) {
        // Por cada zona, tomar su mutex
        pthread_mutex_lock(&graph->zones[i].lock);

        printf("[%s] ", graph->zones[i].code);

        // Norte (ejemplo; los otros sentidos serían análogos)
        if (graph->zones[i].north) {
            float ratio = (float)graph->zones[i].north_vehicles
                          / (float)graph->zones[i].north_capacity;
            printf("N:%d", graph->zones[i].north_vehicles);
            if (ratio >= CAPACITY_THRESHOLD_3)    printf("!!! ");
            else if (ratio >= CAPACITY_THRESHOLD_2) printf("!! ");
            else if (ratio >= CAPACITY_THRESHOLD_1) printf("! ");
            else                                   printf(" ");
        }

        // Aquí podrías añadir Sur, Este y Oeste igual que Norte...

        printf("\n");
        pthread_mutex_unlock(&graph->zones[i].lock);
    }

    pthread_rwlock_unlock(&graph->rwlock);
}

void print_zone_details(Zone *zone) {
    // Tomar mutex de la zona para leer sus campos
    pthread_mutex_lock(&zone->lock);

    printf("Código: %s\n", zone->code);
    printf("Nivel: %d\n", zone->level);
    if (zone->is_source) {
        printf("Desempleados: %d\n", zone->available);
    } else {
        printf("Puestos libres: %d\n", zone->available);
    }
    printf("Puntos acumulados: %d\n", zone->points);

    pthread_mutex_unlock(&zone->lock);
}

void print_menu_options(void) {
    printf("\n==== Menú SimCaracas ====\n");
    printf("1) Agregar zona\n");
    printf("2) Agregar arteria vial\n");
    printf("3) Ampliar arteria vial\n");
    printf("4) Guardar grafo actual\n");
    printf("5) Salir del programa\n");
    printf("Seleccione una opción: ");
}
