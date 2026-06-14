import sys
import os
import csv
import math
import struct
import collections
import matplotlib

import re as _re

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
    "lns8_lns32acc": "#2440de",
    "lns8_f32acc":   "#2126b5",
    "bf8":           "#dc2626",
    "bf8_f32acc":   "#b40F26",

    "lns16":         "#16a34a",
    "lns16_lns32acc":"#15803d",
    "lns16_f32acc":  "#126614",
    "bf16":          "#ea580c",
    "bf16_f32acc":   "#c2410c",
}

FMT_MARKER = {
    "lns8":          "o",
    "lns8":          "^",
    "lns8_lns32acc": "v",
    "lns8_f32acc":   "D",
    "bf8":           "D",
    "bf8_f32acc":    "P",
    
    "lns16":         "^",
    "lns16_lns32acc":"v",
    "lns16_f32acc":  "D",
    "bf16":          "D",
    "bf16_f32acc":   "P",
}

FMT_HATCH = {
    "lns8":          "",
    "lns8_lns32acc": "//",
    "lns8_f32acc":   ".",
    "bf8":           "",
    "bf8_f32acc":    "//",

    "lns16":         "",
    "lns16_lns32acc":"//",
    "lns16_f32acc":  ".",
    "bf16":          "",
    "bf16_f32acc":   "//",
}

# Canonical display order for the numerical bar chart
NUMERICAL_FMT_ORDER = [
    "lns8",
    "bf8",
    "lns8_lns32acc", "lns8_f32acc",
    "bf8_f32acc",

    "lns16", 
    "bf16", 
    "lns16_lns32acc", "lns16_f32acc",
    "bf16_f32acc"
]

OP_ORDER  = ["rt", "mul", "div", "add", "sub"]
OP_LABELS = {"rt": "round-trip", "mul": "mul", "div": "div",
             "add": "add", "sub": "sub"}
OP_PRIMARY = {"rt": "rel", "mul": "rel", "div": "rel",
              "add": "abs", "sub": "abs"}

# All formats shown together in the combined ops plot
ALL_FMTS = ["lns8", "bf8", "lns16", "bf16"]

FS_TIE      = 12
FS_LABEL    = 20
FS_TICK     = 14
FS_TITLE    = 22   # panel title  
FS_YTICK    = 20   # op labels    
FS_XTICK    = 18   # band labels  
FS_CELL     = 24   # in-cell fmt  
FS_LEGEND   = 10   # legend       
FS_SUPTITLE = 26
FS_ANNOT    = 7

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


# ---------------------------------------------------------------------------
# Combined ops plot: rows = ops, left col = rel error, right col = abs error
# All 4 formats (lns8, bf8, lns16, bf16) shown together in every panel.
# ---------------------------------------------------------------------------

def norm_band(raw):
    """Strip quotes/whitespace and remove all internal spaces so [1, 2]==[1,2]."""
    return raw.strip().strip('"').replace(" ", "")

def band_sort_key(nb):
    # Special case: the sub-1 band like "[2^(-p+1),1]" — sort before everything
    if "^" in nb and nb.startswith("[2^(-"):
        return -1.0
    # General case: extract the lower bound (first number)
    nums = _re.findall(r"\d+(?:\.\d+)?", nb)
    try:
        return float(nums[0]) if nums else 0.0

    except Exception:
        return 0.0

