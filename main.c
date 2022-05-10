#include "cint.c"

#include <stdio.h>
int cint_print(const cint *num) {
    char *str = cint_to_string(num, 10);
    int res = printf("%s", str);
    free(str);
    return res;
}

int main() {
    unsigned n_fibonacci = 500, n_factorial = 70 ;
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
    printf("Fibonacci %d is ", n_fibonacci);
    cint_print(&A);
    cint_reinit(&A, 1);
    for (unsigned i = 2; i <= n_factorial; ++i) {
        cint_reinit(&B, i);
        cint_mul(&A, &B, &C);
        cint_dup(&A, &C);
    }
    printf("\nFactorial %d is ", n_factorial);
    cint_print(&A);

    h_cint_clears();
}
