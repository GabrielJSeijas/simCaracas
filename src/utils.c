#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include "graph.h"
#include "utils.h"

#define UMBRAL_SATURACION_1 0.5f
#define UMBRAL_SATURACION_2 1.0f
#define UMBRAL_SATURACION_3 1.5f
#define LINEAS_ZONA 4

// Verifica si el código de zona es válido (3 letras)
bool esCodigoZonaValido(const char *codigo) {
    if (strlen(codigo) != 3) return false;
    for (int i = 0; i < 3; i++) {
        if (!isalpha((unsigned char)codigo[i])) return false;
    }
    return true;
}

// Calcula la cantidad de puntos requeridos según el nivel
int calcularPuntosRequeridos(int nivel) {
    int capacidad = 1 << nivel; // 2^nivel
    return capacidad * capacidad;
}

// Muestra el estado del tráfico en cada zona (solo dirección norte)
void mostrarEstadoTrafico(GrafoCiudad *grafo) {
    pthread_rwlock_rdlock(&grafo->cerrojoGrafo);
    printf("\nEstado del tráfico:\n");
    for (int i = 0; i < grafo->totalZonas; i++) {
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
        // Se pueden agregar Sur, Este y Oeste de forma similar

        printf("\n");
        pthread_mutex_unlock(&grafo->zonas[i].mutexZona);
    }
    pthread_rwlock_unlock(&grafo->cerrojoGrafo);
}

// Muestra los detalles de una zona específica
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

// Asigna coordenadas a cada zona para su representación en grid
void asignarCoordenadas(GrafoCiudad *grafo) {
    // Inicializa todas las coordenadas a un valor fuera de rango
    for (int i = 0; i < grafo->totalZonas; i++) {
        grafo->zonas[i].x = 9999;
        grafo->zonas[i].y = 9999;
    }

    if (grafo->totalZonas == 0) return;
    grafo->zonas[0].x = 0;
    grafo->zonas[0].y = 0;

    // Propaga coordenadas para zonas conectadas
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

    // Asigna posiciones artificiales a zonas completamente aisladas
    int next_isolated = 2; // fila alta para aislados, columna fija
    for (int i = 0; i < grafo->totalZonas; i++) {
        if (grafo->zonas[i].x == 9999 && grafo->zonas[i].y == 9999) {
            grafo->zonas[i].x = next_isolated++;
            grafo->zonas[i].y = 3;
        }
    }
}

// Imprime la ciudad en formato de grid avanzado
void imprimirCiudadEnGridAvanzado(GrafoCiudad *grafo) {
    int x_min = 9999, x_max = -9999, y_min = 9999, y_max = -9999;
    for (int i = 0; i < grafo->totalZonas; i++) {
        if (grafo->zonas[i].x < x_min) x_min = grafo->zonas[i].x;
        if (grafo->zonas[i].x > x_max) x_max = grafo->zonas[i].x;
        if (grafo->zonas[i].y < y_min) y_min = grafo->zonas[i].y;
        if (grafo->zonas[i].y > y_max) y_max = grafo->zonas[i].y;
    }

    for (int x = x_max; x >= x_min; x--) {
        // Imprime bloque de 4 líneas por zona en la fila
        for (int sublinea = 0; sublinea < LINEAS_ZONA; sublinea++) {
            for (int y = y_min; y <= y_max; y++) {
                Zona *z = NULL;
                for (int i = 0; i < grafo->totalZonas; i++)
                    if (grafo->zonas[i].x == x && grafo->zonas[i].y == y)
                        z = &grafo->zonas[i];
                if (z) {
                    switch (sublinea) {
                        case 0: printf("%-6s    ", z->codigo); break;                  // Código
                        case 1: printf("Nv%-3d    ", z->nivel); break;                // Nivel
                        case 2:
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
                // Carretera ESTE-OESTE (solo en la línea 2)
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

        // Capacidad ESTE-OESTE (debajo de cada fila de zonas)
        for (int y = y_min; y <= y_max; y++) {
            Zona *z = NULL;
            for (int i = 0; i < grafo->totalZonas; i++)
                if (grafo->zonas[i].x == x && grafo->zonas[i].y == y)
                    z = &grafo->zonas[i];
            if (z) printf("        ");
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

        // Carretera SUR y capacidad SUR
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

// Actualiza los totales globales de empleados y desempleados
void actualizarTotalesEmpleo(GrafoCiudad *ciudad) {
    int totalEmpleados = 0, totalDesempleados = 0;
    pthread_rwlock_rdlock(&ciudad->cerrojoGrafo);
    for (int i = 0; i < ciudad->totalZonas; i++) {
        Zona *z = &ciudad->zonas[i];
        if (z->esFuente)
            totalDesempleados += z->disponibles;
        else
            totalEmpleados += z->empleadosPresentes;
    }
    pthread_rwlock_unlock(&ciudad->cerrojoGrafo);
    ciudad->totalEmpleados = totalEmpleados;
    ciudad->totalDesempleados = totalDesempleados;
}

// Muestra un resumen del empleo en la ciudad
void mostrarResumenEmpleo(const GrafoCiudad *ciudad) {
    pthread_rwlock_rdlock(&ciudad->cerrojoGrafo);
    printf("\n===== Resumen del día =====\n");
    printf("Empleados trabajando : %d\n", ciudad->totalEmpleados);
    printf("Desempleados en casa : %d\n", ciudad->totalDesempleados);
    printf("===========================\n");
    pthread_rwlock_unlock(&ciudad->cerrojoGrafo);
}
