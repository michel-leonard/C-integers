#include "big-num.c"

#include <stdio.h>

// From oeis.org/A033932, generate various prime numbers using PRIME = factorial(INDEX of the number) + number
const int prime_generator[] = {1, 1, 1, 1, 5, 7, 7, 11, 23, 17, 11, 1, 29, 67, 19, 43, 23, 31, 37, 89, 29, 31, 31, 97, 131, 41, 59, 1, 67, 223, 107, 127, 79, 37, 97, 61, 131};

#define INIT_CINT_ARRAY(SIZE, V_1, V_2, V_3, V_4, V_5, V_6, V_7) \
    cint_sheet * sheet = cint_new_sheet(SIZE); \
    cint nums[7], *V_1 = nums, *V_2 = nums + 1, *V_3 = nums + 2, *V_4 = nums + 3, *V_5 = nums + 4, *V_6 = nums + 5, *V_7 = nums + 6; \
    for (int i = 0, j = (int) sizeof(nums) / sizeof(*nums); i < j; ++i) \
        cint_init(nums + i, SIZE, V_1 == V_2 && V_2 == V_3 && V_3 == V_4 && V_4 == V_5 && V_5 == V_6 && V_6 == V_7);

#define FREE_CINT_ARRAY() \
    for (int i = 0, j = (int) sizeof(nums) / sizeof(*nums); i < j; ++i) \
        free((nums + i)->mem); \
    cint_clear_sheet(sheet);

int cint_equals(const cint *lhs, const cint *rhs) {

	// The numbers are equal in terms of data length.
	if ((lhs->end - lhs->mem) != (rhs->end - rhs->mem))
		return 0 != printf("[ERROR] The lengths of LHS and RHS are different: %ld vs %ld\n", lhs->end - lhs->mem, rhs->end - rhs->mem);

	// The numbers are equals in terms of sign.
	if (lhs->nat != rhs->nat)
		return 0 != printf("[ERROR] The signs of the numbers are different: %ld vs %ld\n", lhs->nat, rhs->nat);

	// The allocated size is sufficient for the number representation.
	if (lhs->size < (size_t) (lhs->end - lhs->mem) || rhs->size < (size_t) (rhs->end - rhs->mem))
		return 0 != printf("[ERROR] Size mismatch error: %zu or %zu isn't large enough\n", lhs->size, rhs->size);

	// Data verification.
	size_t length = (lhs->end - lhs->mem) * sizeof(h_cint_t);
	if (memcmp(lhs->mem, rhs->mem, length) != 0)
		return 0 != printf("[ERROR] Memory comparison asserts that the numbers are different\n");

	// Well-formed END.
	if (*lhs->end != 0 || *rhs->end != 0)
		return 0 != printf("[ERROR] The end of the number in terms of data isn't equal to 0\n");

	// Compare two numbers that are equals to 0.
	if (lhs->end == lhs->mem && rhs->end == rhs->mem)
		return 0;

	if ((lhs->end != lhs->mem && *(lhs->end - 1) == 0) || (rhs->end != rhs->mem && *(rhs->end - 1) == 0))
		return 0 != printf("[ERROR] No data present before the END pointer\n");

	return 0;
}

int test_cint_arithmetic_identities(uint64_t *seed) {

	INIT_CINT_ARRAY(4000, A, B, C, Result1, Result2, TMP, _)

	for (int bits_A = 1; bits_A <= 180; bits_A++) {
		for (int bits_B = 1; bits_B <= 180; bits_B++) {

			cint_random_bits(A, bits_A, seed);
			cint_random_bits(B, bits_B, seed);
			A->nat = (*seed & 1) ? 1 : -1;
			B->nat = (*seed & 2) ? 1 : -1;

			// Ensures that (A + B) - B == A
			cint_add(A, B, Result1);
			cint_sub(Result1, B, Result2);
			if (cint_equals(A, Result2) != 0)
				return 0 != printf("[ERROR] (A + B) - B != A for a %d-bit value of A and a %d-bit value of B\n", bits_A, bits_B);

			// Ensures that (A * B) / B == A (si B ≠ 0)
			if (B->end != B->mem) {
				cint_mul(A, B, Result1);
				cint_div(sheet, Result1, B, Result2, C);
				if (cint_equals(A, Result2) != 0)
					return 0 != printf("[ERROR] (A * B) / B != A for a %d-bit value of A and a %d-bit value of B\n", bits_A, bits_B);
			}
		}
	}

	FREE_CINT_ARRAY()
	return 0;
}

