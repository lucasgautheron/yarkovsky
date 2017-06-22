CC = g++
CFLAGS = -Wall -O3 -fopenmp -D GUI
EXEC_NAME = phys
INCLUDES =  -I../include/ `sdl-config --cflags`
LIBS = -lm `sdl-config --libs` -lz  -fopenmp -lSDL -lSDL_image -lz -lGL -lGLU
OBJ_FILES = main.o maths.o physics.o render.o stream.o tools.o
INSTALL_DIR = ../../bin/

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
