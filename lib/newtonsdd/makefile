CXX      = g++
CXXFLAGS = --std=c++17 -Wall -Werror -I./include

SRC   = src/newtonsdd.cpp
BUILD = build
BIN   = build/newtonsdd
LNS   = lns_tables

all: $(BIN)

$(BIN): $(SRC) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

build:
	mkdir -p $(BUILD) $(LNS)

.PHONY: all clean

clean:
	rm -rf $(LNS) $(BUILD)
