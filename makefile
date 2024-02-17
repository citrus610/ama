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

STATIC_LIB = -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -lsetupapi -lhid -static

SRC_AI = core/*.cpp ai/*.cpp ai/search/*.cpp

SRC_PPC = ppc/engine/*.cpp ppc/gui/*.cpp ppc/*.cpp

.PHONY: all puyop ppc test clean makedir

all: puyop ppc

ppc: makedir
	@$(CXX) $(CXXFLAGS) $(SRC_AI) ppc/engine/*.cpp ppc/*.cpp $(STATIC_LIB) -o bin/ppc/ppc.exe

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