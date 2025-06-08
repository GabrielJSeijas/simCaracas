#ifndef CONFIG_H
#define CONFIG_H

/// Parámetros de la simulación
typedef struct {
    int    ticksPorDia;       // Cuántos ticks tiene un día
    float  segundosPorTick;   // Duración de cada tick en segundos
    int    nivelMaximoZona;   // Nivel máximo permitido para las zonas
    char  *archivoCarga;      // Ruta al CSV inicial (NULL si ninguno)
} Configuracion;

/// Lee y procesa los argumentos de la línea de comandos
Configuracion parsearArgumentos(int argc, char *argv[]);

/// Muestra por pantalla la configuración resultante
void mostrarConfiguracion(const Configuracion *config);

#endif // CONFIG_H