def plot_ops_combined(ops_rows):
    """One figure: n_ops rows × 2 cols (rel | abs). All formats overlaid."""

    # Per-format band lists using normalized names; track raw->norm mapping for lookup
    fmt_bands    = {}   # fmt -> [normalized band, ...]
    row_norm_map = {}   # (format, raw_band) -> normalized band  (for fast lookup later)

    for fmt in ALL_FMTS:
        bs, seen = [], set()

        for row in ops_rows:
            if row["format"] == fmt:
                raw = row["band"]
                nb  = norm_band(raw)
                row_norm_map[(fmt, raw)] = nb

                if nb not in seen:
                    seen.add(nb)
                    bs.append(nb)

        fmt_bands[fmt] = bs

    # Union, sorted numerically by the lower bound of the range
    seen_union, all_norm = set(), []
    for fmt in ALL_FMTS:
        for nb in fmt_bands[fmt]:
            if nb not in seen_union:
                seen_union.add(nb)
                all_norm.append(nb)
 
    union_bands = sorted(all_norm, key=band_sort_key)
    band_idx    = {nb: i for i, nb in enumerate(union_bands)}
    n_union     = len(union_bands)

    metrics = [
        ("avg_rel", "avg relative error"),
        ("avg_abs", "avg absolute error"),
    ]

    # Compute global y-limits per metric
    global_ylims = {}
    for metric_col, _ in metrics:
        all_vals = []
        for fmt in ALL_FMTS:
            for row in ops_rows:
                if row["format"] == fmt:
                    v = safe_float(row.get(metric_col))
                    if v is not None and v > 0:
                        all_vals.append(v)
        if all_vals:
            log_margin = 0.15
            global_ylims[metric_col] = (
                10 ** (math.log10(min(all_vals)) - log_margin),
                10 ** (math.log10(max(all_vals)) + log_margin),
            )
        else:
            global_ylims[metric_col] = (None, None)

    n_rows = len(OP_ORDER)
    n_cols = 2  # left = rel, right = abs
 
    fig, axes = plt.subplots(
        n_rows, n_cols,
        figsize=(max(26, n_union * 1.2), 5 * n_rows),
        sharex="col",
    )
    fig.suptitle(
        "Arithmetic accuracy — 8-bit & 16-bit formats",
        fontsize=FS_TITLE + 4,
        y=1.01,
    )

    for col_idx, (metric_col, metric_label) in enumerate(metrics):
        ymin_plot, ymax_plot = global_ylims[metric_col]

        for row_idx, op in enumerate(OP_ORDER):
            ax = axes[row_idx][col_idx]

            for fmt in ALL_FMTS:
                xs, ys = [], []

                for nb in fmt_bands[fmt]:
                    match = [
                        r for r in ops_rows
                        if r["format"] == fmt
                        and r["op"].strip() == op
                        and norm_band(r["band"]) == nb
                    ]

                    v = safe_float(match[0][metric_col]) if match else None
                    xs.append(band_idx[nb])
                    ys.append(v if v is not None else float("nan"))

                ax.semilogy(
                    xs, ys,
                    label=fmt,
                    color=FMT_COLOR.get(fmt, "grey"),
                    marker=FMT_MARKER.get(fmt, "o"),
                    linewidth=2.0,
                    markersize=6,
                )

            if ymin_plot is not None:
                ax.set_ylim(ymin_plot, ymax_plot)

            ax.set_xlim(-0.5, n_union - 0.5)
            ax.set_xticks(range(n_union))
            ax.set_xticklabels(union_bands, rotation=40, ha="right", fontsize=FS_TICK)
            ax.set_ylabel(metric_label, fontsize=FS_LABEL)
            ax.tick_params(axis="y", labelsize=FS_TICK)

            side = "relative error" if col_idx == 0 else "absolute error"
            ax.set_title(
                f"{OP_LABELS[op]} — {side}",
                fontsize=FS_TITLE,
                pad=6,
            )
            ax.grid(True, which="major", alpha=0.25)
            ax.yaxis.set_major_formatter(ticker.LogFormatterMathtext())

            if row_idx == 0:
                ax.legend(fontsize=FS_LEGEND)

    fig.tight_layout(pad=2.0, h_pad=3.0, w_pad=3.0)
    save(fig, "ops_errors.png")


# ---------------------------------------------------------------------------
# Combined heatmap: 2×2 grid — rows = 8-bit / 16-bit, cols = rel / abs
# All label fontsizes and in-cell format label fontsize doubled.
# ---------------------------------------------------------------------------

