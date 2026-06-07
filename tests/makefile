CXX = g++
DEBUG =
OPT   =

ROOT_DIR = ..
LIB_DIR  = $(ROOT_DIR)/include

CXXFLAGS = -std=c++17 -Wall -Werror -I$(LIB_DIR)/lns
CXXFLAGS += $(DEBUG)
CXXFLAGS += $(OPT)

# ---------- RISC-V Toolchain ----------
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
                       -std=c++17 -Wall -Werror -I$(LIB_DIR)/lns -DRISCV \
                       -Wno-implicit-dereference
RISCV_TEST_ASFLAGS  := -march=rv32imf_zicsr -mabi=ilp32

RPP_START_SRC   := riscpp/linker/_start.s
RPP_LINK_SCRIPT := riscpp/linker/baremetal.ld

TEST_SRCS  := $(wildcard test_files/*.cpp)
TEST_OBJS  := $(patsubst test_files/%.cpp, riscpp/build_artifacts/%.o,   $(TEST_SRCS))
TEST_ELFS  := $(patsubst test_files/%.cpp, riscpp/build_artifacts/%.elf,  $(TEST_SRCS))
TEST_H_INS := $(patsubst test_files/%.cpp, riscpp/build_artifacts/%_code_bram_init.h, $(TEST_SRCS))

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: all build_generator compile_riscpp_tests clean
.SECONDARY:

all: compile_riscpp_tests

# ---------- host-side test generator ----------
build_generator:
	@echo "$(BLUE)Compiling host-side test generator...$(RESET)"
	@mkdir -p test_files
	$(CXX) $(CXXFLAGS) -DSPLINE_XF tests.cpp -o riscpp/test_gen
	@echo "$(GREEN)Build complete → riscpp/test_gen$(RESET)"

# ---------- RISC-V pipeline ----------
compile_riscpp_tests: $(TEST_H_INS)
	@echo "$(GREEN)All $(words $(TEST_H_INS)) Rpp code/data memory headers successfully generated inside riscpp/build_artifacts/!$(RESET)"

# Passo 1: Montar o _start.s
riscpp/build_artifacts/_start.o: $(RPP_START_SRC)
	@mkdir -p riscpp/build_artifacts
	@echo "$(BLUE)Assembling bootstraper → $@$(RESET)"
	$(RISCV_AS) $(RISCV_TEST_ASFLAGS) -o $@ $<

# Passo 2: Compilar o ficheiro de teste .cpp para .o
riscpp/build_artifacts/%.o: test_files/%.cpp
ifdef RISCV_UNAVAILABLE
	$(error "Neither riscv64-unknown-elf-gcc nor riscv32-unknown-elf-gcc is installed!")
endif
	@mkdir -p riscpp/build_artifacts
	@echo "$(BLUE)Compiling test source → $@$(RESET)"
	$(RISCV_GCC) $(RISCV_TEST_CXXFLAGS) -o $@ $<

# Passo 3: Linkar
riscpp/build_artifacts/%.elf: riscpp/build_artifacts/%.o riscpp/build_artifacts/_start.o $(RPP_LINK_SCRIPT)
	@echo "$(BLUE)Linking ELF via linkerscript → $@$(RESET)"
	$(RISCV_LD) -T$(RPP_LINK_SCRIPT) -m elf32lriscv -o $@ riscpp/build_artifacts/_start.o $<

# Passo 4: Dump, extração hex e geração de .h da BRAM
riscpp/build_artifacts/%_code_bram_init.h: riscpp/build_artifacts/%.elf
	@echo "$(BLUE)Processing artifacts and extracting hex layouts for $*...$(RESET)"
	$(RISCV_DUMP) -D $< > riscpp/build_artifacts/$*.disasm
	$(RISCV_DUMP) -d $< > riscpp/build_artifacts/$*.text
	$(RISCV_READ) -sW $< > riscpp/build_artifacts/$*.table
	$(RISCV_READ) -x .data -x .sdata -x .rodata $< > riscpp/build_artifacts/$*.data
	# Processamento de Instruções (Code BRAM)
	python3 riscpp/scripts/extract_hex.py riscpp/build_artifacts/$*.text
	@mv instructions.hex riscpp/build_artifacts/$*.instructions.hex
	python3 riscpp/scripts/include_gen.py riscpp/build_artifacts/$*.instructions.hex -o $@ -n $*_code_bram_init
	# Processamento de Dados (Data BRAM)
	python3 riscpp/scripts/extract_data.py riscpp/build_artifacts/$*.data
	@mv memory_image.hex riscpp/build_artifacts/$*.memory_image.hex
	python3 riscpp/scripts/include_gen.py riscpp/build_artifacts/$*.memory_image.hex -o riscpp/build_artifacts/$*_data_bram_init.h -n $*_data_bram_init
	@rm -f instructions.hex memory_image.hex

clean:
	@echo "$(BLUE)Cleaning...$(RESET)"
	rm -rf test_files riscpp/build_artifacts
	@echo "$(GREEN)Done$(RESET)"
