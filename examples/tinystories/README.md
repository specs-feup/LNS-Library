# tinystories

Runs [Andrej Karpathy's llama2.c](https://github.com/karpathy/llama2.c) 
TinyStories inference in lns16 and bf16, using the LNS simulation API as a 
drop-in replacement for the original float weights.

Three variants are provided:

- **`tiny/tiny_lns16.cpp`** — inference with lns16 (Q8.7), weights converted from the original `.bin` format; accumulation-heavy operations (rmsnorm, attention scores, softmax) use fp32 accumulators
- **`tiny/tiny_lns16_lns32acc.cpp`** — same as above but accumulation-heavy operations use lns32 accumulators instead of fp32, keeping all computation in log-space
- **`tiny/tiny_bf16.cpp`** — inference with bf16, same pipeline

Model weights must be converted before running. The converters in `convert/` handle this.

---

## Getting the models

Run the following from `apps/bfloat_vs_lns/tinystories/`:

```bash
# Original float32 weights (required for conversion)
wget -P models/ https://huggingface.co/karpathy/tinyllamas/resolve/main/stories15M.bin
wget -P models/ https://huggingface.co/karpathy/tinyllamas/resolve/main/stories42M.bin
wget -P models/ https://huggingface.co/karpathy/tinyllamas/resolve/main/stories260K/stories260K.bin

# Tokenizers
wget -P models/ https://github.com/karpathy/llama2.c/raw/master/tokenizer.bin
wget -P models/ https://huggingface.co/karpathy/tinyllamas/resolve/main/stories260K/tok512.bin
```

Pre-converted lns16 weights (XF spline format) are already committed under 
`models/lns/` for the 15M and 260K checkpoints. If you need to reconvert or 
produce XMB variants, use the converters below.

---

## Building

From `apps/bfloat_vs_lns/tinystories/`:

```bash
# lns16 inference (XF spline)
make tiny_xf

# lns16 inference (XF spline, lns32 accumulators)
make tiny_xf_lns32acc

# lns16 inference (XMB spline — faster, avoids division in interpolation)
make tiny_xmb

# lns16 inference (XMB spline, lns32 accumulators)
make tiny_xmb_lns32acc

# bf16 inference
make tiny_bf16

# Weight converters
make convert_bf16
make convert_lns16
```

All binaries land in `build/`.

---

## Converting weights

The converters take a subcommand: `model` for weight files, `tokenizer` 
for tokenizer files. The tokenizer subcommand requires the vocabulary size 
as a third argument (32000 for the standard tokenizer, 512 for tok512).

```bash
make convert_models
make convert_tokenizers
```

Or manually:

```bash
# lns16 (XF) — models
./build/convert/convert_lns16_xf model     models/stories15M.bin          models/lns/stories15M_lns16_xf.bin
./build/convert/convert_lns16_xf model     models/stories260K.bin         models/lns/stories260K_lns16_xf.bin

# lns16 (XF) — tokenizers
./build/convert/convert_lns16_xf tokenizer models/tokenizer.bin    32000  models/lns/tokenizer_lns16_xf.bin
./build/convert/convert_lns16_xf tokenizer models/tok512.bin       512    models/lns/tokenizer260K_lns16_xf.bin

# bf16 — models
./build/convert/convert_bf16     model     models/stories15M.bin          models/bf16/stories15M_bf16.bin
./build/convert/convert_bf16     model     models/stories260K.bin         models/bf16/stories260K_bf16.bin

# bf16 — tokenizers
./build/convert/convert_bf16     tokenizer models/tokenizer.bin    32000  models/bf16/tokenizer_bf16.bin
./build/convert/convert_bf16     tokenizer models/tok512.bin       512    models/bf16/tokenizer260K_bf16.bin
```

---

## Running

```bash
# lns16 XF — 15M model
./build/tiny/tiny_xf models/lns/stories15M_lns16_xf.bin -z models/tokenizer.bin

# lns16 XF with lns32 accumulators — 15M model
./build/tiny/tiny_xf_lns32acc models/lns/stories15M_lns16_xf.bin -z models/tokenizer.bin

# lns16 XMB — 15M model
./build/tiny/tiny_xmb models/lns/stories15M_lns16_xf.bin -z models/tokenizer.bin

# lns16 XMB with lns32 accumulators — 15M model
./build/tiny/tiny_xmb_lns32acc models/lns/stories15M_lns16_xf.bin -z models/tokenizer.bin

# bf16 — 15M model
./build/tiny/tiny_bf16 models/stories15M.bin -z models/tokenizer.bin
```

Spline tables are loaded automatically from the default path 
(`lib/spline/lns_tables/`). If you're running from a non-standard 
working directory, set `LNS_TABLE_PATH` or adjust the path in the source.

---

## lns32 accumulator variant

`tiny_lns16_lns32acc.cpp` replaces the fp32 accumulators in the hybrid 
baseline with lns32 (Q8.23) accumulators, keeping all computation in log-space. 
The weights and activations remain lns16; only the running sums in rmsnorm, 
attention score accumulation, attention-weighted value accumulation, and 
residual additions are widened to lns32 before being narrowed back to lns16. 
This isolates the contribution of the accumulator precision from that of the 
weight/activation format.

---

## Cleaning

```bash
make clean
```
