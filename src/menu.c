// src/menu.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "graph.h"
#include "fileio.h"
#include "zone.h"
#include "utils.h"

#define TAM_BUFFER 128

void iniciarMenuInteractivo(GrafoCiudad *grafo,
                            Configuracion configuracion)
{
    (void)configuracion;  // silenciar unused-parameter
    char entrada[TAM_BUFFER];
    int  opcion;

    while (1) {
        mostrarOpcionesMenuPrincipal();
        if (!fgets(entrada, TAM_BUFFER, stdin)) {
            puts("No se recibió entrada. Saliendo...");
            return;
        }
        opcion = atoi(entrada);

        switch (opcion) {
            /* ... casos 1 y 2 idénticos ... */

            case 3: {
                char origen[4], destino[4];
                int  nuevaCap;

                printf("Zona origen: ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                sscanf(entrada, "%3s", origen);

                printf("Zona destino: ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                sscanf(entrada, "%3s", destino);

                printf("Nueva capacidad: ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                nuevaCap = atoi(entrada);

                Zona *z1 = buscarZona(grafo, origen);
                Zona *z2 = buscarZona(grafo, destino);
                if (!z1 || !z2) {
                    puts("Alguna zona no existe.");
                } else if (!ampliarArteria(grafo, z1, z2, nuevaCap)) {
                    puts("No se pudo ampliar la arteria.");
                } else {
                    printf("Arteria %s->%s ampliada a %d.\n",
                           origen, destino, nuevaCap);
                }
                break;
            }

            /* ... resto de casos ... */
        }
    }
}
