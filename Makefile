# Makefile
CC = g++
CFLAGS = -std=c++14 -Wall
LDFLAGS = -lSDL2

SOURCES = main.cpp Player.cpp Bullet.cpp Enemy.cpp Wave.cpp Upgrade.cpp GameState.cpp Background.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = shooter_game

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)