def plot_ops_heatmap_combined(ops_rows, samples):
    """
    One figure with 4 panels:
        col 0 = relative error   col 1 = absolute error
        row 0 = 8-bit (lns8 vs bf8)
        row 1 = 16-bit (lns16 vs bf16)
    """

    pairs = [
        ("8-bit  lns8 vs bf8",   "lns8",  "bf8"),
        ("16-bit  lns16 vs bf16", "lns16", "bf16"),
    ]
    metric_keys = [
        ("rel", "avg relative error"),
        ("abs", "avg absolute error"),
    ]

    use_wilcoxon = bool(samples)

    # Collect band sets per pair (both pairs should share the same bands)
    band_sets = {}
    for pair_label, lns_fmt, bf_fmt in pairs:
        bs, seen = [], set()

        for row in ops_rows:
            if row["format"] in (lns_fmt, bf_fmt):
                b = row["band"].strip().strip('"')

                if b not in seen:
                    seen.add(b)
                    bs.append(b)

        band_sets[pair_label] = bs

    n_ops = len(OP_ORDER)

    n_band_max = max(len(bs) for bs in band_sets.values())
    fig_w = max(28, n_band_max * 1.6 * 2 + 4)
    fig_h = n_ops * 1.8 * 2 + 4

    fig, axes = plt.subplots(
        2, 2,
        figsize=(fig_w, fig_h),
    )
    fig.suptitle(
        "Heatmap — winner per op × band  (relative error | absolute error)",
        fontsize=FS_SUPTITLE,
        y=1.01,
    )

    for row_idx, (pair_label, lns_fmt, bf_fmt) in enumerate(pairs):
        band_set = band_sets[pair_label]
        n_band   = len(band_set)

        for col_idx, (metric_key, metric_label) in enumerate(metric_keys):
            metric_col = f"avg_{metric_key}"
            ax = axes[row_idx][col_idx]

            data = np.full((n_ops, n_band), 0.5)

            for ri, op in enumerate(OP_ORDER):
                for ci, band in enumerate(band_set):
                    def get_agg(fmt, _mc=metric_col, _op=op, _band=band):
                        match = [
                            r for r in ops_rows
                            if  r["format"]                  == fmt
                            and r["op"].strip()              == _op
                            and r["band"].strip().strip('"') == _band
                        ]
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

                            if p >= ALPHA or abs(rank_biserial_r) < NUM_TIE:
                                data[ri, ci] = 0.5
                                continue

                    data[ri, ci] = 0.0 if vl <= vb else 1.0

            cmap = matplotlib.colors.ListedColormap([
                FMT_COLOR.get(lns_fmt, "#2563eb"),
                "#d1d5db",
                FMT_COLOR.get(bf_fmt,  "#dc2626"),
            ])
            ax.imshow(data, cmap=cmap, vmin=0.0, vmax=1.0, aspect="auto")

            ax.set_xticks(range(n_band))
            ax.set_xticklabels(band_set, rotation=45, ha="right", fontsize=FS_XTICK)
            ax.set_yticks(range(n_ops))
            ax.set_yticklabels([OP_LABELS[o] for o in OP_ORDER], fontsize=FS_YTICK)

            sig_note = f"Mann-Whitney p<{ALPHA}" if use_wilcoxon else "pointwise"
            ax.set_title(
                f"{pair_label}\n{metric_label}  ({sig_note})",
                fontsize=FS_TITLE,
                pad=10,
            )

            for ri in range(n_ops):
                for ci in range(n_band):
                    v = data[ri, ci]
                    is_tie     = (v == 0.5)
                    cell_label = lns_fmt if v < 0.5 else (bf_fmt if v > 0.5 else "tie")

                    ax.text(
                        ci, ri, cell_label,
                        ha="center", va="center",
                        fontsize=FS_CELL,
                        color="white" if not is_tie else "black",
                        fontweight="bold",
                    )

            legend_handles = [
                mpatches.Patch(color=FMT_COLOR.get(lns_fmt, "#2563eb"), label=lns_fmt),
                mpatches.Patch(color="#d1d5db",                         label="tie"),
                mpatches.Patch(color=FMT_COLOR.get(bf_fmt,  "#dc2626"), label=bf_fmt),
            ]
            ax.legend(
                handles=legend_handles,
                loc="upper right",
                bbox_to_anchor=(1.0, -0.18),
                ncol=3,
                fontsize=FS_LEGEND,
                framealpha=0.9,
            )

    fig.tight_layout(pad=2.5, h_pad=4.0, w_pad=3.0)
    save(fig, "ops_heatmap_combined_rel_abs.png")


