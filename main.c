#include "cint.c"

#include <stdio.h>

int main(void) {
	char buf[8192];
	unsigned n_fibonacci = 2000, n_factorial = 200;
	cint_sheet *sheet = cint_new_sheet(4000);
	cint nums[4], *A = nums, *B = A + 1, *C = A + 2, *D = A + 3;
	for (int i = 0; i < 4; ++i)
		cint_init(nums + i, 4000, i != 0);

	for (unsigned i = 0; i < n_fibonacci; ++i) {
		cint_dup(C, A);
		cint_addi(A, B);
		cint_dup(B, C);
	}
	printf("Fibonacci %u is %s", n_fibonacci, cint_to_string_buffer(A, buf, 10));

	cint_reinit(A, 1);
	for (unsigned i = 0; i < n_factorial;) {
		cint_reinit(D, ++i);
		cint_mul(A, D, B);
		cint_reinit(D, ++i);
		cint_mul(B, D, A);
	}
	printf("\nFactorial %u is %s", n_factorial, cint_to_string_buffer(n_factorial & 1 ? B : A, buf, 10));
	for (int i = 0; i < 4; ++i)
		free(nums[i].mem);
	cint_clear_sheet(sheet);
}

// You can put it into a main.c file then compile + execute :
// gcc -O3 -std=c99 -Wall -pedantic main.c ; ./a.out ;
