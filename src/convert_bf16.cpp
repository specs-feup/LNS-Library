#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "bfloatsim.hpp"

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

// ----------------------------------------------------------------------------
// tokenizer conversion: f32 scores -> bf16 scores
// ----------------------------------------------------------------------------

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

    bf16 score_bf16 = bf16(score_f32);

    if (fwrite(&score_bf16.bits, sizeof(u16), 1, fout) != 1) {
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

// ----------------------------------------------------------------------------
// model conversion: f32 weights -> bf16 weights
// file layout: [Config][f32...f32]
// weights are a flat array immediately after the config header,
// so we just convert every f32 to bf16 without needing to know the layout
// ----------------------------------------------------------------------------

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

  // stream all weights through bf16 conversion
  const i32 BATCH = 4096;
  f32  buf_f32[BATCH];
  bf16 buf_bf16[BATCH];

  size_t n;
  size_t total = 0;

  while ((n = fread(buf_f32, sizeof(f32), BATCH, fin)) > 0) {
    for (size_t i = 0; i < n; i++)
      buf_bf16[i] = bf16(buf_f32[i]);

    if (fwrite(buf_bf16, sizeof(bf16), n, fout) != n) {
      fprintf(stderr, "failed write\n");
      exit(EXIT_FAILURE);
    }

    total += n;
  }

  fclose(fin);
  fclose(fout);
  printf("model: %s -> %s (%zu weights converted)\n", input_path, output_path, total);
}

// ----------------------------------------------------------------------------
// main
// ----------------------------------------------------------------------------

i32 main(i32 argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr,
      "usage:\n"
      "  model      <input.bin> <output.bin>\n"
      "  tokenizer  <input.bin> <vocab_size> <output.bin>\n"
    );
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "model") == 0 && argc == 4) {
    convert_model(argv[2], argv[3]);

  } else if (strcmp(argv[1], "tokenizer") == 0 && argc == 5) {
    convert_tokenizer(argv[2], argv[4], atoi(argv[3]));

  } else {
    fprintf(stderr, "invalid arguments\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
