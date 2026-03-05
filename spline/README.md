# LNS Spline Table Tester/Generator

C++ program that implements the greedy spline generation 
algorithm to create lookup tables for LNS addition and subtraction.
It can either generate the tables or test their precision against an expected error.

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

  * **To generate lookup tables:**

    ```bash
    ./spline --gen <method> <s_table_plus> <s_table_minus>
    ```

      * `<method>`: `--xf` or `--xmb`
      * `<s_table_plus>`: Table size for the addition function.
      * `<s_table_minus>`: Table size for the subtraction function.
      * *Example:* `./spline --gen --xf 64 64`

  * **To test the precision of tables:**

    ```bash
    ./spline --test <method> <max_s_table>
    ```

      * `<method>`: `--xf` or `--xmb`
      * `<max_s_table>`: The maximum table size to test.
      * *Example:* `./spline --test --xf 128`
