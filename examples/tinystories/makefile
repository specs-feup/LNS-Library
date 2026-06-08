CXX = g++
DEBUG =
OPT   =

ROOT_DIR      = ../..
LNS_DIR       = $(ROOT_DIR)/lib
BUILD         = build
BUILD_CONVERT = build/convert
BUILD_TINY    = build/tiny

CXXFLAGS  = -std=c++17 -Wall -Werror -I$(LNS_DIR)
CXXFLAGS += $(DEBUG)
CXXFLAGS += $(OPT)

SRC_DIR        = .
TINY_LNS16_SRC = $(SRC_DIR)/tiny/tiny_lns16.cpp
TINY_BF16_SRC  = $(SRC_DIR)/tiny/tiny_bf16.cpp
CONV_BF16_SRC  = $(SRC_DIR)/convert/convert_bf16.cpp
CONV_LNS16_SRC = $(SRC_DIR)/convert/convert_lns16.cpp

LNS_TABLES = $(LNS_DIR)/spline/lns_tables

MODELS_DIR    = models
MODELS_LNS    = $(MODELS_DIR)/lns
MODELS_BF16   = $(MODELS_DIR)/bf16

# Model / tokenizer paths
MODEL_15M         		 = $(MODELS_DIR)/stories15M.bin
MODEL_42M         		 = $(MODELS_DIR)/stories42M.bin
MODEL_260K        		 = $(MODELS_DIR)/stories260K.bin
TOKENIZER         		 = $(MODELS_DIR)/tokenizer.bin
TOKENIZER_260K    		 = $(MODELS_DIR)/tokenizer260K.bin

MODEL_42M_LNS_XF   		 = $(MODELS_LNS)/stories15M_lns16_xf.bin
MODEL_15M_LNS_XF   		 = $(MODELS_LNS)/stories15M_lns16_xf.bin
MODEL_260K_LNS_XF  		 = $(MODELS_LNS)/stories260K_lns16_xf.bin
TOKENIZER_LNS_XF   		 = $(MODELS_LNS)/tokenizer_lns16_xf.bin
TOKENIZER_260K_LNS_XF  = $(MODELS_LNS)/tokenizer260K_lns16_xf.bin

MODEL_42M_LNS_XMB  		 = $(MODELS_LNS)/stories15M_lns16_xmb.bin
MODEL_15M_LNS_XMB  		 = $(MODELS_LNS)/stories15M_lns16_xmb.bin
MODEL_260K_LNS_XMB 		 = $(MODELS_LNS)/stories260K_lns16_xmb.bin
TOKENIZER_LNS_XMB  		 = $(MODELS_LNS)/tokenizer_lns16_xmb.bin
TOKENIZER_260K_LNS_XMB = $(MODELS_LNS)/tokenizer260K_lns16_xmb.bin

MODEL_42M_BF16       	 = $(MODELS_BF16)/stories15M_bf16.bin
MODEL_15M_BF16       	 = $(MODELS_BF16)/stories15M_bf16.bin
MODEL_260K_BF16      	 = $(MODELS_BF16)/stories260K_bf16.bin
TOKENIZER_BF16       	 = $(MODELS_BF16)/tokenizer_bf16.bin
TOKENIZER_260K_BF16  	 = $(MODELS_BF16)/tokenizer260K_bf16.bin

VOCAB_SIZE      = 32000
VOCAB_SIZE_260K = 512

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: all tiny_xf tiny_xmb tiny_bf16 \
        convert_bf16 convert_lns16_xf convert_lns16_xmb convert_lns16 \
        models models_dir \
        convert_models convert_models_lns convert_models_bf16 \
        convert_tokenizers convert_tokenizers_lns convert_tokenizers_bf16 \
        run_xf_15M run_xf_260K \
        run_xmb_15M run_xmb_260K \
        run_bf16_15M run_bf16_260K \
        clean

# ---------- default ----------
all: tiny_xf tiny_xmb tiny_bf16 convert_lns16 convert_bf16

# ---------- tiny ----------
tiny_xf: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XF $(TINY_LNS16_SRC) -o $(BUILD_TINY)/tiny_xf -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_xf$(RESET)"

tiny_xmb: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XMB $(TINY_LNS16_SRC) -o $(BUILD_TINY)/tiny_xmb -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_xmb$(RESET)"

