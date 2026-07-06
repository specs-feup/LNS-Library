CXX = g++

CXX_SYSINCLUDE ?= $(shell $(CXX) -E -x c++ -v /dev/null 2>&1 \
    | sed -n '/\#include <...>/,/End of search list./p' \
    | grep '^ ' | head -1 | xargs)

ifeq ($(CXX_SYSINCLUDE),)
  CXX_SYSINCLUDE = /usr/local/include
endif

LIB_DIR = lib

LIB_DIR = lib

HDR_LNS        		= $(LIB_DIR)/lns.hpp
HDR_LNS_INL       = $(LIB_DIR)/lns.inl
HDR_LNS_F_EXT_INL = $(LIB_DIR)/lns_f_ext.inl
HDR_LNSSIM     		= $(LIB_DIR)/lnssim.hpp
HDR_LNSSIM_INL 		= $(LIB_DIR)/lnssim.inl
HDR_LUTS       		= $(LIB_DIR)/lnsluts.hpp
HDR_UTILS      		= $(LIB_DIR)/utils.h
HDR_BFLOATSIM  		= $(LIB_DIR)/bfloatsim.hpp
HDR_BFLOATSIM_INL = $(LIB_DIR)/bfloatsim.inl

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
	install -m 644 $(HDR_LNS)        		$(CXX_SYSINCLUDE)/lns
	install -m 644 $(HDR_LNS_INL)       $(CXX_SYSINCLUDE)/lns.inl
	install -m 644 $(HDR_LNS_F_EXT_INL) $(CXX_SYSINCLUDE)/lns_f_ext.inl
	install -m 644 $(HDR_LNSSIM)     		$(CXX_SYSINCLUDE)/lnssim
	install -m 644 $(HDR_LNSSIM_INL) 		$(CXX_SYSINCLUDE)/lnssim.inl
	install -m 644 $(HDR_LUTS)       		$(CXX_SYSINCLUDE)/lnsluts
	install -m 644 $(HDR_UTILS)      		$(CXX_SYSINCLUDE)/lnsutils
	install -m 644 $(HDR_BFLOATSIM)  		$(CXX_SYSINCLUDE)/bfloatsim
	install -m 644 $(HDR_BFLOATSIM_INL) $(CXX_SYSINCLUDE)/bfloatsim.inl
	@echo "$(GREEN)Done$(RESET)"

uninstall:
	@echo "$(BLUE)Removing installed headers from $(CXX_SYSINCLUDE)...$(RESET)"
	rm -f $(CXX_SYSINCLUDE)/lns
	rm -f $(CXX_SYSINCLUDE)/lns.inl
	rm -f $(CXX_SYSINCLUDE)/lns_f_ext.inl
	rm -f $(CXX_SYSINCLUDE)/lnssim
	rm -f $(CXX_SYSINCLUDE)/lnssim.inl
	rm -f $(CXX_SYSINCLUDE)/lnsluts
	rm -f $(CXX_SYSINCLUDE)/lnsutils
	rm -f $(CXX_SYSINCLUDE)/bfloatsim
	rm -f $(CXX_SYSINCLUDE)/bfloatsim.inl
	@echo "$(GREEN)Uninstall complete$(RESET)"