def plot_numerical(num_rows, error_col, ylabel, title_suffix, filename):
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
    fmts_seen  = [
        f 
        for f in NUMERICAL_FMT_ORDER
        if any(f in tests[t] for t in test_names)
    ]

    if not fmts_seen:
        print("  no numerical data — skipping")
        return

    x     = np.arange(len(test_names))
    width = 0.8 / len(fmts_seen)

    fig, ax = plt.subplots(figsize=(max(14, len(test_names) * 2.0), 10))

    for i, fmt in enumerate(fmts_seen):
        vals   = [
            tests[t].get(fmt, float("nan"))
            for t in test_names
        ]
        offset = (i - len(fmts_seen) / 2.0 + 0.5) * width

        ax.bar(x + offset, vals, width * 0.9,
               label=fmt,
               color=FMT_COLOR.get(fmt, "grey"),
               hatch=FMT_HATCH.get(fmt, ""),
               alpha=0.85,
               edgecolor="white" if not FMT_HATCH.get(fmt) else "#333333")

    pairs_for_tie = [("lns8", "bf8"), ("lns16", "bf16")]
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
                bar_h  = max(vl, vb)
                y_line = bar_h * 1.15

                xl = x[ti] + (lns_i - len(fmts_seen) / 2.0 + 0.5) * width
                xr = x[ti] + (bf_i  - len(fmts_seen) / 2.0 + 0.5) * width

                ax.annotate(
                    "", xy=(xr, y_line), xytext=(xl, y_line),
                    arrowprops=dict(arrowstyle="-", color="black", lw=1.2)
                )
                ax.text(
                    (xl + xr) / 2, y_line * 1.05, "~tie",
                    ha="center", va="bottom", fontsize=FS_TIE
                )

    acc_pairs = [
        ("lns8", "lns8_lns32acc"),
        ("bf8", "bf8_f32acc"),
        ("lns16", "lns16_lns32acc"),
        ("bf16", "bf16_f32acc")
    ]

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
                x_bar = (
                    x[ti] + (
                        acc_i - len(fmts_seen) / 2.0 + (
                            0.3 if acc_f in ("bf8_f32acc", "bf16_f32acc") else 0
                        )
                    ) * width
                )

                ax.text(x_bar, v_acc * 1.3, f"×{ratio:.1f}",
                        ha="center", va="top", fontsize=FS_ANNOT,
                        color="black", fontweight="bold")

    ax.set_yscale("log")
    ax.set_xticks(x)
    ax.set_xticklabels(test_names, rotation=35, ha="right", fontsize=FS_TICK)
    ax.set_ylabel(ylabel, fontsize=FS_LABEL)
    ax.tick_params(axis="y", labelsize=FS_TICK)
    ax.set_title(
        f"Numerical tests — {title_suffix}\n"
        f"(~tie = rel. diff < {NUM_TIE:.0%}; ×N on top of bar = acc-variant improvement (base variant / acc variant))",
        fontsize=FS_TITLE,
        pad=10,
    )
    ax.legend(fontsize=FS_LEGEND)
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

    # Single combined ops figure: rel error (left) | abs error (right), all formats
    plot_ops_combined(ops_rows)

    # Single combined heatmap: 2 rows (8-bit / 16-bit) × 2 cols (rel | abs)
    plot_ops_heatmap_combined(ops_rows, samples)

    plot_numerical(
        num_rows,
        "rel_err",
        "relative error  (lower = better)",
        "relative error by format",
        "numerical_rel.png"
    )
    plot_numerical(
        num_rows,
        "abs_err",
        "absolute error  (lower = better)",
        "absolute error by format",
        "numerical_abs.png"
    )

    print("done.")


if __name__ == "__main__":
    main()