tiny_bf16: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp $(TINY_BF16_SRC) -o $(BUILD_TINY)/tiny_bf16 -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_bf16$(RESET)"

# ---------- converters ----------
convert_lns16_xf: $(BUILD_CONVERT)
	@echo "$(BLUE)Compiling convert_lns16 (XF)...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF $(CONV_LNS16_SRC) -o $(BUILD_CONVERT)/convert_lns16_xf
	@echo "$(GREEN)Build complete → $(BUILD_CONVERT)/convert_lns16_xf$(RESET)"

convert_lns16_xmb: $(BUILD_CONVERT)
	@echo "$(BLUE)Compiling convert_lns16 (XMB)...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB $(CONV_LNS16_SRC) -o $(BUILD_CONVERT)/convert_lns16_xmb
	@echo "$(GREEN)Build complete → $(BUILD_CONVERT)/convert_lns16_xmb$(RESET)"

convert_lns16: convert_lns16_xf convert_lns16_xmb

convert_bf16: $(BUILD_CONVERT)
	@echo "$(BLUE)Compiling convert_bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) $(CONV_BF16_SRC) -o $(BUILD_CONVERT)/convert_bf16
	@echo "$(GREEN)Build complete → $(BUILD_CONVERT)/convert_bf16$(RESET)"

# ---------- model download ----------
models_dir:
	mkdir -p $(MODELS_DIR) $(MODELS_LNS) $(MODELS_BF16)

models: models_dir
	@echo "$(BLUE)Downloading float32 model weights...$(RESET)"
	wget -nc -P $(MODELS_DIR) https://huggingface.co/karpathy/tinyllamas/resolve/main/stories15M.bin
	wget -nc -P $(MODELS_DIR) https://huggingface.co/karpathy/tinyllamas/resolve/main/stories42M.bin
	wget -nc -P $(MODELS_DIR) https://huggingface.co/karpathy/tinyllamas/resolve/main/stories260K/stories260K.bin
	wget -nc -P $(MODELS_DIR) https://github.com/karpathy/llama2.c/raw/master/tokenizer.bin
	test -f $(MODELS_DIR)/tokenizer260K.bin || wget -O $(MODELS_DIR)/tokenizer260K.bin https://huggingface.co/karpathy/tinyllamas/resolve/main/stories260K/tok512.bin
	@echo "$(GREEN)Models downloaded → $(MODELS_DIR)/$(RESET)"

# ---------- model conversion ----------
convert_models_lns_xf: convert_lns16_xf $(MODELS_LNS)
	@echo "$(BLUE)Converting models to lns16 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns model $(MODEL_42M)  $(MODEL_42M_LNS_XF)
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns model $(MODEL_15M)  $(MODEL_15M_LNS_XF)
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns model $(MODEL_260K) $(MODEL_260K_LNS_XF)
	@echo "$(GREEN)LNS16 models ready → $(MODELS_LNS)/$(RESET)"

convert_models_lns_xmb: convert_lns16_xmb $(MODELS_LNS)
	@echo "$(BLUE)Converting models to lns16 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns model $(MODEL_42M)  $(MODEL_42M_LNS_XMB)
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns model $(MODEL_15M)  $(MODEL_15M_LNS_XMB)
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns model $(MODEL_260K) $(MODEL_260K_LNS_XMB)
	@echo "$(GREEN)LNS16 models ready → $(MODELS_LNS)/$(RESET)"

convert_models_bf16: convert_bf16 $(MODELS_BF16)
	@echo "$(BLUE)Converting models to bf16...$(RESET)"
	$(BUILD_CONVERT)/convert_bf16 model $(MODEL_42M)  $(MODEL_42M_BF16)
	$(BUILD_CONVERT)/convert_bf16 model $(MODEL_15M)  $(MODEL_15M_BF16)
	$(BUILD_CONVERT)/convert_bf16 model $(MODEL_260K) $(MODEL_260K_BF16)
	@echo "$(GREEN)BF16 models ready → $(MODELS_BF16)/$(RESET)"

convert_models: convert_models_lns_xf convert_models_lns_xmb convert_models_bf16

