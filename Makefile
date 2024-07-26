OBJS = $(wildcard src/*.cpp)
CC = g++
COMPILER_FLAGS = -O3 -w -I ./include
LINKER_FLAGS = -lSDL2 -lSDL2_ttf -lSDL2_mixer
OBJ_NAME = shrinky


all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)