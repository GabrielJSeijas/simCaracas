// src/fileio.c

#include "fileio.h"    // load_city_from_csv, save_city_to_csv :contentReference[oaicite:0]{index=0}
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
    // Saltar cabecera
    if (!fgets(line, sizeof(line), file)) {
        fclose(file);
        return false;
    }

    while (fgets(line, sizeof(line), file)) {
        char *tokens[12];
        char *saveptr = NULL;
        int i = 0;

        for (char *tok = strtok_r(line, ",", &saveptr);
             tok && i < 11;
             tok = strtok_r(NULL, ",", &saveptr))
        {
            // Eliminar comillas si las hubiera
            if (tok[0]=='"' && tok[strlen(tok)-1]=='"') {
                tok[strlen(tok)-1] = '\0';
                tok++;
            }
            tokens[i++] = tok;
        }
        if (i < 11) continue;

        // tokens[0]=code, [1]=type(F/S), [2]=level, [3]=points,
        // [4]=north, [5]=capN, [6]=south, [7]=capS, [8]=east, [9]=capE, [10]=west, [11]=capW
        const char *code    = tokens[0];
        bool       is_src   = (tokens[1][0]=='F');
        int        level    = atoi(tokens[2]);
        int        points   = atoi(tokens[3]);

        Zone *z = graph_add_zone(graph, code, level, is_src);
        if (!z) continue;
        z->points = points;

        struct { Direction dir; const char *nbr; const char *cap; } conns[4] = {
            { NORTH, tokens[4], tokens[5] },
            { SOUTH, tokens[6], tokens[7] },
            { EAST,  tokens[8], tokens[9] },
            { WEST,  tokens[10], tokens[11] }
        };

        for (int j = 0; j < 4; j++) {
            if (conns[j].nbr[0] != '\0') {
                Zone *o = graph_find_zone(graph, conns[j].nbr);
                if (o) {
                    int cap = atoi(conns[j].cap);
                    graph_connect_zones(graph, z, o, conns[j].dir, cap);
                }
            }
        }
    }

    fclose(file);
    return true;
}

bool save_city_to_csv(const CityGraph *city, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error creando %s: %s\n", filename, strerror(errno));
        return false;
    }
    // Cabecera
    fprintf(f,
        "code,type,level,points,"
        "north,cap_north,"
        "south,cap_south,"
        "east,cap_east,"
        "west,cap_west\n");

    for (int i = 0; i < city->count; i++) {
        const Zone *z = &city->zones[i];
        fprintf(f,
            "%s,%c,%d,%d,"
            "%s,%d,"
            "%s,%d,"
            "%s,%d,"
            "%s,%d\n",
            z->code,
            z->is_source ? 'F' : 'S',
            z->level,
            z->points,
            z->north ? z->north->code : "",
            z->north_capacity,
            z->south ? z->south->code : "",
            z->south_capacity,
            z->east  ? z->east->code  : "",
            z->east_capacity,
            z->west  ? z->west->code  : "",
            z->west_capacity
        );
    }

    fclose(f);
    return true;
}
