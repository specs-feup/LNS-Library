#include "spline.hpp"

#define DEBUG

void print_help() {
  std::cout << "spline <--gen | --test> <args> <lns>" << std::endl;
  std::cout << "    --gen  <--xf | --xmb> <s_table+ (rows): [2,1024]> <s_table- (rows): [2,1024]>" << std::endl
            << "      <s_table float2lns (rows): [2,1024]> <s_table lns2float (rows): [2,1024]>"   << std::endl
            << "      <--lns16 | --lns8> <int digits: lns8 - [4,6], lns16 - [4,14]>"               << std::endl;
  std::cout << "    --test <--xf | --xmb> <max s_table (rows): [2,1024]>"
               "      <--lns16 | --lns8> <int digits: lns8 - [4,6], lns16 - [4,14]>"               << std::endl;
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
  if (argc != 6 && argc != 9) {
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

  if ((!gen && !test) || (!xf && !xmb)) {
    #ifdef DEBUG
      std::cout << "2" << std::endl;
    #endif
    print_error();
    exit(1);
  }

  const bool two_sizes  = gen && (xf || xmb);
  const i32  lns_idx    = 4 + (two_sizes ? 3 : 0);   // argv index of --lns16/--lns8
  const i32  digits_idx = 5 + (two_sizes ? 3 : 0);   // argv index of int_digits

  if (argc != 6 + (two_sizes ? 3 : 0)) {
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
    if (!strisdigit(argv[3]) || !strisdigit(argv[4])) {
      std::cerr << "[ERROR]: spline sizes must be numbers" << std::endl;
      print_help();
      exit(1);
    }

    const size_t
      s_table_plus  = strtol(argv[3], NULL, 10),
      s_table_minus = strtol(argv[4], NULL, 10),
      s_table_f2l   = strtol(argv[5], NULL, 10),
      s_table_l2f   = strtol(argv[6], NULL, 10);

    if (
      s_table_plus < 2 || s_table_plus > 1024 || s_table_minus < 2 || s_table_minus > 1024 ||
      s_table_f2l  < 2 || s_table_f2l  > 1024 || s_table_l2f   < 2 || s_table_l2f   > 1024
    ) {
      std::cerr << "[ERROR]: table sizes must be between 2 and 1024" << std::endl;
      exit(1);
    }

    std::vector<SplinePoint>
      spline_plus, spline_minus,
      spline_f2l,  spline_l2f;
    greedy_spline_add_func  (spline_plus,  s_table_plus,  precision);
    greedy_spline_sub_func  (spline_minus, s_table_minus, precision);
    greedy_spline_float2lns (spline_f2l,   s_table_f2l);
    greedy_spline_lns2float (spline_l2f,   s_table_l2f);

    write_tables_binary(
      spline_plus, spline_minus, spline_f2l, spline_l2f,
      lns16, xf, int_digits, precision
    );

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
  if (s_max < 2 || s_max > 1024) {
    std::cerr << "[ERROR]: table size must be between 8 and 1024" << std::endl;
    exit(1);
  }

  std::cout << "Test: " << std::endl;
  std::cout << "  " << (xf ? "xf" : "xmb") << std::endl;
  std::cout << "  max table size: " << s_max << std::endl;
  std::cout << "  lns" << (lns16 ? "16" : "8")
            << " Q" << ((lns16 ? 15 : 7) - precision) << "." << precision << std::endl;

  test_add_table_sizes(s_max, xf, lns16, precision);
  test_sub_table_sizes(s_max, xf, lns16, precision);

  test_float2lns_table_sizes(s_max, xf, lns16, precision);
  test_lns2float_table_sizes(s_max, xf, lns16, precision);

  return 0;
}
