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

clean:
	@echo "$(BLUE)Cleaning all...$(RESET)"
	$(MAKE) -C examples/bench clean
	$(MAKE) -C examples/tinystories clean
	@echo "$(GREEN)Done$(RESET)"

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
