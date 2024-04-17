CXX = g++

ifeq ($(PROF), true)
CXXPROF += -pg -no-pie
else
CXXPROF += -s
endif

ifeq ($(BUILD), debug)
CXXFLAGS += -fdiagnostics-color=always -DUNICODE -std=c++20 -Wall -Og -pg -no-pie
else
CXXFLAGS += -DUNICODE -DNDEBUG -std=c++20 -O3 -msse4 -mbmi2 -flto $(CXXPROF) -march=native
endif

ifeq ($(PEXT), true)
CXXFLAGS += -DPEXT
endif

PPC_NAME = ppc

ifeq ($(GUI), true)
CXXFLAGS += -DGUI
PPC_NAME = ppc_gui
endif

STATIC_LIB = -lsetupapi -lhid

SRC_AI = core/*.cpp ai/*.cpp ai/search/*.cpp

SDL_DIR = C:/Users/nwint/Desktop/data/c++/lib/SDL2/x86_64-w64-mingw32
SDL_INC = -I$(SDL_DIR)/include/SDL2
SDL_LIB = -L$(SDL_DIR)/lib -lmingw32 -lSDL2main -lSDL2

A = -mwindows

GUI_DIR = C:/Users/nwint/Desktop/data/c++/lib/imgui
GUI_INC = -I$(GUI_DIR) -I$(GUI_DIR)/backends
GUI_SRC = $(GUI_DIR)/*.cpp $(GUI_DIR)/backends/imgui_impl_sdl2.cpp $(GUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

.PHONY: all puyop ppc test clean makedir

all: puyop ppc

ppc: makedir
	@$(CXX) $(CXXFLAGS) $(SRC_AI) $(SDL_INC) $(GUI_INC) $(GUI_SRC) ppc/*.cpp $(STATIC_LIB) $(SDL_LIB) -o bin/ppc/$(PPC_NAME).exe

puyop: makedir
	@$(CXX) $(CXXFLAGS) $(SRC_AI) puyop/*.cpp -o bin/puyop/puyop.exe

tuner: makedir
	@$(CXX) $(CXXFLAGS) $(SRC_AI) tuner/*.cpp -o bin/tuner/tuner.exe

test: makedir
	@$(CXX) $(CXXFLAGS) $(SRC_AI) test/*.cpp -o bin/test/test.exe

clean: makedir
	@rm -rf bin
	@make makedir

makedir:
	@mkdir -p bin
	@mkdir -p bin/puyop
	@mkdir -p bin/ppc
	@mkdir -p bin/test
	@mkdir -p bin/tuner/data

.DEFAULT_GOAL := puyop