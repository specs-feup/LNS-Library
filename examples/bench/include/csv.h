#ifndef __CSV_H__
#define __CSV_H__

#include "utils.h"
#include "stats.h"
#include <stdio.h>

// ---------------------------------------------------------------------------
// csv_out — writes all benchmark results into results/results.csv,
// creating the directory if it does not exist.
//
// Two record kinds share one file, distinguished by the "test_kind" column:
//
//   "ops"      — per-interval arithmetic accuracy (bench_ops)
//                columns: format, interval, op, avg_rel, max_rel, avg_abs, max_abs
//
//   "numerical"— single-scalar numerical tests (bench_numerical)
//                columns: format, test, variant, got, expected, abs_err, rel_err
//
// The file is opened once (csv_open), rows are appended, then closed
// (csv_close). Both modules call csv_write_ops / csv_write_scalar as they
// produce results; main controls the open/close lifecycle.
// ---------------------------------------------------------------------------

void csv_open (const char* results_dir);
void csv_close(void);

// Append one row for an arithmetic op benchmark.
// interval_label : e.g. "[1, 2]"
// op_name    : "rt" | "mul" | "div" | "add" | "sub"
void csv_write_ops(
  const char*     fmt_name,
  const char*     interval_label,
  const char*     op_name,
  const op_stats* s
);

// Append one row for a scalar numerical test.
// test_name  : e.g. "geometric_progression"
// variant    : e.g. "lns16" or "bf16"
void csv_write_scalar(
  const char*          fmt_name,
  const char*          test_name,
  const char*          variant,
  const scalar_result* r
);

// ---------------------------------------------------------------------------
// samples.bin — binary file for per-sample error distributions.
//
// Used by plot.py to run Mann-Whitney U tests on op heatmap cells.
//
// File layout
// -----------
//   [ data region ]
//     For each (fmt, interval, op) group, written sequentially:
//       N × f32   abs_samples
//       N × f32   rel_samples
//
//   [ index region ]  — offset stored in last 8 bytes of file
//     u32  n_entries
//     For each entry:
//       char fmt[16]
//       char interval[32]
//       char op[8]
//       u64  data_offset   — byte offset of abs_samples in file
//       u32  count         — N
//
//   [ u64 ] index_offset   — last 8 bytes: byte offset of index region
//
// Reader workflow:
//   1. seek(-8, SEEK_END), read index_offset
//   2. seek(index_offset), read n_entries, then n_entries index records
//   3. for each entry: seek(data_offset), read count f32 abs_samples,
//      then count f32 rel_samples (immediately following)
// ---------------------------------------------------------------------------

// Open samples.bin for writing (called by csv_open internally).
// Close and flush index (called by csv_close internally).
// Individual groups are written via csv_write_ops.
void samples_open (const char* results_dir);
void samples_close(void);

#endif // !__CSV_OUT_H__
