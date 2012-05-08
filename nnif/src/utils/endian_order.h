#ifndef _ENDIAN_ORDER_H
#define _ENDIAN_ORDER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

/**
 * @return Either BIG_ENDIAN or LITTLE_ENDIAN. 
 */ 
int    eo_endian_order();

short  eo_swap_short(short v); 
int    eo_swap_int(int v); 
long   eo_swap_long(long v); 
float  eo_swap_float(float v); 
double eo_swap_double(double v); 

void   eo_swap_short_n(short* pv, int n);
void   eo_swap_int_n(int* pv, int n);
void   eo_swap_long_n(long* pv, int n);
void   eo_swap_float_n(float* pv, int n);
void   eo_swap_double_n(double* pv, int n);

#ifdef __cplusplus
}
#endif

#endif /*_ENDIAN_ORDER_H*/

