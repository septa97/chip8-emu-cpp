CC = g++
OBJS = main.cpp
LINKER_FLAGS = -lSDL2
OBJ_NAME = chip8

all : $(OBJS)
	$(CC) $(OBJS) $(LINKER_FLAGS) -o $(OBJ_NAME)
