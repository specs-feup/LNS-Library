CXX = g++
DEBUG =
OPT		=

CXX_SYSINCLUDE ?= $(shell $(CXX) -v -x c++ /dev/null 2>&1 \
    | grep '^ /' | head -1 | xargs)

ifeq ($(CXX_SYSINCLUDE),)
  CXX_SYSINCLUDE = /usr/local/include
endif

BUILD_DIR = build
SRC_DIR   = src
LIB_DIR   = lib
PREFIX   ?= /usr/local

CXXFLAGS  = -std=c++17 -Wall -Werror \
            -I./$(LIB_DIR)/lnssim \
            -I./$(LIB_DIR)/bfloatsim

CXXFLAGS += $(DEBUG)
CXXFLAGS += $(OPT)

# ---------- RISC-V toolchain detection ----------
RISCV64_TOOLCHAIN := $(shell which riscv64-unknown-elf-gcc 2>/dev/null)
RISCV32_TOOLCHAIN := $(shell which riscv32-unknown-elf-gcc 2>/dev/null)

ifeq ($(RISCV64_TOOLCHAIN),)
    ifeq ($(RISCV32_TOOLCHAIN),)
        RISCV_UNAVAILABLE := 1
    else
        RISCV_TOOLCHAIN_PREFIX := riscv32-unknown-elf
        $(info Using riscv32 toolchain)
    endif
else
    RISCV_TOOLCHAIN_PREFIX := riscv64-unknown-elf
    $(info Using riscv64 toolchain)
endif

RISCV_GCC      := $(RISCV_TOOLCHAIN_PREFIX)-gcc
RISCV_FLAGS    := -march=rv32imf_zicsr -mabi=ilp32 -nostartfiles -Ttext 0
RISCV_CXXFLAGS := -std=c++17 -Wall -Werror -I./$(LIB_DIR)/lns -DRISCV

# ---------- sources ----------
MAIN_SRC = $(SRC_DIR)/main.cpp
MAIN_OBJ = $(BUILD_DIR)/main.o
TARGET   = $(BUILD_DIR)/lns_test

TINY_LNS16_SRC  			= $(SRC_DIR)/tiny_lns16.cpp
TINY_BF16_SRC   			= $(SRC_DIR)/tiny_bf16.cpp
TINY_LNS16_RISCPP_SRC = $(SRC_DIR)/tiny_lns16_riscpp.cpp
CONV_BF16_SRC   			= $(SRC_DIR)/convert_bf16.cpp
CONV_LNS16_SRC  			= $(SRC_DIR)/convert_lns16.cpp

# ---------- headers ----------
HDR_LNS       = $(LIB_DIR)/lns/lns.hpp
HDR_LNSSIM    = $(LIB_DIR)/lnssim/lnssim.hpp
HDR_LUTS      = $(LIB_DIR)/lnssim/lnsluts.hpp
HDR_UTILS_LNS = $(LIB_DIR)/lnssim/utils.h
HDR_BFLOATSIM = $(LIB_DIR)/bfloatsim/bfloatsim.hpp
HDR_UTILS_BF  = $(LIB_DIR)/bfloatsim/utils.h

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: all xf xmb test install uninstall loc clean \
        tiny_xf tiny_xmb \
        convert_bf16 convert_lns16 \
        test_xf test_xmb

all: xf

# ---------- main lns_test ----------
xf: $(BUILD_DIR)
	@echo "$(BLUE)Compiling lns_test with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF -c $(MAIN_SRC) -o $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -DSPLINE_XF $(MAIN_OBJ) -o $(TARGET)
	@echo "$(GREEN)Build complete → $(TARGET) (XF)$(RESET)"

xmb: $(BUILD_DIR)
	@echo "$(BLUE)Compiling lns_test with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB -c $(MAIN_SRC) -o $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB $(MAIN_OBJ) -o $(TARGET)
	@echo "$(GREEN)Build complete → $(TARGET) (XMB)$(RESET)"

# ---------- tiny ----------
tiny_xf: $(BUILD_DIR)
	@echo "$(BLUE)Compiling tiny with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XF $(TINY_LNS16_SRC) -o $(BUILD_DIR)/tiny_xf -lm
	@echo "$(GREEN)Build complete → $(BUILD_DIR)/tiny_xf$(RESET)"

tiny_xmb: $(BUILD_DIR)
	@echo "$(BLUE)Compiling tiny with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XMB $(TINY_LNS16_SRC) -o $(BUILD_DIR)/tiny_xmb -lm
	@echo "$(GREEN)Build complete → $(BUILD_DIR)/tiny_xmb$(RESET)"

tiny_bf16: $(BUILD_DIR)
	@echo "$(BLUE)Compiling tiny with bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp $(TINY_BF16_SRC) -o $(BUILD_DIR)/tiny_bf16 -lm
	@echo "$(GREEN)Build complete → $(BUILD_DIR)/tiny_bf16$(RESET)"

