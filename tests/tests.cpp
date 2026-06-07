#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cstring>

#include <lnssim.hpp>
using lns16 = lns<16, 8, 7>;

#define TEST_MAX_LOC           20
#define TEST_MAX_CHAR_PER_LINE 512

#define CREATE_DEFINITION(ident, expr) \
  index += snprintf(&test[index], s_test - index, "  lns16 %s = %s;\n", ident, expr);

#define CREATE_STMT(ident, expr) \
  index += snprintf(&test[index], s_test - index, "  %s = %s;\n", ident, expr);

const char* OPS[] = {"+", "-", "*", "/"};
const i32 NUM_OPS = 4;

const char* VARS[] = {"a", "b"};
const i32 NUM_VARS = 2;

void gen_random_expression(char* buffer, u32 max_len, i32 depth, bool declared) {
  // Base case: Force a leaf node if depth is 0, or randomly choose a leaf
  if (depth <= 0 || (rand() % 3 == 0)) {
    if (rand() % 2 == 0 && declared) {
      // Option A: Use an existing variable
      snprintf(buffer, max_len, "%s", VARS[rand() % NUM_VARS]);
    } else {
      // Option B: Generate a random float literal
      // LNS typically hates 0.0 or negative numbers depending on implementation, 
      // so tailor these ranges to your LNS edge cases.
      f32 val = ((f32)rand() / (f32)RAND_MAX) * 10.0f + 0.1f;
      lns16 tmp(val);
      u16 raw;
      memcpy(&raw, &tmp.bits, sizeof(u16));
      snprintf(buffer, max_len, "lns16(%uu /*= %.4ff*/)", raw, val);
    }
  } else {
    // Recursive case: Generate ( Left_Expr Op Right_Expr )
    char left[128];
    char right[128];
    
    gen_random_expression(left, sizeof(left), depth - 1, declared);
    gen_random_expression(right, sizeof(right), depth - 1, declared);
    
    const char* op = OPS[rand() % NUM_OPS];
    
    // Wrap in parentheses to ensure correct order of operations
    snprintf(buffer, max_len, "(%s %s %s)", left, op, right);
  }
}

void gen_code_snippet(char* test, const u32 s_test, u32& index) {
  char expr[TEST_MAX_CHAR_PER_LINE];

  gen_random_expression(expr, sizeof(expr), 0, false);
  CREATE_DEFINITION("a", expr);
  
  gen_random_expression(expr, sizeof(expr), 0, false);
  CREATE_DEFINITION("b", expr);

  gen_random_expression(expr, sizeof(expr), 2, true);
  CREATE_STMT("a", expr);

  gen_random_expression(expr, sizeof(expr), 2, true);
  CREATE_STMT("b", expr);

  gen_random_expression(expr, sizeof(expr), 3, true);
  CREATE_STMT("result", expr);
}

void print_help() {
  printf("LNS Library + RISC++ LNSU Test Generator\n");
  printf("  ./test <path/to/lns/xf/table> <num tests> <args>\n");
  printf("    --seed: random integer seed for test generation\n");
};

i32 main(const i32 argc, const char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "[ERROR]: expected number of tests\n");
    print_help();
    return 1;
  }

  lns16_read_tables(argv[1]);

  const u32 num_tests = (u32)strtol(argv[2], nullptr, 10);

  bool arg_seed = false;
  u32 _seed = 0;

  if (argc > 3) {
    arg_seed = argc == 5 && strcmp(argv[3], "--seed") == 0;

    if (arg_seed)
      _seed = strtol(argv[4], nullptr, 10);
    else {
      fprintf(stderr, "[ERROR]: unexpected format for arguments\n");
      print_help();
      return 1;
    }
  }

  srand(arg_seed ? _seed : time(NULL));

  const char* test_header =
    "#include \"utils.h\"\n"
    "#include \"lns.hpp\"\n"
    "volatile lns16 result;\n"
    "extern \"C\" int my_main(void) {\n"
  ;

  const char* test_footer =
    "  return 0;\n"
    "}"
  ;

  const u64 s_test =
    strlen(test_header) +
    strlen(test_footer) +
    TEST_MAX_LOC * TEST_MAX_CHAR_PER_LINE * sizeof(u8) +
    1;

  char* test = (char*)malloc(s_test);
  if (test == nullptr) {
    fprintf(stderr, "[ERROR]: could not allocate memory for test\n");
    return 1;
  }

  for (u32 i = 0; i < num_tests; i++) {
    test[0] = '\0';
    strcpy(test, test_header);

    u32 curr_index = strlen(test);
    gen_code_snippet(test, s_test, curr_index);

    strcat(test, test_footer);

    char 
      timestamp[20],
      filename[64];

    time_t raw_time = time(NULL);
    if (raw_time == (time_t)(-1)) {
      fprintf(stderr, "[ERROR]: failed to get system time\n");
      return 1;
    }

    struct tm* local_time = localtime(&raw_time);
    if (local_time == NULL) {
      fprintf(stderr, "[ERROR]: failed to convert to local time\n");
      return 1;
    }

    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", local_time);
    snprintf(filename, sizeof(filename), "test_files/test_lns16_%s_%u.cpp", timestamp, i);

    FILE* file = fopen(filename, "w");
    if (file == nullptr) {
      fprintf(stderr, "[ERROR]: failed to create test file\n");
      return 1;
    }

    if (fprintf(file, "%s", test) < 0) {
      fprintf(stderr, "[WARN]: failed to write to test file %s\n", filename);
      fclose(file);
      return 1;
    }

    fclose(file);
  }
  
  return 0;
}