int test_cint_distributivity_associativity(uint64_t *seed) {
	INIT_CINT_ARRAY(4000, A, B, C, LHS, RHS, TMP, _);

	for (int bits_A = 1; bits_A <= 90; bits_A++) {
		for (int bits_B = 1; bits_B <= 90; bits_B++) {
			for (int bits_C = 1; bits_C <= 90; bits_C++) {

				cint_random_bits(A, bits_A, seed);
				cint_random_bits(B, bits_B, seed);
				cint_random_bits(C, bits_C, seed);
				A->nat = (*seed & 1) ? 1 : -1;
				B->nat = (*seed & 2) ? 1 : -1;
				C->nat = (*seed & 4) ? 1 : -1;

				// Ensures the distributivity as A * (B + C) == A * B + A * C
				cint_add(B, C, LHS);
				cint_mul(A, LHS, TMP), cint_dup(LHS, TMP);
				cint_mul(A, B, RHS);
				cint_mul(A, C, TMP), cint_dup(A, TMP);
				cint_addi(RHS, A);
				if (cint_equals(LHS, RHS) != 0)
					return 0 != printf("[ERROR] Distributivity isn't okay when A has %d bits, B has %d bits and C has %d bits\n", bits_A, bits_B, bits_C);

				// Ensures the first rule of associativity as (A + B) + C == A + (B + C)
				cint_add(A, B, LHS);
				cint_addi(LHS, C);
				cint_add(B, C, RHS);
				cint_addi(RHS, A);
				if (cint_equals(LHS, RHS) != 0)
					return 0 != printf("[ERROR] Associativity (rule 1) isn't okay when A has %d bits, B has %d bits and C has %d bits\n", bits_A, bits_B, bits_C);

				//  Ensures the second rule of associativity as (A * B) * C == A * (B * C)
				cint_mul(A, B, LHS);
				cint_mul(LHS, C, TMP), cint_dup(LHS, TMP);
				cint_mul(B, C, RHS);
				cint_mul(A, RHS, TMP), cint_dup(RHS, TMP);
				if (cint_equals(LHS, RHS) != 0)
					return 0 != printf("[ERROR] Associativity (rule 2) isn't okay when A has %d bits, B has %d bits and C has %d bits\n", bits_A, bits_B, bits_C);
			}
		}
	}
	FREE_CINT_ARRAY()
	return 0;
}

int test_cint_shifts(uint64_t *seed) {
	cint A, B;
	cint_sheet *sheet = cint_new_sheet(4000);
	for (int bits = 20; bits <= 200; bits++) {
		cint_init(&A, 4000, 0);
		cint_init(&B, 4000, 0);

		cint_random_bits(&A, bits, seed);
		cint_dup(&B, &A);

		if (cint_equals(&A, &B) != 0)
			return 0 != printf("[ERROR] The duplication failed for %d-bit\n", bits);

		int x = (int) (*seed % 101);
		int y = (int) ((*seed / 2) % 101);

		cint_left_shifti(&B, x);
		if (cint_compare(&A, &B) == (x == 0))
			return 0 != printf("[ERROR] The %d-bit left shift should modify the number\n", x);

		cint_left_shifti(&B, y);
		cint_right_shifti(&B, x + y);
		if (cint_compare(&A, &B) != 0)
			return 0 != printf("[ERROR] The number shouldn't be affected by left(%d) + left(%d) + right(%d)\n", x, y, x + y);

		x = (int) (*seed % 101);
		cint_left_shifti(&B, x);
		y = (int) ((*seed / 3) % (x + 1));
		cint_right_shifti(&B, y);
		cint_right_shifti(&B, x - y);
		if (cint_equals(&A, &B) != 0)
			return 0 != printf("[ERROR] The number shouldn't be affected by left(%d) + right(%d) + right(%d)\n", x, y, x - y);

		free(A.mem);
		free(B.mem);
	}

	cint_clear_sheet(sheet);
	return 0;
}