tiny_riscpp: $(BUILD_DIR)
ifdef RISCV_UNAVAILABLE
	$(error "Neither riscv64-unknown-elf-gcc nor riscv32-unknown-elf-gcc is installed!")
endif
	@echo "$(BLUE)Compiling tiny_riscpp and generating assembly...$(RESET)"
	# Generate the assembly file (.s)
	$(RISCV_GCC) $(RISCV_FLAGS) $(RISCV_CXXFLAGS) -DSPLINE_XF -S \
		$(TINY_LNS16_RISCPP_SRC) -o $(BUILD_DIR)/tiny_riscpp.s
	# Compile the actual binary
	$(RISCV_GCC) $(RISCV_FLAGS) $(RISCV_CXXFLAGS) -DSPLINE_XF \
		$(TINY_LNS16_RISCPP_SRC) -o $(BUILD_DIR)/tiny_riscpp -lm
	@echo "$(GREEN)Build complete → $(BUILD_DIR)/tiny_riscpp and $(BUILD_DIR)/tiny_riscpp.s$(RESET)"

# ---------- converters ----------
convert_bf16: $(BUILD_DIR)
	@echo "$(BLUE)Compiling convert_bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) $(CONV_BF16_SRC) -o $(BUILD_DIR)/convert_bf16
	@echo "$(GREEN)Build complete → $(BUILD_DIR)/convert_bf16$(RESET)"

convert_lns16: $(BUILD_DIR)
	@echo "$(BLUE)Compiling convert_lns16 (XF + XMB)...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF  $(CONV_LNS16_SRC) -o $(BUILD_DIR)/convert_lns16_xf
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB $(CONV_LNS16_SRC) -o $(BUILD_DIR)/convert_lns16_xmb
	@echo "$(GREEN)Build complete → convert_lns16_xf, convert_lns16_xmb$(RESET)"

# ---------- test runs ----------
test_xf: xf
	@echo "$(GREEN)Running lns_test with XF tables...$(RESET)"
	$(TARGET) spline/lns_tables/xf_8_q4_3.lns spline/lns_tables/xf_16_q8_7.lns 100000

test_xmb: xmb
	@echo "$(GREEN)Running lns_test with XMB tables...$(RESET)"
	$(TARGET) spline/lns_tables/xmb_8_q4_3.lns spline/lns_tables/xmb_16_q8_7.lns 100000

# ---------- install / uninstall ----------
install:
	@echo "$(BLUE)Installing headers to $(CXX_SYSINCLUDE)...$(RESET)"
	install -d $(CXX_SYSINCLUDE)
	install -m 644 $(HDR_LNS)       $(CXX_SYSINCLUDE)/lns.hpp
	install -m 644 $(HDR_LNSSIM)    $(CXX_SYSINCLUDE)/lnssim.hpp
	install -m 644 $(HDR_LUTS)      $(CXX_SYSINCLUDE)/lnsluts.hpp
	install -m 644 $(HDR_UTILS_LNS) $(CXX_SYSINCLUDE)/lns_utils.h
	install -m 644 $(HDR_BFLOATSIM) $(CXX_SYSINCLUDE)/bfloatsim.hpp
	install -m 644 $(HDR_UTILS_BF)  $(CXX_SYSINCLUDE)/bfloat_utils.h
	@echo "$(GREEN)Done$(RESET)"

uninstall:
	@echo "$(BLUE)Removing installed headers from $(CXX_SYSINCLUDE)...$(RESET)"
	rm -f $(CXX_SYSINCLUDE)/lns.hpp
	rm -f $(CXX_SYSINCLUDE)/lnssim.hpp
	rm -f $(CXX_SYSINCLUDE)/lnsluts.hpp
	rm -f $(CXX_SYSINCLUDE)/lns_utils.h
	rm -f $(CXX_SYSINCLUDE)/bfloatsim.hpp
	rm -f $(CXX_SYSINCLUDE)/bfloat_utils.h
	@echo "$(GREEN)Uninstall complete$(RESET)"

# ---------- misc ----------
$(BUILD_DIR):
	mkdir -p $@

loc:
	@echo "----------------------------------------"
	@printf "%-10s | %10s\n" "Language" "Lines"
	@echo "----------------------------------------"
	@cpp_lines=$$(find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
		! -path "./$(BUILD_DIR)/*" -print0 | xargs -0 cat 2>/dev/null | wc -l); \
	printf "%-10s | %10d\n" "C/C++" $$cpp_lines; \
	for ext in c py sh asm s txt md; do \
		lines=$$(find . -type f -name "*.$$ext" ! -path "./$(BUILD_DIR)/*" -print0 \
			| xargs -0 cat 2>/dev/null | wc -l); \
		[ $$lines -gt 0 ] && printf "%-10s | %10d\n" "$$ext" "$$lines"; \
	done
	@echo "----------------------------------------"

clean:
	@echo "$(BLUE)Cleaning build directory...$(RESET)"
	rm -rf $(BUILD_DIR)
	@echo "$(GREEN)Cleanup complete$(RESET)"
