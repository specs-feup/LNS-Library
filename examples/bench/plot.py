"""
plot.py — Visualise lns_bench results.

Usage:
    python3 plot.py [results_dir]

    results_dir defaults to "results".  All PNG plots are written there.

Reads:  results_dir/results.csv
        results_dir/samples.bin
Writes: results_dir/ops_avg_rel.png
        results_dir/ops_avg_abs.png
        results_dir/numerical_rel.png
        results_dir/ops_heatmap_lns8_bf8.png
        results_dir/ops_heatmap_lns16_bf16.png
"""

import sys
import os
import csv
import math
import struct
import collections
import matplotlib

matplotlib.use("Agg")

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
from scipy import stats as scipy_stats

RESULTS_DIR  = sys.argv[1] if len(sys.argv) > 1 else "results"
CSV_PATH     = os.path.join(RESULTS_DIR, "results.csv")
SAMPLES_PATH = os.path.join(RESULTS_DIR, "samples.bin")

ALPHA     = 0.01   # Mann-Whitney significance threshold for heatmap
NUM_TIE   = 0.05   # relative-difference threshold for numerical tie

FMT_COLOR = {
    "lns8":  "#2563eb",
    "bf8":   "#dc2626",
    "lns16": "#16a34a",
    "bf16":  "#ea580c",
}
FMT_MARKER = {
    "lns8":  "o",
    "bf8":   "s",
    "lns16": "^",
    "bf16":  "D",
}
OP_ORDER  = ["rt", "mul", "div", "add", "sub"]
OP_LABELS = {"rt": "round-trip", "mul": "mul", "div": "div",
             "add": "add", "sub": "sub"}
OP_PRIMARY = {"rt": "rel", "mul": "rel", "div": "rel",
              "add": "abs", "sub": "abs"}


def load_csv(path):
    ops_rows, numerical_rows = [], []

    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            kind = row.get("test_kind", "").strip()

            if kind == "ops":
                ops_rows.append(row)
            elif kind == "numerical":
                numerical_rows.append(row)

    return ops_rows, numerical_rows


def load_samples(path):
    """
    Returns dict keyed by (fmt, band, op) ->
        {"abs": np.ndarray, "rel": np.ndarray}
    Returns {} if file is missing.
    """
    if not os.path.exists(path):
        return {}

    out = {}
    with open(path, "rb") as f:
        f.seek(-8, 2)
        index_offset = struct.unpack("<Q", f.read(8))[0]

        f.seek(index_offset)
        n_entries = struct.unpack("<I", f.read(4))[0]

        entries = []
        for _ in range(n_entries):
            fmt         = f.read(16).rstrip(b"\x00").decode()
            band        = f.read(32).rstrip(b"\x00").decode()
            op          = f.read(8).rstrip(b"\x00").decode()
            data_offset = struct.unpack("<Q", f.read(8))[0]
            count       = struct.unpack("<I", f.read(4))[0]
            entries.append((fmt, band, op, data_offset, count))

        for fmt, band, op, data_offset, count in entries:
            f.seek(data_offset)
            raw = f.read(count * 4 * 2)
            arr = np.frombuffer(raw, dtype=np.float32)
            out[(fmt, band, op)] = {
                "abs": arr[:count],
                "rel": arr[count:],
            }

    return out


def safe_float(v):
    try:
        x = float(v)
        return x if math.isfinite(x) else None

    except (ValueError, TypeError):
        return None


def save(fig, name):
    path = os.path.join(RESULTS_DIR, name)
    fig.savefig(path, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"  wrote {path}")


