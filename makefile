CXX = g++

CXX_SYSINCLUDE ?= $(shell $(CXX) -v -x c++ /dev/null 2>&1 \
    | grep '^ /' | head -1 | xargs)
ifeq ($(CXX_SYSINCLUDE),)
  CXX_SYSINCLUDE = /usr/local/include
endif

BUILD_DIR = build
PREFIX   ?= /usr/local
CXXFLAGS  = -std=c++17 -Wall -Werror -g -O2 -I./lib/sim

MAIN_SRC = src/main.cpp
MAIN_OBJ = $(BUILD_DIR)/main.o
TARGET   = $(BUILD_DIR)/lns_test

HDR_LNS    = lib/lns/lns.hpp
HDR_LNSSIM = lib/sim/lns_sim.hpp
HDR_LUTS   = lib/sim/lns_luts.hpp
HDR_UTILS  = lib/sim/utils.h

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: all test install uninstall loc clean

all: $(BUILD_DIR) $(TARGET)

test: all
	@echo "$(BLUE)================= Running tests =================$(RESET)"
	@total=0; passed=0; failed=0; \
	for f in test/*; do \
		[ -f "$$f" ] || continue; \
		total=$$((total+1)); \
		name=$$(basename "$$f"); \
		num=$$(echo "$$name" | sed 's/[^0-9]//g'); \
		expected="test/true_test$$num.bin"; \
		output="test/test$$num.bin"; \
		printf "$(BLUE)Test %s: $(RESET)" "$$name"; \
		$(TARGET) "$$f" > "test/test$$num.log" 2>&1; \
		status=$$?; \
		if [ -f "$$expected" ]; then \
			if diff "$$output" "$$expected" > /dev/null 2>&1; then \
				echo "$(GREEN)PASSED$(RESET)"; passed=$$((passed+1)); \
			else \
				echo "$(RED)FAILED (diff mismatch)$(RESET)"; failed=$$((failed+1)); \
			fi; \
		else \
			if [ $$status -eq 0 ]; then \
				echo "$(GREEN)PASSED$(RESET)"; passed=$$((passed+1)); \
			else \
				echo "$(RED)FAILED$(RESET)"; failed=$$((failed+1)); \
			fi; \
		fi; \
	done; \
	echo "$(BLUE)=================================================$(RESET)"; \
	echo "$(GREEN)PASSED $$passed/$$total tests$(RESET)"; \
	if [ $$failed -ne 0 ]; then \
		echo "$(RED)$$failed tests failed$(RESET)"; exit 1; \
	else \
		echo "$(GREEN)All tests passed ✔$(RESET)"; \
	fi

install:
	@echo "$(BLUE)Installing headers to $(CXX_SYSINCLUDE)...$(RESET)"
	install -d $(CXX_SYSINCLUDE)
	install -m 644 $(HDR_LNS)    $(CXX_SYSINCLUDE)/lns
	install -m 644 $(HDR_LNSSIM) $(CXX_SYSINCLUDE)/lnssim
	install -m 644 $(HDR_LUTS)   $(CXX_SYSINCLUDE)/lns_luts.hpp
	install -m 644 $(HDR_UTILS)  $(CXX_SYSINCLUDE)/lns_utils.h
	@echo "$(GREEN)Done — you can now use #include <lns> and #include <lnssim>$(RESET)"

uninstall:
	@echo "$(BLUE)Removing installed headers from $(CXX_SYSINCLUDE)...$(RESET)"
	rm -f $(CXX_SYSINCLUDE)/lns
	rm -f $(CXX_SYSINCLUDE)/lnssim
	rm -f $(CXX_SYSINCLUDE)/lns_luts.hpp
	rm -f $(CXX_SYSINCLUDE)/lns_utils.h
	@echo "$(GREEN)Uninstall complete$(RESET)"

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

$(BUILD_DIR):
	mkdir -p $@

$(MAIN_OBJ): $(MAIN_SRC) $(HDR_LNS) $(HDR_LNSSIM) $(HDR_LUTS) $(HDR_UTILS) | $(BUILD_DIR)
	@echo "$(BLUE)Compiling $(MAIN_SRC)...$(RESET)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(MAIN_OBJ)
	@echo "$(BLUE)Linking $(TARGET)...$(RESET)"
	$(CXX) $(CXXFLAGS) $< -o $@
	@echo "$(GREEN)Build complete → $(TARGET)$(RESET)"

test_xf: $(BUILD_DIR)
	@echo "$(BLUE)Compiling with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF $(MAIN_SRC) -o $(TARGET)
	@echo "$(GREEN)Running lns_test with XF tables...$(RESET)"
	$(TARGET) spline/lns_tables/xf_8_q4_3.lns spline/lns_tables/xf_16_q8_7.lns 100000

test_xmb: $(BUILD_DIR)
	@echo "$(BLUE)Compiling with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB $(MAIN_SRC) -o $(TARGET)
	@echo "$(GREEN)Running lns_test with XMB tables...$(RESET)"
	$(TARGET) spline/lns_tables/xmb_8_q4_3.lns spline/lns_tables/xmb_16_q8_7.lns 100000

clean:
	@echo "$(BLUE)Cleaning build directory...$(RESET)"
	rm -rf $(BUILD_DIR)
	@echo "$(GREEN)Cleanup complete$(RESET)"
