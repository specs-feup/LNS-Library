CXX = g++
DEBUG =
OPT   =

ROOT_DIR = ../..
LIB_DIR  = $(ROOT_DIR)/include
LNS_DIR  = $(ROOT_DIR)/include/lns
BUILD    = build

CXXFLAGS = -std=c++17 -Wall -Werror -I$(LIB_DIR)/lns
CXXFLAGS += $(DEBUG)
CXXFLAGS += $(OPT)

SRC_DIR  = .
MAIN_SRC = $(SRC_DIR)/main.cpp
MAIN_OBJ = $(BUILD)/main.o
TARGET   = $(BUILD)/lns_test

TINY_LNS16_SRC = $(SRC_DIR)/tiny_lns16.cpp
TINY_BF16_SRC  = $(SRC_DIR)/tiny_bf16.cpp
CONV_BF16_SRC  = $(SRC_DIR)/convert_bf16.cpp
CONV_LNS16_SRC = $(SRC_DIR)/convert_lns16.cpp

LNS_TABLES = $(LNS_DIR)/spline/lns_tables

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: all xf xmb tiny_xf tiny_xmb tiny_bf16 convert_bf16 convert_lns16 \
        test_xf test_xmb clean

all: xf

# ---------- main lns_test ----------
xf: $(BUILD)
	@echo "$(BLUE)Compiling lns_test with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF -c $(MAIN_SRC) -o $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -DSPLINE_XF $(MAIN_OBJ) -o $(TARGET)
	@echo "$(GREEN)Build complete → $(TARGET) (XF)$(RESET)"

xmb: $(BUILD)
	@echo "$(BLUE)Compiling lns_test with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB -c $(MAIN_SRC) -o $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB $(MAIN_OBJ) -o $(TARGET)
	@echo "$(GREEN)Build complete → $(TARGET) (XMB)$(RESET)"

# ---------- tiny ----------
tiny_xf: $(BUILD)
	@echo "$(BLUE)Compiling tiny with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XF $(TINY_LNS16_SRC) -o $(BUILD)/tiny_xf -lm
	@echo "$(GREEN)Build complete → $(BUILD)/tiny_xf$(RESET)"

tiny_xmb: $(BUILD)
	@echo "$(BLUE)Compiling tiny with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XMB $(TINY_LNS16_SRC) -o $(BUILD)/tiny_xmb -lm
	@echo "$(GREEN)Build complete → $(BUILD)/tiny_xmb$(RESET)"

tiny_bf16: $(BUILD)
	@echo "$(BLUE)Compiling tiny with bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp $(TINY_BF16_SRC) -o $(BUILD)/tiny_bf16 -lm
	@echo "$(GREEN)Build complete → $(BUILD)/tiny_bf16$(RESET)"

# ---------- converters ----------
convert_bf16: $(BUILD)
	@echo "$(BLUE)Compiling convert_bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) $(CONV_BF16_SRC) -o $(BUILD)/convert_bf16
	@echo "$(GREEN)Build complete → $(BUILD)/convert_bf16$(RESET)"

convert_lns16: $(BUILD)
	@echo "$(BLUE)Compiling convert_lns16 (XF + XMB)...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF  $(CONV_LNS16_SRC) -o $(BUILD)/convert_lns16_xf
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB $(CONV_LNS16_SRC) -o $(BUILD)/convert_lns16_xmb
	@echo "$(GREEN)Build complete → convert_lns16_xf, convert_lns16_xmb$(RESET)"

# ---------- test runs ----------
test_xf: xf
	@echo "$(GREEN)Running lns_test with XF tables...$(RESET)"
	$(TARGET) $(LNS_TABLES)/xf_8_q4_3.lns $(LNS_TABLES)/xf_16_q8_7.lns 100000

test_xmb: xmb
	@echo "$(GREEN)Running lns_test with XMB tables...$(RESET)"
	$(TARGET) $(LNS_TABLES)/xmb_8_q4_3.lns $(LNS_TABLES)/xmb_16_q8_7.lns 100000

# ---------- misc ----------
$(BUILD):
	mkdir -p $@

clean:
	@echo "$(BLUE)Cleaning...$(RESET)"
	rm -rf $(BUILD)
