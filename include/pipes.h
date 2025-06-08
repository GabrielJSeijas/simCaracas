#ifndef PIPES_H
#define PIPES_H

#include "graph.h"

/// Tipos de mensaje para comunicación entre procesos
typedef enum {
    ACTUALIZACION_ZONA,     ///< Notifica un cambio en una zona
    ACTUALIZACION_TRAFICO,  ///< Actualiza el estado del tránsito
    INICIO_DIA,             ///< Señal de comienzo de jornada
    FIN_DIA                 ///< Señal de fin de jornada
} TipoMensaje;

/// Contenido de un mensaje de zona
typedef struct {
    TipoMensaje tipo;  ///< Tipo de mensaje
    Zona         zona; ///< Copia de la zona modificada
} MensajeZona;

/// Mensaje sencillo de inicio/fin de día
typedef struct {
    TipoMensaje tipo;  ///< INICIO_DIA o FIN_DIA
} MensajeDia;

/// Estructura para enviar actualización de vehículos entre zonas
typedef struct {
    char               origen[4];   ///< Código de zona de origen
    char               destino[4];  ///< Código de zona de destino
    int                vehiculos;   ///< Número de vehículos en tránsito
} ActualizacionTransito;

/// Envia un mensaje de actualización de zona por la tubería
void enviarActualizacionZona(int fdTuberia, const Zona *zona);

/// Envia un mensaje de actualización de tránsito por la tubería
void enviarActualizacionTransito(int fdTuberia,
                                 const ActualizacionTransito *actualizacion);

/// Recibe cualquier mensaje de la tubería en el búfer destinoMensaje
/// @return true si leyó exactamente tam bytes, false de lo contrario
bool recibirMensaje(int fdTuberia, void *destinoMensaje, size_t tam);

/// Señala el inicio de un día de simulación
void enviarInicioDia(int fdTuberia);

/// Señala el fin de un día de simulación
void enviarFinDia(int fdTuberia);

#endif // PIPES_H
