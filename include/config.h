#ifndef CONFIG_H
#define CONFIG_H

// Estructura para almacenar la configuración de la simulación
typedef struct {
    int    ticksPorDia;        // Cantidad de ticks en un día
    float  segundosPorTick;    // Duración de cada tick en segundos
    int    nivelMaximoZona;    // Nivel máximo permitido para las zonas
    char  *archivoCarga;       // Ruta al archivo CSV inicial (NULL si no se usa)
} Configuracion;

// Procesa los argumentos de la línea de comandos y retorna la configuración
Configuracion parsearArgumentos(int argc, char *argv[]);

// Muestra la configuración actual por pantalla
void mostrarConfiguracion(const Configuracion *config);

#endif // CONFIG_H
