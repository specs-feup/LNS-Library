CXX = g++
DEBUG =
OPT        =

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

# ---------- RISC-V Toolchain & Utilities ----------
ifeq ($(RISCV_TOOLCHAIN_PREFIX),)
  ifeq ($(shell which riscv32-unknown-elf-gcc 2>/dev/null),)
    ifeq ($(shell which riscv64-unknown-elf-gcc 2>/dev/null),)
      RISCV_UNAVAILABLE := 1
    else
      RISCV_TOOLCHAIN_PREFIX := riscv64-unknown-elf
    endif
  else
    RISCV_TOOLCHAIN_PREFIX := riscv32-unknown-elf
  endif
endif

RISCV_GCC  := $(RISCV_TOOLCHAIN_PREFIX)-gcc
RISCV_AS   := $(RISCV_TOOLCHAIN_PREFIX)-as
RISCV_LD   := $(RISCV_TOOLCHAIN_PREFIX)-ld
RISCV_DUMP := $(RISCV_TOOLCHAIN_PREFIX)-objdump
RISCV_READ := $(RISCV_TOOLCHAIN_PREFIX)-readelf

RISCV_TEST_CXXFLAGS := -c -O0 -nostartfiles -nostdlib -ffreestanding \
                       -fno-stack-protector -fno-exceptions \
                       -fno-unwind-tables -fno-asynchronous-unwind-tables \
                       -march=rv32imf_zicsr -mabi=ilp32 \
                       -std=c++17 -Wall -Werror -I./lib/lns -DRISCV \
                       -Wno-implicit-dereference
RISCV_TEST_ASFLAGS  := -march=rv32imf_zicsr -mabi=ilp32

# Ficheiros do ecossistema Rpp
RPP_START_SRC   := tools/_start.s
RPP_LINK_SCRIPT := tools/baremetal.ld

