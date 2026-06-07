# LNS — Logarithmic Number System Library

A C++ header-only library implementing the Logarithmic Number System (LNS) for 8-bit and 16-bit fixed-point formats, with a hardware compiler header targeting custom RISC-V LNS instructions and a software simulation API for testing and development.

---

## What is LNS?

In LNS, a number is represented as a fixed-point exponent — the value is implicitly $2^e$. Multiplication and division become exact integer additions and subtractions of exponents, with no rounding error beyond quantization. Addition and subtraction require a correction term $\log_2(1 \pm 2^{\text{diff}})$ which is approximated via a precomputed spline lookup table.

The two supported formats are:

- **lns8 Q4.3** — 8-bit, 1 sign bit, 4-bit integer exponent, 3-bit fractional exponent. Representable range: $2^{-8}$ to $2^{7.875}$.
- **lns16 Q8.7** — 16-bit, 1 sign bit, 8-bit integer exponent, 7-bit fractional exponent. Representable range: $2^{-128}$ to $2^{127.992}$.

---

## Why LNS?

**Multiplication and division are exact.** Since values are stored as exponents, multiplying two LNS numbers is a single integer add on the exponent field. Division is a subtract. No mantissa multiplier is needed.

**Square root is a single shift.** $\sqrt{2^e} = 2^{e/2}$, so square root is an arithmetic right shift by one bit — exact up to quantization.

**Uniform relative precision.** The quantization step is always the same fraction of the value across the entire representable range, unlike floating point which has higher absolute precision near zero.

**Addition and subtraction are the weak point.** Adding two LNS values requires computing $\log_2(1 + 2^{\text{diff}})$ where $\text{diff}$ is the difference of the exponents. This cannot be done exactly in fixed point and must be approximated via a lookup table. The quality of this approximation is what the spline tables in this library are designed to maximise.

LNS is most attractive for multiply-heavy workloads such as neural network inference, signal processing filters, and Bayesian computation, where the hardware savings on multiply significantly outweigh the cost of approximate addition.

---

## Headers (`lib/`)

### `lns.hpp` — Hardware compiler header

Intended for use when targeting a custom RISC-V processor with native LNS instructions. Defines the `lns<N>` type and maps arithmetic operators to custom RISC-V instruction mnemonics via inline assembly, using the `.insn` directive to emit custom opcodes directly into the floating-point register file. This is not meant for simulation — use `lnssim.hpp` for that.

Predefined type aliases: `lns8`, `lns16`, `lns32`, `lns64`.

### `lnssim.hpp` — Software simulation API

The main API for running LNS computations in software. Defines `lns<N, I, F>` parameterised by total bit width, integer exponent bits, and fractional exponent bits. Implements all arithmetic operators (`+`, `-`, `*`, `/`) and conversion to/from `float`.

Addition and subtraction dispatch to the spline LUT functions in `lnsluts.hpp`. Square root is exact — since a value is stored as $2^e$, `.sqrt()` reduces to an arithmetic right shift of the exponent by one bit, with no table lookup.

Must define either `SPLINE_XF` or `SPLINE_XMB` before including, and load the corresponding table files at runtime:

```cpp
#define SPLINE_XMB
#include <lnssim.hpp>

using lns8  = lns<8,  4, 3>;
using lns16 = lns<16, 8, 7>;

lns8_read_tables ("lib/spline/lns_tables/lns8_q4_3_xmb.lns");
lns16_read_tables("lib/spline/lns_tables/lns16_q8_7_xmb.lns");

lns16 a(1.5f), b(2.0f);
float result  = (float)(a * b);   // exact — integer add on exponents
float result2 = (float)(a + b);   // approximated via spline LUT
float result3 = (float)a.sqrt();  // exact — exp >>= 1

lns_close();

```

After running `sudo make install` from the repository root, headers are installed flat to your compiler's system include path and can be included directly using angle brackets:

```cpp
#include <lnssim.hpp>
#include <lnsluts.hpp>

```

### `lnsluts.hpp` — LUT definitions and table I/O

Defines the spline structs and provides `lns8_read_tables` / `lns16_read_tables` to load precomputed `.lns` table files at runtime, and `lns_close` to free them. Two spline formats are supported at compile time:

* **`SPLINE_XF`** — stores `(x, f)` pairs, interpolates linearly between function values.
* **`SPLINE_XMB`** — stores `(x, m, b)` per segment, evaluates $m \cdot x + b$ directly. Faster — avoids the division inherent in XF interpolation.

