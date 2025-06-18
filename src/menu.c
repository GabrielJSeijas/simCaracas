#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "graph.h"
#include "fileio.h"
#include "utils.h"

// Menú para agregar una zona al grafo
void agregarZonaMenu(GrafoCiudad *grafo) {
    char codigo[16];
    int nivel;
    char resp[16];

    printf("Código de zona (3 letras): ");
    if (!fgets(codigo, sizeof(codigo), stdin)) return;
    codigo[strcspn(codigo, "\r\n")] = '\0';

    if (strlen(codigo) != 3) {
        printf("El código debe tener exactamente 3 caracteres.\n");
        return;
    }

    // Verifica si ya existe una zona con ese código
    if (buscarZona(grafo, codigo) != NULL) {
        printf("Ya existe una zona con el código %s.\n", codigo);
        return;
    }

    printf("Nivel (potencia de 2): ");
    char nivelStr[16];
    if (!fgets(nivelStr, sizeof(nivelStr), stdin)) return;
    nivel = atoi(nivelStr);

    printf("¿Es fuente? (s/n): ");
    if (!fgets(resp, sizeof(resp), stdin)) return;

    if (agregarZona(grafo, codigo, nivel, (resp[0]=='s'||resp[0]=='S'))){
        printf("Zona %s agregada (nivel %d, %s).\n",
                codigo, nivel,
                (resp[0]=='s'||resp[0]=='S') ? "fuente" : "sumidero");
        asignarCoordenadas(grafo);
    } else {
        printf("Error al agregar la zona %s.\n", codigo);
    }
}

// Conecta dos zonas en la primera dirección disponible (bidireccional)
int conectarZonasBidireccional(GrafoCiudad *grafo, Zona *a, Zona *b, int capacidad) {
    if (!a->norte && !b->sur) {
        a->norte = b; a->capacidadNorte = capacidad;
        b->sur = a;  b->capacidadSur   = capacidad;
        return 1;
    }
    if (!a->sur && !b->norte) {
        a->sur = b;  a->capacidadSur   = capacidad;
        b->norte = a; b->capacidadNorte = capacidad;
        return 1;
    }
    if (!a->este && !b->oeste) {
        a->este = b; a->capacidadEste = capacidad;
        b->oeste = a; b->capacidadOeste = capacidad;
        return 1;
    }
    if (!a->oeste && !b->este) {
        a->oeste = b; a->capacidadOeste = capacidad;
        b->este = a;  b->capacidadEste  = capacidad;
        return 1;
    }
    // No hay dirección disponible
    return 0;
}

// Menú para agregar una arteria vial entre dos zonas
void agregarArteriaMenu(GrafoCiudad *grafo) {
    char origen[16], destino[16];
    int capacidad;

    printf("Zona origen: ");
    if (!fgets(origen, sizeof(origen), stdin)) return;
    origen[strcspn(origen, "\r\n")] = '\0';

    printf("Zona destino: ");
    if (!fgets(destino, sizeof(destino), stdin)) return;
    destino[strcspn(destino, "\r\n")] = '\0';

    printf("Capacidad inicial: ");
    char capStr[16];
    if (!fgets(capStr, sizeof(capStr), stdin)) return;
    capacidad = atoi(capStr);

    Zona *z1 = buscarZona(grafo, origen);
    Zona *z2 = buscarZona(grafo, destino);
    if (!z1 || !z2) {
        puts("Alguna zona no existe.");
    } else if (conectarZonasBidireccional(grafo, z1, z2, capacidad)) {
        printf("Arteria %s <-> %s creada (cap=%d) en la primera dirección libre.\n", origen, destino, capacidad);
        asignarCoordenadas(grafo);
    } else {
        puts("No se pudo crear la arteria: no hay direcciones libres.");
    }
}

// Menú para ampliar la capacidad de una arteria vial existente
void ampliarArteriaMenu(GrafoCiudad *grafo) {
    char origen[16], destino[16];
    int nuevaCap;

    printf("Zona origen: ");
    if (!fgets(origen, sizeof(origen), stdin)) return;
    origen[strcspn(origen, "\r\n")] = '\0';

    printf("Zona destino: ");
    if (!fgets(destino, sizeof(destino), stdin)) return;
    destino[strcspn(destino, "\r\n")] = '\0';

    printf("Nueva capacidad: ");
    char capStr[16];
    if (!fgets(capStr, sizeof(capStr), stdin)) return;
    nuevaCap = atoi(capStr);

    Zona *z1 = buscarZona(grafo, origen);
    Zona *z2 = buscarZona(grafo, destino);
    if (!z1 || !z2) {
        puts("Alguna zona no existe.");
    } else if (!ampliarArteria(grafo, z1, z2, nuevaCap)) {
        puts("No se pudo ampliar la arteria.");
    } else {
        printf("Arteria %s->%s ampliada a %d.\n", origen, destino, nuevaCap);
    }
}

// Menú para guardar el grafo actual en un archivo CSV
void guardarGrafoMenu(GrafoCiudad *grafo) {
    char nombreArchivo[128];

    printf("Nombre de archivo CSV: ");
    if (!fgets(nombreArchivo, sizeof(nombreArchivo), stdin)) return;
    nombreArchivo[strcspn(nombreArchivo, "\r\n")] = '\0';

    if (guardarCiudadEnCSV(grafo, nombreArchivo))
        printf("Grafo guardado en '%s'.\n", nombreArchivo);
    else
        printf("Error al guardar en '%s'.\n", nombreArchivo);
}
