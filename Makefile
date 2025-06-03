CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra
LDFLAGS = -lSDL2
DEBUG_FLAGS = -g -O0 -DDEBUG

SOURCES = main.cpp Game.cpp Entity.cpp Player.cpp Boss.cpp InputHandler.cpp Renderer.cpp Timer.cpp Sif.cpp
OBJECTS = $(addprefix build/, $(SOURCES:.cpp=.o))
EXECUTABLE = boss_fight

all: build/$(EXECUTABLE)

build/$(EXECUTABLE): $(OBJECTS) | build
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

build/%.o: %.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

debug: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(SOURCES) -o build/$(EXECUTABLE)_debug $(LDFLAGS)

clean:
	rm -f $(OBJECTS) build/$(EXECUTABLE) build/$(EXECUTABLE)_debug

run:
	./build/$(EXECUTABLE)

.PHONY: all clean run debug