def plot_ops_metric(ops_rows, metric_col, metric_label, filename_suffix):
    pairs = [
        ("8-bit",  ["lns8",  "bf8"]),
        ("16-bit", ["lns16", "bf16"]),
    ]

    fig, axes = plt.subplots(
        len(OP_ORDER), len(pairs),
        figsize=(14, 3.2 * len(OP_ORDER)),
        sharex="col",
    )
    fig.suptitle(f"Arithmetic accuracy — {metric_label} by band", fontsize=13, y=1.01)

    for col, (pair_label, fmts) in enumerate(pairs):
        band_set, seen = [], set()
        for row in ops_rows:
            if row["format"] in fmts:
                b = row["band"].strip().strip('"')

                if b not in seen:
                    seen.add(b)
                    band_set.append(b)

        for row_idx, op in enumerate(OP_ORDER):
            ax = axes[row_idx][col]
            for fmt in fmts:
                vals = []

                for band in band_set:
                    match = [r for r in ops_rows
                             if r["format"] == fmt
                             and r["op"].strip() == op
                             and r["band"].strip().strip('"') == band]

                    vals.append(safe_float(match[0][metric_col]) if match else None)

                yv = [v if v is not None else float("nan") for v in vals]
                ax.semilogy(range(len(band_set)), yv,
                            label=fmt,
                            color=FMT_COLOR.get(fmt, "grey"),
                            marker=FMT_MARKER.get(fmt, "o"),
                            linewidth=1.6, markersize=5)

            ax.set_xticks(range(len(band_set)))
            ax.set_xticklabels(band_set, rotation=35, ha="right", fontsize=7)
            ax.set_ylabel(metric_label, fontsize=8)
            ax.set_title(f"{pair_label} — {OP_LABELS[op]}", fontsize=9)
            ax.grid(True, which="both", alpha=0.25)
            ax.yaxis.set_major_formatter(ticker.LogFormatterMathtext())

            if row_idx == 0:
                ax.legend(fontsize=8)

    fig.tight_layout()
    save(fig, f"ops_{filename_suffix}.png")


def plot_ops_heatmap(ops_rows, samples, pair_label, fmts, filename):
    lns_fmt, bf_fmt = fmts

    band_set, seen = [], set()
    for row in ops_rows:
        if row["format"] in fmts:
            b = row["band"].strip().strip('"')
            if b not in seen:
                seen.add(b)
                band_set.append(b)

    n_ops  = len(OP_ORDER)
    n_band = len(band_set)
    data   = np.full((n_ops, n_band), 0.5)

    use_wilcoxon = bool(samples)

    for ri, op in enumerate(OP_ORDER):
        key     = OP_PRIMARY[op]
        metric  = f"avg_{key}"

        for ci, band in enumerate(band_set):
            def get_agg(fmt):
                match = [r for r in ops_rows
                         if r["format"] == fmt
                         and r["op"].strip() == op
                         and r["band"].strip().strip('"') == band]
                return safe_float(match[0][metric]) if match else None

            vl = get_agg(lns_fmt)
            vb = get_agg(bf_fmt)
            if vl is None or vb is None:
                continue

            if use_wilcoxon:
                sl = samples.get((lns_fmt, band, op), {}).get(key)
                sb = samples.get((bf_fmt,  band, op), {}).get(key)
                if sl is not None and sb is not None and len(sl) and len(sb):
                    _, p = scipy_stats.mannwhitneyu(sl, sb, alternative="two-sided")
                    if p >= ALPHA:
                        data[ri, ci] = 0.5
                        continue

            data[ri, ci] = 0.0 if vl <= vb else 1.0

    fig, ax = plt.subplots(figsize=(max(6, n_band * 1.1), n_ops * 0.9 + 1.2))
    cmap = matplotlib.colors.ListedColormap(
        [FMT_COLOR.get(lns_fmt, "#2563eb"),
         "#d1d5db",
         FMT_COLOR.get(bf_fmt,  "#dc2626")]
    )
    ax.imshow(data, cmap=cmap, vmin=0.0, vmax=1.0, aspect="auto")

    ax.set_xticks(range(n_band))
    ax.set_xticklabels(band_set, rotation=40, ha="right", fontsize=8)
    ax.set_yticks(range(n_ops))
    ax.set_yticklabels([OP_LABELS[o] for o in OP_ORDER], fontsize=9)

    sig_note = f"Mann-Whitney p<{ALPHA}" if use_wilcoxon else "pointwise comparison"
    ax.set_title(
        f"{pair_label} winner per op × band  ({sig_note})\n"
        f"(primary: avg_rel for rt/mul/div,  avg_abs for add/sub)\n"
        f"■ {lns_fmt}  □ {bf_fmt}  ░ tie",
        fontsize=10
    )

    for ri in range(n_ops):
        for ci in range(n_band):
            v = data[ri, ci]
            label = lns_fmt if v < 0.5 else (bf_fmt if v > 0.5 else "tie")
            ax.text(ci, ri, label, ha="center", va="center",
                    fontsize=7, color="white" if v != 0.5 else "black",
                    fontweight="bold")

    fig.tight_layout()
    save(fig, filename)


