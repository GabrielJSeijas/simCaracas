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
    (void)configuracion;  // parámetro no usado aquí
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
            case 1: {
                // 1) Agregar zona
                char codigo[4];
                int  nivel;
                char resp;

                printf("Código de zona (3 letras): ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                if (sscanf(entrada, "%3s", codigo) != 1) break;

                printf("Nivel (potencia de 2): ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                nivel = atoi(entrada);

                printf("¿Es fuente? (s/n): ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                resp = entrada[0];

                if (agregarZona(grafo, codigo, nivel, (resp=='s'||resp=='S')))
                    printf("Zona %s agregada (nivel %d, %s).\n",
                           codigo, nivel,
                           (resp=='s'||resp=='S') ? "fuente" : "sumidero");
                else
                    printf("Error al agregar la zona %s.\n", codigo);
                break;
            }

            case 2: {
                // 2) Agregar arteria vial
                char origen[4], destino[4];
                int  capacidad;

                printf("Zona origen: ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                if (sscanf(entrada, "%3s", origen) != 1) break;

                printf("Zona destino: ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                if (sscanf(entrada, "%3s", destino) != 1) break;

                printf("Capacidad inicial: ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                capacidad = atoi(entrada);

                Zona *z1 = buscarZona(grafo, origen);
                Zona *z2 = buscarZona(grafo, destino);
                if (!z1 || !z2) {
                    puts("Alguna zona no existe.");
                } else if (!conectarZonas(grafo, z1, z2, ESTE, capacidad)) {
                    puts("No se pudo crear la arteria (quizá ya existe).");
                } else {
                    printf("Arteria %s->%s creada (cap=%d).\n",
                           origen, destino, capacidad);
                }
                break;
            }

            case 3: {
                // 3) Ampliar arteria vial
                char origen[4], destino[4];
                int  nuevaCap;

                printf("Zona origen: ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                if (sscanf(entrada, "%3s", origen) != 1) break;

                printf("Zona destino: ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                if (sscanf(entrada, "%3s", destino) != 1) break;

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

            case 4: {
                // 4) Guardar grafo actual
                char nombreArchivo[TAM_BUFFER];

                printf("Nombre de archivo CSV: ");
                if (!fgets(entrada, TAM_BUFFER, stdin)) break;
                entrada[strcspn(entrada, "\r\n")] = '\0';
                strcpy(nombreArchivo, entrada);

                if (guardarCiudadEnCSV(grafo, nombreArchivo))
                    printf("Grafo guardado en '%s'.\n", nombreArchivo);
                else
                    printf("Error al guardar en '%s'.\n", nombreArchivo);
                break;
            }

            case 5:
                // 5) Salir
                puts("¡Hasta luego!");
                return;

            default:
                puts("Opción inválida. Intenta de nuevo.");
        }
    }
}
