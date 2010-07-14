/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnBase.c                                                      */
/* Purpose:     Implementation file for basic neural net functions            */
/* Remarks:                                                                   */
/* Author:      Norman Fomferra (SCICON)                                      */
/*              Tel:   +49 4152 1457 (tel)                                    */
/*              Fax:   +49 4152 1455 (fax)                                    */
/*              Email: Norman.Fomferra@gkss.de                                */
/*////////////////////////////////////////////////////////////////////////////*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "NnBase.h"

/*////////////////////////////////////////////////////////////////////////////*/
/* Neural net object (NN_PNET) methods                                        */
/*////////////////////////////////////////////////////////////////////////////*/

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_CreateNet                                                   */
/* Purpose:    Creates an initialized neural net object                       */
/* Remarks:    The number of layers is set to 1 by default.                   */
/*             No layers are created.                                         */
/*             The default net attribute settings are:                        */
/*                 anVersion[0] = NN_VERSION_MAJOR                            */
/*                 anVersion[1] = NN_VERSION_MINOR                            */
/*                 nNumLayers   = 1                                           */
/*                 iInpLayer    = -1                                          */
/*                 iOutLayer    = -1                                          */
/*                 nPrecision   = NN_PREC_DOUBLE                              */
/* Returns:    NN_OK (or zero) for success, NN_OUT_OF_MEMORY otherwise        */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_CreateNet (NN_PNET* ppNet)
{
	NN_PNET pNet;

	assert(ppNet != NULL);
	
	pNet = (NN_PNET) calloc(1, sizeof (NN_NET));
	if (pNet == NULL)
		return Nn_SetOutOfMemoryError();

	/* Set the net attribute default values */
	pNet->na.anVersion[0] = NN_VERSION_MAJOR;
	pNet->na.anVersion[1] = NN_VERSION_MINOR;
	pNet->na.nNumLayers   =  1; /* One by default */
	pNet->na.iInpLayer    = -1; /* Means 'not set' */
	pNet->na.iOutLayer    = -1; /* Means 'not set' */
	pNet->na.nPrecision   = NN_PREC_DOUBLE;
	pNet->aLayers         = NULL;

	*ppNet = pNet;
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_DeleteNet                                                   */
/* Purpose:    Releases all memory allocated by the neural net object         */
/* Remarks:    The function deletes also all layers, units and connections    */
/*             owned by the net object                                        */
/* Returns:    No return value                                                */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_DeleteNet (NN_PNET pNet)
{
	if (pNet == NULL)
		return;
	
	Nn_DeleteLayers(pNet);
	free(pNet);
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Net methods concerning the layer objects (NN_PLAYER)                       */

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_LayersCreated                                                 */
/* Purpose:  Checks whether the layers of a neural net object have been       */
/*           created or not                                                   */
/* Returns:  TRUE if the layers have been created, FALSE otherwise            */
/*////////////////////////////////////////////////////////////////////////////*/

BOOL Nn_LayersCreated (const NN_PNET pNet)
{
	assert(pNet != NULL);
	return pNet->na.nNumLayers > 0 && pNet->aLayers != NULL;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_CreateLayers                                                */
/* Purpose:    Creates all layers of an initialized neural net object         */
/* Remarks:    The number of layers must previously have been set to a value  */
/*             greater or equal one.                                          */
/*             The input layer index is set to zero,                          */
/*             if it was not set before (-1)                                  */
/*             The output layer index is set to the number of layers minus one, */
/*             if it was not set before (-1)                                   */
/*             The default layer attribute settings are:                       */
/*                 iLayer    = <Current layer index in loop>                   */
/*                 nNumUnits = 1                                               */
/*                 nInpFnId  = NN_FUNC_SUM_1                                   */
/*                 nActFnId  = NN_FUNC_SIGMOID_1                               */
/*                 nOutFnId  = NN_FUNC_IDENTITY                                */
/*                 fActSlope = 1.0                                             */
/*                 fActThres = 0.0                                             */
/* Returns:    NN_OK (or zero) for success, NN_OUT_OF_MEMORY otherwise         */
/*//////////////////////////////////////////////////////////////////////////// */

NN_STATUS Nn_CreateLayers (NN_PNET pNet)
{
	short     iL;
	NN_PLAYER pLayer;

	assert(pNet != NULL);
	assert(!Nn_LayersCreated(pNet));

	/* Nothing to do */
	if (pNet->na.nNumLayers == 0)
		return NN_OK;

	/* Set default value for the input layer index */
	if (pNet->na.iInpLayer < 0)
		pNet->na.iInpLayer = 0;
	/* Set default value for the output layer index */
	if (pNet->na.iOutLayer < 0)
		pNet->na.iOutLayer = (short)(pNet->na.nNumLayers - 1);

	/* Allocate space for the layer structures */
	pNet->aLayers = (NN_ALAYERS) calloc(pNet->na.nNumLayers, sizeof (NN_LAYER));
	if (pNet->aLayers == NULL)
		return Nn_SetOutOfMemoryError();

	/* Set default layer attributes */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get the layer at the given position */
		pLayer = Nn_GetLayerAt(pNet, iL);
		/* Set the layer attribute default values */
		pLayer->la.iLayer    = iL;
		pLayer->la.nNumUnits = 1; 
		pLayer->la.nInpFnId  = NN_FUNC_SUM_1;  
		pLayer->la.nActFnId  = NN_FUNC_SIGMOID_1;  
		pLayer->la.nOutFnId  = NN_FUNC_IDENTITY;
		pLayer->la.fActSlope = 1.0F;
		pLayer->la.fActThres = 0.0F;
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_DeleteLayers                                                */
/* Purpose:    Releases all memory allocated by the layers of the neural      */
/*             net object                                                     */
/* Remarks:    The function deletes also all units and connections owned by   */
/*             the layers                                                     */
/* Returns:    No return value                                                */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_DeleteLayers (NN_PNET pNet)
{
	int iL;
	NN_PLAYER pLayer;

	/* Nothing to do */
	if (pNet == NULL || pNet->aLayers == NULL)
		return;

	/* Delete the units of the layer */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get layer address */
		pLayer = Nn_GetLayerAt(pNet, iL);
		
		/* Delete all units of the layer */
		Nn_DeleteUnits(pLayer);
	}

	/* Delete the layers */
	free(pNet->aLayers);
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_GetLayerAt                                                  */
/* Purpose:    Get the layer of the net at a given position                   */
/* Returns:    Layer object                                                   */
/*////////////////////////////////////////////////////////////////////////////*/

NN_PLAYER Nn_GetLayerAt(const NN_PNET pNet, int iL)
{
	assert(pNet != NULL);
	assert(iL >= 0 && iL < pNet->na.nNumLayers);
	assert(pNet->aLayers != NULL);
	return pNet->aLayers + iL;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_GetInputLayer                                               */
/* Purpose:    Gets the input layer of the net                                */
/* Returns:    Layer object                                                   */
/*////////////////////////////////////////////////////////////////////////////*/

NN_PLAYER Nn_GetInputLayer(const NN_PNET pNet)
{
	assert(pNet != NULL);
	return Nn_GetLayerAt(pNet, pNet->na.iInpLayer);
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_GetOutputLayer                                               */
/* Purpose:    Gets the output layer of the net                                */
/* Returns:    Layer object                                                   */
/*////////////////////////////////////////////////////////////////////////////*/

NN_PLAYER Nn_GetOutputLayer(const NN_PNET pNet)
{
	assert(pNet != NULL);
	return Nn_GetLayerAt(pNet, pNet->na.iOutLayer);
}

/*/////////////////////////////////////////////////////////////////////*/
/* Layer object (NN_PLAYER) methods                                    */
/*/////////////////////////////////////////////////////////////////////*/

/*////////////////////////////////////////////////////////////////////////////*/
/* Layer methods concerning the units (NN_PUNIT)                              */

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_UnitsCreated                                                  */
/* Purpose:  Checks whether the units of a layer object have been             */
/*           created or not                                                   */
/* Returns:  TRUE if the units have been created, FALSE otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

BOOL Nn_UnitsCreated (const NN_PLAYER pLayer)
{
	assert(pLayer != NULL);
	return pLayer->la.nNumUnits > 0 && pLayer->aUnits != NULL;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_CreateUnits                                                 */
/* Purpose:    Creates all units of an initialized layer object               */
/* Remarks:    The number of units must previously have been set to a value   */
/*             greater or equal one.                                          */
/*             The default unit attribute settings are:                       */
/*                 iLayer    = <Current layer index in loop>                  */
/*                 iUnit     = <Current unit index in loop>                   */
/*                 nNumConns = 0                                              */
/*                 fInpBias  = 0.0                                            */
/*                 fInpScale = 1.0                                            */
/*                 fOutBias  = 0.0                                            */
/*                 fOutScale = 1.0                                            */
/* Returns:    NN_OK (or zero) for success, NN_OUT_OF_MEMORY otherwise        */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_CreateUnits (NN_PLAYER pLayer)
{
	short     iU;
	NN_PUNIT  pUnit;

	assert(pLayer != NULL);
	assert(!Nn_UnitsCreated(pLayer));

	/* Nothing to do */
	if (pLayer->la.nNumUnits == 0)
		return NN_OK;

	/* Allocate heap space for the units of the layer */
	pLayer->aUnits = (NN_AUNITS) calloc(pLayer->la.nNumUnits, sizeof (NN_UNIT));
	if (pLayer->aUnits == NULL)
		return Nn_SetOutOfMemoryError();

	/* Set default unit attributes: */
	/* For all units               */
	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get the unit at the */
		pUnit = Nn_GetUnitAt(pLayer, iU);
		
		/* Set default values */
		pUnit->ua.iLayer    = pLayer->la.iLayer; /* Layer identifier */
		pUnit->ua.iUnit     = iU;   /* Unit identifier              */
		pUnit->ua.nNumConns = 0;    /* Number of connections        */
		pUnit->ua.fInpBias  = 0.0;  /* Input function bias          */
		pUnit->ua.fInpScale = 1.0;  /* Input function scaling factor */
		pUnit->ua.fOutBias  = 0.0;  /* Output function bias         */
		pUnit->ua.fOutScale = 1.0;  /* Output function scaling factor */
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_DeleteUnits                                                 */
/* Purpose:    Releases all memory allocated by the units of the layer object */
/* Remarks:    The function deletes also all connections and if present, the  */
/*             inverse co-variance matrix                                     */
/* Returns:    No return value                                                */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_DeleteUnits (NN_PLAYER pLayer)
{
	short iU;
	NN_PUNIT  pUnit;

	/* Nothing to do */
	if (pLayer == NULL || pLayer->aUnits == NULL)
		return;

	for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
	{
		/* Get unit */
		pUnit = Nn_GetUnitAt(pLayer, iU);
		/* Delete connections of the unit */
		Nn_DeleteConns(pUnit);
		/* Delete inverse co-variance matrix of the unit */
		Nn_DeleteMatrix(pUnit);
	}
	
	/* Delete units */
	free(pLayer->aUnits);
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_GetUnitAt                                                   */
/* Purpose:    Get the unit of a layer at a given position                    */
/* Returns:    Unit object                                                    */
/*////////////////////////////////////////////////////////////////////////////*/

NN_PUNIT Nn_GetUnitAt(const NN_PLAYER pLayer, short iU)
{
	assert(pLayer != NULL);
	assert(iU >= 0 && iU < pLayer->la.nNumUnits);
	assert(pLayer->aUnits != NULL);
	return pLayer->aUnits + iU;
}

/*/////////////////////////////////////////////////////////////////////*/
/* Unit object (NN_PUNIT) methods                                      */
/*/////////////////////////////////////////////////////////////////////*/

/*////////////////////////////////////////////////////////////////////////////*/
/* Unit methods concerning the connections (NN_PCONN)                         */

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ConnsCreated                                                  */
/* Purpose:  Checks whether the connections of a unithave been created or not */
/* Returns:  TRUE if the connections have been created, FALSE otherwise       */
/*////////////////////////////////////////////////////////////////////////////*/

BOOL Nn_ConnsCreated (const NN_PUNIT pUnit)
{
	assert(pUnit != NULL);
	return pUnit->ua.nNumConns > 0 && pUnit->aConns != NULL;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_CreateConns                                                 */
/* Purpose:    Creates all connections of an initialized unit object          */
/* Remarks:    The number of connections must previously have been set to a value */
/*             greater or equal zero.                                             */
/*             The default connection attribute settings are:                     */
/*                 iLayer  = 0                                                    */
/*                 iUnit   = 0                                                    */
/*                 fWeight = 0.0                                                  */
/* Returns:    NN_OK (or zero) for success, NN_OUT_OF_MEMORY otherwise            */
/*////////////////////////////////////////////////////////////////////////////    */

NN_STATUS Nn_CreateConns (NN_PUNIT pUnit)
{
	assert(pUnit != NULL);
	assert(!Nn_ConnsCreated(pUnit));

	/* Nothing to do */
	if (pUnit->ua.nNumConns == 0)
		return NN_OK;

	/* Allocate heap space for the units of the layer */
	pUnit->aConns = (NN_ACONNS) calloc(pUnit->ua.nNumConns, sizeof (NN_CONN));
	if (pUnit->aConns == NULL)
		return Nn_SetOutOfMemoryError();

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_DeleteConns                                                   */
/* Purpose:  Releases all memory allocated by the connections of the unit object */
/* Returns:  No return value                                                    */
/*////////////////////////////////////////////////////////////////////////////  */

void Nn_DeleteConns (NN_PUNIT pUnit)
{
	if (pUnit == NULL || pUnit->aConns == NULL)
		return;
	free(pUnit->aConns);
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_GetConnAt                                                     */
/* Purpose:  Get the incoming connection of a unit at a given position        */
/* Returns:  Connection object                                                */
/*////////////////////////////////////////////////////////////////////////////*/

NN_PCONN Nn_GetConnAt (const NN_PUNIT pUnit, short iC)
{
	assert(pUnit != NULL);
	assert(iC >= 0 && iC < pUnit->ua.nNumConns);
	assert(pUnit->aConns != NULL);
	return pUnit->aConns + iC;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_SetConnAt                                                     */
/* Purpose:  Sets the incoming connection of a unit at a given position       */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_SetConnAt (NN_PUNIT pUnit, short iC, const NN_PCONN pConn)
{
	assert(pUnit != NULL);
	assert(pUnit->aConns != NULL);
	assert(iC >= 0 && iC < pUnit->ua.nNumConns);
	assert(pConn != NULL);
	pUnit->aConns[iC] = *pConn;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Unit methods concerning the inverse co-variance matrix                     */

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MatrixCreated                                                 */
/* Purpose:  Checks whether the inverse co-variance matrix of a unit has been */
/*           created or not                                                   */
/* Returns:  TRUE if the matrix has been created, FALSE otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

BOOL Nn_MatrixCreated (const NN_PUNIT pUnit)
{
	assert(pUnit != NULL);
	return pUnit->ua.nNumConns > 0 && pUnit->ppfMatrix != NULL;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_CreateMatrix                                                */
/* Purpose:    Creates the inverse co-variance matrix of an initialized unit object */
/* Remarks:    The number of connections must previously have been set to a value  */
/*             greater or equal zero. The size of the matrix will be the square of */
/*             the number of incoming connections.                                 */
/* Returns:    NN_OK (or zero) for success, NN_OUT_OF_MEMORY otherwise             */
/*////////////////////////////////////////////////////////////////////////////     */

NN_STATUS Nn_CreateMatrix (NN_PUNIT pUnit)
{
	short iC;

	assert(pUnit != NULL);
	assert(!Nn_MatrixCreated(pUnit));
	
	/* Nothing to do */
	if (pUnit->ua.nNumConns == 0)
		return NN_OK;

	/* Allocate heap space for the units of the layer */
	pUnit->ppfMatrix = (NN_FLOAT**) calloc(pUnit->ua.nNumConns, sizeof (NN_FLOAT*));
	if (pUnit->aConns == NULL)
		return Nn_SetOutOfMemoryError();

	/* Important: Mark that the unit has a matrix! */
	pUnit->ua.bHasMatrix = TRUE;

	/* Allocate heap space for the matrix */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		pUnit->ppfMatrix[iC] = (NN_FLOAT*) calloc(pUnit->ua.nNumConns, sizeof (NN_FLOAT));
		if (pUnit->ppfMatrix[iC] == NULL)
			return Nn_SetOutOfMemoryError();
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_DeleteMatrix                                                  */
/* Purpose:  Releases the memory allocated by the inverse co-variance matrix  */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_DeleteMatrix (NN_PUNIT pUnit)
{
	short iC;

	/* Nothing to do */
	if (pUnit == NULL || pUnit->ppfMatrix == NULL)
		return;
	
	/* For all matrix rows */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		/* Free matrix row */
		if (pUnit->ppfMatrix[iC] != NULL)
			free(pUnit->ppfMatrix[iC]);
	}

	/* Free row vector */
	free(pUnit->ppfMatrix);
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_GetMatrix                                                     */
/* Purpose:  Gets the inverse co-variance matrix of a unit                    */
/* Returns:  The inverse co-variance matrix of a unit                         */
/*////////////////////////////////////////////////////////////////////////////*/

NN_FLOAT** Nn_GetMatrix (NN_PUNIT pUnit)
{
	assert(pUnit != NULL);
	return pUnit->ppfMatrix;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_GetMatrixRowAt                                                */
/* Purpose:  Gets a row of the inverse co-variance matrix of a unit at a given */
/*           position                                                         */
/* Returns:  A row of the inverse co-variance matrix                          */
/*////////////////////////////////////////////////////////////////////////////*/

NN_FLOAT* Nn_GetMatrixRowAt (NN_PUNIT pUnit, short iC)
{
	assert(pUnit != NULL);
	return pUnit->ppfMatrix[iC];
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_GetMatrixRowAt                                                */
/* Purpose:  Gets an element of the inverse co-variance matrix of a unit at a */
/*           given position                                                   */
/* Returns:  An element of the inverse co-variance matrix                     */
/*////////////////////////////////////////////////////////////////////////////*/

NN_FLOAT Nn_GetMatrixElemAt (const NN_PUNIT pUnit, short iCRow, short iCCol)
{
	assert(pUnit != NULL);
	assert(pUnit->ppfMatrix != NULL);
	assert(iCRow > 0 && iCRow < pUnit->ua.nNumConns);
	assert(iCCol > 0 && iCCol < pUnit->ua.nNumConns);
	assert(pUnit->ppfMatrix[iCRow] != NULL);
	return pUnit->ppfMatrix[iCRow][iCCol];
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_GetMatrixRowAt                                                */
/* Purpose:  Sets an element of the inverse co-variance matrix of a unit at a */
/*           given position                                                   */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_SetMatrixElemAt (NN_PUNIT pUnit, short iCRow, short iCCol, NN_FLOAT fM)
{
	assert(pUnit != NULL);
	assert(pUnit->ppfMatrix != NULL);
	assert(iCRow > 0 && iCRow < pUnit->ua.nNumConns);
	assert(iCCol > 0 && iCCol < pUnit->ua.nNumConns);
	assert(pUnit->ppfMatrix[iCRow] != NULL);
	pUnit->ppfMatrix[iCRow][iCCol] = fM;
}

/*/////////////////////////////////////////////////////////////////////*/
/* Output stream functions                                             */
/*/////////////////////////////////////////////////////////////////////*/

/* Module local output stream */
static FILE* g_pOutStream = NULL;

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_GetOutStream                                                */
/* Purpose:    Gets the global output stream for this module                  */
/* Returns:    Output stream                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

FILE* Nn_GetOutStream () { return g_pOutStream; }

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_SetOutStream                                                */
/* Purpose:    Sets the global output stream for this module                  */
/* Returns:    No return value                                                */
/*////////////////////////////////////////////////////////////////////////////*/

void  Nn_SetOutStream (FILE* pOutStream) { g_pOutStream = pOutStream; }

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_Printf                                                      */
/* Purpose:    Behaves exactly as the printf function, but uses the global    */
/*             output stream of the module.                                   */
/* Returns:    No return value                                                */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_Printf (PCSTR pchFormat, ...)
{
	va_list pArgList;

	assert(pchFormat != NULL);

	if (Nn_GetOutStream() == NULL)
		return;

	va_start(pArgList, pchFormat);
	vfprintf(Nn_GetOutStream(), pchFormat, pArgList);
	va_end(pArgList);
}

/*/////////////////////////////////////////////////////////////////////*/
/* Error functions                                                     */
/*/////////////////////////////////////////////////////////////////////*/

/* Module local error code (last one) */
static NN_STATUS g_nErrNo = NN_OK;

/* Module local error message (last one) */
static char g_pchErrMsg [512];

/* Module local number of errors */
static int g_nNumErrors = 0;

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_GetErrNo                                                    */
/* Purpose:    Gets the error code of the last error                          */
/* Returns:    NN_OK if no error occured, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS  Nn_GetErrNo ()     { return g_nErrNo; }

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_GetErrMsg                                                   */
/* Purpose:    Gets the message of the last error                             */
/* Returns:    Empty string if no error occured, otherwise an error message   */
/*////////////////////////////////////////////////////////////////////////////*/

PCSTR      Nn_GetErrMsg ()    { return g_pchErrMsg; }

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_GetNumErrors                                                */
/* Purpose:    Gets the number of errors since the last call of Nn_ClearError */
/* Returns:    Number of errors since the last call of Nn_ClearError          */
/*////////////////////////////////////////////////////////////////////////////*/

int        Nn_GetNumErrors () { return g_nNumErrors; }

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_ClearError                                                  */
/* Purpose:    Clears the last occured error                                  */
/* Remarks:    Sets the last error code to NN_OK,                             */
/*             sets the number of errors to zero,                             */
/*             clears the last error message                                  */
/* Returns:    No return value                                                */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_ClearError ()                  
{ 
	g_nErrNo       = NN_OK; 
	g_pchErrMsg[0] = '\0';
	g_nNumErrors   = 0; 
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_Error                                                       */
/* Purpose:    Sets an error                                                  */
/* Remarks:    Sets the last error code and message (used like printf),       */
/*             increments the number of errors,                               */
/*             prints the message to the global output stream (if it is not NULL) */
/* Returns:    The error code that has been set                                  */
/*////////////////////////////////////////////////////////////////////////////   */

NN_STATUS Nn_Error (NN_STATUS nErrNo, PCSTR pchFormat, ...)
{
	va_list pArgList;

	assert(pchFormat != NULL);

	va_start(pArgList, pchFormat);
	vsprintf(g_pchErrMsg, pchFormat, pArgList);
	va_end(pArgList);

	g_nErrNo = nErrNo;
	g_nNumErrors++;

	if (Nn_GetOutStream() != NULL)
	{
		fputs(g_pchErrMsg, Nn_GetOutStream());
		fputs("\n", Nn_GetOutStream());
	}

/*getchar(); */
	return nErrNo;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function:   Nn_SetOutOfMemoryError                                         */
/* Purpose:    Sets the "Out of memory" error                                 */
/* Returns:    NN_OUT_OF_MEMORY                                               */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_SetOutOfMemoryError ()
{
	return Nn_Error(NN_OUT_OF_MEMORY, NN_ERR_PREFIX "out of memory");
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_SetFileWriteError                                             */
/* Purpose:  Sets the global error code to NN_FILE_WRITE_ERROR and displays   */
/*           a corresponding message.                                         */
/* Returns:  NN_FILE_WRITE_ERROR                                              */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_SetFileWriteError ()
{
	return Nn_Error(NN_FILE_WRITE_ERROR, NN_ERR_PREFIX "can't write to binary file");
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_SetFileReadError                                              */
/* Purpose:  Sets the global error code to NN_FILE_READ_ERROR and displays    */
/*           a corresponding message.                                         */
/* Returns:  NN_FILE_READ_ERROR                                               */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_SetFileReadError ()
{
	return Nn_Error(NN_FILE_READ_ERROR, NN_ERR_PREFIX "can't read from binary file");
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_SetInvalidSectionIDError                                      */
/* Purpose:  Sets the global error code to NN_INVALID_SECTION_ID and displays */
/*           a corresponding message.                                         */
/* Returns:  NN_INVALID_SECTION_ID                                            */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_SetInvalidSectionIDError ()
{
	return Nn_Error(NN_INVALID_SECTION_ID, NN_ERR_PREFIX "invalid section ID");
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_SetInvalidSectionSizeError                                    */
/* Purpose:  Sets the global error code to NN_INVALID_SECTION_SIZE and displays */
/*           a corresponding message.                                          */
/* Returns:  NN_INVALID_SECTION_SIZE                                           */
/*//////////////////////////////////////////////////////////////////////////// */

NN_STATUS Nn_SetInvalidSectionSizeError ()
{
	return Nn_Error(NN_INVALID_SECTION_SIZE, NN_ERR_PREFIX "invalid section size");
}

/*////////////////////////////////////////////////////////////////////////////*/