# ---------- tokenizer conversion ----------
convert_tokenizers_lns_xf: convert_lns16_xf $(MODELS_LNS)
	@echo "$(BLUE)Converting tokenizers to lns16 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns tokenizer $(TOKENIZER)      $(VOCAB_SIZE)      $(TOKENIZER_LNS_XF)
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns tokenizer $(TOKENIZER_260K) $(VOCAB_SIZE_260K) $(TOKENIZER_260K_LNS_XF)
	@echo "$(GREEN)LNS16 tokenizers ready → $(MODELS_LNS)/$(RESET)"

convert_tokenizers_lns_xmb: convert_lns16_xmb $(MODELS_LNS)
	@echo "$(BLUE)Converting tokenizers to lns16 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns tokenizer $(TOKENIZER)      $(VOCAB_SIZE)      $(TOKENIZER_LNS_XMB)
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns tokenizer $(TOKENIZER_260K) $(VOCAB_SIZE_260K) $(TOKENIZER_260K_LNS_XMB)
	@echo "$(GREEN)LNS16 tokenizers ready → $(MODELS_LNS)/$(RESET)"

convert_tokenizers_bf16: convert_bf16 $(MODELS_BF16)
	@echo "$(BLUE)Converting tokenizers to bf16...$(RESET)"
	$(BUILD_CONVERT)/convert_bf16 tokenizer $(TOKENIZER)      $(VOCAB_SIZE)      $(TOKENIZER_BF16)
	$(BUILD_CONVERT)/convert_bf16 tokenizer $(TOKENIZER_260K) $(VOCAB_SIZE_260K) $(TOKENIZER_260K_BF16)
	@echo "$(GREEN)BF16 tokenizers ready → $(MODELS_BF16)/$(RESET)"

convert_tokenizers: convert_tokenizers_lns_xf convert_tokenizers_lns_xmb convert_tokenizers_bf16

$(MODELS_LNS):
	mkdir -p $@

$(MODELS_BF16):
	mkdir -p $@

# ---------- run (lns16 XF) ----------
run_xf_42M: tiny_xf
	@echo "$(BLUE)Running tiny_xf — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_xf $(LNS_TABLES)/lns16_q8_7_xf.lns $(MODEL_42M_LNS_XF) -z $(TOKENIZER_LNS_XF)

run_xf_15M: tiny_xf
	@echo "$(BLUE)Running tiny_xf — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_xf $(LNS_TABLES)/lns16_q8_7_xf.lns $(MODEL_15M_LNS_XF) -z $(TOKENIZER_LNS_XF)

run_xf_260K: tiny_xf
	@echo "$(BLUE)Running tiny_xf — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_xf $(LNS_TABLES)/lns16_q8_7_xf.lns $(MODEL_260K_LNS_XF) -z $(TOKENIZER_260K_LNS_XF)

# ---------- run (lns16 XMB) ----------
run_xmb_42M: tiny_xmb
	@echo "$(BLUE)Running tiny_xmb — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns $(MODEL_42M_LNS_XMB) -z $(TOKENIZER_LNS_XMB)

run_xmb_15M: tiny_xmb
	@echo "$(BLUE)Running tiny_xmb — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns $(MODEL_15M_LNS_XMB) -z $(TOKENIZER_LNS_XMB)

run_xmb_260K: tiny_xmb
	@echo "$(BLUE)Running tiny_xmb — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns $(MODEL_260K_LNS_XMB) -z $(TOKENIZER_260K_LNS_XMB)

# ---------- run (bf16) ----------
run_bf16_42M: tiny_bf16
	@echo "$(BLUE)Running tiny_bf16 — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_bf16 $(MODEL_42M_BF16) -z $(TOKENIZER_BF16)

run_bf16_15M: tiny_bf16
	@echo "$(BLUE)Running tiny_bf16 — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_bf16 $(MODEL_15M_BF16) -z $(TOKENIZER_BF16)

run_bf16_260K: tiny_bf16
	@echo "$(BLUE)Running tiny_bf16 — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_bf16 $(MODEL_260K_BF16) -z $(TOKENIZER_260K_BF16)

# ---------- build dirs ----------
$(BUILD_CONVERT):
	mkdir -p $@

$(BUILD_TINY):
	mkdir -p $@

# ---------- misc ----------
clean:
	@echo "$(BLUE)Cleaning...$(RESET)"
	rm -rf $(BUILD)
