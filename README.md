# LNS — Logarithmic Number System Library

A C++ library implementing the Logarithmic Number System (LNS) for 8-bit and 16-bit fixed-point formats, with hardware compiler support and a software simulation API for testing and benchmarking.

---

## What is LNS?

In LNS, a number is represented as a fixed-point exponent — the value is implicitly `2^e`. This means multiplication and division become exact integer additions and subtractions of exponents, with no rounding error beyond quantization. Addition and subtraction require a correction term `log2(1 ± 2^diff)` which is approximated via a precomputed spline lookup table.

The two supported formats are:

- **lns8 Q4.3** — 8-bit, 1 sign bit, 4-bit integer exponent, 3-bit fractional exponent. Representable range: `2^-8` to `2^7.875`.
- **lns16 Q8.7** — 16-bit, 1 sign bit, 8-bit integer exponent, 7-bit fractional exponent. Representable range: `2^-128` to `2^127.992`.

---

## Why LNS?

Floating point is the default numeric format for nearly all compute workloads, but it carries inherent costs. Multiplication requires a full integer multiply of the mantissa, division is expensive in hardware, and the exponent and mantissa fields are encoded separately in a way that wastes area and power in silicon.

LNS sidesteps these costs by storing only the exponent. This has several consequences:

**Multiplication and division are free.** Since values are stored as exponents, multiplying two LNS numbers is just adding their exponent fields — a single integer add. Division is a subtract. No mantissa multiplier is needed, which significantly reduces hardware area and power.

**Square root is a single shift.** `sqrt(2^e) = 2^(e/2)`, so square root is an arithmetic right shift of the exponent by one bit. This is exact up to quantization.

**Uniform relative precision.** Floating point has higher absolute precision near zero and lower near large values. LNS has uniform relative precision across its entire range because the exponent is stored in fixed point — the quantization step is always the same fraction of the value.

**Addition and subtraction are the weak point.** Adding two LNS values requires computing `log2(1 + 2^diff)` where `diff` is the difference of the exponents. This cannot be done exactly in fixed-point arithmetic and must be approximated via a lookup table. The quality of this approximation is what the spline tables in this library are designed to maximise.

LNS is most attractive for workloads that are multiply-heavy and addition-light, such as neural network inference, signal processing filters, and Bayesian computation, where the hardware savings on multiply significantly outweigh the cost of approximate addition.

---

## Components

### `lib/lns/lns.hpp` — Hardware compiler header

This header is intended for use when targeting a custom RISC-V processor with native LNS instructions. It defines the `lns<N>` type and maps arithmetic operators to custom RISC-V instruction mnemonics via inline assembly macros. Including this header allows the compiler to emit the correct custom opcodes for LNS operations rather than falling back to software emulation. Square root is supported via a dedicated `LNS_SQRT` custom instruction, emitted through the `.sqrt()` member.

This is not meant for simulation — use `lns_sim.hpp` for that.

### `lib/sim/lns_sim.hpp` — Software simulation API

The main API for running LNS computations in software. Defines the `lns<N, I, F>` template struct parameterised by total bit width, integer exponent bits, and fractional exponent bits. Implements all arithmetic operators (`+`, `-`, `*`, `/`) and conversion to/from `float`.

Addition and subtraction dispatch to the LUT compute functions from `lns_luts.hpp`. To use the simulation, link against the precomputed spline tables for your format (see below).

Square root is also supported via `.sqrt()`. Because a value is stored as `2^e`, its square root is `2^(e/2)`, which reduces to a single arithmetic right shift of the exponent — no LUT, no approximation, exact up to quantization. It is only defined for non-negative values and will assert otherwise.

Typical usage:

```cpp
#define SPLINE_XMB
#include <lnssim>

using lns8  = lns<8,  4, 3>;
using lns16 = lns<16, 8, 7>;

lns8_read_tables("spline/lns_tables/xmb_8_q4_3.lns");
lns16_read_tables("spline/lns_tables/xmb_16_q8_7.lns");

lns16 a(1.5f), b(2.0f);
float result  = (a * b).convert();    // exact in LNS
float result2 = (a + b).convert();    // approximated via LUT
float result3 = a.sqrt().convert();   // exact — just exp >>= 1

lns_close();  // free spline table memory
```

### `lib/sim/lns_luts.hpp` — LUT compute and table I/O

Implements spline interpolation for the add/sub correction tables, and provides `lns8_read_tables` / `lns16_read_tables` to load precomputed table files at runtime. Two spline formats are supported, selected at compile time:

- `SPLINE_XF` — stores `(x, f)` pairs and interpolates linearly between function values.
- `SPLINE_XMB` — stores `(x, m, b)` per segment and evaluates `m*x + b` directly, which is faster and avoids the division inherent in XF interpolation.

Define one of these macros before including any library header.

### `spline/` — Table generation tool

A standalone C++ tool that generates the binary `.lns` table files consumed by `lns_luts.hpp`. It implements a greedy spline generation algorithm to fit approximations to the functions `log2(1 + 2^x)` (add) and `log2(1 - 2^x)` (sub) over the relevant domain for each format, and writes the result to disk. It can also test table precision against an expected error threshold without generating files.

Precomputed tables are provided in `spline/lns_tables/` for all format and spline type combinations:

| File | Format | Spline type |
|---|---|---|
| `xf_8_q4_3.lns`   | lns8 Q4.3  | XF  |
| `xf_16_q8_7.lns`  | lns16 Q8.7 | XF  |
| `xmb_8_q4_3.lns`  | lns8 Q4.3  | XMB |
| `xmb_16_q8_7.lns` | lns16 Q8.7 | XMB |

