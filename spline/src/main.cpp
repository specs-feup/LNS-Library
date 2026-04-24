#include "spline.hpp"

#define DEBUG

void print_help() {
  std::cout << "spline <--gen | --test> <args> <lns>" << std::endl;

  std::cout << std::endl << "  Add/Sub tables:" << std::endl;
  std::cout << "    --gen  <--xf | --xmb> <s_table+ (rows): [2,1024]> <s_table- (rows): [2,1024]>"
               " <--lns16 | --lns8> <int digits: lns8 - [4,6], lns16 - [4,14]>" << std::endl;
  std::cout << "    --test <--xf | --xmb> <max s_table (rows): [8,1024]>"
               " <--lns16 | --lns8> <int digits: lns8 - [4,6], lns16 - [4,14]>" << std::endl;

  std::cout << std::endl << "  Convert tables (float<->lns):" << std::endl;
  std::cout << "    --gen  <--f2l | --l2f> <s_table (rows): [2,1024]>"
               " <--lns16 | --lns8> <int digits: lns8 - [4,6], lns16 - [4,14]>" << std::endl;
  std::cout << "    --test <--f2l | --l2f> <max s_table (rows): [8,1024]>"
               " <--lns16 | --lns8> <int digits: lns8 - [4,6], lns16 - [4,14]>" << std::endl;

  std::cout << std::endl << "  --f2l : float-to-lns  (greedy spline of log2(1 + x))" << std::endl;
  std::cout << "  --l2f : lns-to-float  (greedy spline of 2^x - 1)"              << std::endl;
}

void print_error() {
  std::cerr << "[ERROR]: must choose between generating tables into output file or testing tables" << std::endl;
  print_help();
}

bool strisdigit(char* str) {
  if (str == NULL || *str == '\0')
    return false;
  str += *str == '+' || *str == '-';
  for (; *str != '\0'; str++)
    if (!isdigit(*str))
      return false;
  return true;
}

i32 main(i32 argc, char* argv[]) {
  if (argc < 6 || argc > 7) {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
      print_help();
      return 0;
    }
    print_error();
    exit(1);
  }

  char* command = argv[1];
  const bool gen  = strcmp(command, "--gen")  == 0;
  const bool test = strcmp(command, "--test") == 0;
  const bool xf   = strcmp(argv[2], "--xf")  == 0;
  const bool xmb  = strcmp(argv[2], "--xmb") == 0;
  const bool f2l  = strcmp(argv[2], "--f2l") == 0;
  const bool l2f  = strcmp(argv[2], "--l2f") == 0;

  if ((!gen && !test) || (!xf && !xmb && !f2l && !l2f)) {
    #ifdef DEBUG
      std::cout << "2" << std::endl;
    #endif
    print_error();
    exit(1);
  }

  // gen + add/sub (xf/xmb) consumes two size args; everything else has one.
  const bool two_sizes = gen && (xf || xmb);
  const int  lns_idx   = 4 + (int)two_sizes;   // argv index of --lns16/--lns8
  const int  digits_idx = 5 + (int)two_sizes;   // argv index of int_digits

  if (argc != 6 + (int)two_sizes) {
    #ifdef DEBUG
      std::cout << "argc mismatch" << std::endl;
    #endif
    print_error();
    exit(1);
  }

  const bool lns16 = strcmp(argv[lns_idx], "--lns16") == 0;
  const bool lns8  = strcmp(argv[lns_idx], "--lns8")  == 0;
  if (!lns16 && !lns8) {
    #ifdef DEBUG
      std::cout << "3" << std::endl;
    #endif
    print_error();
    exit(1);
  }

  if (!strisdigit(argv[digits_idx])) {
    std::cerr << "[ERROR]: integer digits must be a number: " << argv[digits_idx] << std::endl;
    print_help();
    exit(1);
  }
  const i16 int_digits = (i16)strtol(argv[digits_idx], NULL, 10);
  if (int_digits < 4 || int_digits > 14 || (int_digits > 6 && lns8)) {
    std::cerr << "[ERROR]: integer digits out of range" << std::endl;
    print_help();
    exit(1);
  }
  const i16 precision = (lns16 ? 15 : 7) - int_digits;

  
  if (gen) {
    // --- Add / Sub tables (xf or xmb) ---
    if (xf || xmb) {
      if (!strisdigit(argv[3]) || !strisdigit(argv[4])) {
        std::cerr << "[ERROR]: spline sizes must be numbers" << std::endl;
        print_help();
        exit(1);
      }

      const size_t
        s_table_plus  = strtol(argv[3], NULL, 10),
        s_table_minus = strtol(argv[4], NULL, 10);

      if (s_table_plus < 2 || s_table_plus > 1024 || s_table_minus < 2 || s_table_minus > 1024) {
        std::cerr << "[ERROR]: table sizes must be between 2 and 1024" << std::endl;
        exit(1);
      }

      std::vector<SplinePoint> spline_plus, spline_minus;
      greedy_spline_add_func(spline_plus,  s_table_plus,  precision);
      greedy_spline_sub_func(spline_minus, s_table_minus, precision);

      write_tables_binary(spline_plus, spline_minus, lns16, xf, int_digits, precision);
      return 0;
    }

    // --- Convert tables (f2l or l2f) ---
    if (!strisdigit(argv[3])) {
      std::cerr << "[ERROR]: spline size must be a number" << std::endl;
      print_help();
      exit(1);
    }

    const size_t s_table = strtol(argv[3], NULL, 10);
    if (s_table < 2 || s_table > 1024) {
      std::cerr << "[ERROR]: table size must be between 2 and 1024" << std::endl;
      exit(1);
    }

    std::vector<SplinePoint> spline;
    if (f2l) greedy_spline_float2lns(spline, s_table, precision);
    else     greedy_spline_lns2float(spline, s_table, precision);

    write_convert_table_binary(spline, lns16, f2l, int_digits, precision);
    return 0;
  }

  if (!strisdigit(argv[3])) {
    #ifdef DEBUG
      std::cout << "3" << std::endl;
    #endif
    std::cerr << "[ERROR]: spline size must be a number" << std::endl;
    print_help();
    exit(1);
  }

  const size_t s_max = strtol(argv[3], NULL, 10);
  if (s_max < 8 || s_max > 1024) {
    std::cerr << "[ERROR]: table size must be between 8 and 1024" << std::endl;
    exit(1);
  }

  std::cout << "Test: " << std::endl;
  std::cout << "  " << (xf ? "xf" : xmb ? "xmb" : f2l ? "f2l" : "l2f") << std::endl;
  std::cout << "  max table size: " << s_max << std::endl;
  std::cout << "  lns" << (lns16 ? "16" : "8")
            << " Q" << ((lns16 ? 15 : 7) - precision) << "." << precision << std::endl;

  if (xf || xmb) {
    test_add_table_sizes(s_max, xf, lns16, precision);
    test_sub_table_sizes(s_max, xf, lns16, precision);
  } else {
    // f2l and l2f always use xf evaluation (unidirectional conversion)
    if (f2l)
      test_float2lns_table_sizes(s_max, /*xf_mode=*/true, lns16, precision);
    else
      test_lns2float_table_sizes (s_max, /*xf_mode=*/true, lns16, precision);
  }

  return 0;
}
