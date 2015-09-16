#include <stdio.h>
#include <stdlib.h>

#include "endian_order.h"

void eo_swap2(const char* pv1, char* pv2); 
void eo_swap4(const char* pv1, char* pv2); 
void eo_swap8(const char* pv1, char* pv2);


int eo_endian_order() 
{
    static const short one = 1;
    return *((const char*) &one) == 0 ? BIG_ENDIAN : LITTLE_ENDIAN;
} 

short eo_swap_short(short v) 
{
   short v2;
   if (sizeof (short) == 2)
       eo_swap2((const char*) &v, (char*) &v2);
   else 
       eo_swap4((const char*) &v, (char*) &v2);
   return v2;
}

int eo_swap_int(int v) 
{
   int v2;
   if (sizeof (int) == 4)
       eo_swap4((const char*) &v, (char*) &v2);
   else 
       eo_swap8((const char*) &v, (char*) &v2);
   return v2;
}

long eo_swap_long(long v) 
{
   long v2;
   if (sizeof (long) == 4)
       eo_swap4((const char*) &v, (char*) &v2);
   else
       eo_swap8((const char*) &v, (char*) &v2);   
   return v2;
}

float eo_swap_float(float v) 
{
   float v2;
   eo_swap4((const char*) &v, (char*) &v2);
   return v2;
}

double eo_swap_double(double v) 
{
   double v2;
   eo_swap8((const char*) &v, (char*) &v2);
   return v2;
}

void eo_swap_short_n(short* pv, int n) 
{
    int i;
    for (i = 0; i < n; i++, pv++) 
        *pv = eo_swap_short(*pv);
}

void eo_swap_int_n(int* pv, int n)
{
    int i;
    for (i = 0; i < n; i++, pv++)
        *pv = eo_swap_int(*pv);
}

void eo_swap_long_n(long* pv, int n)
{
    int i;
    for (i = 0; i < n; i++, pv++) 
        *pv = eo_swap_long(*pv);
}

void eo_swap_double_n(double* pv, int n) 
{
    int i;
    for (i = 0; i < n; i++, pv++) 
        *pv = eo_swap_double(*pv);
}

void eo_swap2(const char* pv1, char* pv2) 
{
   pv2[0] = pv1[1];
   pv2[1] = pv1[0];
}

void eo_swap4(const char* pv1, char* pv2) 
{
   pv2[0] = pv1[3];
   pv2[1] = pv1[2];
   pv2[2] = pv1[1];
   pv2[3] = pv1[0];
}

void eo_swap8(const char* pv1, char* pv2) 
{
   pv2[0] = pv1[7];
   pv2[1] = pv1[6];
   pv2[2] = pv1[5];
   pv2[3] = pv1[4];
   pv2[4] = pv1[3];
   pv2[5] = pv1[2];
   pv2[6] = pv1[1];
   pv2[7] = pv1[0];
}