int test_cint_string_conversion(uint64_t *seed) {
	cint A;
	char buf[1024];
	cint_sheet *sheet = cint_new_sheet(4000);

	for (int bits = 1; bits <= 100; bits++) {
		cint_init(&A, 4000, 0);
		cint_random_bits(&A, bits, seed);

		if ((int) cint_count_bits(&A) != bits)
			return 0 != printf("[ERROR] The bit count isn't correct: want %d and found %zu\n", bits, cint_count_bits(&A));

		for (int base = 2; base <= 62; base++) {
			cint_to_string_buffer(&A, buf, base);
			int real_digits = (int) strlen(buf);
			int approx_digits = (int) cint_approx_digits_from_bits(bits, base);
			int approx_bits = (int) cint_approx_bits_from_digits(real_digits, base);

			if (approx_digits < real_digits + 1)
				return 0 != printf("[ERROR] The approximation of the numbers of digits for %d-bit in base %d should be greater than %d, not like %d\n", bits, base, real_digits, approx_digits);

			if (approx_bits < bits)
				return 0 != printf("[ERROR] The number of bits for a %d characters long string in base %d should be greater than or equal to %d, not like %d\n", real_digits, base, bits, approx_bits);

			if (!((approx_bits - bits < 8 || approx_bits < bits * 11 / 10) && (approx_digits - real_digits < 8 || approx_digits < real_digits * 11 / 10)))
				return 0 != printf("[ERROR] The approximation in base %d is too large, %d >>= %d and/or %d >>= %d \n", base, approx_digits, real_digits, approx_bits, bits);

			if (approx_digits == real_digits)
				printf("[WARNING] Approximation is exact in base %d, the null character that ends the strings could become problematic\n", base);

		}

		free(A.mem);
	}

	cint_clear_sheet(sheet);
	return 0;
}

int test_cint_random_bits(uint64_t *seed) {
	char buf[256];
	cint num, reconstructed;
	cint_sheet *sheet = cint_new_sheet(4000);
	for (int bits = 1; bits <= 150; bits++) {
		for (int sign = 1; sign >= -1; sign -= 2) {
			cint_init(&num, 4000, 0);
			cint_random_bits(&num, bits, seed);
			num.nat = sign;
			// Converts the number to a string and ensure the bit size match.
			cint_to_string_buffer(&num, buf, 2);
			size_t expected_length = bits + (sign == -1 ? 1 : 0);
			if (strlen(buf) != expected_length)
				return 0 != printf("[ERROR] The bit count for %d bits isn't correct: want %zu and found %lu\n", bits, expected_length, strlen(buf));

			for (int base = 2; base <= 62; ++base) {
				// Ensure that for all bases, we can convert the number to a string
				cint_to_string_buffer(&num, buf, base);
				cint_init(&reconstructed, 4000, 0);
				// And this string is able to reconstruct the number
				cint_reinit_by_string(&reconstructed, buf, base);
				if (cint_equals(&num, &reconstructed) != 0)
					return 0 != printf("[ERROR] Reconstructing the %d bits number in base %d failed\n", bits, base);

			}
			free(num.mem);
			free(reconstructed.mem);
		}
	}

	cint_clear_sheet(sheet);
	return 0;
}

