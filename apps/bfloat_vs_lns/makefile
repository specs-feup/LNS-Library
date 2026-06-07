CXX = g++
DEBUG =
OPT   =

ROOT_DIR      = ../..
LIB_DIR       = $(ROOT_DIR)/include
LNS_DIR       = $(ROOT_DIR)/include/lns
BUILD         = build
BUILD_CONVERT = build/convert
BUILD_TINY    = build/tiny

CXXFLAGS  = -std=c++17 -Wall -Werror -I$(LIB_DIR)/lns
CXXFLAGS += $(DEBUG)
CXXFLAGS += $(OPT)

SRC_DIR        = .
TINY_LNS16_SRC = $(SRC_DIR)/tiny/tiny_lns16.cpp
TINY_BF16_SRC  = $(SRC_DIR)/tiny/tiny_bf16.cpp
CONV_BF16_SRC  = $(SRC_DIR)/convert/convert_bf16.cpp
CONV_LNS16_SRC = $(SRC_DIR)/convert/convert_lns16.cpp

LNS_TABLES = $(LNS_DIR)/spline/lns_tables

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: tiny_xf tiny_xmb tiny_bf16 convert_bf16 convert_lns16 bench clean

# ---------- tiny ----------
tiny_xf: $(BUILD_TINY) $(BUILD)
	@echo "$(BLUE)Compiling tiny with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XF $(TINY_LNS16_SRC) -o $(BUILD)/tiny/tiny_xf -lm
	@echo "$(GREEN)Build complete → $(BUILD)/tiny/tiny_xf$(RESET)"

tiny_xmb: $(BUILD_TINY) $(BUILD)
	@echo "$(BLUE)Compiling tiny with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XMB $(TINY_LNS16_SRC) -o $(BUILD)/tiny/tiny_xmb -lm
	@echo "$(GREEN)Build complete → $(BUILD)/tiny/tiny_xmb$(RESET)"

tiny_bf16: $(BUILD_TINY) $(BUILD)
	@echo "$(BLUE)Compiling tiny with bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp $(TINY_BF16_SRC) -o $(BUILD)/tiny/tiny_bf16 -lm
	@echo "$(GREEN)Build complete → $(BUILD)/tiny/tiny_bf16$(RESET)"

# ---------- converters ----------
convert_bf16: $(BUILD_CONVERT) $(BUILD)
	@echo "$(BLUE)Compiling convert_bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) $(CONV_BF16_SRC) -o $(BUILD)/convert/convert_bf16
	@echo "$(GREEN)Build complete → $(BUILD)/convert/convert_bf16$(RESET)"

convert_lns16: $(BUILD_CONVERT) $(BUILD)
	@echo "$(BLUE)Compiling convert_lns16 (XF)...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF  $(CONV_LNS16_SRC) -o $(BUILD)/convert/convert_lns16_xf
	@echo "$(GREEN)Build complete → convert/convert_lns16_xf $(RESET)"

# ---------- bench ----------
bench:
	@echo "$(BLUE)Delegating to bench/makefile...$(RESET)"
	$(MAKE) -C bench $(filter-out $@, $(MAKECMDGOALS))

# ---------- misc ----------
$(BUILD):
	mkdir -p $@

$(BUILD_CONVERT):
	mkdir -p $@

$(BUILD_TINY):
	mkdir -p $@

clean:
	@echo "$(BLUE)Cleaning...$(RESET)"
	rm -rf $(BUILD)
