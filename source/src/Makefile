CC = g++

CFLAGS = -Wall -O0 -g -fopenmp
EXEC_NAME = phys
INCLUDES =  -I../include/ `sdl-config --cflags`
LIBS = -lm `sdl-config --libs` -lz  -fopenmp -lSDL -lSDL_image -lz 
OBJ_FILES = main.o maths.o physics.o render.o stream.o tools.o
INSTALL_DIR = ../../bin/

ifndef GUI
	GUI = 0
endif

ifeq ($(GUI), 1)
	override CFLAGS+= -D GUI
    override LIBS+= -lGL -lGLU
endif

all: $(EXEC_NAME)

clean:
	rm $(EXEC_NAME) $(OBJ_FILES)

$(EXEC_NAME): $(OBJ_FILES)
	$(CC) -o $(EXEC_NAME) $(OBJ_FILES) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

%.o: %.cc
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

%.o: %.c
	gcc $(CFLAGS) $(INCLUDES) -o $@ -c $<

install:
	cp $(EXEC_NAME) $(INSTALL_DIR)
