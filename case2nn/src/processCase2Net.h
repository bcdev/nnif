#ifndef processCase2Net_H_INCL
#define processCase2Net_H_INCL

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration for neural net structure */
struct SNnNet;

/* Pointer to neural net structure */
typedef struct SNnNet* NN_PNET;

/**
 * The processCase2Net function processes a neural net which was converted from the
 * GKSS-FFBP format to the NNF format used by the MERIS level 2 processor.
 * <p>
 * The original FFBP net was trained with input vectors having the following definition: * <p>
 *     1: sun_thet in [1.8, 82.3] <br>
 *     2: thetav in [0.07261, 45] <br>
 *     3: phi in [0, 180] <br>
 *     4: log(rlw(412.3)) in [-9.959, -1.50657] <br>
 *     5: log(rlw(442.3)) in [-9.71272, -1.50686] <br>
 *     6: log(rlw(489.7)) in [-7.7646, -1.52844] <br>
 *     7: log(rlw(509.6)) in [-7.96234, -1.6178] <br>
 *     8: log(rlw(559.5)) in [-7.54114, -1.7376] <br>
 *     9: log(rlw(619.4)) in [-9.912, -2.27191] <br>
 *    10: log(rlw(664.3)) in [-10.2, -2.51176] <br>
 *    11: log(rlw(708.1)) in [-10.41, -2.83689] <br>
 * <p>
 * The original FFBP net was trained with output vectors having the following definition:
 * <p>
 *     1: log(conc_bpart) in [-2.987, 3.807] <br>
 *     2: log(conc_apig) in [-5.298, -0.0001] <br>
 *     3: log(conc_agelb) in [-5.298, 0.4055] <br>
 * <p>
 * The ranges are used to normalize the in- and output vectors to values
 * in the range [0, 1].
 * This normalisation must be part of the neural net
 * given by <code>pNet</code> and is not performed within the processCase2Net
 * function.
 * 
 * @param pNet the neural net
 * @param pdInp input vector, points to an array of at least 11 double values
 * @param pdOut output vector, points to an array of at least 4 double values
 *              the last element contains the out-of-scope flag
 *              having either the value 0.0 or 1.0
 */
void processCase2Net(NN_PNET pNet, const double* pdInp, double* pdOut);

#ifdef __cplusplus
}
#endif

#endif /* processCase2Net_H_INCL */
