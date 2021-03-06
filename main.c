#include "cint.c"

#include <stdio.h>

int cint_print(const cint *num) {
    char *str = cint_to_string(num, 10);
    int res = printf("%s", str);
    free(str);
    return res;
}

int main(void) {
    unsigned n_fibonacci = 500, n_factorial = 70;
    h_cint_begin();
    cint A, B, C;
    cint_init(&A, 4000, 0);
    cint_init(&B, 4000, 1);
    cint_init(&C, 4000, 1);
    for (unsigned i = 0; i < n_fibonacci; ++i) {
        cint_dup(&C, &A);
        cint_addi(&A, &B);
        cint_dup(&B, &C);
    }
    printf("Fibonacci %u is ", n_fibonacci);
    cint_print(&A);

    cint_reinit(&A, 1);
    for (unsigned i = 0; i < n_factorial;) {
        cint_mul(&A, cint_immediate(++i), &B);
        cint_mul(&B, cint_immediate(++i), &A);
    }
    printf("\nFactorial %u is ", n_factorial);
    cint_print(n_factorial & 1 ? &B : &A);

    free(A.mem);
    free(B.mem);
    free(C.mem);
    h_cint_clears();
}

// You can put it into a main.c file then compile + execute :
// gcc -O3 -std=c99 -Wall -pedantic main.c ; ./a.out ;
