/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnProc.c                                                      */
/* Purpose:     Implementation of the neural net processing routines          */
/* Remarks:     Interface defined in NnProc.h                                 */
/* Author:      Norman Fomferra (SCICON)                                      */
/*              Tel:   +49 4152 1457 (tel)                                    */
/*              Fax:   +49 4152 1455 (fax)                                    */
/*              Email: Norman.Fomferra@gkss.de                                */
/*////////////////////////////////////////////////////////////////////////////*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

#include "NnBase.h"
#include "NnProc.h"

/*////////////////////////////////////////////////////////////////////////////*/
/* Module local prototypes:                                                   */
/*                                                                            */

void Nn_SetInput_f32   (NN_PLAYER pLayer, const float  * afInp);
void Nn_SetInput       (NN_PLAYER pLayer, const double * adInp);

void Nn_GetOutput_f32  (NN_PLAYER pLayer, float  * afOut);
void Nn_GetOutput      (NN_PLAYER pLayer, double * adOut);

void Nn_CalcInpFn  (NN_PNET pNet, NN_PLAYER pLayer);
void Nn_CalcActFn  (NN_PLAYER pLayer);
void Nn_CalcOutFn  (NN_PLAYER pLayer);

void Nn_CalcInpFnZero  (NN_PNET pNet, NN_PLAYER pLayer);
void Nn_CalcInpFnSum1  (NN_PNET pNet, NN_PLAYER pLayer);
void Nn_CalcInpFnSum2  (NN_PNET pNet, NN_PLAYER pLayer);

void Nn_CalcActFnIdentity    (NN_PLAYER pLayer);
void Nn_CalcActFnThreshold   (NN_PLAYER pLayer);
void Nn_CalcActFnLinear      (NN_PLAYER pLayer);
void Nn_CalcActFnSemiLinear  (NN_PLAYER pLayer);
void Nn_CalcActFnSigmoid1    (NN_PLAYER pLayer);
void Nn_CalcActFnSigmoid2    (NN_PLAYER pLayer);
void Nn_CalcActFnRbf1        (NN_PLAYER pLayer);
void Nn_CalcActFnRbf2        (NN_PLAYER pLayer);

void Nn_CalcOutFnIdentity    (NN_PLAYER pLayer);
void Nn_CalcOutFnLinear      (NN_PLAYER pLayer);
void Nn_CalcOutFnQuadratic   (NN_PLAYER pLayer);
void Nn_CalcOutFnExponential (NN_PLAYER pLayer);
void Nn_CalcOutFnLogarithmic (NN_PLAYER pLayer);

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ProcessNet_f32                                                */
/* Purpose:  Computes the net output from a given net input for 4 byte floats. */
/* Remarks:  IMPORTANT: This function shall only be used if a previous call to */
/*           Nn_AssertSemanticIntegrity returned NN_OK.                        */
/* Returns:  No return value                                                   */
/*//////////////////////////////////////////////////////////////////////////// */

