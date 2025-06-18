#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <pthread.h>

#define MAX_ZONAS           100
#define LONGITUD_COD_ZONA    4

typedef enum {
    NORTE,
    SUR,
    ESTE,
    OESTE,
    NINGUNA
} Direccion;

// Estructura que representa una zona de la ciudad
typedef struct Zona {
    char            codigo[LONGITUD_COD_ZONA]; // Código identificador de la zona
    int             nivel;                     // Nivel de la zona
    int             puntos;                    // Puntos de la zona
    bool            esFuente;                  // Indica si es una zona fuente
    int             disponibles;               // Recursos disponibles
    int             empleadosPresentes;        // Empleados presentes en la zona

    struct Zona    *norte, *sur, *este, *oeste; // Punteros a zonas adyacentes

    int             capacidadNorte;
    int             capacidadSur;
    int             capacidadEste;
    int             capacidadOeste;

    int             vehiculosNorte;
    int             vehiculosSur;
    int             vehiculosEste;
    int             vehiculosOeste;
    int             x, y; // Coordenadas de la zona

    pthread_mutex_t mutexZona; // Mutex para acceso concurrente a la zona
} Zona;

// Estructura que representa el grafo de la ciudad
typedef struct {
    Zona            *zonas;                // Arreglo dinámico de zonas
    int              totalZonas;           // Número total de zonas
    int              capacidadReservada;   // Capacidad reservada para el arreglo de zonas
    int              totalEmpleados;       // Total de empleados en la ciudad
    int              totalDesempleados;    // Total de desempleados en la ciudad
    int              nivelMaximoZonas;     // Nivel máximo entre todas las zonas
    pthread_rwlock_t cerrojoGrafo;         // Cerrojo para acceso concurrente al grafo
} GrafoCiudad;

// Inicializa el grafo de la ciudad
void iniciarGrafo(GrafoCiudad *grafo);
// Libera los recursos asociados al grafo
void destruirGrafo(GrafoCiudad *grafo);

// Agrega una nueva zona al grafo
Zona* agregarZona(GrafoCiudad *grafo, const char *codigo, int nivel, bool esFuente);
// Conecta dos zonas en una dirección específica con una capacidad inicial
bool conectarZonas(GrafoCiudad *grafo, Zona *z1, Zona *z2, Direccion dir, int capacidadInicial);
// Busca una zona por su código
Zona* buscarZona(GrafoCiudad *grafo, const char *codigo);
// Devuelve la dirección opuesta a la dada
Direccion direccionOpuesta(Direccion dir);
// Amplía la capacidad de una arteria existente entre dos zonas
bool ampliarArteria(GrafoCiudad *grafo, Zona *zonaOrigen, Zona *zonaDestino, int nuevaCapacidad);

#endif // GRAPH_H
