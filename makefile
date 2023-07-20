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

.PHONY: all cli test nn ppc clean makedir

all: cli test tuner nn ppc

cli: makedir
	@$(CXX) $(CXXFLAGS) "core\*.cpp" "ai\*.cpp" "cli\*.cpp" -o "bin\cli\cli.exe"

test: makedir
	@$(CXX) $(CXXFLAGS) -DTUNER "core\*.cpp" "ai\*.cpp" "test\*.cpp" -o "bin\test\test.exe"

tuner: makedir
	@$(CXX) $(CXXFLAGS) -DTUNER "core\*.cpp" "ai\*.cpp" "tuner\*.cpp" -o "bin\tuner\tuner.exe"

ppc: makedir
	@$(CXX) $(CXXFLAGS) "core\*.cpp" "ai\*.cpp" "ppc\*.cpp" $(STATIC_LIB) -o "bin\ppc\ppc.exe"

nn: makedir
	@$(CXX) $(CXXFLAGS) -DTUNER "core\*.cpp" "ai\*.cpp" "nn\*.cpp" -o "bin\nn\nn.exe"

puyop: makedir
	@$(CXX) $(CXXFLAGS) "core\*.cpp" "ai\*.cpp" "puyop\*.cpp" -o "bin\puyop\puyop.exe"

clean: makedir
	@rm -rf bin
	@make makedir

makedir:
	@mkdir -p bin
	@mkdir -p bin\cli
	@mkdir -p bin\test
	@mkdir -p bin\tuner\data
	@mkdir -p bin\ppc
	@mkdir -p bin\nn
	@mkdir -p bin\puyop

.DEFAULT_GOAL := cli