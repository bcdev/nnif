#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <NnBase.h>
#include <NnProc.h>
#include "processCase2Net.h"

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
void processCase2Net(NN_PNET pNet, const double* pdInp, double* pdOut)
{
	double adInp[11];

	adInp[ 0] = pdInp[ 0];
	adInp[ 1] = pdInp[ 1];
	adInp[ 2] = pdInp[ 2];
	adInp[ 3] = log(pdInp[ 3]);
	adInp[ 4] = log(pdInp[ 4]);
	adInp[ 5] = log(pdInp[ 5]);
	adInp[ 6] = log(pdInp[ 6]);
	adInp[ 7] = log(pdInp[ 7]);
	adInp[ 8] = log(pdInp[ 8]);
	adInp[ 9] = log(pdInp[ 9]);
	adInp[10] = log(pdInp[10]);

	Nn_ProcessNet(pNet, adInp, pdOut);

	pdOut[ 0] = exp(pdOut[ 0]);
	pdOut[ 1] = exp(pdOut[ 1]);
	pdOut[ 2] = exp(pdOut[ 2]);
}
