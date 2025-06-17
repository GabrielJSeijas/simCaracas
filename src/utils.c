#include <stdbool.h>
#include "graph.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#define UMBRAL_SATURACION_1 0.5f
#define UMBRAL_SATURACION_2 1.0f
#define UMBRAL_SATURACION_3 1.5f
#define LINEAS_ZONA 4



bool esCodigoZonaValido(const char *codigo) {
    if (strlen(codigo) != 3) return false;
    for (int i = 0; i < 3; i++) {
        if (!isalpha((unsigned char)codigo[i])) return false;
    }
    return true;
}

int calcularPuntosRequeridos(int nivel) {
    int capacidad = 1 << nivel; // 2^nivel
    return capacidad * capacidad;
}

void mostrarEstadoTrafico(GrafoCiudad *grafo) {
    // Lock de lectura global
    pthread_rwlock_rdlock(&grafo->cerrojoGrafo);
    printf("\nEstado del tráfico:\n");
    for (int i = 0; i < grafo->totalZonas; i++) {
        // Lock por zona
        pthread_mutex_lock(&grafo->zonas[i].mutexZona);

        printf("[%s] ", grafo->zonas[i].codigo);
        if (grafo->zonas[i].norte) {
            float proporcion = (float)grafo->zonas[i].vehiculosNorte
                              / (float)grafo->zonas[i].capacidadNorte;
            printf("N:%d", grafo->zonas[i].vehiculosNorte);
            if (proporcion >= UMBRAL_SATURACION_3)     printf("!!! ");
            else if (proporcion >= UMBRAL_SATURACION_2) printf("!! ");
            else if (proporcion >= UMBRAL_SATURACION_1) printf("! ");
            else                                      printf(" ");
        }

        // Aquí podrías añadir Sur, Este y Oeste de forma similar...

        printf("\n");
        pthread_mutex_unlock(&grafo->zonas[i].mutexZona);
    }
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
}

void mostrarDetallesZona(Zona *zona) {
    pthread_mutex_lock(&zona->mutexZona);
    printf("Código           : %s\n", zona->codigo);
    printf("Nivel            : %d\n", zona->nivel);
    if (zona->esFuente)
        printf("Desempleados     : %d\n", zona->disponibles);
    else
        printf("Puestos libres   : %d\n", zona->disponibles);
    printf("Puntos acumulados: %d\n", zona->puntos);
    pthread_mutex_unlock(&zona->mutexZona);
}

void asignarCoordenadas(GrafoCiudad *grafo) {
    for (int i = 0; i < grafo->totalZonas; i++) {
        grafo->zonas[i].x = 9999;
        grafo->zonas[i].y = 9999;
    }

    if (grafo->totalZonas == 0) return;
    grafo->zonas[0].x = 0;
    grafo->zonas[0].y = 0;

    bool cambiado = true;
    while (cambiado) {
        cambiado = false;
        for (int i = 0; i < grafo->totalZonas; i++) {
            Zona *z = &grafo->zonas[i];
            if (z->x == 9999) continue;

            if (z->norte && z->norte->x == 9999) {
                z->norte->x = z->x + 1;
                z->norte->y = z->y;
                cambiado = true;
            }
            if (z->sur && z->sur->x == 9999) {
                z->sur->x = z->x - 1;
                z->sur->y = z->y;
                cambiado = true;
            }
            if (z->este && z->este->x == 9999) {
                z->este->x = z->x;
                z->este->y = z->y + 1;
                cambiado = true;
            }
            if (z->oeste && z->oeste->x == 9999) {
                z->oeste->x = z->x;
                z->oeste->y = z->y - 1;
                cambiado = true;
            }
        }
    }
}

