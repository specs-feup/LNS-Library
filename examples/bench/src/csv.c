#include "csv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

static FILE* g_csv     = NULL;
static FILE* g_samples = NULL;

typedef struct {
  char 
    fmt[16],
    band[32],
    op[8];
  u64 data_offset;
  u32 count;
} sample_entry;

static sample_entry* g_index    = NULL;
static u32           g_n_entries = 0;
static u32           g_index_cap = 0;

static void make_dir(const char* dir) {
  if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
    fprintf(stderr, "csv: cannot create '%s': %s\n", dir, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

static FILE* open_file(const char* path, const char* mode) {
  FILE* f = fopen(path, mode);
  if (!f) {
    fprintf(stderr, "csv: cannot open '%s': %s\n", path, strerror(errno));
    exit(EXIT_FAILURE);
  }
  return f;
}

void csv_open(const char* results_dir) {
  make_dir(results_dir);

  char path[512];
  snprintf(path, sizeof(path), "%s/results.csv", results_dir);
  g_csv = open_file(path, "w");

  fprintf(g_csv,
    "test_kind,format,band,op,"
    "avg_rel,max_rel,avg_abs,max_abs,"
    "test_name,variant,got,abs_err,rel_err\n"
  );

  samples_open(results_dir);
}

void csv_close(void) {
  if (g_csv) {
    fclose(g_csv);
    g_csv = NULL;
  }
  samples_close();
}

void samples_open(const char* results_dir) {
  char path[512];
  snprintf(path, sizeof(path), "%s/samples.bin", results_dir);
  g_samples = open_file(path, "wb");
}

void samples_close(void) {
  if (!g_samples)
		return;

  u64 index_offset = (u64)ftell(g_samples);

  fwrite(&g_n_entries, sizeof(u32), 1, g_samples);
  for (u32 i = 0; i < g_n_entries; i++) {
    fwrite(g_index[i].fmt,          1,           16, g_samples);
    fwrite(g_index[i].band,         1,           32, g_samples);
    fwrite(g_index[i].op,           1,            8, g_samples);
    fwrite(&g_index[i].data_offset, sizeof(u64),  1, g_samples);
    fwrite(&g_index[i].count,       sizeof(u32),  1, g_samples);
  }
  fwrite(&index_offset, sizeof(u64), 1, g_samples);

  fclose(g_samples);
  g_samples = NULL;

  free(g_index);
  g_index     = NULL;
  g_n_entries = 0;
  g_index_cap = 0;
}

void csv_write_ops(
  const char*     fmt_name,
  const char*     band_label,
  const char*     op_name,
  const op_stats* s
) {
  if (!g_csv)
		return;

  fprintf(g_csv,
    "ops,%s,\"%s\",%s,"
    "%.8e,%.8e,%.8e,%.8e,"
    ",,,,,\n",
    fmt_name, band_label, op_name,
    (double)stats_avg_rel(s), (double)s->max_rel_err,
    (double)stats_avg_abs(s), (double)s->max_abs_err
  );

  if (!g_samples || !s->count)
		return;

  if (g_n_entries >= g_index_cap) {
    g_index_cap = g_index_cap ? g_index_cap * 2 : 64;
    g_index = (sample_entry*)realloc(g_index, g_index_cap * sizeof(sample_entry));
  }

  sample_entry* e = &g_index[g_n_entries++];
  strncpy(e->fmt,  fmt_name,   sizeof(e->fmt)  - 1); e->fmt[sizeof(e->fmt)-1]   = '\0';
  strncpy(e->band, band_label, sizeof(e->band) - 1); e->band[sizeof(e->band)-1] = '\0';
  strncpy(e->op,   op_name,    sizeof(e->op)   - 1); e->op[sizeof(e->op)-1]     = '\0';
  e->data_offset = (u64)ftell(g_samples);
  e->count       = s->count;

  fwrite(s->abs_samples, sizeof(f32), s->count, g_samples);
  fwrite(s->rel_samples, sizeof(f32), s->count, g_samples);
}

void csv_write_scalar(
  const char*          fmt_name,
  const char*          test_name,
  const char*          variant,
  const scalar_result* r
) {
  if (!g_csv)
		return;

  fprintf(g_csv,
    "numerical,%s,,,,,,,"
    "%s,%s,"
    "%.15e,%.15e,%.15e\n",
    fmt_name,
    test_name, variant,
    r->got, r->abs_err, r->rel_err
  );
}
