#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();

    //double* ptr = sf_malloc(4);
    double* ptr = sf_malloc(4000);
    double* ptr1= sf_malloc(16);
    sf_show_heap();

    //sf_malloc(sizeof(double));
    //*ptr2 = 2;
    ///not showiung the split free block on the heap  to fix
    *ptr = 4;
    *ptr1 = 4;

    //sf_realloc(ptr, 10);


     //sf_free(ptr);
     sf_free(ptr1);

    // sf_show_heap();
    // sf_free(ptr1);

    sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
