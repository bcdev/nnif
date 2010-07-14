/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnMemIO.c                                                     */
/* Purpose:     Implementation of the neural net memory I/O routines          */
/* Remarks:     Interface defined in NnMemIO.h                                */
/* Author:      Norman Fomferra (SCICON)                                      */
/*              Tel:   +49 4152 1457 (tel)                                    */
/*              Fax:   +49 4152 1455 (fax)                                    */
/*              Email: Norman.Fomferra@gkss.de                                */
/*////////////////////////////////////////////////////////////////////////////*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "NnBase.h"
#include "NnCheck.h"
#include "NnMemIO.h"

/*////////////////////////////////////////////////////////////////////////////*/
/* Module local memory I/O stream                                             */

static NN_MSTREAM* g_stream;

/*////////////////////////////////////////////////////////////////////////////*/
/* Module local prototypes:                                                   */
/*                                                                            */
NN_STATUS Nn_ReadMemHeader  (long* pnSectionID, long* pnSectionSize);
NN_STATUS Nn_ReadMemNet     (NN_PNET   pNet);
NN_STATUS Nn_ReadMemLayer   (NN_PLAYER pLayer);
NN_STATUS Nn_ReadMemUnit    (NN_PUNIT  pUnit);
NN_STATUS Nn_ReadMemConns   (NN_PUNIT  pUnit);
NN_STATUS Nn_ReadMemMatrix  (NN_PUNIT  pUnit);

