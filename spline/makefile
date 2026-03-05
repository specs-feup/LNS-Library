CXX      = g++
CXXFLAGS = --std=c++17 -Wall -Werror -I./include

SRC 	= src/spline.cpp
BUILD = build
BIN 	= build/spline
LNS 	= lns_tables

all: $(BIN)

$(BIN): $(SRC) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

build:
	mkdir -p $(BUILD) $(LNS)

.PHONY: all clean

tables: $(BIN) | $(LNS)
	$(BIN) --gen --xf  40 34 --lns8  4
	$(BIN) --gen --xf  10 50 --lns16 8
	$(BIN) --gen --xmb 41 40 --lns8  4
	$(BIN) --gen --xmb 12 42 --lns16 8

clean:
	rm -rf $(LNS) $(BUILD)
