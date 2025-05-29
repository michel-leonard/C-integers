# Big Numbers in C

The **tiny Big Integer Library** (released "as is", into the public domain, without any warranty, express or implied) is provided for handling large integers. It includes a variety of basic and advanced mathematical functions to support calculations. This solution does not use global variables but computation sheets, so it is stateless and thread-safe.

## Get Started

To build the executable, macOS and Linux users should use the **Terminal**, while Windows users should use **PowerShell**. The procedure takes just a few minutes :

1. Open a Terminal or PowerShell.  
2. Navigate to the directory containing the source code using the `cd` command.  
3. Compile the demo using the command `gcc -Wall -pedantic -O2 -std=c99 main.c -o demo`.
   
If `gcc` is not installed on Windows, you can install [MinGW](https://winlibs.com/), which provides it. Then, simply replace `demo` with `demo.exe` in the command above.
  
- **macOS users**: Replace `gcc` with `clang`, which acts the same and is available natively.
- **Linux users**: Install `gcc` with `sudo apt update && sudo apt install gcc`.  

### Running the demo  

Compilation takes a few seconds, then you can start the demo :
- **Windows**: `./demo.exe`
- **macOS** and **Linux**: `./demo`

The software will quickly display the results of the tests.

```
                   Arithmetic identities ... [PASS]
        Distributivity and Associativity ... [PASS]
                   Left and Right shifts ... [PASS]
                       String conversion ... [PASS]
                        Random generator ... [PASS]
                                Division ... [PASS]
                        Power and modulo ... [PASS]
                Double precision numbers ... [PASS]
                            Corner cases ... [PASS]
                               Primality ... [PASS]

Completed with 10 success and 0 failures.
```
## Developer Message

This library strikes a balance between real-world needs and code simplicity, it is most efficient when dealing with integers that are a few hundred bits long, but can handle large numbers such as computing 10000!. Designed to be lightweight, it consists of only about 1000 lines of code and has no dependencies, adhering to the C99 standard.

## Key Structures

### `cint` Structure

The `cint` structure is used to represent large integers, with its value stored in a dynamically allocated array of `h_cint_t` (typically `int64_t`):

```c
typedef struct {
    h_cint_t *mem;    // Memory storing the least significant bits (little-endian format)
    h_cint_t *end;    // Memory storing the most significant bits (end-1)
    h_cint_t nat;     // -1 for negative, +1 for positive (zero is positive)
    size_t size;      // Allocated size, at least (end - mem)
} cint;
```

### `cint_sheet` Structure

The `cint_sheet` structure is used to manage temporary variables required for certain operations. It allows efficient memory usage when performing computations that require multiple intermediate results.

```c
typedef struct {
    cint temp[10];    // Array of temporary variables for large number operations
} cint_sheet;
```

## Functions

### Memory Management

- **`cint_new_sheet(size_t bits)`**  
  Allocates a new `cint_sheet` for storing temporary variables needed during calculations.
  
- **`cint_clear_sheet(cint_sheet *sheet)`**  
  Clears the memory used by a `cint_sheet`, releasing all allocated resources.

- **`h_cint_tmp(cint_sheet *sheet, int id, const cint *least)`**  
  Allocates memory for a temporary `cint` variable within the provided `cint_sheet`.

### Integer Initialization & Conversion

- **`cint_init(cint *num, size_t bits, long long int val)`**  
  Initializes a `cint` with a specific size (in bits) and a long integer value.

- **`cint_init_by_string(cint *num, size_t bits, const char *str, int base)`**  
  Initializes a `cint` from a string representation of a number in a given base.

- **`cint_to_int(cint *num)`**  
  Converts a `cint` to a standard `long long int` (64-bit), truncating the value if necessary.

- **`cint_to_string(const cint *num, int base)`**  
  Converts a `cint` to a string in the specified base (e.g., decimal, hexadecimal).

### Arithmetic Operations

These operations modify the original `cint` (in-place), meaning the result of the operation is stored directly in one of the input variables:

- **`cint_addi(cint *lhs, const cint *rhs)`**  
  Adds `rhs` to `lhs` in place.

- **`cint_subi(cint *lhs, const cint *rhs)`**  
  Subtracts `rhs` from `lhs` in place.

- **`cint_muli(cint *lhs, const cint *rhs)`**  
  Multiplies `lhs` by `rhs` in place.

- **`cint_divi(cint *lhs, const cint *rhs)`**  
  Divides `lhs` by `rhs` in place.

- **`cint_left_shifti(cint *num, size_t bits)`**  
  Left shifts `num` by the specified number of bits.

- **`cint_right_shifti(cint *num, size_t bits)`**  
  Right shifts `num` by the specified number of bits.

### Modular Arithmetic

- **`cint_mul_mod(cint_sheet *sheet, const cint *lhs, const cint *rhs, const cint *mod, cint *res)`**  
  Computes the product of `lhs` and `rhs` modulo `mod`, storing the result in `res`.

- **`cint_pow_mod(cint_sheet *sheet, const cint *n, const cint *exp, const cint *mod, cint *res)`**  
  Computes `n` raised to the power `exp` modulo `mod`, storing the result in `res`.

- **`cint_modular_inverse(cint_sheet *sheet, const cint *lhs, const cint *rhs, cint *res)`**  
  Computes the modular inverse of `lhs` modulo `rhs`, storing the result in `res`.

### Advanced Operations

- **`cint_is_prime(cint_sheet *sheet, const cint *N, int iterations, uint64_t *seed)`**  
  Uses the Miller-Rabin primality test to check if `N` is prime. Temporary variables are allocated from `sheet`.

- **`cint_gcd(cint_sheet *sheet, const cint *lhs, const cint *rhs, cint *gcd)`**  
  Computes the greatest common divisor (GCD) of `lhs` and `rhs`, storing the result in `gcd`.

- **`cint_sqrt(cint_sheet *sheet, const cint *num, cint *res, cint *rem)`**  
  Computes the square root of `num`, storing the result in `res` and the remainder in `rem`.

### Helper Functions

- **`cint_checksum(const cint *num)`**  
  Computes a checksum for the given `cint`.

- **`cint_count_bits(const cint *num)`**  
  Returns the number of bits required to represent `num`.

- **`cint_count_zeros(const cint *num)`**  
  Returns the number of trailing zeros in `num`.

- **`cint_compare(const cint *lhs, const cint *rhs)`**  
  Compares two `cint` values.

- **`cint_to_double(const cint *num)`**  
  Converts a `cint` to a `double`.

- **`cint_nth_root(cint_sheet *sheet, const cint *num, unsigned nth, cint *res)`**  
  Computes the nth root of `num`, storing the result in `res`.


## Real use

This **tiny Big Integer Library** is used by a factorization software to operate its [Quadratic Sieve](https://github.com/michel-leonard/C-Quadratic-Sieve), intended for users who factor numbers up to 75+ digits and enjoy applying mathematics to software.