NN_STATUS Nn_WriteMemHeader (long nSectionID, long nSectionSize);
NN_STATUS Nn_WriteMemNet    (const NN_PNET   pNet);
NN_STATUS Nn_WriteMemLayer  (const NN_PLAYER pLayer);
NN_STATUS Nn_WriteMemUnit   (const NN_PUNIT  pUnit);
NN_STATUS Nn_WriteMemConns  (const NN_PUNIT  pUnit);
NN_STATUS Nn_WriteMemMatrix (const NN_PUNIT  pUnit);

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CreateNetFromMemFile                                          */
/* Purpose:  Reads a neural net object from a binary NNFF file.               */
/* Remarks:  If the neural net object has been successfully read from the     */
/*           memory, the function Nn_AssertSemanticIntegrity is called with   */
/*           the number of input and output units. The return value of        */
/*           Nn_AssertSemanticIntegrity is returned by Nn_CreateFromMemFile.  */
/*           The comparision of the number of input and output units can      */
/*           be supressed if passed each a -1.                                */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_CreateNetFromMemFile
(
	PCMEM    pMem,          /* Source memory block */
	size_t   nMemSize,      /* Size (in bytes) of the memory block */
	size_t*  pnBytesRead,   /* Return: Number of bytes read (pointer can be NULL) */
	NN_PNET* ppNet,         /* Return: The resulting neural net object           */
	int      nNumInpUnits,  /* Number of input units                             */
	int      nNumOutUnits   /* Number of output units                            */
)
{
	NN_STATUS  nns;
	
	assert(pMem != NULL);
	assert(ppNet != NULL);

	/* Set the global error code to NN_OF (or zero) */
	Nn_ClearError();
	/* Create an empty neural net object */
	nns = Nn_CreateNet(ppNet);
	/* If there was enough memory */
	if (nns == NN_OK)
	{
		/* Open the NNFF memory chunk and save its handle global */
		g_stream = Nn_MOpen((PMEM)pMem, nMemSize, "r");
		if (g_stream != NULL)
		{
			/* Read the neural net object from the open file */
			nns = Nn_ReadMemNet(*ppNet);
			/* Set the number of bytes that have been read */
			if (pnBytesRead != NULL)
				*pnBytesRead = Nn_MPos(g_stream);
			/* Close the NNFF memory chunk */
			Nn_MClose(g_stream);
		}
		else
			return Nn_Error(NN_OUT_OF_MEMORY, NN_ERR_PREFIX "can't open memory file");
	}

	/* If the neural net object was read successfully */
	if (nns == NN_OK)
		/* Check and, if necessary, correct its internal semantic integrity */
		nns = Nn_AssertSemanticIntegrity(*ppNet, nNumInpUnits, nNumOutUnits);
	/* If the net was not read successfully */
	else
	{
		/* Realease the object instance */
		Nn_DeleteNet(*ppNet);
		*ppNet = NULL;
	}

	return nns;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadMemHeader                                                 */
/* Purpose:  Reads a section header to identify the following section in NNFF */
/* Returns:  NN_OK (or zero) for success, NN_FILE_READ_ERROR otherwise        */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_ReadMemHeader 
(
	long* pnSectionID,   /* Section ID (4 byte code, 4th is zero) */
	long* pnSectionSize  /* Size of the following section (4 byte integer) */
)
{
	assert(pnSectionID != NULL);
	assert(pnSectionSize != NULL);
	assert(g_stream != NULL);

	/* Read the section identifier (4 bytes) */
	Nn_MRead(pnSectionID,   sizeof (long), 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileReadError();
	
	/* Read the section size (4 bytes) */
	Nn_MRead(pnSectionSize, sizeof (long), 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileReadError();

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadMemNet                                                    */
/* Purpose:  Reads the complete neural net from the open NNFF file            */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_ReadMemNet (NN_PNET pNet)
{
	NN_STATUS  nns;
	short      iL, iU;
	NN_PLAYER  pLayer;
	NN_PUNIT   pUnit;
	long       nSectionID, nSectionSize;

	assert(pNet != NULL);
	assert(g_stream != NULL);

	/* Read the neural net section header */
	nns = Nn_ReadMemHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(long*)NN_NET_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_NET_SECTION_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Read the complete neural nat section */
	Nn_MRead(&pNet->na, NN_NET_SECTION_SIZE, 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileReadError();

	/* Create all layers for the neural net object */
	nns = Nn_CreateLayers(pNet);
	if (nns != NN_OK)
		return nns;

	/* For all layers just created */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get the layer at the given position */
		pLayer = Nn_GetLayerAt(pNet, iL);
		/* Read the layer from the NNFF memory chunk */
		nns = Nn_ReadMemLayer(pLayer);
		if (nns != NN_OK)
			return nns;
	}

	/* For all layers just read */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get the layer at the given position */
		pLayer = Nn_GetLayerAt(pNet, iL);
		/* For all units of the layer that were just created */
		for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
		{
			/* Get the unit at the given position */
			pUnit = Nn_GetUnitAt(pLayer, iU);
			/* Read the unit from the NNFF memory chunk */
			nns = Nn_ReadMemUnit(pUnit);
			if (nns != NN_OK)
				return nns;
		}
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadMemLayer                                                  */
/* Purpose:  Reads a layer section from the NNFF memory chunk                 */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_ReadMemLayer (NN_PLAYER pLayer)
{
	NN_STATUS nns;
	long nSectionID, nSectionSize;
	
	assert(pLayer != NULL);
	assert(g_stream != NULL);

	/* Read the layer section header */
	nns = Nn_ReadMemHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(long*)NN_LAYER_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_LAYER_SECTION_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Read the complete layer section from the file */
	Nn_MRead(&pLayer->la, NN_LAYER_SECTION_SIZE, 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileReadError();

	nns = NN_OK;

	/* If the layer has units, create them */
	if (pLayer->la.nNumUnits > 0)
		nns = Nn_CreateUnits(pLayer);
	
	return nns;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadMemUnit                                                   */
/* Purpose:  Reads a unit section from the NNFF memory chunk                  */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_ReadMemUnit (NN_PUNIT  pUnit)
{
	NN_STATUS nns;
	long      nSectionID, nSectionSize;
	
	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Read the unit section header */
	nns = Nn_ReadMemHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(long*)NN_UNIT_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_UNIT_SECTION_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Read the complete unit section from the NNFF memory chunk */
	Nn_MRead(&pUnit->ua, NN_UNIT_SECTION_SIZE, 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileReadError();

	/* If the units has incomming connections */
	if (pUnit->ua.nNumConns > 0)
	{
		nns = Nn_ReadMemConns(pUnit);
		if (nns != NN_OK)
			return nns;
	}

	/* If the unit has a matrix definition */
	if (pUnit->ua.nNumConns > 0 && pUnit->ua.bHasMatrix)
	{
		nns = Nn_ReadMemMatrix(pUnit);
		if (nns != NN_OK)
			return nns;
	}

	/* Fine */
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadMemConnections                                            */
/* Purpose:  Reads all incoming connections of a unit from the NNFF memory chunk */
/* Returns:  NN_OK (or zero) for success, an error code otherwise               */
/*////////////////////////////////////////////////////////////////////////////  */

NN_STATUS Nn_ReadMemConns (NN_PUNIT  pUnit)
{
	NN_STATUS nns;
	NN_PCONN  pConn;
	short     iC;
	long      nSectionID, nSectionSize;
	
	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Read the connection section header */
	nns = Nn_ReadMemHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(long*)NN_CONN_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_CONN_ENTRY_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Create the connections */
	nns = Nn_CreateConns(pUnit);
	if (nns != NN_OK)
		return nns;

	/* Read all connections from the NNFF memory chunk */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		/* Get the connection at the given position */
		pConn = Nn_GetConnAt(pUnit, iC);

		/* Read the connection from the NNF file */
		Nn_MRead(&pConn->ca,
			  NN_CONN_ENTRY_SIZE, 
			  1, 
			  g_stream);
		if (Nn_MError(g_stream))
			return Nn_SetFileReadError();
	}

	/* Fine */
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadMemMatrix                                                 */
/* Purpose:  Reads the inverse co-variance matrix for a unit from the NNFF memory chunk */
/* Returns:  NN_OK (or zero) for success, an error code otherwise                      */
/*////////////////////////////////////////////////////////////////////////////         */

NN_STATUS Nn_ReadMemMatrix (NN_PUNIT pUnit)
{
	NN_STATUS nns;
	NN_FLOAT* pfRow;
	short     iC;
	long      nSectionID, nSectionSize;
	
	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Read the connection section header */
	nns = Nn_ReadMemHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(long*)NN_MATRIX_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_MATRIX_ENTRY_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Create the matrix */
	nns = Nn_CreateMatrix(pUnit);
	if (nns != NN_OK)
		return nns;

	/* Read all matrix rows from the NNFF memory chunk */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		/* Get the matrix row at the given row position */
		pfRow = Nn_GetMatrixRowAt(pUnit, iC);

		/* Read the row */
		Nn_MRead(pfRow, 
			  NN_MATRIX_ENTRY_SIZE * pUnit->ua.nNumConns,
			  1,
			  g_stream);
		if (Nn_MError(g_stream))
			return Nn_SetFileReadError();
	}

	/* Fine */
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteNetToMemFile                                             */
/* Purpose:  Writes a neural net object to a binary NNFF file                 */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteNetToMemFile 
(
	PMEM          pMem,           /* Destination memory block */
	size_t        nMemSize,       /* Size (in bytes) of the block */
	size_t*       pnBytesWritten, /* Number of bytes written (can be NULL) */
	const NN_PNET pNet            /* Net object to write                  */
)
{
	NN_STATUS  nns;

	assert(pMem != NULL);
	assert(pNet != NULL);

	/* Set the global error code to NN_OF (or zero) */
	Nn_ClearError();

	/* Open the NNFF memory file */
	g_stream = Nn_MOpen(pMem, nMemSize, "w");
	if (g_stream != NULL)
	{
		/* Write the neural net object to the file */
		nns = Nn_WriteMemNet(pNet);
		/* Set the number of bytes that have been written */
		if (pnBytesWritten != NULL)
			*pnBytesWritten = Nn_MPos(g_stream);
		/* Close the NNFF memory file */
		Nn_MClose(g_stream);
	}
	else
		return Nn_Error(NN_OUT_OF_MEMORY, NN_ERR_PREFIX "can't open memory file");

	return nns;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteMemHeader                                                */
/* Purpose:  Writes a section header to identify the following section in NNFF */
/* Returns:  NN_OK (or zero) for success, NN_FILE_WRITE_ERROR otherwise       */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteMemHeader (long nSectionID, long nSectionSize)
{
	assert(g_stream != NULL);

	/* Write the section identifier (4 bytes) */
	Nn_MWrite(&nSectionID,   sizeof (long), 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileWriteError();

	/* Write the section size (4 bytes) */
	Nn_MWrite(&nSectionSize, sizeof (long), 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileWriteError();
	
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteMemNet                                                   */
/* Purpose:  Writes the complete neural net from the open NNFF file           */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteMemNet (const NN_PNET pNet)
{
	NN_STATUS  nns;
	short      iL, iU;
	NN_PLAYER  pLayer;
	NN_PUNIT   pUnit;
	
	assert(pNet != NULL);
	assert(g_stream != NULL);

	/* Write the net header */
	nns = Nn_WriteMemHeader(*(long*)NN_NET_SECTION_ID, NN_NET_SECTION_SIZE);
	if (nns != NN_OK)
		return nns;
	
	/* Write the complete net section */
	Nn_MWrite(&pNet->na, NN_NET_SECTION_SIZE, 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileWriteError();
	
	/* Write all layer sections to the NNFF memory chunk */
	/* For all layers                                   */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get the layer at the given position */
		pLayer = Nn_GetLayerAt(pNet, iL);
		/* Write the layer section */
		nns = Nn_WriteMemLayer(pLayer);
		if (nns != NN_OK)
			return nns;
	}
	
	/* Write the sections of all units to the NNFF memory chunk */
	/* For all layers                                          */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get the layer at the given position */
		pLayer = Nn_GetLayerAt(pNet, iL);
		/* For all units */
		for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
		{
			/* Get the unit at the given position */
			pUnit = Nn_GetUnitAt(pLayer, iU);
			/* Write unit section */
			nns = Nn_WriteMemUnit(pUnit);
			if (nns != NN_OK)
				return nns;
		}
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteMemLayer                                                 */
/* Purpose:  Writes a layer section to the NNFF memory chunk                  */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteMemLayer (const NN_PLAYER pLayer)
{
	NN_STATUS nns;

	assert(pLayer != NULL);
	assert(g_stream != NULL);

	/* Write the layer section header to the NNFF memory chunk */
	nns = Nn_WriteMemHeader(*(long*)NN_LAYER_SECTION_ID, NN_LAYER_SECTION_SIZE);
	if (nns != NN_OK)
		return nns;
	
	/* Write the layer section to the NNFF memory chunk */
	Nn_MWrite(&pLayer->la, NN_LAYER_SECTION_SIZE, 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileWriteError();

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteMemUnit                                                  */
/* Purpose:  Writes a unit section to the NNFF memory chunk                   */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteMemUnit  (const NN_PUNIT pUnit)
{
	NN_STATUS nns;

	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Write the unit section header to the NNFF memory chunk */
	nns = Nn_WriteMemHeader(*(long*)NN_UNIT_SECTION_ID, NN_UNIT_SECTION_SIZE);
	if (nns != NN_OK)
		return nns;

	/* Write the unit section to the NNFF memory chunk */
	Nn_MWrite(&pUnit->ua, NN_UNIT_SECTION_SIZE, 1, g_stream);
	if (Nn_MError(g_stream))
		return Nn_SetFileWriteError();

	/* If the unit has incoming connections */
	if (pUnit->ua.nNumConns > 0 && pUnit->aConns != NULL)
	{
		/* Write all connections directly after the unit section: */
		nns =  Nn_WriteMemConns(pUnit);
		if (nns != NN_OK)
			return nns;
	}

	/* If the unit has a matrix defined */
	if (pUnit->ua.nNumConns > 0 && pUnit->ppfMatrix != NULL)
	{
		/* Write the matrix definitiondirectly after the connection section: */
		nns =  Nn_WriteMemMatrix(pUnit);
		if (nns != NN_OK)
			return nns;
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteMemConns                                                 */
/* Purpose:  Writes all incoming connections to the NNFF memory chunk         */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteMemConns (const NN_PUNIT pUnit)
{
	NN_STATUS nns;
	NN_PCONN  pConn;
	short     iC;

	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Write the connection section header to the NNFF memory chunk */
	nns = Nn_WriteMemHeader(*(long*)NN_CONN_SECTION_ID, NN_CONN_ENTRY_SIZE);
	if (nns != NN_OK)
		return nns;

	/* Write all connections directly after the unit section: */
	/* For all connections                                   */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		/* Get the connection at the given position */
		pConn = Nn_GetConnAt(pUnit, iC);

		/* Write the connection to the NNFF memory chunk */
		Nn_MWrite(&pConn->ca, 
			      NN_CONN_ENTRY_SIZE, 
			      1, 
			      g_stream);
		if (Nn_MError(g_stream))
			return Nn_SetFileWriteError();
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteMemMatrix                                                */
/* Purpose:  Writes a unit matrix to the NNFF memory chunk                    */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteMemMatrix  (const NN_PUNIT pUnit)
{
	NN_STATUS nns;
	NN_FLOAT* pfRow;
	short     iC;

	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Write the unit section header to the NNFF memory chunk */
	nns = Nn_WriteMemHeader(*(long*)NN_MATRIX_SECTION_ID, NN_MATRIX_ENTRY_SIZE);
	if (nns != NN_OK)
		return nns;

	/* Write the matrix row by row directly after the connections */
	/* For all matrix rows (the size of the matrix is pUnit->ua.nNumConns ^ 2) */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		/* Get the matrix row at the given position */
		pfRow = Nn_GetMatrixRowAt(pUnit, iC);

		/* Write the row to the NNFF memory chunk */
		Nn_MWrite(pfRow, 
			      NN_MATRIX_ENTRY_SIZE * pUnit->ua.nNumConns,
			      1,
			      g_stream);
		if (Nn_MError(g_stream))
			return Nn_SetFileReadError();
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Memory stream routines                                                     */

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MOpen                                                         */
/* Purpose:  Standard library 'fopen' equivalent for a memory stream          */
/* Returns:  The memory stream if it can be created, NULL otherwise           */
/*////////////////////////////////////////////////////////////////////////////*/

NN_MSTREAM* Nn_MOpen (PMEM pMem, size_t nMemSize, PCSTR pchMode)
{
	NN_MSTREAM* pMStream;

	assert(pchMode != NULL);

	if (pMem == NULL || nMemSize == 0U)
		return NULL;
	
	pMStream = (NN_MSTREAM*) calloc(1, sizeof (NN_MSTREAM));
	if (pMStream == NULL)
		return NULL;

	pMStream->pMemBase  = pMem;
	pMStream->nCurrPos  = 0U;
	pMStream->nLastPos  = nMemSize;
	pMStream->nOpenMode = (*pchMode == 'r' || *pchMode == 'R') ? 'r' : 'w';
	pMStream->nErrNo    = NN_OK;
	
	return pMStream;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MClose                                                        */
/* Purpose:  Standard library 'fclose' equivalent for a memory stream         */
/* Returns:  Zero if the stream was successfully closed                       */
/*////////////////////////////////////////////////////////////////////////////*/

int Nn_MClose (NN_MSTREAM* pMStream)
{
	if (pMStream == NULL)
		return 1;
	
	free(pMStream);
	return 0;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MRead                                                         */
/* Purpose:  Standard library 'fread' equivalent for a memory stream          */
/* Returns:  The number of full items actually read, which may be less than   */
/*           nCount if an error occurs or if the end of the memory block is   */
/*           reached before reaching nCount.                                  */
/*////////////////////////////////////////////////////////////////////////////*/

size_t Nn_MRead
(
	void*       pBuffer, /* Pointer to the item or list of items to read */
	size_t      nSize,   /* Size (in bytes) of an item                  */
	size_t      nCount,  /* Number of items to read                     */
	NN_MSTREAM* pMStream /* Memory stream                               */
)
{
	size_t iCount;

	assert(pBuffer != NULL);
	assert(pMStream != NULL);

	for (iCount = 0; iCount < nCount; iCount++)
	{
		if (pMStream->nCurrPos + nSize > pMStream->nLastPos)
		{
			pMStream->nErrNo = 1;
			break;
		}
		
		memcpy(pBuffer, pMStream->pMemBase + pMStream->nCurrPos, nSize);
		pMStream->nCurrPos += nSize;
	}
	
	return iCount;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MWrite                                                        */
/* Purpose:  Standard library 'fwrite' equivalent for a memory stream         */
/* Returns:  The number of full items actually written, which may be less than */
/*           nCount if an error occurs or if the end of the memory block is    */
/*           reached before reaching nCount.                                   */
/*//////////////////////////////////////////////////////////////////////////// */

size_t Nn_MWrite
(
	const void* pBuffer, /* Pointer to the item or list of items to read */
	size_t      nSize,   /* Size (in bytes) of an item                  */
	size_t      nCount,  /* Number of items to read                     */
	NN_MSTREAM* pMStream /* Memory stream                               */
)
{
	size_t iCount;
	
	assert(pBuffer != NULL);
	assert(pMStream != NULL);

	if (pMStream->nOpenMode != 'w')
	{
		pMStream->nErrNo = 3;
		return 0;
	}

	for (iCount = 0; iCount < nCount; iCount++)
	{
		if (pMStream->nCurrPos + nSize > pMStream->nLastPos)
		{
			pMStream->nErrNo = 2;
			break;
		}
		
		memcpy(pMStream->pMemBase + pMStream->nCurrPos, pBuffer, nSize);
		pMStream->nCurrPos += nSize;
	}
	
	return iCount;
}

/*////////////////////////////////////////////////////////////////////////////*/
