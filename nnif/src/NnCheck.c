/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnCheck.c                                                     */
/* Purpose:     Implementation of the neural net checking routines            */
/* Remarks:     Interface defined in NnCheck.h                                */
/* Author:      Norman Fomferra (SCICON)                                      */
/*              Tel:   +49 4152 1457 (tel)                                    */
/*              Fax:   +49 4152 1455 (fax)                                    */
/*              Email: Norman.Fomferra@gkss.de                                */
/*////////////////////////////////////////////////////////////////////////////*/

#include <stdio.h>
#include <assert.h>

#include "NnBase.h"
#include "NnCheck.h"

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_AssertSemanticIntegrity                                     */
/* Purpose:    Checks and corrects the sementic integrity of a neural net object */
/* Remarks:    This function is called before the Nn_ReadBinFile returns.        */
/*             If a net object is created in memory, this function shall be      */
/*             called before Nn_ProcessNet is used.                              */
/* Returns:    NN_OK if the semantic integrity of the net is observed, an        */
/*             error code otherwise. The error code can be the following         */
/*             - NN_INCONSITENT_NET                                              */
/*             - NN_INVALID_FIELD                                                */
/*             - NN_INCOMPLETE_STRUCTURE                                         */
/*             - NN_INVALID_PARAMETER                                            */
/*////////////////////////////////////////////////////////////////////////////   */

