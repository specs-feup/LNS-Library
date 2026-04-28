# LNS Spline Table Tester/Generator

C++ program that implements the greedy spline generation algorithm to create lookup tables for LNS addition, subtraction, and float↔LNS conversion. It can either generate the tables or test their precision against an expected error.

### How to Build

To build the executable, run the following command in your terminal:

```bash
make
```

To remove the compiled executable, run:

```bash
make clean
```

To generate the default tables:
```bash
make tables
```

### How to Use

The compiled `spline` executable has two main modes of operation: generating tables (`--gen`) and testing table sizes (`--test`).
In the `--gen` case, the generated tables will be written to the `lns_tables/` directory.

Both modes require specifying the spline representation, the LNS format, and the number of integer digits.

**Arguments shared by both modes:**

- `<method>`: `--xf` or `--xmb`
  - `--xf`: stores `(x, f)` point pairs; lower memory usage.
  - `--xmb`: precomputes `(x, m, b)` segment coefficients; reduces runtime to a single multiply-add.
- `<--lns16 | --lns8>`: selects the LNS format.
- `<int digits>`: number of integer bits in the fixed-point exponent.
  - `--lns8`: valid range `[4, 6]`
  - `--lns16`: valid range `[4, 14]`

---

* **To generate lookup tables:**

  ```bash
  ./spline --gen <method> <s_table_plus> <s_table_minus> <s_table_f2l> <s_table_l2f> <--lns16 | --lns8> <int digits>
  ```

  - `<s_table_plus>`: number of rows for the addition function table `[2, 1024]`.
  - `<s_table_minus>`: number of rows for the subtraction function table `[2, 1024]`.
  - `<s_table_f2l>`: number of rows for the float-to-LNS conversion table `[2, 1024]`.
  - `<s_table_l2f>`: number of rows for the LNS-to-float conversion table `[2, 1024]`.

  *Example:* `./spline --gen --xf 64 64 32 32 --lns16 8`

---

* **To test the precision of tables:**

  ```bash
  ./spline --test <method> <max_s_table> <--lns16 | --lns8> <int digits>
  ```

  - `<max_s_table>`: maximum table size to test, for all four functions `[8, 1024]`.

  *Example:* `./spline --test --xmb 128 --lns16 8`

  The test mode evaluates all four functions (addition, subtraction, float-to-LNS, and LNS-to-float) up to the specified table size.

---

* **To print usage information:**

  ```bash
  ./spline --help
  ```
