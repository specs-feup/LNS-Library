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
        results_dir/numerical_abs.png
        results_dir/ops_heatmap_lns8_bf8_rel.png
        results_dir/ops_heatmap_lns8_bf8_abs.png
        results_dir/ops_heatmap_lns16_bf16_rel.png
        results_dir/ops_heatmap_lns16_bf16_abs.png
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
import matplotlib.patches as mpatches

import numpy as np
from scipy import stats as scipy_stats

RESULTS_DIR  = sys.argv[1] if len(sys.argv) > 1 else "results"
CSV_PATH     = os.path.join(RESULTS_DIR, "results.csv")
SAMPLES_PATH = os.path.join(RESULTS_DIR, "samples.bin")

ALPHA     = 0.01   # Mann-Whitney significance threshold for heatmap
NUM_TIE   = 0.05   # relative-difference threshold for numerical tie

FMT_COLOR = {
    "lns8":          "#2563eb",
    "bf8":           "#dc2626",
    "lns16":         "#16a34a",
    "lns16_lns32acc":"#15803d",   # darker green for the lns32-acc variant
    "lns16_f32acc":  "#126614",   # darker green for the f32-acc variant
    "bf16":          "#ea580c",
    "bf16_f32acc":   "#c2410c",   # darker orange for the f32-acc variant
}
FMT_MARKER = {
    "lns8":          "o",
    "bf8":           "s",
    "lns16":         "^",
    "lns16_lns32acc":"v",
    "lns16_f32acc":  "f",
    "bf16":          "D",
    "bf16_f32acc":   "P",
}
FMT_HATCH = {
    "lns8":          "",
    "bf8":           "",
    "lns16":         "",
    "lns16_lns32acc":"//",
    "lns16_f32acc":  ".",
    "bf16":          "",
    "bf16_f32acc":   "//",
}

# Canonical display order for the numerical bar chart
NUMERICAL_FMT_ORDER = ["lns8", "bf8", "lns16", "lns16_lns32acc", "lns16_f32acc", "bf16", "bf16_f32acc"]

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


def compute_global_ylims(ops_rows, metric_col, pairs):
    all_vals = []
    for pair_fmts in [fmts for _, fmts in pairs]:
        for fmt in pair_fmts:
            for row in ops_rows:
                if row["format"] == fmt:
                    v = safe_float(row.get(metric_col))
                    if v is not None and v > 0:
                        all_vals.append(v)
    if not all_vals:
        return None, None
    return min(all_vals), max(all_vals)


def plot_ops_metric(ops_rows, metric_col, metric_label, filename_suffix):
    pairs = [
        ("8-bit",  ["lns8",  "bf8"]),
        ("16-bit", ["lns16", "bf16"]),
    ]

    ymin, ymax = compute_global_ylims(ops_rows, metric_col, pairs)
    if ymin is not None:
        log_margin = 0.15
        ymin_plot = 10 ** (math.log10(ymin) - log_margin)
        ymax_plot = 10 ** (math.log10(ymax) + log_margin)
    else:
        ymin_plot = ymax_plot = None

    fig, axes = plt.subplots(
        len(OP_ORDER), len(pairs),
        figsize=(18, 4.5 * len(OP_ORDER)),
        sharex="col",
    )
    fig.suptitle(f"Arithmetic accuracy — {metric_label} by band", fontsize=14, y=1.01)

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

            if ymin_plot is not None:
                ax.set_ylim(ymin_plot, ymax_plot)

            ax.set_xticks(range(len(band_set)))
            ax.set_xticklabels(band_set, rotation=40, ha="right", fontsize=7)
            ax.set_ylabel(metric_label, fontsize=8)
            ax.set_title(f"{pair_label} — {OP_LABELS[op]}", fontsize=9, pad=6)
            ax.grid(True, which="major", alpha=0.25)
            ax.yaxis.set_major_formatter(ticker.LogFormatterMathtext())

            if row_idx == 0:
                ax.legend(fontsize=8)

    fig.tight_layout(pad=2.0, h_pad=3.0, w_pad=3.0)
    save(fig, f"ops_{filename_suffix}.png")


