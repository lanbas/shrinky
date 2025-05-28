OBJS = $(wildcard src/*.cpp)
CC = g++
COMPILER_FLAGS = -w -I ./include
LINKER_FLAGS = -lSDL2 -lSDL2_ttf -lSDL2_mixer -lSDL2_gfx
OBJ_NAME = shrinky


all : $(OBJS)
	$(CC) $(OBJS) -O3 $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

debug : $(OBJS)
	$(CC) $(OBJS) -g $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)