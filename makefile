# Configuración del compilador
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -pthread -g
LDFLAGS = -pthread

# Nombre del ejecutable
TARGET = simcaracas

# Directorios
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# Fuentes y objetos
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Regla principal
all: directories $(BINDIR)/$(TARGET)

# Crear directorios necesarios
directories:
	@mkdir -p $(OBJDIR) $(BINDIR)

# Enlazar objetos
$(BINDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

# Compilar cada fuente
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Limpieza
clean:
	rm -rf $(OBJDIR) $(BINDIR)

.PHONY: all directories clean