int test_cint_division(uint64_t *seed) {
	INIT_CINT_ARRAY(4000, A, B, Q, R, reconstructed, TMP, _)

	for (int bits_A = 0; bits_A <= 200; bits_A++) {
		for (int bits_B = 1; bits_B <= 200; bits_B++) {

			if (bits_A) {
				cint_random_bits(A, bits_A, seed);
				A->nat = (*seed & 1) ? 1 : -1;
			} else
				cint_erase(A);

			cint_random_bits(B, bits_B, seed);
			B->nat = (*seed & 2) ? 1 : -1;

			// For any numbers A and B, it holds that A = Q * B + R
			cint_div(sheet, A, B, Q, R);
			cint_mul(B, Q, reconstructed);
			cint_addi(reconstructed, R);

			if (cint_equals(A, reconstructed) != 0)
				return 0 != printf("[ERROR] The division isn't okay with A=%d-bit and B=%d-bit\n", bits_A, bits_B);

			if (R->mem != R->end && R->nat != A->nat)
				return 0 != printf("[ERROR] The convention is that the sign of R (%ld) is the sign of A (%ld)\n", R->nat, A->nat);

		}
	}

	FREE_CINT_ARRAY()

	return 0;
}

int test_cint_pow_mod(uint64_t *seed) {
	{
		INIT_CINT_ARRAY(4000, A, E, M, R1, Q, R2, TMP);
		for (int bits_A = 2; bits_A <= 40; ++bits_A) {
			cint_random_bits(A, bits_A, seed);
			for (int bits_E = 2; bits_E <= 6; ++bits_E) {
				cint_random_bits(E, bits_E, seed);
				for (int bits_M = 2; bits_M <= 40; ++bits_M) {
					cint_random_bits(M, bits_M, seed);

					cint_pow_mod(sheet, A, E, M, R1);
					cint_pow(sheet, A, E, TMP);
					cint_div(sheet, TMP, M, Q, R2);

					if (cint_equals(R1, R2) != 0)
						return 0 != printf("[ERROR] The power and modulo is inconsistent\n");

				}
			}
		}
		FREE_CINT_ARRAY()
	}
	{
		INIT_CINT_ARRAY(1000, Original, Large, Power, TMP, E, F, _);
		for(int bits_1 = 2; bits_1 < 40; ++bits_1){
			for(int bits_2 = 20; bits_2 < 800; bits_2 += 40) {
				int real_power = 1;
				cint_random_bits(Large, bits_1, seed);
				cint_dup(Original, Large);
				do cint_mul(Large, Original, TMP), cint_dup(Large, TMP);
				while (++real_power * bits_1 < bits_2);
				cint_reinit(TMP, real_power);
				cint_pow(sheet, Original, TMP, Power);
				const char * format = "[ERROR] The %s function failed with a %zu-bit number (power = %d)\n" ;
				const size_t real_bits = cint_count_bits(Large) ;
				if (cint_equals(Large, Power) != 0)
					return 0 != printf(format, "power", real_bits, real_power);
				cint_nth_root(sheet, Large, real_power, TMP);
				if (cint_equals(Original, TMP) != 0)
					return 0 != printf(format, "nth-root", real_bits, real_power);
				if (real_power != (int)cint_remove(sheet, Large, Original))
					return 0 != printf(format, "remove", real_bits, real_power);
			}
		}
		FREE_CINT_ARRAY();
	}
	return 0;
}

int test_cint_double_roundtrip(uint64_t *seed) {
	INIT_CINT_ARRAY(1500, A, B, C, D, E, F, G);
	double d, e;

	for (int bits = 1; bits <= 1023; bits++) {

		cint_random_bits(A, bits, seed);
		d = cint_to_double(A);
		if (*seed & 1)
			d = -d;
		cint_reinit_by_double(A, d);
		e = cint_to_double(A);
		cint_reinit_by_double(B, e);

		if (d != e)
			return 0 != printf("[ERROR] %g isn't equal to %g when manipulating double precision numbers\n", d, e);

		if (cint_count_bits(A) != (size_t) bits)
			return 0 != printf("[ERROR] The %zu-bit count isn't in accordance with the expected %d-bit\n", cint_count_bits(A), bits);

		if (cint_equals(A, B) != 0)
			return 0 != printf("[ERROR] Manipulating double precision numbers of %d-bit (%g) with cint should functions\n", bits, d);

	}

	FREE_CINT_ARRAY()
	return 0;
}

