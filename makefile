CXX = g++

CXX_SYSINCLUDE ?= $(shell $(CXX) -E -x c++ -v /dev/null 2>&1 \
    | sed -n '/\#include <...>/,/End of search list./p' \
    | grep '^ ' | head -1 | xargs)

ifeq ($(CXX_SYSINCLUDE),)
  CXX_SYSINCLUDE = /usr/local/include
endif

LIB_DIR = lib

LIB_DIR = lib

HDR_LNS        = $(LIB_DIR)/lns.hpp
HDR_LNSSIM     = $(LIB_DIR)/lnssim.hpp
HDR_LNSSIM_INL = $(LIB_DIR)/lnssim.inl
HDR_LUTS       = $(LIB_DIR)/lnsluts.hpp
HDR_UTILS      = $(LIB_DIR)/utils.h
HDR_BFLOATSIM  = $(LIB_DIR)/bfloatsim.hpp

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: all examples tests clean install uninstall loc

all: examples tests

examples:
	@echo "$(BLUE)Building examples...$(RESET)"
	$(MAKE) -C examples/bench
	$(MAKE) -C examples/tinystories

tests:
	@echo "$(BLUE)Building tests...$(RESET)"
	$(MAKE) -C tests

clean:
	@echo "$(BLUE)Cleaning all...$(RESET)"
	$(MAKE) -C examples/bench
	$(MAKE) -C examples/tinystories clean
	$(MAKE) -C tests clean
	@echo "$(GREEN)Done$(RESET)"

# ---------- install / uninstall ----------
install:
	@echo "$(BLUE)Installing headers to $(CXX_SYSINCLUDE)...$(RESET)"
	install -d $(CXX_SYSINCLUDE)
	install -m 644 $(HDR_LNS)        $(CXX_SYSINCLUDE)/lns.hpp
	install -m 644 $(HDR_LNSSIM)     $(CXX_SYSINCLUDE)/lnssim.hpp
	install -m 644 $(HDR_LNSSIM_INL) $(CXX_SYSINCLUDE)/lnssim.inl
	install -m 644 $(HDR_LUTS)       $(CXX_SYSINCLUDE)/lnsluts.hpp
	install -m 644 $(HDR_UTILS)      $(CXX_SYSINCLUDE)/lns_utils.h
	install -m 644 $(HDR_BFLOATSIM)  $(CXX_SYSINCLUDE)/bfloatsim.hpp
	@echo "$(GREEN)Done$(RESET)"

uninstall:
	@echo "$(BLUE)Removing installed headers from $(CXX_SYSINCLUDE)...$(RESET)"
	rm -f $(CXX_SYSINCLUDE)/lns.hpp
	rm -f $(CXX_SYSINCLUDE)/lnssim.hpp
	rm -f $(CXX_SYSINCLUDE)/lnssim.inl
	rm -f $(CXX_SYSINCLUDE)/lnsluts.hpp
	rm -f $(CXX_SYSINCLUDE)/lns_utils.h
	rm -f $(CXX_SYSINCLUDE)/bfloatsim.hpp
	@echo "$(GREEN)Uninstall complete$(RESET)"

# ---------- misc ----------
loc:
	@echo "----------------------------------------"
	@printf "%-10s | %10s\n" "Language" "Lines"
	@echo "----------------------------------------"
	@cpp_lines=$$(find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
		! -path "./examples/*/build/*" ! -path "./tests/riscpp/build_artifacts/*" \
		-print0 | xargs -0 cat 2>/dev/null | wc -l); \
	printf "%-10s | %10d\n" "C/C++" $$cpp_lines; \
	for ext in c py sh asm s txt md; do \
		lines=$$(find . -type f -name "*.$$ext" \
			! -path "./examples/*/build/*" ! -path "./tests/riscpp/build_artifacts/*" \
			-print0 | xargs -0 cat 2>/dev/null | wc -l); \
		[ $$lines -gt 0 ] && printf "%-10s | %10d\n" "$$ext" "$$lines"; \
	done
	@echo "----------------------------------------"
