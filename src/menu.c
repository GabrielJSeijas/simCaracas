#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "menu.h"
#include "utils.h"    // print_menu_options()
#include "graph.h"    // CityGraph, graph_add_zone, graph_connect_zones, graph_find_zone, etc.
#include "fileio.h"   // save_city_to_csv()
#include "traffic.h"  // simulate_morning_traffic(), simulate_evening_traffic(), update_traffic_flows()
#include "zone.h"     // update_zone_points()

#define LINEBUF 128

void menu_loop(CityGraph *city, Config config) {
    char buf[LINEBUF];
    int  opt;

    while (true) {
        print_menu_options();
        if (!fgets(buf, sizeof buf, stdin)) {
            puts("Error leyendo opción, saliendo.");
            break;
        }
        opt = atoi(buf);

        switch (opt) {
        case 1:  // Simular un día completo
            puts("\n>> Simulando un nuevo día...");
            update_zone_points(city, true);
            simulate_morning_traffic(city, config.day_ticks);
            update_traffic_flows(city);
            simulate_evening_traffic(city, config.day_ticks);
            update_zone_points(city, false);
            printf(">> Día simulado: empleados=%d desempleados=%d\n\n",
                   city->total_employed,
                   city->total_unemployed);
            break;

        case 2: { // Agregar zona
            char code[ZONE_CODE_LEN];
            int  level;
            char c;
            bool is_source;

            printf("Código (3 letras): ");
            if (!fgets(buf,sizeof buf,stdin)) break;
            if (sscanf(buf,"%3s", code)!=1) { puts("Código inválido"); break; }

            printf("Nivel (potencia de 2): ");
            if (!fgets(buf,sizeof buf,stdin)) break;
            level = atoi(buf);

            printf("¿Fuente? (s/n): ");
            if (!fgets(buf,sizeof buf,stdin)) break;
            c = buf[0];
            is_source = (c=='s'||c=='S');

            if (graph_add_zone(city, code, level, is_source))
                printf("Zona %s añadida.\n\n", code);
            else
                printf("Error al añadir zona %s (¿repetido o inválido?).\n\n", code);
            break;
        }

        case 3: { // Agregar arteria vial
            char a[ZONE_CODE_LEN], b[ZONE_CODE_LEN];
            int  cap;

            printf("Zona origen (3 letras): ");
            if (!fgets(buf,sizeof buf,stdin)) break;
            sscanf(buf,"%3s", a);

            printf("Zona destino (3 letras): ");
            if (!fgets(buf,sizeof buf,stdin)) break;
            sscanf(buf,"%3s", b);

            printf("Capacidad inicial: ");
            if (!fgets(buf,sizeof buf,stdin)) break;
            cap = atoi(buf);

            Zone *z1 = graph_find_zone(city,a);
            Zone *z2 = graph_find_zone(city,b);
            if (!z1||!z2) {
                puts("Zona no encontrada.\n");
                break;
            }
            // Aquí podrías preguntar dirección; por defecto EAST:
            if (graph_connect_zones(city, z1, z2, EAST, cap))
                printf("Arteria %s->%s (cap=%d) creada.\n\n", a,b,cap);
            else
                puts("Error al crear arteria (quizá ya existía).\n");
            break;
        }

        case 4: { // Ampliar arteria
            char a[ZONE_CODE_LEN], b[ZONE_CODE_LEN];
            int  newcap;

            printf("Zona origen: ");
            if (!fgets(buf,sizeof buf,stdin)) break;
            sscanf(buf,"%3s", a);

            printf("Zona destino: ");
            if (!fgets(buf,sizeof buf,stdin)) break;
            sscanf(buf,"%3s", b);

            printf("Nueva capacidad: ");
            if (!fgets(buf,sizeof buf,stdin)) break;
            newcap = atoi(buf);

            // Tendrás que implementar en graph.c:
            // bool graph_expand_road(CityGraph*, Zone*, Zone*, int);
            Zone *z1 = graph_find_zone(city,a);
            Zone *z2 = graph_find_zone(city,b);
            if (!z1||!z2) {
                puts("Zona no encontrada.\n");
                break;
            }
            if (graph_expand_road(city, z1, z2, newcap))
                printf("Arteria ampliada a %d.\n\n", newcap);
            else
                puts("No se pudo ampliar (quizá no existe).\n");
            break;
        }

        case 5: { // Guardar el grafo
            char fname[LINEBUF];
            printf("Archivo de salida (CSV): ");
            if (!fgets(fname,sizeof fname,stdin)) break;
            fname[strcspn(fname,"\r\n")] = '\0';

            if (save_city_to_csv(city, fname))
                printf("Grafo guardado en %s\n\n", fname);
            else
                printf("Error guardando en %s\n\n", fname);
            break;
        }

        case 6: // Salir
            puts("Saliendo...");
            return;

        default:
            puts("Opción inválida, intente de nuevo.\n");
        }
    }
}
