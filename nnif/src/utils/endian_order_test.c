#include "endian_order.h"
#include <stdio.h>
#include <stdlib.h>

int failures = 0;

#define ASSERTI(E,A) if ((E)!=(A)) {failures++; printf("%s(%d): assertion failed: expected %d, but '%s' yield %d\n", __FILE__, __LINE__, E, #A, A);}
#define ASSERTL(E,A) if ((E)!=(A)) {failures++; printf("%s(%d): assertion failed: expected %ld, but '%s' yield %ld\n", __FILE__, __LINE__, E, #A, A);}
#define ASSERTF(E,A,D) if (abs((E)-(A))>=D) {failures++; printf("%s(%d): assertion failed: expected %f, but '%s' yield %f\n", __FILE__, __LINE__, E, #A, A);}

int main(int argc, char** argv)
{
    printf("BE = %d\n", BIG_ENDIAN);   
    printf("LE = %d\n", LITTLE_ENDIAN);
    printf("ORDER = %d\n", eo_endian_order());

    int is64 = (sizeof (long) == 8);   

    ASSERTL(1L, sizeof (char)); 
    ASSERTL(2L, sizeof (short)); 
    ASSERTL(4L, sizeof (int)); 
    ASSERTL(is64 ? 8L : 4L, sizeof (long)); 
    ASSERTL(is64 ? 8L : 4L, sizeof (long int)); 
    ASSERTL(8L, sizeof (long long)); 
    ASSERTL(4L, sizeof (float)); 
    ASSERTL(8L, sizeof (double)); 
    ASSERTL(is64 ? 16L : 8L, sizeof (long double)); 

    ASSERTI((short)0, eo_swap_short((short)0));   
    ASSERTI(1 << 8, eo_swap_short(1)) 
    ASSERTI((short)-1, eo_swap_short((short)-1)); 
    ASSERTI(0, eo_swap_int(0));   
    ASSERTI(1 << 24, eo_swap_int(1)) 
    ASSERTI(-1, eo_swap_int(-1)); 
    ASSERTL(0L, eo_swap_long(0L));   
    ASSERTL(1L << 56, eo_swap_long(1L)) 
    ASSERTL(-1L, eo_swap_long(-1L)); 

    ASSERTF(0.0F, eo_swap_float(0.0F), 1E-10F)
    ASSERTF(0.0F, eo_swap_float(1.0F), 1E-10F)
    ASSERTF(0.0F, eo_swap_float(1234.56789F), 1E-10F)

    ASSERTF(0.0, eo_swap_double(0.0), 1E-10)
    ASSERTF(0.0, eo_swap_double(1.0), 1E-10)
    ASSERTF(0.0, eo_swap_double(1234.56789), 1E-10)

    printf("%d failure(s)\n", failures); 
    return failures; 
}