void imprimirCiudadEnGridAvanzado(GrafoCiudad *grafo) {
    int x_min = 9999, x_max = -9999, y_min = 9999, y_max = -9999;
    for (int i = 0; i < grafo->totalZonas; i++) {
        if (grafo->zonas[i].x < x_min) x_min = grafo->zonas[i].x;
        if (grafo->zonas[i].x > x_max) x_max = grafo->zonas[i].x;
        if (grafo->zonas[i].y < y_min) y_min = grafo->zonas[i].y;
        if (grafo->zonas[i].y > y_max) y_max = grafo->zonas[i].y;
    }

    for (int x = x_max; x >= x_min; x--) {
        // IMPRIME BLOQUE DE 4 LÍNEAS POR ZONA EN LA FILA
        for (int sublinea = 0; sublinea < LINEAS_ZONA; sublinea++) {
            for (int y = y_min; y <= y_max; y++) {
                Zona *z = NULL;
                for (int i = 0; i < grafo->totalZonas; i++)
                    if (grafo->zonas[i].x == x && grafo->zonas[i].y == y)
                        z = &grafo->zonas[i];
                if (z) {
                    switch (sublinea) {
                        case 0: printf("%-6s    ", z->codigo); break;                  // Código
                        case 1: printf("Nvl%-3d    ", z->nivel); break;                // Nivel
                        case 2: // Desempleados/libres
                            if (z->esFuente)
                                printf("Desem:%-3d ", z->disponibles);
                            else
                                printf("Libres:%-3d ", z->disponibles);
                            break;
                        case 3: printf("Pts:%-4d  ", z->puntos); break;              // Puntos
                    }
                } else {
                    printf("          "); // Espacio vacío
                }
                // Carretera ESTE-OESTE (solo en la línea 2, por ejemplo)
                if (y < y_max && sublinea == 2) {
                    Zona *zE = NULL;
                    for (int i = 0; i < grafo->totalZonas; i++)
                        if (grafo->zonas[i].x == x && grafo->zonas[i].y == y+1)
                            zE = &grafo->zonas[i];
                    if (z && z->este == zE && z->capacidadEste > 0) {
                        float ocup = (float)z->vehiculosEste / z->capacidadEste;
                        if (ocup >= 1.5) printf(" -!!!- ");
                        else if (ocup >= 1.0) printf(" -!!- ");
                        else if (ocup >= 0.5) printf(" -!- ");
                        else printf(" - - ");
                    } else {
                        printf("      ");
                    }
                } else if (y < y_max) {
                    printf("      ");
                }
            }
            printf("\n");
        }

        // CAPACIDAD ESTE-OESTE (debajo de cada fila de zonas)
        for (int y = y_min; y <= y_max; y++) {
            Zona *z = NULL;
            for (int i = 0; i < grafo->totalZonas; i++)
                if (grafo->zonas[i].x == x && grafo->zonas[i].y == y)
                    z = &grafo->zonas[i];
            if (z) printf("        "); // espacio, puedes imprimir otra cosa aquí si quieres
            else printf("        ");
            // Capacidad ESTE-OESTE
            if (y < y_max) {
                Zona *zE = NULL;
                for (int i = 0; i < grafo->totalZonas; i++)
                    if (grafo->zonas[i].x == x && grafo->zonas[i].y == y+1)
                        zE = &grafo->zonas[i];
                if (z && z->este == zE && z->capacidadEste > 0)
                    printf("%6d ", z->capacidadEste);
                else
                    printf("       ");
            }
        }
        printf("\n");

        // CARRETERA SUR y CAPACIDAD SUR
        if (x > x_min) {
            // Congestión SUR
            for (int y = y_min; y <= y_max; y++) {
                Zona *z = NULL, *zS = NULL;
                for (int i = 0; i < grafo->totalZonas; i++) {
                    if (grafo->zonas[i].x == x && grafo->zonas[i].y == y) z = &grafo->zonas[i];
                    if (grafo->zonas[i].x == x-1 && grafo->zonas[i].y == y) zS = &grafo->zonas[i];
                }
                if (z && z->sur == zS && z->capacidadSur > 0) {
                    float ocup = (float)z->vehiculosSur / z->capacidadSur;
                    if (ocup >= 1.5) printf("  !!!   ");
                    else if (ocup >= 1.0) printf("  !!    ");
                    else if (ocup >= 0.5) printf("  !     ");
                    else printf("  |     ");
                } else {
                    printf("        ");
                }
                // Espacio entre columnas
                if (y < y_max) printf("       ");
            }
            printf("\n");
            // Capacidad SUR
            for (int y = y_min; y <= y_max; y++) {
                Zona *z = NULL, *zS = NULL;
                for (int i = 0; i < grafo->totalZonas; i++) {
                    if (grafo->zonas[i].x == x && grafo->zonas[i].y == y) z = &grafo->zonas[i];
                    if (grafo->zonas[i].x == x-1 && grafo->zonas[i].y == y) zS = &grafo->zonas[i];
                }
                if (z && z->sur == zS && z->capacidadSur > 0)
                    printf("%7d ", z->capacidadSur);
                else
                    printf("        ");
                if (y < y_max) printf("      ");
            }
            printf("\n");
        }
    }
}