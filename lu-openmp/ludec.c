#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

/*
 * An example realization of LU-decomposition algorithm
 * for solving linear algebra equation systems using OpenMP
 */

enum {
    n = 1000
};

void print_a_matrix(double *a, int n)
{
    uint64_t i, j;
    for (i = 0; i < n; ++i) {
        printf("| ");
        for (j = 0; j < n; ++j) {
            printf("%.9lf ", a[i * n + j]);
        }
        printf(" |\n");
    }
    printf("\n");
}

void *supermalloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Sorry, malloc failed.\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

double supertimer()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

void lu_dec_serial_no_piv(double *a, int n)
{
    int i, k;
    
    for (k = 0; k < n - 1; ++k) {
        for(i = k + 1; i < n; i++) {
            a[i * n + k] /= a[k * n + k];
        }

        for (i = k + 1; i < n; i++) {
            int j;
            double a_ik = a[i * n + k];
            for (j = k + 1; j < n; j++) {
                a[i * n + j] -= a_ik * a[k * n + j];
            }
        }
    }
}

void lu_dec_parallel_no_piv(double *a, int n)
{
    int i, k;
    
    for (k = 0; k < n - 1; ++k) {
        for(i = k + 1; i < n; i++) {
            a[i * n + k] /= a[k * n + k];
        }

        #pragma omp parallel for shared(a, n, k) private(i) schedule(static, 64)
        for (i = k + 1; i < n; i++) {
            int j;
            double a_ik = a[i * n + k];
            for (j = k + 1; j < n; j++) {
                a[i * n + j] -= a_ik * a[k * n + j];
            }
        }
    }
}

int lu_dec_serial(double *a, int pivot[], int n)
{
    int i, j, k;
    double *p_k, *p_row, *p_col;
    double max;
    
    for (k = 0, p_k = a; k < n; p_k += n, k++) {
        
        // Finding the pivot row.
        
        pivot[k] = k;
        max = fabs(*(p_k + k));
        for (j = k + 1, p_row = p_k + n, p_col = p_row; j < n; j++, p_row += n) {
            if (max < fabs(*(p_row + k))) {
                max = fabs(*(p_row + k));
                pivot[k] = j;
                p_col = p_row;
            }
        }
        
        // Interchanging the pivot row with the current row (if they differ).
        
        if (pivot[k] != k) {
            for (j = 0; j < n; j++) {
                max = *(p_k + j);
                *(p_k + j) = *(p_col + j);
                *(p_col + j) = max;
            }
        }
        
        // If our matrix turns out to be singular, returning an error.
        
        if (*(p_k + k) == 0.0)
            return -1;

        for (j = k + 1; j < n; j++) {
            *(p_k + j) /= *(p_k + k);
        }
        
        p_row = p_k + n;
        for (i = k + 1; i < n; i++) {
            for (j = k + 1; j < n; j++)
                *(p_row + j) -= *(p_row + k) * *(p_k + j);
            p_row += n;
        }
    }
    return 0;
}

int lu_dec_parallel(double *a, int pivot[], int n)
{
    int i, j, k;
    double *p_k, *p_row, *p_col;
    double max;
    
    for (k = 0, p_k = a; k < n; p_k += n, k++) {
        
        // Finding the pivot row.
        
        pivot[k] = k;
        max = fabs(*(p_k + k));
        for (j = k + 1, p_row = p_k + n, p_col = p_row; j < n; j++, p_row += n) {
            if (max < fabs(*(p_row + k))) {
                max = fabs(*(p_row + k));
                pivot[k] = j;
                p_col = p_row;
            }
        }
        
        // Interchanging the pivot row with the current row (if they differ).
        
        if (pivot[k] != k) {
            for (j = 0; j < n; j++) {
                max = *(p_k + j);
                *(p_k + j) = *(p_col + j);
                *(p_col + j) = max;
            }
        }
        
        // If our matrix turns out to be singular, returning an error.
        
        if (*(p_k + k) == 0.0)
            return -1;

        for (j = k + 1; j < n; j++) {
            *(p_k + j) /= *(p_k + k);
        }
        
        p_row = p_k + n;
        #pragma omp parallel for shared(p_row, p_k, n, k) private(i) schedule(dynamic)
        for (i = k + 1; i < n; i++) {
            for (j = k + 1; j < n; j++)
                *(p_row + j) -= *(p_row + k) * *(p_k + j);
            p_row += n;
        }
    }
    return 0;
}

int lu_solve(double *LU, double B[], int pivot[], double x[], int n)
{
    int i, k;
    double *p_k;
    double dum;
   
    for (k = 0, p_k = LU; k < n; p_k += n, k++) {
        if (pivot[k] != k) {
            dum = B[k];
            B[k] = B[pivot[k]];
            B[pivot[k]] = dum;
        }
        x[k] = B[k];
        for (i = 0; i < k; i++)
            x[k] -= x[i] * *(p_k + i);
        x[k] /= *(p_k + k);
    }

    for (k = n - 1, p_k = LU + n * (n - 1); k >= 0; k--, p_k -= n) {
        if (pivot[k] != k) {
            dum = B[k];
            B[k] = B[pivot[k]];
            B[pivot[k]] = dum;
        }
        for (i = k + 1; i < n; i++)
            x[k] -= x[i] * *(p_k + i);
        if (*(p_k + k) == 0.0)
            return -1;
    }
    
    return 0;
}

double run_serial()
{
    double *a, *b, *x;
    int *pivot;
    
    a = supermalloc(sizeof(*a) * n * n);
    b = supermalloc(sizeof(*b) * n);
    x = supermalloc(sizeof(*x) * n);
    pivot = supermalloc(sizeof(*pivot) * n);
    
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            a[i * n + j] = i + j + 2.0;
    
    for (int i = 0; i < n; ++i)
        b[i] = x[i] = pivot[i] = 0.0;
    
    //print_a_matrix(a, n);
    
    double t = supertimer();
    //lu_dec_serial_no_piv(a, n);
    lu_dec_serial(a, pivot, n);
    lu_solve(a, b, pivot, x, n);
    t = supertimer() - t;
    
    printf("Elapsed time (serial): %.6lf sec.\n", t);
    free(a);
    free(b);
    free(x);
    return t;
}

double run_parallel()
{
    double *a, *b, *x;
    int *pivot;
    
    a = supermalloc(sizeof(*a) * n * n);
    b = supermalloc(sizeof(*b) * n);
    x = supermalloc(sizeof(*x) * n);
    pivot = supermalloc(sizeof(*pivot) * n);
    
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            a[i * n + j] = i + j + 2.0;
    
    for (int i = 0; i < n; ++i)
        b[i] = x[i] = pivot[i] = 0.0;
    
    //print_a_matrix(a, n);
    
    double t = supertimer();
    //lu_dec_parallel_no_piv(a, n);
    lu_dec_parallel(a, pivot, n);
    lu_solve(a, b, pivot, x, n);
    t = supertimer() - t;
    
    printf("Elapsed time (parallel): %.6lf sec.\n", t);
    free(a);
    free(b);
    free(x);
    return t;
}

int main(int argc, char **argv)
{
    printf("Linear equation systems solving program, using LU-decomposition algorithm.\n");
    printf("A[n][n] matrix is used, n is %d.\n", n);
    printf("Using %d threads, %d processors.\n", omp_get_max_threads(), omp_get_num_procs());
    double tser = run_serial();
    double tpar = run_parallel();
    printf("Speedup is %.2lf\n", tser / tpar);
    return 0;
}
