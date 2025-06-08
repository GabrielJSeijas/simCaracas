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

typedef struct Zona {
    char            codigo[LONGITUD_COD_ZONA];
    int             nivel;
    int             puntos;
    bool            esFuente;
    int             disponibles;

    struct Zona    *norte;
    struct Zona    *sur;
    struct Zona    *este;
    struct Zona    *oeste;

    int             capacidadNorte;
    int             capacidadSur;
    int             capacidadEste;
    int             capacidadOeste;

    int             vehiculosNorte;
    int             vehiculosSur;
    int             vehiculosEste;
    int             vehiculosOeste;

    pthread_mutex_t mutexZona;
} Zona;

typedef struct {
    Zona            *zonas;
    int              totalZonas;
    int              capacidadReservada;

    int              totalEmpleados;
    int              totalDesempleados;

    pthread_rwlock_t cerrojoGrafo;
} GrafoCiudad;

/* Inicialización y destrucción */
void iniciarGrafo(GrafoCiudad *grafo);
void destruirGrafo(GrafoCiudad *grafo);

/* Operaciones sobre el grafo */
Zona*   agregarZona      (GrafoCiudad *grafo,
                          const char *codigo,
                          int nivel,
                          bool esFuente);
bool    conectarZonas    (GrafoCiudad *grafo,
                          Zona *z1,
                          Zona *z2,
                          Direccion dir,
                          int capacidadInicial);
Zona*   buscarZona       (GrafoCiudad *grafo,
                          const char *codigo);
void    mostrarGrafo     (GrafoCiudad *grafo);
Direccion direccionOpuesta(Direccion dir);

/* Amplía la capacidad de una arteria ya existente */
bool    ampliarArteria   (GrafoCiudad *grafo,
                          Zona *zonaOrigen,
                          Zona *zonaDestino,
                          int nuevaCapacidad);

#endif // GRAPH_H
