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

SRC_DIR        					 = .
TINY_LNS16_LNS32ACC_SRC  = $(SRC_DIR)/tiny/tiny_lns16_lns32acc.cpp
TINY_LNS8_LNS32ACC_SRC   = $(SRC_DIR)/tiny/tiny_lns16_lns32acc.cpp
TINY_LNS16_SRC 					 = $(SRC_DIR)/tiny/tiny_lns16.cpp
TINY_LNS8_SRC 					 = $(SRC_DIR)/tiny/tiny_lns8.cpp
TINY_BF16_SRC  					 = $(SRC_DIR)/tiny/tiny_bf16.cpp
TINY_BF8_SRC  					 = $(SRC_DIR)/tiny/tiny_bf8.cpp
CONV_BF16_SRC  					 = $(SRC_DIR)/convert/convert_bf16.cpp
CONV_BF8_SRC  					 = $(SRC_DIR)/convert/convert_bf8.cpp
CONV_LNS16_SRC 					 = $(SRC_DIR)/convert/convert_lns16.cpp
CONV_LNS8_SRC 					 = $(SRC_DIR)/convert/convert_lns8.cpp

LNS_TABLES = $(LNS_DIR)/spline/lns_tables

MODELS_DIR = models
MODELS_LNS = $(MODELS_DIR)/lns
MODELS_BF  = $(MODELS_DIR)/bf

# Model / tokenizer paths
MODEL_15M         		   = $(MODELS_DIR)/stories15M.bin
MODEL_42M         		   = $(MODELS_DIR)/stories42M.bin
MODEL_260K        		   = $(MODELS_DIR)/stories260K.bin
TOKENIZER         		   = $(MODELS_DIR)/tokenizer.bin
TOKENIZER_260K    		   = $(MODELS_DIR)/tokenizer260K.bin

MODEL_42M_LNS8_XF   	   = $(MODELS_LNS)/stories42M_lns8_xf.bin
MODEL_15M_LNS8_XF   	   = $(MODELS_LNS)/stories15M_lns8_xf.bin
MODEL_260K_LNS8_XF  	   = $(MODELS_LNS)/stories260K_lns8_xf.bin
TOKENIZER_LNS8_XF   		 = $(MODELS_LNS)/tokenizer_lns8_xf.bin
TOKENIZER_260K_LNS8_XF   = $(MODELS_LNS)/tokenizer260K_lns8_xf.bin

MODEL_42M_LNS8_XMB  		 = $(MODELS_LNS)/stories42M_lns8_xmb.bin
MODEL_15M_LNS8_XMB  		 = $(MODELS_LNS)/stories15M_lns8_xmb.bin
MODEL_260K_LNS8_XMB 		 = $(MODELS_LNS)/stories260K_lns8_xmb.bin
TOKENIZER_LNS8_XMB  		 = $(MODELS_LNS)/tokenizer_lns8_xmb.bin
TOKENIZER_260K_LNS8_XMB  = $(MODELS_LNS)/tokenizer260K_lns8_xmb.bin

MODEL_42M_LNS16_XF   	   = $(MODELS_LNS)/stories42M_lns16_xf.bin
MODEL_15M_LNS16_XF   	   = $(MODELS_LNS)/stories15M_lns16_xf.bin
MODEL_260K_LNS16_XF  	   = $(MODELS_LNS)/stories260K_lns16_xf.bin
TOKENIZER_LNS16_XF   		 = $(MODELS_LNS)/tokenizer_lns16_xf.bin
TOKENIZER_260K_LNS16_XF  = $(MODELS_LNS)/tokenizer260K_lns16_xf.bin

MODEL_42M_LNS16_XMB  		 = $(MODELS_LNS)/stories42M_lns16_xmb.bin
MODEL_15M_LNS16_XMB  		 = $(MODELS_LNS)/stories15M_lns16_xmb.bin
MODEL_260K_LNS16_XMB 		 = $(MODELS_LNS)/stories260K_lns16_xmb.bin
TOKENIZER_LNS16_XMB  		 = $(MODELS_LNS)/tokenizer_lns16_xmb.bin
TOKENIZER_260K_LNS16_XMB = $(MODELS_LNS)/tokenizer260K_lns16_xmb.bin