To regenerate the default tables, run `make tables` from `spline/`:

```bash
cd spline
make
make tables
```

The default tables are generated with the following parameters:

```bash
./build/spline --gen --xf  40 34 --lns8  4   # xf_8_q4_3.lns
./build/spline --gen --xf  10 50 --lns16 8   # xf_16_q8_7.lns
./build/spline --gen --xmb 41 40 --lns8  4   # xmb_8_q4_3.lns
./build/spline --gen --xmb 12 42 --lns16 8   # xmb_16_q8_7.lns
```

To test the precision of a table without generating it, use the `--test` mode:

```bash
./build/spline --test --xmb 128 --lns16 8
```

For full usage details and options refer to `spline/README.md`.

---

## Building

```bash
# Build the test binary with XMB tables (default)
make test_xmb

# Build and test with XF tables
make test_xf

# Install headers to system include path
make install

# Remove installed headers
make uninstall
```

---

## Testing

The test binary (`src/main.cpp`) takes table file paths and an optional test count:

```bash
./build/lns_test spline/lns_tables/xmb_8_q4_3.lns spline/lns_tables/xmb_16_q8_7.lns 10000
```

It runs random tests for `rt` (round-trip), `mul`, `div`, `add`, and `sub` against f32 ground truth for lns8, lns16, and bfloat16, then prints a comparison table showing min/avg/max absolute and relative error per operation and which format wins each criterion.

### Test input range and exponent capping

Random inputs are generated by sampling a random exponent uniformly over the format's exponent bit range and converting to float via `2^e`. However the full theoretical exponent range cannot be used safely in tests for two reasons.

First, addition of two large values can produce a result that overflows the format. For lns8, adding two values near `2^7 = 128` produces `256 = 2^8` which has no representation. Second, lns8 has only 3 fractional exponent bits, giving a quantization step of `2^(1/8) ≈ 9%` per step. Near the top of the range this coarse granularity causes encoded values to round to unexpectedly large magnitudes, contaminating error measurements.

For this reason the test generator caps the sampled exponent before converting:

| Format | Exponent cap | Value range | Worst-case add result |
|---|---|---|---|
| lns8 Q4.3  | `[-3, +3]` | `[0.125, 8.0]`     | `16.0 = 2^4`, well within `2^7.875` |
| lns16 Q8.7 | `[-6, +6]` | `[0.015625, 64.0]` | `128.0 = 2^7`, well within `2^127`  |

lns16 can afford a wider range because its 7 fractional bits give fine enough quantization (`2^(1/128) ≈ 0.5%` per step) that values near the boundary do not jump to unexpected magnitudes.

The same capping applies to the bfloat16 tests even though bfloat16 can theoretically represent values up to `2^127`. The reason is that bfloat16 has only 7 mantissa bits, so for large exponents the fractional part of the value is entirely consumed by the integer part — there are no bits left to represent sub-unit precision. For example, at `2^20 = 1048576`, the smallest representable step is `2^(20-7) = 8192`, meaning any value in that range is rounded to the nearest multiple of 8192. Comparing such a coarsely quantized result against an f32 ground truth produces large absolute errors that say nothing meaningful about the format's precision characteristics. By capping both lns16 and bfloat16 inputs to the same range, the benchmark measures precision on equal footing rather than penalising both formats for operating outside their useful range.

---

## Format Comparison: lns16 Q8.7 vs bfloat16

Both formats are 16 bits. bfloat16 uses 8 exponent bits and 7 mantissa bits. lns16 Q8.7 stores the exponent directly in Q8.7 fixed point.

In practice, lns16 outperforms bfloat16 on round-trip, multiply, and divide due to its exact exponent arithmetic. bfloat16 has an edge on addition because it operates directly in the linear domain, while LNS addition depends on the quality of the spline approximation. Subtraction of nearly equal operands causes cancellation errors in both formats.
---

## Planned: lns32 and lns64

Support for 32-bit and 64-bit LNS formats is in development. These formats present a different challenge from lns8 and lns16 — the domain of the add/sub correction function `log2(1 ± 2^x)` grows significantly, and the spline tables used for the smaller formats become impractically large. A different approximation strategy is needed that is simultaneously more precise, fast to evaluate, and low on memory.

The current candidate is Newton's divided differences, which constructs an interpolating polynomial through a set of known sample points. Unlike uniform splines, divided differences can concentrate sample points in regions where the function curves sharply (near `diff = 0`) and use coarser spacing in the flat tail, achieving high accuracy with far fewer coefficients. Evaluation is done via Horner's method, which is numerically stable and reduces to a small fixed number of multiply-add operations regardless of polynomial degree.

Other candidates under consideration include minimax polynomial approximation (Remez algorithm), which minimises the worst-case error across the entire domain rather than passing exactly through sample points, and piecewise Chebyshev approximation, which combines the efficiency of segmented tables with near-optimal polynomial coefficients per segment.

The goal is to support lns32 Q16.15 and lns64 Q32.31 with approximation error comparable to the precision of the format itself, without requiring lookup tables that would be impractical to store in hardware.

## Models

```
wget https://huggingface.co/karpathy/tinyllamas/resolve/main/stories42M.bin
```

```
wget https://huggingface.co/karpathy/tinyllamas/resolve/main/stories15M.bin
```

```
wget https://huggingface.co/karpathy/tinyllamas/resolve/main/stories260K.bin
```

```
wget https://github.com/karpathy/llama2.c/raw/master/tokenizer.bin
```

```
wget https://github.com/schuhandreas/embedllama-stm32h7a3/raw/main/weights/tok512.bin -O tokenizer260k.bin
```