void Nn_ProcessNet_f32
(
	NN_PNET        pNet,  /* The neural net object */
	const float*   afInp, /* Net input vector     */
	float*         afOut  /* Net output vector    */
)
{
	short     iL;
	NN_PLAYER pLayer;

	/* For all layers */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get the layer at the given position */
		pLayer = pNet->aLayers + iL;

		/* Calculate the input function */
		Nn_CalcInpFn(pNet, pLayer);

		/* If this is the input layer, add the input vector (4 bytes) */
		if (iL == pNet->na.iInpLayer)
			Nn_SetInput_f32(pLayer, afInp);

		/* Calculate the activation function */
		Nn_CalcActFn(pLayer);
		
		/* Calculate the output function */
		Nn_CalcOutFn(pLayer);

		/* If this is the output layer, get the output vector (4 bytes) */
		if (iL == pNet->na.iOutLayer)
			Nn_GetOutput_f32(pLayer, afOut);
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ProcessNet                                               */
/* Purpose:  Computes the net output from a given net input for 8 byte floats. */
/* Remarks:  IMPORTANT: This function shall only be used if a previous call to */
/*           Nn_AssertSemanticIntegrity returned NN_OK.                        */
/* Returns:  No return value                                                   */
/*//////////////////////////////////////////////////////////////////////////// */

void Nn_ProcessNet
(
	NN_PNET        pNet,  /* The neural net object */
	const double*  adInp, /* Net input vector     */
	double*        adOut  /* Net output vector    */
)
{
	short     iL;
	NN_PLAYER pLayer;

	/* For all layers */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get the layer at the given position */
		pLayer = pNet->aLayers + iL;

		/* Calculate the input function */
		Nn_CalcInpFn(pNet, pLayer);

		/* If this is the input layer, add the input vector (8 bytes) */
		if (iL == pNet->na.iInpLayer)
			Nn_SetInput(pLayer, adInp);

		/* Calculate the activation function */
		Nn_CalcActFn(pLayer);
		
		/* Calculate the output function */
		Nn_CalcOutFn(pLayer);

		/* If this is the output layer, get the output vector (8 bytes) */
		if (iL == pNet->na.iOutLayer)
			Nn_GetOutput(pLayer, adOut);
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_SetInput_f32                                                  */
/* Purpose:  Feeds the input layer with the input vector (4 byte float)       */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_SetInput_f32(NN_PLAYER pLayer, const float * afInp)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		pUnit = pLayer->aUnits + iU;
		pUnit->fInp += (NN_FLOAT) afInp[iU];
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_SetInput                                                      */
/* Purpose:  Feeds the input layer with the input vector (8 byte float)       */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_SetInput(NN_PLAYER pLayer, const double * adInp)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		pUnit = pLayer->aUnits + iU;
		pUnit->fInp += adInp[iU];
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_GetOutput_f32                                                 */
/* Purpose:  Gets the output vector (4 byte float) from the output layer      */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_GetOutput_f32(NN_PLAYER pLayer, float * afOut)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		pUnit = pLayer->aUnits + iU;
		afOut[iU] = (float) pUnit->fOut;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_GetOutput                                                     */
/* Purpose:  Gets the output vector (8 byte float) from the output layer      */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_GetOutput(NN_PLAYER pLayer, double * adOut)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		pUnit = pLayer->aUnits + iU;
		adOut[iU] = pUnit->fOut;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcInpFn                                                     */
/* Purpose:  Calculates the input function for the given layer                */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcInpFn(NN_PNET pNet, NN_PLAYER pLayer)
{
	assert(pNet != NULL);
	assert(pLayer != NULL);

	switch (pLayer->la.nInpFnId)
	{
	case NN_FUNC_ZERO:
		Nn_CalcInpFnZero(pNet, pLayer);
		break;
	case NN_FUNC_SUM_1:
		Nn_CalcInpFnSum1(pNet, pLayer);
		break;
	case NN_FUNC_SUM_2:
		Nn_CalcInpFnSum2(pNet, pLayer);
		break;
	default:
		assert(FALSE); /* TODO: Add error handler here... */
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcActFn                                                     */
/* Purpose:  Calculates the activation function for the given layer           */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcActFn(NN_PLAYER pLayer)
{
	assert(pLayer != NULL);

	switch (pLayer->la.nActFnId)
	{
	case NN_FUNC_IDENTITY:
		Nn_CalcActFnIdentity(pLayer);
		break;
	case NN_FUNC_THRESHOLD:
		Nn_CalcActFnThreshold(pLayer);
		break;
	case NN_FUNC_LINEAR:
		Nn_CalcActFnLinear(pLayer);
		break;
	case NN_FUNC_SEMILINEAR:
		Nn_CalcActFnSemiLinear(pLayer);
		break;
	case NN_FUNC_SIGMOID_1:
		Nn_CalcActFnSigmoid1(pLayer);
		break;
	case NN_FUNC_SIGMOID_2:
		Nn_CalcActFnSigmoid2(pLayer);
		break;
	case NN_FUNC_RBF_1:
		Nn_CalcActFnRbf1(pLayer);
		break;
	case NN_FUNC_RBF_2:
		Nn_CalcActFnRbf2(pLayer);
		break;
	default:
		assert(FALSE); /* TODO: Add error handler here... */
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcOutFn                                                     */
/* Purpose:  Calculates the output function for the given layer               */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcOutFn(NN_PLAYER pLayer)
{
	switch (pLayer->la.nOutFnId)
	{
	case NN_FUNC_IDENTITY:
		Nn_CalcOutFnIdentity(pLayer);
		break;
	case NN_FUNC_LINEAR:
		Nn_CalcOutFnLinear(pLayer);
		break;
	case NN_FUNC_QUADRATIC:
		Nn_CalcOutFnQuadratic(pLayer);
		break;
	case NN_FUNC_EXPONENTIAL:
		Nn_CalcOutFnExponential(pLayer);
		break;
	case NN_FUNC_LOGARITHMIC:
		Nn_CalcOutFnLogarithmic(pLayer);
		break;
	default:
		assert(FALSE); /* TODO: Add error handler here... */
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcInpFnZero                                                 */
/* Purpose:  Calculates the zero input function for the given layer           */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcInpFnZero(NN_PNET pNet, NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		pUnit = pLayer->aUnits + iU;
		pUnit->fInp = 0.0;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcInpFnSum1                                                 */
/* Purpose:  Calculates the sum 1 input function for the given layer          */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcInpFnSum1(NN_PNET pNet, NN_PLAYER pLayer)
{
	short     iU, iC;
	NN_PUNIT  pUnit;
	NN_PCONN  pConn;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;

		/* Initialize unit input to zero */
		pUnit->fInp = 0.0;

		/* If the unit does not have any incoming connections, continue */
		if (pUnit->ua.nNumConns == 0)
			continue;

		/* For all incoming connections of the given unit */
		for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
		{
			/* Get the connection at the given position */
			pConn = pUnit->aConns + iC;
			/* Add the weighted output of the source unit to the unit input */
			pUnit->fInp += pConn->pUnit->fOut * pConn->ca.fWeight;
		}
		
		/* Calculate the resulting unit input */
		pUnit->fInp *= pUnit->ua.fInpScale;
		pUnit->fInp += pUnit->ua.fInpBias;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcInpFnSum2                                                 */
/* Purpose:  Calculates the sum 2 input function for the given layer          */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcInpFnSum2(NN_PNET pNet, NN_PLAYER pLayer)
{
	short     iU, iC;
	NN_PUNIT  pUnit;
	NN_PCONN  pConn;
	NN_FLOAT  fOut, fOutSum;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;

		/* Initialize output sum to zero */
		fOutSum = 0.0;
		
		/* Initialize unit input to zero */
		pUnit->fInp = 0.0;  

		/* If the unit does not have any incoming connections, continue */
		if (pUnit->ua.nNumConns <= 0)
			continue;

		/* For all incoming connections of the given unit */
		for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
		{
			/* Get the connection at the given position */
			pConn = pUnit->aConns + iC;
			/* Get the output of the source unit */
			fOut  = pConn->pUnit->fOut;
			/* Add the weighted output of the source unit to the unit input */
			pUnit->fInp += fOut * pConn->ca.fWeight;
			/* Add the output to the output sum */
			fOutSum += fOut;
		}
		
		/* Calculate the resulting unit input */
		pUnit->fInp /= fOutSum;
		pUnit->fInp *= pUnit->ua.fInpScale;
		pUnit->fInp += pUnit->ua.fInpBias;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcActFnIdentity                                             */
/* Purpose:  Calculates the identity activation function for the given layer  */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcActFnIdentity(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Activation is equal to the input */
		pUnit->fAct = pUnit->fInp;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcActFnThreshold                                            */
/* Purpose:  Calculates the threshold activation function for the given layer */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcActFnThreshold(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;
	NN_FLOAT  fT = pLayer->la.fActThres;
	NN_FLOAT  fS = pLayer->la.fActSlope;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Calculate threshold function */
		pUnit->fAct = fS * (pUnit->fInp - fT);
		if (pUnit->fAct < 0.0)
			pUnit->fAct = 0.0;
		if (pUnit->fAct > 0.0)
			pUnit->fAct = 1.0;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcActFnLinear                                               */
/* Purpose:  Calculates the linear activation function for the given layer    */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcActFnLinear(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;
	NN_FLOAT  fT = pLayer->la.fActThres;
	NN_FLOAT  fS = pLayer->la.fActSlope;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Calculate linear function */
		pUnit->fAct = fS * (pUnit->fInp - fT);
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcActFnSemiLinear                                           */
/* Purpose:  Calculates the semi-linear activation function for the given layer */
/*           (See neural net interface document PO-TN-MEL-GS-0025)             */
/* Returns:  No return value                                                   */
/*//////////////////////////////////////////////////////////////////////////// */

void Nn_CalcActFnSemiLinear(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;
	NN_FLOAT  fT = pLayer->la.fActThres;
	NN_FLOAT  fS = pLayer->la.fActSlope;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Calculate semi-linear function */
		pUnit->fAct = fS * (pUnit->fInp - fT);
		if (pUnit->fAct < 0.0)
			pUnit->fAct = 0.0;
		if (pUnit->fAct > 1.0)
			pUnit->fAct = 1.0;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcActFnSigmoid1                                             */
/* Purpose:  Calculates the sigmoid 1 activation function for the given layer */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcActFnSigmoid1(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;
	NN_FLOAT  fT = pLayer->la.fActThres;
	NN_FLOAT  fS = pLayer->la.fActSlope;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Calculate sigmoid 1 function */
		pUnit->fAct = 1.0 / (1.0 + exp(fT - fS * pUnit->fInp));
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcActFnSigmoid2                                             */
/* Purpose:  Calculates the sigmoid 1 activation function for the given layer */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Remarks:  NOT IMPLEMENTED YET                                              */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcActFnSigmoid2(NN_PLAYER pLayer)
{
	Nn_CalcActFnIdentity(pLayer);
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcActFnRbf1                                                 */
/* Purpose:  Calculates the radial base function 1 for the given layer        */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Remarks:  NOT IMPLEMENTED YET                                              */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcActFnRbf1(NN_PLAYER pLayer)
{
	Nn_CalcActFnIdentity(pLayer);
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcActFnRbf2                                                 */
/* Purpose:  Calculates the radial base function 2 for the given layer        */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Remarks:  NOT IMPLEMENTED YET                                              */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcActFnRbf2(NN_PLAYER pLayer)
{
	Nn_CalcActFnIdentity(pLayer);
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcOutFnIdentity                                             */
/* Purpose:  Calculates the identity output function for the given layer      */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcOutFnIdentity(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Calculate identity function */
		pUnit->fOut = pUnit->fAct;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcOutFnLinear                                               */
/* Purpose:  Calculates the linear output function for the given layer        */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcOutFnLinear(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Calculate linear function */
		pUnit->fOut = pUnit->ua.fOutScale * pUnit->fAct + pUnit->ua.fOutBias;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcOutFnQuadratic                                            */
/* Purpose:  Calculates the quadratic output function for the given layer     */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcOutFnQuadratic(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Calculate linear function */
		pUnit->fOut = pUnit->ua.fOutScale * pUnit->fAct + pUnit->ua.fOutBias;
		/* Calculate quadratic function */
        pUnit->fOut *= pUnit->fOut;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcOutFnExponential                                          */
/* Purpose:  Calculates the exponential output function for the given layer   */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcOutFnExponential(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Calculate exponential function */
		pUnit->fOut = exp(pUnit->ua.fOutScale * pUnit->fAct + pUnit->ua.fOutBias);
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CalcOutFnLogarithmic                                          */
/* Purpose:  Calculates the exponential output function for the given layer   */
/*           (See neural net interface document PO-TN-MEL-GS-0025)            */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_CalcOutFnLogarithmic(NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;

	/* For all units of the given layer */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the given position */
		pUnit = pLayer->aUnits + iU;
		/* Calculate logarithmic function */
		pUnit->fOut = log(pUnit->ua.fOutScale * pUnit->fAct + pUnit->ua.fOutBias);
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