TEST_SRCS   := $(wildcard riscpp_test/test_files/*.cpp)

TEST_OBJS   := $(patsubst riscpp_test/test_files/%.cpp, riscpp_test/test_asm/%.o, $(TEST_SRCS))
TEST_ELFS   := $(patsubst riscpp_test/test_files/%.cpp, riscpp_test/test_asm/%.elf, $(TEST_SRCS))
TEST_H_INS  := $(patsubst riscpp_test/test_files/%.cpp, riscpp_test/test_asm/%_code_bram_init.h, $(TEST_SRCS))

# ---------- sources ----------
MAIN_SRC = $(SRC_DIR)/main.cpp
MAIN_OBJ = $(BUILD_DIR)/main.o
TARGET   = $(BUILD_DIR)/lns_test

TINY_LNS16_SRC        = $(SRC_DIR)/tiny_lns16.cpp
TINY_BF16_SRC         = $(SRC_DIR)/tiny_bf16.cpp
TINY_LNS16_RISCPP_SRC = $(SRC_DIR)/tiny_lns16_riscpp.cpp
CONV_BF16_SRC         = $(SRC_DIR)/convert_bf16.cpp
CONV_LNS16_SRC        = $(SRC_DIR)/convert_lns16.cpp

# ---------- headers ----------
HDR_LNS        = $(LIB_DIR)/lns/lns.hpp
HDR_LNSSIM     = $(LIB_DIR)/lnssim/lnssim.hpp
HDR_LNSSIM_INL = $(LIB_DIR)/lnssim/lnssim.inl
HDR_LUTS       = $(LIB_DIR)/lnssim/lnsluts.hpp
HDR_UTILS_LNS  = $(LIB_DIR)/lnssim/utils.h
HDR_BFLOATSIM  = $(LIB_DIR)/bfloatsim/bfloatsim.hpp
HDR_UTILS_BF   = $(LIB_DIR)/bfloatsim/utils.h

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: all xf xmb test install uninstall loc clean \
        tiny_xf tiny_xmb \
        convert_bf16 convert_lns16 \
        test_xf test_xmb \
        build_generator compile_riscpp_tests

# Força o GNU Make a preservar todos os ficheiros gerados (.o, .elf, .text, etc.) dentro de test_asm
.SECONDARY:

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

# ---------- host-side test generator ----------
build_generator:
	@echo "$(BLUE)Compiling host-side test generator...$(RESET)"
	@mkdir -p riscpp_test/test_files
	$(CXX) $(CXXFLAGS) -DSPLINE_XF riscpp_test/tests.cpp -o riscpp_test/test_gen
	@echo "$(GREEN)Build complete → riscpp_test/test_gen$(RESET)"

# Target corrigido para apontar para a lista real de ficheiros .h gerados finais
compile_riscpp_tests: $(TEST_H_INS)
	@echo "$(GREEN)All $(words $(TEST_H_INS)) Rpp code/data memory headers successfully generated inside riscpp_test/test_asm/!$(RESET)"

# Passo 1: Montar o _start.s comum do ecossistema tools/ para a pasta test_asm
riscpp_test/test_asm/_start.o: $(RPP_START_SRC)
	@mkdir -p riscpp_test/test_asm
	@echo "$(BLUE)Assembling bootstraper → $@$(RESET)"
	$(RISCV_AS) $(RISCV_TEST_ASFLAGS) -o $@ $<

# Passo 2: Compilar o ficheiro de teste .cpp gerado para .o
riscpp_test/test_asm/%.o: riscpp_test/test_files/%.cpp
ifdef RISCV_UNAVAILABLE
	$(error "Neither riscv64-unknown-elf-gcc nor riscv32-unknown-elf-gcc is installed!")
endif
	@mkdir -p riscpp_test/test_asm
	@echo "$(BLUE)Compiling test source → $@$(RESET)"
	$(RISCV_GCC) $(RISCV_TEST_CXXFLAGS) -o $@ $<

# Passo 3: Linkar o _start.o e o %.o do teste usando o linker direto (ld) e o tools/baremetal.ld
riscpp_test/test_asm/%.elf: riscpp_test/test_asm/%.o riscpp_test/test_asm/_start.o $(RPP_LINK_SCRIPT)
	@echo "$(BLUE)Linking ELF via linkerscript → $@$(RESET)"
	$(RISCV_LD) -T$(RPP_LINK_SCRIPT) -m elf32lriscv -o $@ riscpp_test/test_asm/_start.o $<

# Passo 4: Executar o processo de dump, extração hex e geração de ficheiros .h da BRAM mapeando a pasta tools/
riscpp_test/test_asm/%_code_bram_init.h: riscpp_test/test_asm/%.elf
	@echo "$(BLUE)Processing artifacts and extracting hex layouts for $*...$(RESET)"
	$(RISCV_DUMP) -D $< > riscpp_test/test_asm/$*.disasm
	$(RISCV_DUMP) -d $< > riscpp_test/test_asm/$*.text
	$(RISCV_READ) -sW $< > riscpp_test/test_asm/$*.table
	$(RISCV_READ) -x .data -x .sdata -x .rodata $< > riscpp_test/test_asm/$*.data
	# Processamento de Instruções (Code BRAM)
	python3 tools/extract_hex.py riscpp_test/test_asm/$*.text
	@mv instructions.hex riscpp_test/test_asm/$*.instructions.hex
	python3 tools/include_gen.py riscpp_test/test_asm/$*.instructions.hex -o $@ -n $*_code_bram_init
	# Processamento de Dados (Data BRAM)
	python3 tools/extract_data.py riscpp_test/test_asm/$*.data
	@mv memory_image.hex riscpp_test/test_asm/$*.memory_image.hex
	python3 tools/include_gen.py riscpp_test/test_asm/$*.memory_image.hex -o riscpp_test/test_asm/$*_data_bram_init.h -n $*_data_bram_init
	@rm -f instructions.hex memory_image.hex

# ---------- install / uninstall ----------
install:
	@echo "$(BLUE)Installing headers to $(CXX_SYSINCLUDE)...$(RESET)"
	install -d $(CXX_SYSINCLUDE)
	install -m 644 $(HDR_LNS)        $(CXX_SYSINCLUDE)/lns.hpp
	install -m 644 $(HDR_LNSSIM)     $(CXX_SYSINCLUDE)/lnssim.hpp
	install -m 644 $(HDR_LNSSIM_INL) $(CXX_SYSINCLUDE)/lnssim.inl
	install -m 644 $(HDR_LUTS)       $(CXX_SYSINCLUDE)/lnsluts.hpp
	install -m 644 $(HDR_UTILS_LNS)  $(CXX_SYSINCLUDE)/lns_utils.h
	install -m 644 $(HDR_BFLOATSIM)  $(CXX_SYSINCLUDE)/bfloatsim.hpp
	install -m 644 $(HDR_UTILS_BF)   $(CXX_SYSINCLUDE)/bfloat_utils.h
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
	rm -rf riscpp_test/test_files
	rm -rf riscpp_test/test_asm
	@echo "$(GREEN)Cleanup complete$(RESET)"