int test_cint_is_prime(uint64_t *seed) {

	INIT_CINT_ARRAY(4000, P1, P2, TMP, Product, E, F, G);

	const int limit = sizeof(prime_generator) / sizeof(*prime_generator);

	for (int idx1 = 0; idx1 < limit; idx1++) {
		for (int idx2 = 0; idx2 < limit; idx2++) {

			int n1 = prime_generator[idx1];
			int n2 = prime_generator[idx2];

			cint_factorial(sheet, idx1, P1);
			cint_reinit(TMP, n1);
			cint_addi(P1, TMP);
			cint_factorial(sheet, idx2, P2);
			cint_reinit(TMP, n2);
			cint_addi(P2, TMP);

			if (!cint_is_prime(sheet, P1, -1, seed) || !cint_is_prime(sheet, P2, -1, seed))
				return 0 != printf("[ERROR] The generated prime number isn't detected as prime at index %d or %d\n", idx1, idx2);

			cint_mul(P1, P2, Product);

			if (cint_is_prime(sheet, Product, -1, seed))
				return 0 != printf("[ERROR] The product of two prime numbers shouldn't be detected as a prime number\n");
		}
	}
	FREE_CINT_ARRAY()
	return 0;
}

__attribute__((unused)) void print_factorial(const unsigned factorial_n) {
	int bits = 2; // Print the factorial.
	for (unsigned i = 1; i < factorial_n; ++i, ++bits)
		for (unsigned j = i; j >>= 1; ++bits);
	cint_sheet *sheet = cint_new_sheet(bits);
	cint *res = h_cint_tmp(sheet, 9, sheet->temp);
	cint_factorial(sheet, factorial_n, res);
	char *str = cint_to_string(res, 10);
	printf("%17d! = %s\n", factorial_n, str);
	free(str);
	cint_clear_sheet(sheet);
}

int test_cint_corner_cases(uint64_t * seed){

	{
		INIT_CINT_ARRAY(128, Original, Expected, Delta, Result, TMP_1, TMP_2, _);

		// Additions around the mask.
		for (int bits = 1; bits < cint_exponent - 1; ++bits) {
			cint_random_bits(Original, bits, seed);
			const long long int val = cint_to_int(Original);
			for (int offset = -10; offset < 10; ++offset) {
				cint_reinit(Expected, cint_mask + offset);
				cint_reinit(Delta, cint_mask + offset - val);
				cint_dup(Result, Original), cint_addi(Result, Delta);
				if (cint_equals(Expected, Result) != 0)
					return 0 != printf("The addition around the mask isn't consistent");
				if ((offset == 0 && Result->mem[0] != cint_mask) || (0 < offset && Result->mem[0] != (offset - 1)))
					return 0 != printf("The addition result around the mask isn't consistent");
				if (Result->end - Result->mem - 1 != (0 < offset))
					return 0 != printf("The data length after the addition isn't consistent");
			}
		}

		// Subtractions around the mask.
		for (int bits = (int) cint_exponent + 1; bits < cint_exponent << 1; ++bits) {
			cint_random_bits(Original, bits, seed);
			const long long int val = cint_to_int(Original);
			for (int offset = -10; offset < 10; ++offset) {
				cint_reinit(Expected, cint_mask + offset);
				cint_reinit(Delta, val - cint_mask - offset);
				cint_dup(Result, Original), cint_subi(Result, Delta);
				if (cint_equals(Expected, Result) != 0)
					return 0 != printf("The subtraction around the mask isn't consistent");
				if ((offset == 0 && Result->mem[0] != cint_mask) || (0 < offset && Result->mem[0] != offset - 1))
					return 0 != printf("The subtraction result around the mask isn't consistent");
				if (Result->end - Result->mem - 1 != (0 < offset))
					return 0 != printf("The data length after the subtraction isn't consistent");
			}
		}
		FREE_CINT_ARRAY();
	}

	{
		// Divisions exact around the mask.
		INIT_CINT_ARRAY(256, Dividend, Quotient, Remainder, Expected, TMP_1, TMP_2, _);
		for(int shift = 0; shift < 5; ++shift) {
			for (int offset = -1; offset <= 1; ++offset) {
				cint_reinit(Dividend, cint_mask + offset);
				cint_left_shifti(Dividend, cint_exponent * shift);
				cint_dup(TMP_1, Dividend);
				cint_dup(TMP_2, Dividend);
				for (int i = 0; i <= cint_exponent; ++i) {
					cint_addi(TMP_2, TMP_1);
					cint_dup(TMP_1, TMP_2);
					cint_div(sheet, TMP_2, Dividend, Quotient, Remainder);
					cint_reinit(Expected, 2LL << i);
					if (cint_equals(Quotient, Expected) != 0 || Remainder->mem != Remainder->end)
						return 0 != printf("The division around the mask failed with shift = %d, offset = %d and i = %d\n", shift, offset, i);
				}
			}
		}
		FREE_CINT_ARRAY();
	}

	return 0 ;
}