def plot_ops_heatmap(ops_rows, samples, pair_label, fmts, filename, metric_key):
    lns_fmt, bf_fmt = fmts
    metric_col = f"avg_{metric_key}"

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
        for ci, band in enumerate(band_set):
            def get_agg(fmt, _mc=metric_col, _op=op, _band=band):
                match = [r for r in ops_rows
                         if  r["format"]                  == fmt
                         and r["op"].strip()              == _op
                         and r["band"].strip().strip('"') == _band]
                return safe_float(match[0][_mc]) if match else None

            vl = get_agg(lns_fmt)
            vb = get_agg(bf_fmt)
            if vl is None or vb is None:
                continue

            if use_wilcoxon:
                sl = samples.get((lns_fmt, band, op), {}).get(metric_key)
                sb = samples.get((bf_fmt,  band, op), {}).get(metric_key)

                if sl is not None and sb is not None and len(sl) and len(sb):
                    n1, n2 = len(sl), len(sb)
                    u_stat, p = scipy_stats.mannwhitneyu(sl, sb, alternative="two-sided")
                    rank_biserial_r = 1 - (2 * u_stat) / (n1 * n2)
                    print(f"  [wilcoxon] {lns_fmt} vs {bf_fmt} | {op} | {band} | p={p:.4f} | r={rank_biserial_r:.3f} | N={n1},{n2}")

                    if p >= ALPHA or abs(rank_biserial_r) < NUM_TIE:
                        data[ri, ci] = 0.5
                        continue
                else:
                    print(f"  [wilcoxon] MISSING samples for {lns_fmt}/{bf_fmt} | {op} | {band} — falling back to pointwise")

            data[ri, ci] = 0.0 if vl <= vb else 1.0

    fig_w = max(10, n_band * 1.4)
    fig_h = n_ops * 1.4 + 2.5
    fig, ax = plt.subplots(figsize=(fig_w, fig_h))

    cmap = matplotlib.colors.ListedColormap(
        [FMT_COLOR.get(lns_fmt, "#2563eb"),
         "#d1d5db",
         FMT_COLOR.get(bf_fmt,  "#dc2626")]
    )
    ax.imshow(data, cmap=cmap, vmin=0.0, vmax=1.0, aspect="auto")

    ax.set_xticks(range(n_band))
    ax.set_xticklabels(band_set, rotation=45, ha="right", fontsize=9)
    ax.set_yticks(range(n_ops))
    ax.set_yticklabels([OP_LABELS[o] for o in OP_ORDER], fontsize=10)

    metric_label = "avg relative error" if metric_key == "rel" else "avg absolute error"
    sig_note = f"Mann-Whitney p<{ALPHA}" if use_wilcoxon else "pointwise comparison"

    ax.set_title(
        f"{pair_label} — {metric_label}\nwinner per op × band  ({sig_note})",
        fontsize=11,
        pad=12,
    )

    legend_handles = [
        mpatches.Patch(color=FMT_COLOR.get(lns_fmt, "#2563eb"), label=lns_fmt),
        mpatches.Patch(color="#d1d5db",                         label="tie"),
        mpatches.Patch(color=FMT_COLOR.get(bf_fmt,  "#dc2626"), label=bf_fmt),
    ]
    ax.legend(
        handles=legend_handles,
        loc="upper right",
        bbox_to_anchor=(1.0, -0.12),
        ncol=3,
        fontsize=10,
        framealpha=0.9,
    )

    for ri in range(n_ops):
        for ci in range(n_band):
            v = data[ri, ci]
            label = lns_fmt if v < 0.5 else (bf_fmt if v > 0.5 else "tie")
            ax.text(ci, ri, label, ha="center", va="center",
                    fontsize=8, color="white" if v != 0.5 else "black",
                    fontweight="bold")

    fig.tight_layout(pad=2.0)
    save(fig, filename)


