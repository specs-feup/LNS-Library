CXX      = g++
DEBUG    =
OPT      =
ROOT_DIR = ../../..
LIB_DIR  = $(ROOT_DIR)/include
LNS_DIR  = $(ROOT_DIR)/include/lns
BUILD    = build

CXXFLAGS  = -std=c++17 -Wall -Werror -Isrc -Iinclude -I$(LIB_DIR)/lns
CXXFLAGS += $(DEBUG)
CXXFLAGS += $(OPT)

SRC_DIR  = src
INC_DIR  = include

SRCS = main.cpp

TARGET = $(BUILD)/bench

LNS_TABLES = $(LNS_DIR)/spline/lns_tables

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: all xf xmb test_xf test_xmb clean

all: xf

# ---------- build targets ----------

xf: $(BUILD)
	@echo "$(BLUE)Compiling bench with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF $(SRCS) -o $(TARGET) -lm
	@echo "$(GREEN)Build complete → $(TARGET) (XF)$(RESET)"

xmb: $(BUILD)
	@echo "$(BLUE)Compiling bench with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB $(SRCS) -o $(TARGET) -lm
	@echo "$(GREEN)Build complete → $(TARGET) (XMB)$(RESET)"

# ---------- test runs ----------

test_xf: xf
	@echo "$(GREEN)Running bench with XF tables...$(RESET)"
	$(TARGET) $(LNS_TABLES)/lns8_q4_3_xf.lns $(LNS_TABLES)/lns16_q8_7_xf.lns 100000

test_xmb: xmb
	@echo "$(GREEN)Running bench with XMB tables...$(RESET)"
	$(TARGET) $(LNS_TABLES)/lns8_q4_3_xmb.lns $(LNS_TABLES)/lns16_q8_7_xmb.lns 100000

# ---------- misc ----------

$(BUILD):
	mkdir -p $@

clean:
	@echo "$(BLUE)Cleaning bench...$(RESET)"
	rm -rf $(BUILD)
