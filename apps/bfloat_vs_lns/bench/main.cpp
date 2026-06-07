/*
 * lns_bench — LNS vs BF arithmetic accuracy benchmark
 *
 * usage:  lns_bench <lns8.lns> <lns16.lns> [n_samples] [results_dir]
 *
 *   lns8.lns     — precomputed add/sub lookup table for lns8
 *   lns16.lns    — precomputed add/sub lookup table for lns16
 *   n_samples    — Monte Carlo samples per op per band  (default 100000)
 *   results_dir  — directory for CSV output             (default "results")
 *
 * Output
 * ------
 *   Console  : per-band detail blocks + numerical test summaries
 *   CSV      : results_dir/results.csv  (created if missing)
 *   Plots    : run plot.py after the binary to populate results_dir
 */


#include "lnssim.hpp"

typedef lns<16, 8, 7> lns16;
typedef lns<8,  4, 3> lns8;

#include "bench_ops.h"
#include "bench_ops.cpp"

#include "bench_numerical.h"
#include "bench_numerical.cpp"

#include "csv.h"
#include "csv.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N          100000
#define DEFAULT_RESULTS    "results"

static void usage(const char* prog) {
  fprintf(
    stderr,
    "usage: %s <lns8.lns> <lns16.lns> [n_samples] [results_dir]\n"
    "\n"
    "  lns8.lns    — lookup table for lns8  (from lnssim)\n"
    "  lns16.lns   — lookup table for lns16 (from lnssim)\n"
    "  n_samples   — Monte Carlo samples per op per band (default %d)\n"
    "  results_dir — output directory for CSV + plots (default \"%s\")\n",
    prog, DEFAULT_N, DEFAULT_RESULTS
  );
}

i32 main(i32 argc, const char* argv[]) {
  if (argc < 3) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  const char* lns8_path  = argv[1];
  const char* lns16_path = argv[2];
  u32         n_samples  = (argc > 3) ? (u32)atoi(argv[3]) : DEFAULT_N;
  const char* results    = (argc > 4) ? argv[4]            : DEFAULT_RESULTS;

  if (n_samples == 0) {
    fprintf(stderr, "error: n_samples must be > 0\n");
    return EXIT_FAILURE;
  }

  // Load LNS lookup tables (required before any lns arithmetic).
  lns8_read_tables (lns8_path);
  lns16_read_tables(lns16_path);

  printf("lns_bench  n=%u  results='%s'\n\n", n_samples, results);

  // Open CSV — creates results/ if needed.
  csv_open(results);

  // Run benchmarks.
  bench_ops_run      (n_samples, results);
  bench_numerical_run(results);

  // Flush and close CSV.
  csv_close();

  printf("\nresults written to %s/results.csv\n", results);
  printf("run  python3 plot.py %s  to generate plots\n", results);

  lns_close();
  return EXIT_SUCCESS;
}