---

## Spline Table Generation

The tool located in `lib/spline/` is a standalone utility that generates the binary `.lns` table files. It implements a greedy spline fitting algorithm over $\log_2(1 + 2^x)$ (add) and $\log_2(1 - 2^x)$ (sub) for each format, and can test table precision against an error threshold.

Build and generate the default tables:

```bash
cd lib/spline
make
# Generates files inside lib/spline/lns_tables/

```

This updates or produces four files inside `lib/spline/lns_tables/`:

| File | Format | Spline type |
| --- | --- | --- |
| `lns8_q4_3_xf.lns` | lns8 Q4.3 | XF |
| `lns8_q4_3_xmb.lns` | lns8 Q4.3 | XMB |
| `lns16_q8_7_xf.lns` | lns16 Q8.7 | XF |
| `lns16_q8_7_xmb.lns` | lns16 Q8.7 | XMB |

To test precision directly using the built tool without overwriting tables:

```bash
./build/spline --test --xmb 128 --lns16 8

```

---

## Building

From the repository root directory:

```bash
make examples    # builds examples targets

sudo make install   # installs headers flat to system include path
make uninstall      # removes installed headers from system include path

```

---

## Examples

### `examples/bench/` — LNS vs BFloat accuracy benchmark

Monte Carlo arithmetic accuracy benchmark comparing lns8 vs bf8 (E4M3) and lns16 vs bf16 across five operations (round-trip, mul, div, add, sub), broken down by operand magnitude band, with Mann-Whitney significance testing on 100k samples per cell. See the [bench README](examples/bench/README.md) for full methodology and execution details.

#### Per-operation winner (Mann-Whitney p < 0.01, n = 100 000)

| Operation | Winner | Reason |
| --- | --- | --- |
| mul | BF | LNS round-trip quantisation noise outweighs its exact-exponent-add property at these bit widths |
| div | **LNS16**/BF8 | Exact integer subtract on the exponent field; BF must round a full mantissa quotient |
| add | BF | IEEE 754 correctly-rounded add; LNS add is anchored to input scale via spline approximation |
| sub | BF | Same as add |
| round-trip | BF | BF slightly better across all bands |

The div advantage holds across all bands and both bit-widths with no exceptions.


 ![8-bit winner heatmap](examples/bench/results/ops_heatmap_lns8_bf8.png)


![16-bit winner heatmap](examples/bench/results/ops_heatmap_lns16_bf16.png)


#### Numerical tests (lns16 vs bf16)


![numerical results](examples/bench/results/numerical_rel.png)


bf16 wins on all workloads involving accumulation or activation functions (pi²/6, sigmoid, GELU). Both formats fail on the alternating harmonic series (severe cancellation) and geometric progression (dynamic range exhaustion at 8-bit).


### `examples/tinystories/` — TinyStories inference in lns16 and bf16

Runs [Andrej Karpathy's llama2.c](https://github.com/karpathy/llama2.c) TinyStories inference using lns16 and bf16 as drop-in replacements for the original float32 weights. Includes weight converters located in `convert/` for both formats and supports both XF and XMB spline variants for lns16 execution located under `tiny/`.

See [tinystories README](examples/tinystories/README.md) for step-by-step build instructions, model download parameters, conversion steps, and tokenization usage.

---

## RISC-V Hardware Integration

The inline-assembly hardware instructions declared in `lib/lns.hpp` target the LNS custom functional unit developed for **RISC++** (a custom RISC-V soft-core design framework developed at [SPeCS](https://specs.fe.up.pt/), INESC TEC / FEUP).

To decouple the core's architecture development from this library, **the target test suites and toolchain compilation flows reside directly in the RISC++ core repository.** Once this library is installed on your system via `make install`, the RISC++ cross-compilation toolchain flags consume these global headers directly to generate bare-metal ELF validation binaries and BRAM initialization structures for hardware simulation blocks.

---

## Planned: lns32 and lns64

The spline table approach used for lns8 and lns16 becomes impractical at wider formats — the domain of $\log_2(1 \pm 2^x)$ grows significantly and table sizes explode. A different approximation strategy is needed.

Candidates under development and tracking inside `lib/newtonsdd/`:

* Newton's divided differences with non-uniform sample point placement
* Minimax polynomial approximation (Remez algorithm)
* Piecewise Chebyshev approximation

---

## Author

Henrique dos Santos Teixeira
