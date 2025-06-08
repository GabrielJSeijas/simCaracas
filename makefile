# Configuración del compilador
CC       = gcc
CFLAGS   = -Wall -Wextra -pedantic -pthread -g -I$(INCDIR)
LDFLAGS  = -pthread

# Nombre del ejecutable
TARGET   = simcaracas

# Directorios
SRCDIR   = src
INCDIR   = include
OBJDIR   = obj
BINDIR   = bin

# Fuentes y objetos
SOURCES  = $(wildcard $(SRCDIR)/*.c)
OBJECTS  = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# --------------------------------------------------------
.PHONY: all directories clean rebuild run

# Target por defecto: crea directorios y compila todo
all: directories $(BINDIR)/$(TARGET)

# Crea los directorios obj/ y bin/ si no existen
directories:
	@mkdir -p $(OBJDIR) $(BINDIR)

# Enlaza todos los objetos en el binario final
$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# Compila cada .c de src/ a .o en obj/
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --------------------------------------------------------
# run: compila y ejecuta el simulador,
# sin parámetros (entra en el menú interactivo)
run: all
	@echo "=== Ejecutando $(TARGET) ==="
	@$(BINDIR)/$(TARGET)

# rebuild: fuerza limpieza y compilación desde cero
rebuild: clean all

# clean: elimina binarios y objetos
clean:
	@echo "Limpiando..."
	@rm -rf $(OBJDIR) $(BINDIR)
	@echo "Listo."

