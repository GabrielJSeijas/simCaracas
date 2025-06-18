#ifndef PIPES_H
#define PIPES_H

#include "graph.h"

/// Tipos de mensaje para comunicación entre procesos
typedef enum {
    ACTUALIZACION_ZONA,     // Notifica un cambio en una zona
    ACTUALIZACION_TRAFICO,  // Actualiza el estado del tránsito
    INICIO_DIA,             // Señal de comienzo de jornada
    FIN_DIA                 // Señal de fin de jornada
} TipoMensaje;

/// Mensaje para actualización de una zona
typedef struct {
    TipoMensaje tipo;  // Tipo de mensaje
    Zona zona;         // Zona modificada
} MensajeZona;

/// Mensaje para inicio o fin de día
typedef struct {
    TipoMensaje tipo;  // INICIO_DIA o FIN_DIA
} MensajeDia;

/// Estructura para enviar actualización de vehículos entre zonas
typedef struct {
    char origen[4];    // Código de zona de origen
    char destino[4];   // Código de zona de destino
    int vehiculos;     // Número de vehículos en tránsito
} ActualizacionTransito;

/// Envía un mensaje de actualización de zona por la tubería
void enviarActualizacionZona(int fdTuberia, const Zona *zona);

/// Envía un mensaje de actualización de tránsito por la tubería
void enviarActualizacionTransito(int fdTuberia, const ActualizacionTransito *actualizacion);

/// Recibe un mensaje de la tubería en el búfer destinoMensaje
/// @return true si leyó exactamente tam bytes, false en caso contrario
bool recibirMensaje(int fdTuberia, void *destinoMensaje, size_t tam);

/// Envía señal de inicio de día de simulación
void enviarInicioDia(int fdTuberia);

/// Envía señal de fin de día de simulación
void enviarFinDia(int fdTuberia);

#endif // PIPES_H