MODEL_42M_BF8       	   = $(MODELS_BF)/stories42M_bf8.bin
MODEL_15M_BF8       	   = $(MODELS_BF)/stories15M_bf8.bin
MODEL_260K_BF8      	   = $(MODELS_BF)/stories260K_bf8.bin
TOKENIZER_BF8       	   = $(MODELS_BF)/tokenizer_bf8.bin
TOKENIZER_260K_BF8  	   = $(MODELS_BF)/tokenizer260K_bf8.bin

MODEL_42M_BF16       	   = $(MODELS_BF)/stories42M_bf16.bin
MODEL_15M_BF16       	   = $(MODELS_BF)/stories15M_bf16.bin
MODEL_260K_BF16      	   = $(MODELS_BF)/stories260K_bf16.bin
TOKENIZER_BF16       	   = $(MODELS_BF)/tokenizer_bf16.bin
TOKENIZER_260K_BF16  	   = $(MODELS_BF)/tokenizer260K_bf16.bin

VOCAB_SIZE      = 32000
VOCAB_SIZE_260K = 512

RED   = \033[031m
GREEN = \033[032m
BLUE  = \033[036m
RESET = \033[0m

.PHONY: all tiny_lns16_xf tiny_lns16_xmb tiny_bf16 \
        convert_bf16 convert_lns16_xf convert_lns16_xmb convert_lns16 \
        models models_dir \
        convert_models convert_models_lns convert_models_bf16 \
        convert_tokenizers convert_tokenizers_lns convert_tokenizers_bf16 \
        run_xf_15M run_xf_260K \
        run_xmb_15M run_xmb_260K \
        run_bf16_15M run_bf16_260K \
        clean

# ---------- default ----------
all: tiny_lns16_xf tiny_lns16_xmb tiny_bf16 convert_lns16 convert_bf16 \
		 tiny_lns8_xf tiny_lns8_xmb tiny_bf8 convert_lns8 convert_bf8

# ---------- tiny ----------
tiny_lns8_xf: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XF $(TINY_LNS8_SRC) -o $(BUILD_TINY)/tiny_lns8_xf -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_lns8_xf$(RESET)"

tiny_lns8_xf_lns32acc: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XF $(TINY_LNS8_LNS32ACC_SRC) -o $(BUILD_TINY)/tiny_lns8_xf_lns32acc -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_lns8_xf_lns32acc$(RESET)"

tiny_lns8_xmb: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XMB $(TINY_LNS8_SRC) -o $(BUILD_TINY)/tiny_lns8_xmb -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_lns8_xmb$(RESET)"

tiny_lns8_xmb_lns32acc: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XMB $(TINY_LNS8_LNS32ACC_SRC) -o $(BUILD_TINY)/tiny_lns8_xmb_lns32acc -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_lns8_xmb_lns32acc$(RESET)"

tiny_bf8: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with bf8...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp $(TINY_BF8_SRC) -o $(BUILD_TINY)/tiny_bf8 -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_bf8$(RESET)"

tiny_lns16_xf: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XF $(TINY_LNS16_SRC) -o $(BUILD_TINY)/tiny_lns16_xf -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_lns16_xf$(RESET)"

tiny_lns16_xf_lns32acc: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XF...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XF $(TINY_LNS16_LNS32ACC_SRC) -o $(BUILD_TINY)/tiny_lns16_xf_lns32acc -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_lns16_xf_lns32acc$(RESET)"

tiny_lns16_xmb: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XMB $(TINY_LNS16_SRC) -o $(BUILD_TINY)/tiny_lns16_xmb -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_lns16_xmb$(RESET)"

tiny_lns16_xmb_lns32acc: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with SPLINE_XMB...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp -DSPLINE_XMB $(TINY_LNS16_LNS32ACC_SRC) -o $(BUILD_TINY)/tiny_lns16_xmb_lns32acc -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_lns16_xmb_lns32acc$(RESET)"

tiny_bf16: $(BUILD_TINY)
	@echo "$(BLUE)Compiling tiny with bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) -fopenmp $(TINY_BF16_SRC) -o $(BUILD_TINY)/tiny_bf16 -lm
	@echo "$(GREEN)Build complete → $(BUILD_TINY)/tiny_bf16$(RESET)"

# ---------- converters ----------
convert_lns8_xf: $(BUILD_CONVERT)
	@echo "$(BLUE)Compiling convert_lns8 (XF)...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF $(CONV_LNS8_SRC) -o $(BUILD_CONVERT)/convert_lns8_xf
	@echo "$(GREEN)Build complete → $(BUILD_CONVERT)/convert_lns8_xf$(RESET)"

convert_lns8_xmb: $(BUILD_CONVERT)
	@echo "$(BLUE)Compiling convert_lns8 (XMB)...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB $(CONV_LNS8_SRC) -o $(BUILD_CONVERT)/convert_lns8_xmb
	@echo "$(GREEN)Build complete → $(BUILD_CONVERT)/convert_lns8_xmb$(RESET)"

convert_lns16_xf: $(BUILD_CONVERT)
	@echo "$(BLUE)Compiling convert_lns16 (XF)...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XF $(CONV_LNS16_SRC) -o $(BUILD_CONVERT)/convert_lns16_xf
	@echo "$(GREEN)Build complete → $(BUILD_CONVERT)/convert_lns16_xf$(RESET)"

convert_lns16_xmb: $(BUILD_CONVERT)
	@echo "$(BLUE)Compiling convert_lns16 (XMB)...$(RESET)"
	$(CXX) $(CXXFLAGS) -DSPLINE_XMB $(CONV_LNS16_SRC) -o $(BUILD_CONVERT)/convert_lns16_xmb
	@echo "$(GREEN)Build complete → $(BUILD_CONVERT)/convert_lns16_xmb$(RESET)"

convert_lns8:  convert_lns8_xf convert_lns8_xmb
convert_lns16: convert_lns16_xf convert_lns16_xmb

convert_bf8: $(BUILD_CONVERT)
	@echo "$(BLUE)Compiling convert_bf8...$(RESET)"
	$(CXX) $(CXXFLAGS) $(CONV_BF8_SRC) -o $(BUILD_CONVERT)/convert_bf8
	@echo "$(GREEN)Build complete → $(BUILD_CONVERT)/convert_bf8$(RESET)"

convert_bf16: $(BUILD_CONVERT)
	@echo "$(BLUE)Compiling convert_bf16...$(RESET)"
	$(CXX) $(CXXFLAGS) $(CONV_BF16_SRC) -o $(BUILD_CONVERT)/convert_bf16
	@echo "$(GREEN)Build complete → $(BUILD_CONVERT)/convert_bf16$(RESET)"

# ---------- model download ----------
models_dir:
	mkdir -p $(MODELS_DIR) $(MODELS_LNS) $(MODELS_BF)

models: models_dir
	@echo "$(BLUE)Downloading float32 model weights...$(RESET)"
	wget -nc -P $(MODELS_DIR) https://huggingface.co/karpathy/tinyllamas/resolve/main/stories15M.bin
	wget -nc -P $(MODELS_DIR) https://huggingface.co/karpathy/tinyllamas/resolve/main/stories42M.bin
	wget -nc -P $(MODELS_DIR) https://huggingface.co/karpathy/tinyllamas/resolve/main/stories260K/stories260K.bin
	wget -nc -P $(MODELS_DIR) https://github.com/karpathy/llama2.c/raw/master/tokenizer.bin
	test -f $(MODELS_DIR)/tokenizer260K.bin || wget -O $(MODELS_DIR)/tokenizer260K.bin https://huggingface.co/karpathy/tinyllamas/resolve/main/stories260K/tok512.bin
	@echo "$(GREEN)Models downloaded → $(MODELS_DIR)/$(RESET)"

# ---------- model conversion ----------
convert_models_lns8_xf: convert_lns8_xf $(MODELS_LNS)
	@echo "$(BLUE)Converting models to lns8 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns8_xf $(LNS_TABLES)/lns8_q4_3_xf.lns model $(MODEL_42M)  $(MODEL_42M_LNS8_XF)
	$(BUILD_CONVERT)/convert_lns8_xf $(LNS_TABLES)/lns8_q4_3_xf.lns model $(MODEL_15M)  $(MODEL_15M_LNS8_XF)
	$(BUILD_CONVERT)/convert_lns8_xf $(LNS_TABLES)/lns8_q4_3_xf.lns model $(MODEL_260K) $(MODEL_260K_LNS8_XF)
	@echo "$(GREEN)LNS8 models ready → $(MODELS_LNS)/$(RESET)"

convert_models_lns8_xmb: convert_lns8_xmb $(MODELS_LNS)
	@echo "$(BLUE)Converting models to lns8 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns8_xmb $(LNS_TABLES)/lns8_q4_3_xmb.lns model $(MODEL_42M)  $(MODEL_42M_LNS8_XMB)
	$(BUILD_CONVERT)/convert_lns8_xmb $(LNS_TABLES)/lns8_q4_3_xmb.lns model $(MODEL_15M)  $(MODEL_15M_LNS8_XMB)
	$(BUILD_CONVERT)/convert_lns8_xmb $(LNS_TABLES)/lns8_q4_3_xmb.lns model $(MODEL_260K) $(MODEL_260K_LNS8_XMB)
	@echo "$(GREEN)LNS8 models ready → $(MODELS_LNS)/$(RESET)"

convert_models_lns16_xf: convert_lns16_xf $(MODELS_LNS)
	@echo "$(BLUE)Converting models to lns16 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns model $(MODEL_42M)  $(MODEL_42M_LNS16_XF)
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns model $(MODEL_15M)  $(MODEL_15M_LNS16_XF)
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns model $(MODEL_260K) $(MODEL_260K_LNS16_XF)
	@echo "$(GREEN)LNS16 models ready → $(MODELS_LNS)/$(RESET)"

convert_models_lns16_xmb: convert_lns16_xmb $(MODELS_LNS)
	@echo "$(BLUE)Converting models to lns16 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns model $(MODEL_42M)  $(MODEL_42M_LNS16_XMB)
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns model $(MODEL_15M)  $(MODEL_15M_LNS16_XMB)
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns model $(MODEL_260K) $(MODEL_260K_LNS16_XMB)
	@echo "$(GREEN)LNS16 models ready → $(MODELS_LNS)/$(RESET)"

convert_models_bf8: convert_bf8 $(MODELS_BF)
	@echo "$(BLUE)Converting models to bf8...$(RESET)"
	$(BUILD_CONVERT)/convert_bf8 model $(MODEL_42M)  $(MODEL_42M_BF8)
	$(BUILD_CONVERT)/convert_bf8 model $(MODEL_15M)  $(MODEL_15M_BF8)
	$(BUILD_CONVERT)/convert_bf8 model $(MODEL_260K) $(MODEL_260K_BF8)
	@echo "$(GREEN)BF8 models ready → $(MODELS_BF)/$(RESET)"

convert_models_bf16: convert_bf16 $(MODELS_BF)
	@echo "$(BLUE)Converting models to bf16...$(RESET)"
	$(BUILD_CONVERT)/convert_bf16 model $(MODEL_42M)  $(MODEL_42M_BF16)
	$(BUILD_CONVERT)/convert_bf16 model $(MODEL_15M)  $(MODEL_15M_BF16)
	$(BUILD_CONVERT)/convert_bf16 model $(MODEL_260K) $(MODEL_260K_BF16)
	@echo "$(GREEN)BF models ready → $(MODELS_BF)/$(RESET)"

convert_models: convert_models_lns8_xf convert_models_lns8_xmb convert_models_lns16_xf convert_models_lns16_xmb convert_models_bf8 convert_models_bf16

# ---------- tokenizer conversion ----------
convert_tokenizers_lns8_xf: convert_lns8_xf $(MODELS_LNS)
	@echo "$(BLUE)Converting tokenizers to lns8 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns8_xf $(LNS_TABLES)/lns8_q4_3_xf.lns tokenizer $(TOKENIZER)      $(VOCAB_SIZE)      $(TOKENIZER_LNS8_XF)
	$(BUILD_CONVERT)/convert_lns8_xf $(LNS_TABLES)/lns8_q4_3_xf.lns tokenizer $(TOKENIZER_260K) $(VOCAB_SIZE_260K) $(TOKENIZER_260K_LNS8_XF)
	@echo "$(GREEN)LNS8 tokenizers ready → $(MODELS_LNS)/$(RESET)"

convert_tokenizers_lns8_xmb: convert_lns8_xmb $(MODELS_LNS)
	@echo "$(BLUE)Converting tokenizers to lns8 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns8_xmb $(LNS_TABLES)/lns8_q4_3_xmb.lns tokenizer $(TOKENIZER)      $(VOCAB_SIZE)      $(TOKENIZER_LNS8_XMB)
	$(BUILD_CONVERT)/convert_lns8_xmb $(LNS_TABLES)/lns8_q4_3_xmb.lns tokenizer $(TOKENIZER_260K) $(VOCAB_SIZE_260K) $(TOKENIZER_260K_LNS8_XMB)
	@echo "$(GREEN)LNS8 tokenizers ready → $(MODELS_LNS)/$(RESET)"

convert_tokenizers_lns16_xf: convert_lns16_xf $(MODELS_LNS)
	@echo "$(BLUE)Converting tokenizers to lns16 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns tokenizer $(TOKENIZER)      $(VOCAB_SIZE)      $(TOKENIZER_LNS16_XF)
	$(BUILD_CONVERT)/convert_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns tokenizer $(TOKENIZER_260K) $(VOCAB_SIZE_260K) $(TOKENIZER_260K_LNS16_XF)
	@echo "$(GREEN)LNS16 tokenizers ready → $(MODELS_LNS)/$(RESET)"

convert_tokenizers_lns16_xmb: convert_lns16_xmb $(MODELS_LNS)
	@echo "$(BLUE)Converting tokenizers to lns16 (XF)...$(RESET)"
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns tokenizer $(TOKENIZER)      $(VOCAB_SIZE)      $(TOKENIZER_LNS16_XMB)
	$(BUILD_CONVERT)/convert_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns tokenizer $(TOKENIZER_260K) $(VOCAB_SIZE_260K) $(TOKENIZER_260K_LNS16_XMB)
	@echo "$(GREEN)LNS16 tokenizers ready → $(MODELS_LNS)/$(RESET)"

convert_tokenizers_bf8: convert_bf8 $(MODELS_BF)
	@echo "$(BLUE)Converting tokenizers to bf8...$(RESET)"
	$(BUILD_CONVERT)/convert_bf8 tokenizer $(TOKENIZER)      $(VOCAB_SIZE)      $(TOKENIZER_BF8)
	$(BUILD_CONVERT)/convert_bf8 tokenizer $(TOKENIZER_260K) $(VOCAB_SIZE_260K) $(TOKENIZER_260K_BF8)

convert_tokenizers_bf16: convert_bf16 $(MODELS_BF)
	@echo "$(BLUE)Converting tokenizers to bf16...$(RESET)"
	$(BUILD_CONVERT)/convert_bf16 tokenizer $(TOKENIZER)      $(VOCAB_SIZE)      $(TOKENIZER_BF16)
	$(BUILD_CONVERT)/convert_bf16 tokenizer $(TOKENIZER_260K) $(VOCAB_SIZE_260K) $(TOKENIZER_260K_BF16)
	@echo "$(GREEN)BF tokenizers ready → $(MODELS_BF)/$(RESET)"

convert_tokenizers: convert_tokenizers_lns8_xf convert_tokenizers_lns8_xmb convert_tokenizers_bf16 convert_tokenizers_lns16_xf convert_tokenizers_lns16_xmb convert_tokenizers_bf8 convert_tokenizers_bf16

$(MODELS_LNS):
	mkdir -p $@

$(MODELS_BF):
	mkdir -p $@

# ---------- run (lns8/lns16 XF) ----------
run_lns8_xf_42M: tiny_lns8_xf
	@echo "$(BLUE)Running tiny_lns8_xf — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xf $(LNS_TABLES)/lns8_q4_3_xf.lns $(MODEL_42M_LNS8_XF) -z $(TOKENIZER_LNS8_XF)

run_lns8_xf_lns32acc_42M: tiny_lns8_xf_lns32acc
	@echo "$(BLUE)Running tiny_lns8_xf_lns32acc — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xf_lns32acc $(LNS_TABLES)/lns8_q4_3_xf.lns $(MODEL_42M_LNS8_XF) -z $(TOKENIZER_LNS8_XF)

run_lns8_xf_15M: tiny_lns8_xf
	@echo "$(BLUE)Running tiny_lns8_xf — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xf $(LNS_TABLES)/lns8_q4_3_xf.lns $(MODEL_15M_LNS8_XF) -z $(TOKENIZER_LNS8_XF)

run_lns8_xf_lns32acc_15M: tiny_lns8_xf_lns32acc
	@echo "$(BLUE)Running tiny_lns8_xf_lns32acc — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xf_lns32acc $(LNS_TABLES)/lns8_q4_3_xf.lns $(MODEL_15M_LNS8_XF) -z $(TOKENIZER_LNS8_XF)

run_lns8_xf_260K: tiny_lns8_xf
	@echo "$(BLUE)Running tiny_lns8_xf — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xf $(LNS_TABLES)/lns8_q4_3_xf.lns $(MODEL_260K_LNS8_XF) -z $(TOKENIZER_260K_LNS8_XF)

run_lns8_xf_lns32acc_260K: tiny_lns8_xf_lns32acc
	@echo "$(BLUE)Running tiny_lns8_xf_lns32acc — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xf_lns32acc $(LNS_TABLES)/lns8_q4_3_xf.lns $(MODEL_260K_LNS8_XF) -z $(TOKENIZER_260K_LNS8_XF)

run_lns16_xf_42M: tiny_lns16_xf
	@echo "$(BLUE)Running tiny_lns16_xf — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns $(MODEL_42M_LNS16_XF) -z $(TOKENIZER_LNS16_XF)

run_lns16_xf_lns32acc_42M: tiny_lns16_xf_lns32acc
	@echo "$(BLUE)Running tiny_lns16_xf_lns32acc — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xf_lns32acc $(LNS_TABLES)/lns16_q8_7_xf.lns $(MODEL_42M_LNS16_XF) -z $(TOKENIZER_LNS16_XF)

run_lns16_xf_15M: tiny_lns16_xf
	@echo "$(BLUE)Running tiny_lns16_xf — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns $(MODEL_15M_LNS16_XF) -z $(TOKENIZER_LNS16_XF)

run_lns16_xf_lns32acc_15M: tiny_lns16_xf_lns32acc
	@echo "$(BLUE)Running tiny_lns16_xf_lns32acc — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xf_lns32acc $(LNS_TABLES)/lns16_q8_7_xf.lns $(MODEL_15M_LNS16_XF) -z $(TOKENIZER_LNS16_XF)

run_lns16_xf_260K: tiny_lns16_xf
	@echo "$(BLUE)Running tiny_lns16_xf — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xf $(LNS_TABLES)/lns16_q8_7_xf.lns $(MODEL_260K_LNS16_XF) -z $(TOKENIZER_260K_LNS16_XF)

run_lns16_xf_lns32acc_260K: tiny_lns16_xf_lns32acc
	@echo "$(BLUE)Running tiny_lns16_xf_lns32acc — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xf_lns32acc $(LNS_TABLES)/lns16_q8_7_xf.lns $(MODEL_260K_LNS16_XF) -z $(TOKENIZER_260K_LNS16_XF)

# ---------- run (lns8/lns16 XMB) ----------
run_lns8_xmb_42M: tiny_lns8_xmb
	@echo "$(BLUE)Running tiny_lns8_xmb — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xmb $(LNS_TABLES)/lns8_q4_3_xmb.lns $(MODEL_42M_LNS8_XMB) -z $(TOKENIZER_LNS8_XMB)

run_lns8_xmb_ln32acc_42M: tiny_lns8_xmb_lns32acc
	@echo "$(BLUE)Running tiny_lns8_xmb_lns32acc — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xmb_lns32acc $(LNS_TABLES)/lns8_q4_3_xmb.lns $(MODEL_42M_LNS8_XMB) -z $(TOKENIZER_LNS8_XMB)

run_lns8_xmb_15M: tiny_lns8_xmb
	@echo "$(BLUE)Running tiny_lns8_xmb — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xmb $(LNS_TABLES)/lns8_q4_3_xmb.lns $(MODEL_15M_LNS8_XMB) -z $(TOKENIZER_LNS8_XMB)

run_lns8_xmb_lns32acc_15M: tiny_lns8_xmb_lns32acc
	@echo "$(BLUE)Running tiny_lns8_xmb_lns32acc — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xmb_lns32acc $(LNS_TABLES)/lns8_q4_3_xmb.lns $(MODEL_15M_LNS8_XMB) -z $(TOKENIZER_LNS8_XMB)

run_lns8_xmb_260K: tiny_lns8_xmb
	@echo "$(BLUE)Running tiny_lns8_xmb — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xmb $(LNS_TABLES)/lns8_q4_3_xmb.lns $(MODEL_260K_LNS8_XMB) -z $(TOKENIZER_260K_LNS8_XMB)

run_lns8_xmb_lns32acc_260K: tiny_lns8_xmb_lns32acc
	@echo "$(BLUE)Running tiny_lns8_xmb_lns32acc — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_lns8_xmb_lns32acc $(LNS_TABLES)/lns8_q4_3_xmb.lns $(MODEL_260K_LNS8_XMB) -z $(TOKENIZER_260K_LNS8_XMB)

run_lns16_xmb_42M: tiny_lns16_xmb
	@echo "$(BLUE)Running tiny_lns16_xmb — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns $(MODEL_42M_LNS16_XMB) -z $(TOKENIZER_LNS16_XMB)

run_lns16_xmb_ln32acc_42M: tiny_lns16_xmb_lns32acc
	@echo "$(BLUE)Running tiny_lns16_xmb_lns32acc — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xmb_lns32acc $(LNS_TABLES)/lns16_q8_7_xmb.lns $(MODEL_42M_LNS16_XMB) -z $(TOKENIZER_LNS16_XMB)

run_lns16_xmb_15M: tiny_lns16_xmb
	@echo "$(BLUE)Running tiny_lns16_xmb — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns $(MODEL_15M_LNS16_XMB) -z $(TOKENIZER_LNS16_XMB)

run_lns16_xmb_lns32acc_15M: tiny_lns16_xmb_lns32acc
	@echo "$(BLUE)Running tiny_lns16_xmb_lns32acc — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xmb_lns32acc $(LNS_TABLES)/lns16_q8_7_xmb.lns $(MODEL_15M_LNS16_XMB) -z $(TOKENIZER_LNS16_XMB)

run_lns16_xmb_260K: tiny_lns16_xmb
	@echo "$(BLUE)Running tiny_lns16_xmb — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xmb $(LNS_TABLES)/lns16_q8_7_xmb.lns $(MODEL_260K_LNS16_XMB) -z $(TOKENIZER_260K_LNS16_XMB)

run_lns16_xmb_lns32acc_260K: tiny_lns16_xmb_lns32acc
	@echo "$(BLUE)Running tiny_lns16_xmb_lns32acc — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_lns16_xmb_lns32acc $(LNS_TABLES)/lns16_q8_7_xmb.lns $(MODEL_260K_LNS16_XMB) -z $(TOKENIZER_260K_LNS16_XMB)

# ---------- run (bf8/bf16) ----------
run_bf8_42M: tiny_bf8
	@echo "$(BLUE)Running tiny_bf8 — stories42M...$(RESET)"
	$(BUILD_TINY)/tiny_bf8 $(MODEL_42M_BF8) -z $(TOKENIZER_BF8)

run_bf8_15M: tiny_bf8
	@echo "$(BLUE)Running tiny_bf8 — stories15M...$(RESET)"
	$(BUILD_TINY)/tiny_bf8 $(MODEL_15M_BF8) -z $(TOKENIZER_BF8)

run_bf8_260K: tiny_bf8
	@echo "$(BLUE)Running tiny_bf8 — stories260K...$(RESET)"
	$(BUILD_TINY)/tiny_bf8 $(MODEL_260K_BF8) -z $(TOKENIZER_260K_BF8)

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