def plot_numerical(num_rows, error_col, ylabel, title_suffix, filename):
    """
    Generic numerical bar chart for either rel_err or abs_err.

    lns16 and lns16_lns32acc bars are placed adjacently for direct comparison,
    as are bf16 and bf16_f32acc. Improvement ratios are printed inside the
    acc-variant bars.
    """
    if not num_rows:
        print("  no numerical rows found — skipping")
        return

    tests = collections.OrderedDict()
    for row in num_rows:
        test = row.get("test_name", "").strip()
        fmt  = row.get("format",    "").strip()
        val  = safe_float(row.get(error_col, ""))

        if test and fmt and val is not None:
            if test not in tests:
                tests[test] = {}
            if fmt not in tests[test] or val < tests[test][fmt]:
                tests[test][fmt] = val

    test_names = list(tests.keys())
    fmts_seen  = [f for f in NUMERICAL_FMT_ORDER
                  if any(f in tests[t] for t in test_names)]

    if not fmts_seen:
        print("  no numerical data — skipping")
        return

    x     = np.arange(len(test_names))
    width = 0.8 / len(fmts_seen)

    fig, ax = plt.subplots(figsize=(max(14, len(test_names) * 2.0), 6))

    for i, fmt in enumerate(fmts_seen):
        vals   = [tests[t].get(fmt, float("nan")) for t in test_names]
        offset = (i - len(fmts_seen) / 2.0 + 0.5) * width
        ax.bar(x + offset, vals, width * 0.9,
               label=fmt,
               color=FMT_COLOR.get(fmt, "grey"),
               hatch=FMT_HATCH.get(fmt, ""),
               alpha=0.85,
               edgecolor="white" if not FMT_HATCH.get(fmt) else "#333333")

    # tie annotations between lns/bf baseline pairs
    pairs_for_tie = [("lns8", "bf8"), ("lns16", "bf16")]
    tie: bool = False
    for lns_f, bf_f in pairs_for_tie:
        if lns_f not in fmts_seen or bf_f not in fmts_seen:
            continue

        lns_i = fmts_seen.index(lns_f)
        bf_i  = fmts_seen.index(bf_f)

        for ti, test in enumerate(test_names):
            vl = tests[test].get(lns_f)
            vb = tests[test].get(bf_f)

            if vl is None or vb is None:
                continue
            lo = min(vl, vb)
            if lo == 0:
                continue

            if abs(vl - vb) / lo < NUM_TIE:
                tie = True

                bar_h  = max(vl, vb)
                y_line = bar_h * 1.15

                xl = x[ti] + (lns_i - len(fmts_seen) / 2.0 + 0.5) * width
                xr = x[ti] + (bf_i  - len(fmts_seen) / 2.0 + 0.5) * width

                ax.annotate("", xy=(xr, y_line), xytext=(xl, y_line),
                            arrowprops=dict(arrowstyle="-", color="black", lw=1.2))
                ax.text((xl + xr) / 2, y_line * 1.05, "~tie",
                        ha="center", va="bottom", fontsize=7)

    # improvement ratio printed inside the acc-variant bars
    acc_pairs = [("lns16", "lns16_lns32acc"), ("bf16", "bf16_f32acc")]
    for base_f, acc_f in acc_pairs:
        if base_f not in fmts_seen or acc_f not in fmts_seen:
            continue

        acc_i = fmts_seen.index(acc_f)

        for ti, test in enumerate(test_names):
            v_base = tests[test].get(base_f)
            v_acc  = tests[test].get(acc_f)
            if v_base is None or v_acc is None or v_base == 0:
                continue

            if v_acc < v_base * (1 - NUM_TIE):
                ratio = v_base / v_acc
                x_bar = x[ti] + (acc_i - len(fmts_seen) / 2.0 + 0.5) * width
                ax.text(x_bar, v_acc * 1.3, f"×{ratio:.1f}",
                        ha="center", va="top", fontsize=7,
                        color="black", fontweight="bold")

    ax.set_yscale("log")
    ax.set_xticks(x)
    ax.set_xticklabels(test_names, rotation=35, ha="right", fontsize=9)
    ax.set_ylabel(ylabel, fontsize=10)
    ax.set_title(
        f"Numerical tests — {title_suffix}\n"
        f"(~tie = rel. diff < {NUM_TIE:.0%}; ×N on top of bar = acc-variant improvement (base variant / acc variant))",
        fontsize=12,
        pad=10,
    )
    ax.legend(fontsize=9)
    ax.grid(True, axis="y", which="major", alpha=0.3)
    ax.yaxis.set_major_formatter(ticker.LogFormatterMathtext())
    fig.tight_layout(pad=2.0)
    save(fig, filename)


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

    for pair_label, fmts, slug in [
        ("8-bit  lns8 vs bf8",   ["lns8",  "bf8"],  "lns8_bf8"),
        ("16-bit  lns16 vs bf16", ["lns16", "bf16"], "lns16_bf16"),
    ]:
        plot_ops_heatmap(ops_rows, samples, pair_label, fmts,
                         f"ops_heatmap_{slug}_rel.png", "rel")
        plot_ops_heatmap(ops_rows, samples, pair_label, fmts,
                         f"ops_heatmap_{slug}_abs.png", "abs")

    plot_numerical(num_rows, "rel_err",
                   "relative error  (lower = better)",
                   "relative error by format",
                   "numerical_rel.png")
    plot_numerical(num_rows, "abs_err",
                   "absolute error  (lower = better)",
                   "absolute error by format",
                   "numerical_abs.png")

    print("done.")


if __name__ == "__main__":
    main()