NN_STATUS Nn_AssertSemanticIntegrity
(
	NN_PNET pNet, 
	int     nNumInpUnits,
	int     nNumOutUnits
)
{
	short     iL, iU, iC;
	NN_PLAYER pLayer, pSrcLayer;
	NN_PUNIT  pUnit;
	NN_PCONN  pConn;
	BOOL      bRbf;

	assert(pNet != NULL);

	/* Check number of layers */
	if (pNet->na.nNumLayers <= 0)
		return Nn_Error(NN_INVALID_ATTRIBUTE, 
			NN_ERR_PREFIX "invalid number of units: %d (should be > 0)", 
			pNet->na.nNumLayers);

	/* Check layer array */
	if (pNet->aLayers == NULL)
		return Nn_Error(NN_INCOMPLETE_STRUCTURE, 
			NN_ERR_PREFIX "no layers defined");

	/* Make the IO of the net consistent: */
	/* Set input layer default index     */
	if (pNet->na.iInpLayer < 0)
		pNet->na.iInpLayer = 0;
	/* Set output layer default index */
	if (pNet->na.iOutLayer < 0)
		pNet->na.iOutLayer = (short)(pNet->na.nNumLayers - 1);

	/* Check input layer index */
	if (pNet->na.iInpLayer >= pNet->na.nNumLayers)
		return Nn_Error(NN_INCONSITENT_NET, 
			NN_ERR_PREFIX "L[%d]: invalid input layer index: %d (should be >= 0 and < %d)", 
			pNet->na.iInpLayer, pNet->na.nNumLayers);

	/* Check output layer index */
	if (pNet->na.iOutLayer >= pNet->na.nNumLayers)
		return Nn_Error(NN_INCONSITENT_NET, 
			NN_ERR_PREFIX "L[%d]: invalid output layer index: %d (should be >= 0 and < %d)", 
			pNet->na.iOutLayer, pNet->na.nNumLayers);

	/* Check number of input units */
	if (nNumInpUnits > 0 && pNet->aLayers[pNet->na.iInpLayer].la.nNumUnits != nNumInpUnits)
		return Nn_Error(NN_INVALID_ATTRIBUTE, 
			NN_ERR_PREFIX "L[%d]: invalid number of input units: %d (%d expected)", 
			pNet->na.iInpLayer, pNet->aLayers[pNet->na.iInpLayer].la.nNumUnits, nNumInpUnits);

	/* Check number of output units */
	if (nNumOutUnits > 0 && pNet->aLayers[pNet->na.iOutLayer].la.nNumUnits != nNumOutUnits)
		return Nn_Error(NN_INVALID_ATTRIBUTE, 
			NN_ERR_PREFIX "L[%d]: invalid number of output units: %d (%d expected)", 
			pNet->na.iOutLayer, pNet->aLayers[pNet->na.iOutLayer].la.nNumUnits, nNumOutUnits);

	/* Check precision */
	switch (pNet->na.nPrecision)
	{
	case NN_PREC_SINGLE:
	case NN_PREC_DOUBLE:
		break;
	default:
		return Nn_Error(NN_INVALID_ATTRIBUTE, 
			NN_ERR_PREFIX "invalid precision: %d");
	}

	/* Check all layers of the net: */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get layer */
		pLayer = Nn_GetLayerAt(pNet, iL);

		/* Set layer ID */
		pLayer->la.iLayer = iL; 

		/* Check number of units */
		if (pLayer->la.nNumUnits <= 0)
			return Nn_Error(NN_INVALID_ATTRIBUTE, 
				NN_ERR_PREFIX "L[%d]: invalid number of units: %d (should be > 0)", 
				iL, pLayer->la.nNumUnits);

		/* Check unit array */
		if (pLayer->aUnits == NULL)
			return Nn_Error(NN_INCOMPLETE_STRUCTURE, 
				NN_ERR_PREFIX "L[%d]: no units defined", 
				iL);

		/* Check input function identifier: */
		switch (pLayer->la.nInpFnId)
		{
		/* List all valid input functions here... */
		case NN_FUNC_ZERO:
		case NN_FUNC_SUM_1:
		case NN_FUNC_SUM_2:
			break;
		default:
			return Nn_Error(NN_INVALID_ATTRIBUTE, 
				NN_ERR_PREFIX "L[%d]: invalid input function ID %d", 
				iL, pLayer->la.nInpFnId);
		}

		/* Check activation function identifier: */
		switch (pLayer->la.nActFnId)
		{
		/* List all valid activation functions here... */
		case NN_FUNC_IDENTITY:
		case NN_FUNC_THRESHOLD:
		case NN_FUNC_LINEAR:
		case NN_FUNC_SEMILINEAR:
		case NN_FUNC_SIGMOID_1:
		case NN_FUNC_SIGMOID_2:
		case NN_FUNC_RBF_1:
		case NN_FUNC_RBF_2:
			break;
		default:
			return Nn_Error(NN_INVALID_ATTRIBUTE, 
				NN_ERR_PREFIX "L[%d]: invalid activation function ID %d", 
				iL, pLayer->la.nActFnId);
		}

		/* Check output function identifier: */
		switch (pLayer->la.nOutFnId)
		{
		/* List all valid output functions here... */
		case NN_FUNC_IDENTITY:
		case NN_FUNC_LINEAR:
		case NN_FUNC_QUADRATIC:
		case NN_FUNC_EXPONENTIAL:
			break;
		default:
			return Nn_Error(NN_INVALID_ATTRIBUTE, 
				NN_ERR_PREFIX "L[%d]: invalid output function ID %d", 
				iL, pLayer->la.nOutFnId);
		}

		/* Check all units of the layer: */
		for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
		{
			/* Get unit */
			pUnit = Nn_GetUnitAt(pLayer, iU);

			/* Set layer and unit ID */
			pUnit->ua.iLayer     = iL; 
			pUnit->ua.iUnit      = iU;

			/* Check number of incoming connections */
			if (pUnit->ua.nNumConns < 0)
				return Nn_Error(NN_INVALID_ATTRIBUTE, 
					NN_ERR_PREFIX "U[%d][%d]: invalid number of connections: %d (should >= 0)", 
					iL, iU, pUnit->ua.nNumConns);

			/* Check connection array */
			if (pUnit->ua.nNumConns > 0 && pUnit->aConns == NULL)
				return Nn_Error(NN_INCOMPLETE_STRUCTURE, 
					NN_ERR_PREFIX "U[%i][%i]: no connections defined", 
					iL, iU);
			
			/* Mark that a matrix is existing */
			pUnit->ua.bHasMatrix = (short)(pUnit->ppfMatrix != NULL);
			/* Check matrix */
			bRbf = pLayer->la.nActFnId == NN_FUNC_RBF_1 ||
			       pLayer->la.nActFnId == NN_FUNC_RBF_2; 

			if (bRbf && pUnit->ua.nNumConns > 0 && !pUnit->ua.bHasMatrix)
				return Nn_Error(NN_INCOMPLETE_STRUCTURE, 
					NN_ERR_PREFIX "U[%d][%d]: no matrix defined", 
					iL, iU);

			if (!bRbf && pUnit->ua.bHasMatrix)
				return Nn_Error(NN_INVALID_ATTRIBUTE, 
					NN_ERR_PREFIX "U[%d][%d]: matrix can't be defined", 
					iL, iU);

			/* Check all incoming connections of the unit */
			for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
			{
				/* Get connection */
				pConn = Nn_GetConnAt(pUnit, iC);

				/* Check source layer index: */
				if (pConn->ca.iLayer < 0 || pConn->ca.iLayer >= pNet->na.nNumLayers)
					return Nn_Error(NN_INCONSITENT_NET, 
						NN_ERR_PREFIX "C[%d][%d][%d]: invalid layer index %d (should be >= 0 and < %d)", 
						iL, iU, iC, pNet->na.nNumLayers);

				/* Get source layer */
				pSrcLayer = Nn_GetLayerAt(pNet, pConn->ca.iLayer);

				/* Check source unit index: */
				if (pConn->ca.iUnit < 0 || pConn->ca.iUnit >= pSrcLayer->la.nNumUnits)
					return Nn_Error(NN_INCONSITENT_NET, 
						NN_ERR_PREFIX "C[%d][%d][%d]: invalid unit index %d (should be >= 0 and < %d)", 
						iL, iU, iC, pSrcLayer->la.nNumUnits);

				/* Set the source unit of the connection */
				pConn->pUnit = Nn_GetUnitAt(pSrcLayer, pConn->ca.iUnit);
			}
		}
	}

	return NN_OK;
}


/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_PrintLayerOutputs                                             */
/* Purpose:  Prints the outputs of all units of all layers                    */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_PrintLayerOutputs(NN_PNET pNet, /* the net */
                          FILE* ostream, /* file output stream */
                          const char * format) /* fprintf output format string 
                                                  for a single unit value */
{
	short     iL;
	NN_PLAYER pLayer;

	short     iU;
	NN_PUNIT  pUnit;

    /* For all layers */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get the layer at the given position */
		pLayer = pNet->aLayers + iL;

		fprintf(ostream, "L[%d]: ", iL+1);

        /* For all units of the given layer */
	    for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	    {
		    /* Get the unit at the given position */
		    pUnit = pLayer->aUnits + iU;
		    /* Print unit output value */
		    if (iU > 0)
                fputs(" ", ostream);
            fprintf(ostream, format == NULL ? "%g" : format, pUnit->fOut);
	    }

        fputs("\n", ostream);
    }
}
