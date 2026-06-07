CXX      = g++
CXXFLAGS = --std=c++17 -Wall -Werror -I./include

SRC   = src/main.cpp src/spline.cpp
BUILD = build
BIN   = build/spline
LNS   = lns_tables

all: $(BIN)

$(BIN): $(SRC) | $(BUILD)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

build:
	mkdir -p $(BUILD) $(LNS)

.PHONY: all clean test tables tables2

test: $(BIN) | $(LNS)
	$(BIN) --test --xf  128 --lns8  4
	$(BIN) --test --xf  128 --lns16 8
	$(BIN) --test --xmb 128 --lns8  4
	$(BIN) --test --xmb 128 --lns16 8

tables: $(BIN) | $(LNS)
	$(BIN) --gen --xf  40 34 4   5   --lns8  4
	$(BIN) --gen --xf  10 50 64  65  --lns16 8
	$(BIN) --gen --xmb 41 40 64  65  --lns8  4
	$(BIN) --gen --xmb 12 42 115 111 --lns16 8

tables2: $(BIN) | $(LNS)
	$(BIN) --gen --xf  40 34  4   5   --lns8  4
	$(BIN) --gen --xf  40 100 64  65  --lns16 8
	$(BIN) --gen --xmb 41 40  64  65  --lns8  4
	$(BIN) --gen --xmb 40 100 115 111 --lns16 8

clean:
	rm -rf $(LNS) $(BUILD)