// This tiny Big Integer Library is used by a factorization software
// to operate the Quadratic Sieve, intended for users who enjoy mathematics
// applied to software. It can factor numbers up to 75 digits, and more.
// URL: https://github.com/michel-leonard/C-Quadratic-Sieve

int main(void) {

	const uint64_t seed = 1;
	printf("The random generator seed is %lu.\n\n", seed);

	struct test {
		const char *name;
		int status;
		uint64_t seed;
		int (*func)(uint64_t *seed);
	} all_tests[50] = {0};

	const int size_tests = sizeof(all_tests) / sizeof(*all_tests);
	for (int i = 0; i < size_tests; ++i)
		all_tests[i].status = -2;

	all_tests[4] = (struct test) {"Arithmetic identities", -1, 0x0776bcda87, &test_cint_arithmetic_identities};
	all_tests[8] = (struct test) {"Distributivity and Associativity", -1, 0x20b3b0cea7e8, &test_cint_distributivity_associativity};
	all_tests[12] = (struct test) {"Left and Right shifts", -1, 0xb8653cfb0c8122, &test_cint_shifts};
	all_tests[16] = (struct test) {"String conversion", -1, 0x513555d4cd, &test_cint_string_conversion};
	all_tests[20] = (struct test) {"Random generator", -1, 0xa44c5dfce7daa, &test_cint_random_bits};
	all_tests[24] = (struct test) {"Division", -1, 0xd060e85ea23472, &test_cint_division};
	all_tests[28] = (struct test) {"Power and modulo", -1, 0xecbec6c317, &test_cint_pow_mod};
	all_tests[32] = (struct test) {"Double precision numbers", -1, 0x0e33070e9f503, &test_cint_double_roundtrip};
	all_tests[36] = (struct test) {"Corner cases", -1, 0xd761de5237, &test_cint_corner_cases};
	all_tests[40] = (struct test) {"Primality", -1, 0xc157613ee82, &test_cint_is_prime};

	int n_success = 0, n_failures = 0;
	for (int i = 0; i < size_tests; ++i)
		if (all_tests[i].status != -2) {
			all_tests[i].seed ^= seed;
			printf("%40s ...\n", all_tests[i].name);
			all_tests[i].status = all_tests[i].func(&all_tests[i].seed);
			n_success += all_tests[i].status == 0;
			n_failures += all_tests[i].status != 0;
		}

	putchar('\n');

	for (int i = 0; i < size_tests; ++i)
		if (all_tests[i].status != -2)
			printf("%40s ... %-10s\n", all_tests[i].name, all_tests[i].status ? "[FAILED]" : "[PASS]");

	printf("\nCompleted with %d success and %d failures.\n", n_success, n_failures);

	return n_failures;
}

// Compilation is done using "gcc -Wall -pedantic -O2 -std=c99 main.c -o demo"
