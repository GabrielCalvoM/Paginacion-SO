# MAIN MAKEFILE GOD
# Compiler
CC = g++

# Project directories
SRC_DIR = src
OBJ_DIR = bin/obj
BIN_INCLUDE = bin/include
BIN_SIM = bin/sim
BIN_UI = bin/ui
PROG = PaginacionSim
MAIN = ${SRC_DIR}/main.cpp

# Detect gtkmm via pkg-config. If not found, we'll build without the UI sources.
GTK_AVAILABLE := $(shell (pkg-config --exists gtkmm-3.0) >/dev/null 2>&1 && echo 1 || echo 0)

ifeq ($(GTK_AVAILABLE),1)
GTK_CFLAGS := $(shell pkg-config --cflags gtkmm-3.0 2>/dev/null)
GTK_LIBS := $(shell pkg-config --libs gtkmm-3.0 2>/dev/null)
else
GTK_CFLAGS :=
GTK_LIBS :=
endif

# Common flags - include both local and bin includes
CFLAGS = $(GTK_CFLAGS) -Wall -Iinclude -I${BIN_INCLUDE}
LIBS = $(GTK_LIBS)

# Gather source files from src directory (filter out main)
SRC = $(filter-out ${SRC_DIR}/main.cpp, $(shell find ${SRC_DIR} -name '*.cpp'))

# Exclude UI sources from src directory
SRC := $(filter-out ${SRC_DIR}/ui/%.cpp ${SRC_DIR}/UI/%.cpp, $(SRC))

# Add sim sources from bin/sim (exclude bin/ui)
BIN_SIM_SRC = $(shell find ${BIN_SIM} -name '*.cpp' 2>/dev/null)
SRC += ${BIN_SIM_SRC}

# If gtkmm isn't available, also exclude app.cpp
ifeq ($(GTK_AVAILABLE),0)
SRC := $(filter-out ${SRC_DIR}/app.cpp, $(SRC))
$(info gtkmm not found; building without UI sources)
$(info defaulting to 'all' target)
endif

# Generate object file paths
OBJ = $(patsubst ${SRC_DIR}/%.cpp, ${OBJ_DIR}/%.o, $(filter ${SRC_DIR}/%.cpp, ${SRC}))
OBJ += $(patsubst ${BIN_SIM}/%.cpp, ${OBJ_DIR}/bin_sim_%.o, $(filter ${BIN_SIM}/%.cpp, ${SRC}))

OBJ_DIRS = $(sort $(dir ${OBJ}))

# Default target: build executable
all: ${PROG}

${PROG}: ${MAIN} ${OBJ} | ${OBJ_DIRS}
	${CC} ${CFLAGS} ${MAIN} ${OBJ} -o $@ ${LIBS} -Wl,-export-dynamic

# Compile src directory files
${OBJ_DIR}/%.o: ${SRC_DIR}/%.cpp | ${OBJ_DIRS}
	${CC} -c $< -o $@ ${CFLAGS}

# Compile bin/sim directory files
${OBJ_DIR}/bin_sim_%.o: ${BIN_SIM}/%.cpp | ${OBJ_DIRS}
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