# MAIN MAKEFILE GOD

CC = g++
CFLAGS = `pkg-config --cflags gtkmm-3.0` -Wall

SRC_DIR = src
OBJ_DIR = bin/obj

PROG = PaginacionSim
MAIN = ${SRC_DIR}/main.cpp
SRC = $(filter-out ${SRC_DIR}/main.cpp, $(shell find ${SRC_DIR} -name '*.cpp'))
OBJ = $(patsubst ${SRC_DIR}/%.cpp, ${OBJ_DIR}/%.o, ${SRC})

LIB_GTK = `pkg-config --libs gtkmm-3.0`
LIB_INC = -Iinclude
LIBS = ${LIB_GTK} ${LIB_INC}

# Default target: build all
${PROG}: ${MAIN} ${OBJ}
	${CC} ${MAIN} -o $@ ${OBJ} ${LIBS} -export-dynamic

${OBJ_DIR}%.o: ${SRC_DIR}%.cpp
	${CC} -c $< -o $@ ${CFLAGS}

clean:
	rm -f ${PROG} ${OBJ}