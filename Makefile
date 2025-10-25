# MAIN MAKEFILE GOD
# Compiler
CC = g++

# Project directories
SRC_DIR = src
OBJ_DIR = bin/obj
PROG = PaginacionSim
MAIN = ${SRC_DIR}/main.cpp

GTK_CFLAGS := $(shell pkg-config --cflags gtkmm-3.0 2>/dev/null)
GTK_LIBS := $(shell pkg-config --libs gtkmm-3.0 2>/dev/null)

# Common flags - include both local and bin includes
CFLAGS = $(GTK_CFLAGS) -Wall -Iinclude -DPROJECT_ROOT='"$(PWD)"'
LIBS = $(GTK_LIBS)

# Gather source files from src directory (filter out main)
SRC = $(filter-out ${SRC_DIR}/main.cpp, $(shell find ${SRC_DIR} -name '*.cpp'))

# Generate object file paths
OBJ = $(patsubst ${SRC_DIR}/%.cpp, ${OBJ_DIR}/%.o, $(filter ${SRC_DIR}/%.cpp, ${SRC}))

OBJ_DIRS = $(sort $(dir ${OBJ}))

# Default target: build executable
all: ${PROG}

${PROG}: ${OBJ} ${MAIN}
	${CC} ${CFLAGS} ${MAIN} ${OBJ} -o $@ ${LIBS} -Wl,-export-dynamic

# Compile src directory files
${OBJ_DIR}/%.o: ${SRC_DIR}/%.cpp ${OBJ_DIRS}
	${CC} -c $< -o $@ ${CFLAGS}

# Ensure object directories exist
${OBJ_DIRS}:
	mkdir -p $@

clean:
	rm -f ${PROG} ${OBJ}
	rm -rf ${OBJ_DIR}

.PHONY: all core clean

core: ${OBJ}
	@echo "Core objects built (UI skipped)."