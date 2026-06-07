#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <lnssim.hpp>

using lns16 = lns<16, 8, 7>;

struct Config {
  i32 
    dim,
    hidden_dim,
    n_layers,
    n_heads,
    n_kv_heads,
    vocab_size,
    seq_len;
};

void convert_tokenizer(const char* input_path, const char* output_path, i32 vocab_size) {
  FILE* fin = fopen(input_path, "rb");
  if (!fin) {
    fprintf(stderr, "couldn't open tokenizer %s\n", input_path);
    exit(EXIT_FAILURE);
  }

  FILE* fout = fopen(output_path, "wb");
  if (!fout) {
    fprintf(stderr, "couldn't open output %s\n", output_path);
    exit(EXIT_FAILURE);
  }

  // copy max_token_length as-is
  i32 max_token_length;
  if (fread(&max_token_length, sizeof(i32), 1, fin) != 1) {
    fprintf(stderr, "failed read\n");
    exit(EXIT_FAILURE);
  }

  if (fwrite(&max_token_length, sizeof(i32), 1, fout) != 1) {
    fprintf(stderr, "failed write\n");
    exit(EXIT_FAILURE);
  }

  for (i32 i = 0; i < vocab_size; i++) {
    f32 score_f32;
    if (fread(&score_f32, sizeof(f32), 1, fin) != 1) {
      fprintf(stderr, "failed read\n");
      exit(EXIT_FAILURE);
    }
    lns16 score_lns = lns16(score_f32);
    if (fwrite(&score_lns, sizeof(lns16), 1, fout) != 1) {
      fprintf(stderr, "failed write\n");
      exit(EXIT_FAILURE);
    }

    i32 len;
    if (fread(&len, sizeof(i32), 1, fin) != 1) {
      fprintf(stderr, "failed read\n");
      exit(EXIT_FAILURE);
    }

    if (fwrite(&len, sizeof(i32), 1, fout) != 1) {
      fprintf(stderr, "failed write\n");
      exit(EXIT_FAILURE);
    }

    char* token = (char*)malloc(len);
    if (fread(token, len, 1, fin) != 1) {
      fprintf(stderr, "failed read\n");
      exit(EXIT_FAILURE);
    }

    if (fwrite(token, len, 1, fout) != 1) {
      fprintf(stderr, "failed write\n");
      exit(EXIT_FAILURE);
    }

    free(token);
  }

  fclose(fin);
  fclose(fout);
  printf("tokenizer: %s -> %s\n", input_path, output_path);
}

void convert_model(const char* input_path, const char* output_path) {
  FILE* fin = fopen(input_path, "rb");
  if (!fin) {
    fprintf(stderr, "couldn't open model %s\n", input_path);
    exit(EXIT_FAILURE);
  }

  FILE* fout = fopen(output_path, "wb");
  if (!fout) {
    fprintf(stderr, "couldn't open output %s\n", output_path);
    exit(EXIT_FAILURE);
  }

  // read and write config header as-is
  Config cfg;
  if (fread(&cfg, sizeof(Config), 1, fin) != 1) {
    fprintf(stderr, "failed read config\n");
    exit(EXIT_FAILURE);
  }
  
  if (fwrite(&cfg, sizeof(Config), 1, fout) != 1) {
    fprintf(stderr, "failed write config\n");
    exit(EXIT_FAILURE);
  }

  printf(
    "config: dim=%d hidden=%d layers=%d heads=%d kv_heads=%d vocab=%d seq=%d\n",
    cfg.dim, cfg.hidden_dim, cfg.n_layers, cfg.n_heads, cfg.n_kv_heads,
    abs(cfg.vocab_size), cfg.seq_len
  );

  // stream all weights through lns16 conversion
  const i32 BATCH = 4096;
  f32   buf_f32[BATCH];
  lns16 buf_lns[BATCH];

  size_t n;
  size_t total = 0;

  while ((n = fread(buf_f32, sizeof(f32), BATCH, fin)) > 0) {
    for (size_t i = 0; i < n; i++)
      buf_lns[i] = lns16(buf_f32[i]);

    if (fwrite(buf_lns, sizeof(lns16), n, fout) != n) {
      fprintf(stderr, "failed write\n");
      exit(EXIT_FAILURE);
    }

    total += n;
  }

  fclose(fin);
  fclose(fout);
  printf("model: %s -> %s (%zu weights converted)\n", input_path, output_path, total);
}

void print_usage() {
  fprintf(stderr,
    "usage:\n"
    "  <table_path.lns> model      <input.bin> <output.bin>\n"
    "  <table_path.lns> tokenizer  <input.bin> <vocab_size> <output.bin>\n"
  );
}

i32 main(i32 argc, char* argv[]) {
  if (argc < 5) {
    print_usage();
    return EXIT_FAILURE;
  }

  const char* table_path = argv[1];
  const char* mode       = argv[2];

  printf("Loading spline tables from: %s\n", table_path);
  lns16_read_tables(table_path);

  i32 status = EXIT_SUCCESS;

  if (strcmp(mode, "model") == 0 && argc == 5) {
    convert_model(argv[3], argv[4]);
  } else if (strcmp(mode, "tokenizer") == 0 && argc == 6) {
    convert_tokenizer(argv[3], argv[5], atoi(argv[4]));
  } else {
    fprintf(stderr, "Error: Invalid arguments or mode mismatch.\n");
    print_usage();
    status = EXIT_FAILURE;
  }

  lns_close();

  return status;
}