def plot_numerical(num_rows):
    if not num_rows:
        print("  no numerical rows found — skipping")
        return

    tests = collections.OrderedDict()
    for row in num_rows:
        test    = row.get("test_name", "").strip()
        fmt     = row.get("format",    "").strip()
        rel_err = safe_float(row.get("rel_err", ""))

        if test and fmt and rel_err is not None:
            if test not in tests:
                tests[test] = {}
            if fmt not in tests[test] or rel_err < tests[test][fmt]:
                tests[test][fmt] = rel_err

    test_names = list(tests.keys())
    fmts_seen  = [f for f in ["lns8", "bf8", "lns16", "bf16"]
                  if any(f in tests[t] for t in test_names)]

    if not fmts_seen:
        print("  no numerical data — skipping")
        return

    pairs = [("lns8", "bf8"), ("lns16", "bf16")]

    x     = np.arange(len(test_names))
    width = 0.8 / len(fmts_seen)

    fig, ax = plt.subplots(figsize=(max(9, len(test_names) * 1.4), 5))

    for i, fmt in enumerate(fmts_seen):
        vals   = [tests[t].get(fmt, float("nan")) for t in test_names]
        offset = (i - len(fmts_seen) / 2.0 + 0.5) * width
        ax.bar(x + offset, vals, width * 0.9,
               label=fmt,
               color=FMT_COLOR.get(fmt, "grey"),
               alpha=0.85)

    # Annotate ties: for each (lns, bf) pair, mark cells where relative
    # difference < NUM_TIE.
    for lns_f, bf_f in pairs:
        if lns_f not in fmts_seen or bf_f not in fmts_seen:
            continue

        lns_i  = fmts_seen.index(lns_f)
        bf_i   = fmts_seen.index(bf_f)

        for ti, test in enumerate(test_names):
            vl = tests[test].get(lns_f)
            vb = tests[test].get(bf_f)

            if vl is None or vb is None:
                continue

            lo = min(vl, vb)
            if lo == 0:
                continue

            if abs(vl - vb) / lo < NUM_TIE:
                # Draw a bracket above both bars.
                bar_h  = max(vl, vb)
                y_line = bar_h * 1.15
                xl     = x[ti] + (lns_i - len(fmts_seen) / 2.0 + 0.5) * width
                xr     = x[ti] + (bf_i  - len(fmts_seen) / 2.0 + 0.5) * width
                ax.annotate("", xy=(xr, y_line), xytext=(xl, y_line),
                            arrowprops=dict(arrowstyle="-", color="black", lw=1.2))
                ax.text((xl + xr) / 2, y_line * 1.05, "~tie",
                        ha="center", va="bottom", fontsize=7)

    ax.set_yscale("log")
    ax.set_xticks(x)
    ax.set_xticklabels(test_names, rotation=30, ha="right", fontsize=9)
    ax.set_ylabel("relative error  (lower = better)", fontsize=10)
    ax.set_title(
        f"Numerical tests — relative error by format\n"
        f"(~tie = rel. diff < {NUM_TIE:.0%})",
        fontsize=12
    )
    ax.legend(fontsize=9)
    ax.grid(True, axis="y", which="both", alpha=0.3)
    ax.yaxis.set_major_formatter(ticker.LogFormatterMathtext())
    fig.tight_layout()
    save(fig, "numerical_rel.png")


def main():
    if not os.path.exists(CSV_PATH):
        print(f"error: {CSV_PATH} not found — run lns_bench first")
        sys.exit(1)

    os.makedirs(RESULTS_DIR, exist_ok=True)
    print(f"reading {CSV_PATH}")

    ops_rows, num_rows = load_csv(CSV_PATH)
    print(f"  {len(ops_rows)} ops rows,  {len(num_rows)} numerical rows")

    samples = load_samples(SAMPLES_PATH)
    if samples:
        print(f"  {len(samples)} sample groups loaded from {SAMPLES_PATH}")
    else:
        print(f"  {SAMPLES_PATH} not found — heatmap will use pointwise comparison")

    print("generating plots...")
    plot_ops_metric(ops_rows, "avg_rel", "avg relative error", "avg_rel")
    plot_ops_metric(ops_rows, "avg_abs", "avg absolute error", "avg_abs")
    plot_ops_heatmap(ops_rows, samples, "8-bit  lns8 vs bf8",
                     ["lns8", "bf8"],   "ops_heatmap_lns8_bf8.png")
    plot_ops_heatmap(ops_rows, samples, "16-bit  lns16 vs bf16",
                     ["lns16", "bf16"], "ops_heatmap_lns16_bf16.png")
    plot_numerical(num_rows)
    print("done.")

if __name__ == "__main__":
    main()
