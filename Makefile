CC = g++
OBJS = src/main.cpp
DISASSEMBLER_OBJS = src/disassembler.cpp
LINKER_FLAGS = -lSDL2
OBJ_NAME = chip8
DISASSEMBLER_OBJ_NAME = disassembler

all : $(OBJS)
	$(CC) -g $(OBJS) $(LINKER_FLAGS) -o $(OBJ_NAME)

disassembler : $(DISASSEMBLER_OBJS)
	$(CC) -g $(DISASSEMBLER_OBJS) -o $(DISASSEMBLER_OBJ_NAME)