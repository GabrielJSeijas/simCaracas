#include "fileio.h"
#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_LINE_LEN 1024

bool load_city_from_csv(CityGraph *graph, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error abriendo %s: %s\n", filename, strerror(errno));
        return false;
    }

    char line[MAX_LINE_LEN];
    
    // Leer y descartar la cabecera
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return false;
    }

    // Leer cada línea del archivo
    while (fgets(line, sizeof(line), file)) {
        char *token;
        char *saveptr;
        char *tokens[12]; // Tenemos 11 campos + NULL
        
        // Dividir la línea en tokens
        int i = 0;
        for (token = strtok_r(line, ",", &saveptr); 
             token && i < 11; 
             token = strtok_r(NULL, ",", &saveptr)) {
            
            // Eliminar comillas si existen
            if (token[0] == '"' && token[strlen(token)-1] == '"') {
                token[strlen(token)-1] = '\0';
                token++;
            }
            
            tokens[i++] = token;
        }
        
        if (i < 11) continue; // Línea incompleta
        
        // Crear la zona
        int level = atoi(tokens[1]);
        int points = atoi(tokens[2]);
        bool is_source = (strstr(tokens[0], "RES") != NULL); // Ejemplo: si el código contiene RES es residencial
        
        Zone *zone = graph_add_zone(graph, tokens[0], level, is_source);
        if (!zone) continue;
        
        zone->points = points;
        
        // Procesar conexiones
        struct {
            Direction dir;
            const char *conn;
            const char *cap;
        } connections[] = {
            {NORTH, tokens[3], tokens[4]},
            {SOUTH, tokens[5], tokens[6]},
            {EAST,  tokens[7], tokens[8]},
            {WEST,  tokens[9], tokens[10]}
        };
        
        for (int j = 0; j < 4; j++) {
            if (strcmp(connections[j].conn, "") != 0 && 
                strcmp(connections[j].conn, "NULL") != 0) {
                
                Zone *other = graph_find_zone(graph, connections[j].conn);
                if (other) {
                    int capacity = atoi(connections[j].cap);
                    graph_connect_zones(graph, zone, other, connections[j].dir, capacity);
                }
            }
        }
    }
    
    fclose(file);
    return true;
}