/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnBinIO.c                                                     */
/* Purpose:     Implementation of the neural net binary I/O routines          */
/* Remarks:     Interface defined in NnBinIO.h                                */
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
#include "NnBinIO.h"
#include "utils/endian_order.h"

/*////////////////////////////////////////////////////////////////////////////*/
/* Module local binary I/O stream                                             */
static FILE* g_stream = NULL;

/*////////////////////////////////////////////////////////////////////////////*/
/* Module local prototypes:                                                   */
/*                                                                            */
NN_STATUS Nn_ReadBinHeader  (int* pnSectionID, int* pnSectionSize);
NN_STATUS Nn_ReadBinNet     (NN_PNET   pNet);
NN_STATUS Nn_ReadBinLayer   (NN_PLAYER pLayer);
NN_STATUS Nn_ReadBinUnit    (NN_PUNIT  pUnit);
NN_STATUS Nn_ReadBinConns   (NN_PUNIT  pUnit);
NN_STATUS Nn_ReadBinMatrix  (NN_PUNIT  pUnit);

NN_STATUS Nn_WriteBinHeader (int nSectionID, int nSectionSize);
NN_STATUS Nn_WriteBinNet    (const NN_PNET   pNet);
NN_STATUS Nn_WriteBinLayer  (const NN_PLAYER pLayer);
NN_STATUS Nn_WriteBinUnit   (const NN_PUNIT  pUnit);
NN_STATUS Nn_WriteBinConns  (const NN_PUNIT  pUnit);
NN_STATUS Nn_WriteBinMatrix (const NN_PUNIT  pUnit);

void eo_swap_net_attrib(NN_NET_ATTRIB* pna);
void eo_swap_layer_attrib(NN_LAYER_ATTRIB* pla); 
void eo_swap_unit_attrib(NN_UNIT_ATTRIB* pua);
void eo_swap_conn_attrib(NN_CONN_ATTRIB* pca); 

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CreateNetFromBinFile                                          */
/* Purpose:  Reads a neural net object from a binary NNFF file.               */
/* Remarks:  The function calls Nn_AssertSemanticIntegrity if the net object  */
/*           was succesfully read in.                                         */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_CreateNetFromBinFile
(
	PCSTR    pchFilePath,   /* Complete path and filename for the NNFF file */
	int      nNumInpUnits,  /* Number of input units                       */
	int      nNumOutUnits,  /* Number of output units                      */
	NN_PNET* ppNet          /* The resulting neural net object             */
)
{
	NN_STATUS  nns;
	
	assert(pchFilePath != NULL);
	assert(ppNet != NULL);

	/* Set the global error code to NN_OF (or zero) */
	Nn_ClearError();
	/* Create an empty neural net object */
	nns = Nn_CreateNet(ppNet);
	/* If there was enough memory */
	if (nns == NN_OK)
	{
		/* Open the NNFF file in binary mode and save its handle global */
		g_stream = fopen(pchFilePath, "rb");
		/* If it can be opened */
		if (g_stream != NULL)
		{
			/* Read the neural net object from the open file */
			nns = Nn_ReadBinNet(*ppNet);
			/* Close the NNFF file */
			fclose(g_stream);
			g_stream = NULL;
		}
		/* If the NNFF file could'nt be opened */
		else
			nns = Nn_Error(NN_CANT_OPEN_FILE, NN_ERR_PREFIX "can't open binary file '%s' for read", pchFilePath);
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
/* Function: Nn_ReadBinHeader                                                 */
/* Purpose:  Reads a section header to identify the following section in NNFF */
/* Returns:  NN_OK (or zero) for success, NN_FILE_READ_ERROR otherwise        */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_ReadBinHeader 
(
	int* pnSectionID,   /* Section ID (4 byte code, 4th is zero) */
	int* pnSectionSize  /* Size of the following section (4 byte integer) */
)
{
	assert(pnSectionID != NULL);
	assert(pnSectionSize != NULL);
	assert(g_stream != NULL);

	/* Read the section identifier (4 bytes) */
	fread(pnSectionID,   sizeof (int), 1, g_stream);
	if (eo_endian_order() != BIG_ENDIAN) 
		eo_swap_int_n(pnSectionID, 1);

	if (ferror(g_stream))
		return Nn_SetFileReadError();
	
	/* Read the section size (4 bytes) */
	fread(pnSectionSize, sizeof (int), 1, g_stream);
	if (eo_endian_order() != BIG_ENDIAN) 
		eo_swap_int_n(pnSectionSize, 1);

	if (ferror(g_stream))
		return Nn_SetFileReadError();

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadBinNet                                                    */
/* Purpose:  Reads the complete neural net from the open NNFF file            */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_ReadBinNet (NN_PNET pNet)
{
	NN_STATUS  nns;
	short      iL, iU;
	NN_PLAYER  pLayer;
	NN_PUNIT   pUnit;
	int       nSectionID, nSectionSize;

	assert(pNet != NULL);
	assert(g_stream != NULL);

	/* Read the neural net section header */
	nns = Nn_ReadBinHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(int*)NN_NET_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_NET_SECTION_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Read the complete neural net section */
	fread(&pNet->na, NN_NET_SECTION_SIZE, 1, g_stream);
	if (eo_endian_order() != BIG_ENDIAN) 
		eo_swap_net_attrib(&pNet->na);

	if (ferror(g_stream))
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
		/* Read the layer from the NNFF file */
		nns = Nn_ReadBinLayer(pLayer);
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
			/* Read the unit from the NNFF file */
			nns = Nn_ReadBinUnit(pUnit);
			if (nns != NN_OK)
				return nns;
		}
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadBinLayer                                                  */
/* Purpose:  Reads a layer section from the NNFF file                         */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_ReadBinLayer (NN_PLAYER pLayer)
{
	NN_STATUS nns;
	int nSectionID, nSectionSize;
	
	assert(pLayer != NULL);
	assert(g_stream != NULL);

	/* Read the layer section header */
	nns = Nn_ReadBinHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(int*)NN_LAYER_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_LAYER_SECTION_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Read the complete layer section from the file */
	fread(&pLayer->la, NN_LAYER_SECTION_SIZE, 1, g_stream);
	if (eo_endian_order() != BIG_ENDIAN) 
		eo_swap_layer_attrib(&pLayer->la);

	if (ferror(g_stream))
		return Nn_SetFileReadError();

	nns = NN_OK;

	/* If the layer has units, create them */
	if (pLayer->la.nNumUnits > 0)
		nns = Nn_CreateUnits(pLayer);
	
	return nns;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadBinUnit                                                   */
/* Purpose:  Reads a unit section from the NNFF file                          */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_ReadBinUnit (NN_PUNIT  pUnit)
{
	NN_STATUS nns;
	int      nSectionID, nSectionSize;
	
	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Read the unit section header */
	nns = Nn_ReadBinHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(int*)NN_UNIT_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_UNIT_SECTION_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Read the complete unit section from the NNFF file */
	fread(&pUnit->ua, NN_UNIT_SECTION_SIZE, 1, g_stream);
        if (eo_endian_order() != BIG_ENDIAN) 
		eo_swap_unit_attrib(&pUnit->ua);

	if (ferror(g_stream))
		return Nn_SetFileReadError();

	/* If the units has incomming connections */
	if (pUnit->ua.nNumConns > 0)
	{
		nns = Nn_ReadBinConns(pUnit);
		if (nns != NN_OK)
			return nns;
	}

	/* If the unit has a matrix definition */
	if (pUnit->ua.nNumConns > 0 && pUnit->ua.bHasMatrix)
	{
		nns = Nn_ReadBinMatrix(pUnit);
		if (nns != NN_OK)
			return nns;
	}

	/* Fine */
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadBinConns                                                  */
/* Purpose:  Reads all incoming connections of a unit from the NNFF file      */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_ReadBinConns (NN_PUNIT  pUnit)
{
	NN_STATUS nns;
	NN_PCONN  pConn;
	short     iC;
	int      nSectionID, nSectionSize;
	
	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Read the connection section header */
	nns = Nn_ReadBinHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(int*)NN_CONN_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_CONN_ENTRY_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Create the connections */
	nns = Nn_CreateConns(pUnit);
	if (nns != NN_OK)
		return nns;

	/* Read all connections from the NNFF file */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		/* Get the connection at the given position */
		pConn = Nn_GetConnAt(pUnit, iC);

		/* Read the connection from the NNF file */
		fread(&pConn->ca,
			  NN_CONN_ENTRY_SIZE, 
			  1, 
			  g_stream);
		if (eo_endian_order() != BIG_ENDIAN) 
			eo_swap_conn_attrib(&pConn->ca);
		if (ferror(g_stream))
			return Nn_SetFileReadError();
	}

	/* Fine */
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ReadBinMatrix                                                 */
/* Purpose:  Reads the inverse co-variance matrix for a unit from the NNFF file */
/* Returns:  NN_OK (or zero) for success, an error code otherwise              */
/*//////////////////////////////////////////////////////////////////////////// */

NN_STATUS Nn_ReadBinMatrix (NN_PUNIT pUnit)
{
	NN_STATUS nns;
	NN_FLOAT* pfRow;
	short     iC;
	int      nSectionID, nSectionSize;
	
	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Read the connection section header */
	nns = Nn_ReadBinHeader(&nSectionID, &nSectionSize);
	if (nns != NN_OK)
		return nns;

	/* If the identifier does not match: error */
	if (nSectionID != *(int*)NN_MATRIX_SECTION_ID)
		return Nn_SetInvalidSectionIDError();
		
	/* If the size is not correct: error */
	if (nSectionSize != NN_MATRIX_ENTRY_SIZE)
		return Nn_SetInvalidSectionSizeError();

	/* Create the matrix */
	nns = Nn_CreateMatrix(pUnit);
	if (nns != NN_OK)
		return nns;

	/* Read all matrix rows from the NNFF file */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		/* Get the matrix row at the given row position */
		pfRow = Nn_GetMatrixRowAt(pUnit, iC);

		/* Read the row */
		fread(pfRow, 
			  NN_MATRIX_ENTRY_SIZE * pUnit->ua.nNumConns,
			  1,
			  g_stream);
		if (eo_endian_order() != BIG_ENDIAN) 
			eo_swap_double_n(pfRow, pUnit->ua.nNumConns);

		if (ferror(g_stream))
			return Nn_SetFileReadError();
	}

	/* Fine */
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteNetToBinFile                                             */
/* Purpose:  Writes a neural net object to a binary NNFF file                 */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteNetToBinFile (const char* pchFilePath, const NN_PNET pNet)
{
	NN_STATUS  nns;

	assert(pchFilePath != NULL);
	assert(pNet != NULL);

	/* Set the global error code to NN_OF (or zero) */
	Nn_ClearError();

	/* Open the NNFF file in binary mode for write */
	g_stream = fopen(pchFilePath, "wb");
	if (g_stream != NULL)
	{
		/* Write the neural net object to the file */
		nns = Nn_WriteBinNet(pNet);
		fclose(g_stream);
		g_stream = NULL;
	}
	else
		nns = Nn_Error(NN_CANT_OPEN_FILE, NN_ERR_PREFIX "can't open binary file '%s' for write", pchFilePath);

	return nns;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteBinHeader                                                */
/* Purpose:  Writes a section header to identify the following section in NNFF */
/* Returns:  NN_OK (or zero) for success, NN_FILE_WRITE_ERROR otherwise       */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteBinHeader (int nSectionID, int nSectionSize)
{
    assert(g_stream != NULL);

	/* Write the section identifier (4 bytes) */
    if (eo_endian_order() != BIG_ENDIAN)
		eo_swap_int_n(&nSectionID, 1);
	fwrite(&nSectionID, sizeof (int), 1, g_stream);
	if (ferror(g_stream))
		return Nn_SetFileWriteError();

	/* Write the section size (4 bytes) */
    if (eo_endian_order() != BIG_ENDIAN)
		eo_swap_int_n(&nSectionSize, 1);
	fwrite(&nSectionSize, sizeof (int), 1, g_stream);
	if (ferror(g_stream))
		return Nn_SetFileWriteError();
	
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteBinNet                                                   */
/* Purpose:  Writes the complete neural net from the open NNFF file           */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteBinNet (const NN_PNET pNet)
{
	NN_STATUS  nns;
	short      iL, iU;
	NN_PLAYER  pLayer;
	NN_PUNIT   pUnit;
        NN_NET_ATTRIB na;
	
	assert(pNet != NULL);
	assert(g_stream != NULL);

	/* Write the net header */
	nns = Nn_WriteBinHeader(*(int*)NN_NET_SECTION_ID, NN_NET_SECTION_SIZE);
	if (nns != NN_OK)
		return nns;
	
	/* Write the complete net section */
        na = pNet->na;
        if (eo_endian_order() != BIG_ENDIAN) 
		eo_swap_net_attrib(&na);
	fwrite(&na, NN_NET_SECTION_SIZE, 1, g_stream);
	if (ferror(g_stream))
		return Nn_SetFileWriteError();
	
	/* Write all layer sections to the NNFF file */
	/* For all layers                           */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		/* Get the layer at the given position */
		pLayer = Nn_GetLayerAt(pNet, iL);
		/* Write the layer section */
		nns = Nn_WriteBinLayer(pLayer);
		if (nns != NN_OK)
			return nns;
	}
	
	/* Write the sections of all units to the NNFF file */
	/* For all layers                                  */
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
			nns = Nn_WriteBinUnit(pUnit);
			if (nns != NN_OK)
				return nns;
		}
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteBinLayer                                                 */
/* Purpose:  Writes a layer section to the NNFF file                          */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteBinLayer (const NN_PLAYER pLayer)
{
	NN_LAYER_ATTRIB la;
	NN_STATUS nns;

	assert(pLayer != NULL);
	assert(g_stream != NULL);

	/* Write the layer section header to the NNFF file */
	nns = Nn_WriteBinHeader(*(int*)NN_LAYER_SECTION_ID, NN_LAYER_SECTION_SIZE);
	if (nns != NN_OK)
		return nns;
	
	/* Write the layer section to the NNFF file */
        la = pLayer->la;
        if (eo_endian_order() != BIG_ENDIAN) 
		eo_swap_layer_attrib(&la);
	fwrite(&la, NN_LAYER_SECTION_SIZE, 1, g_stream);
	if (ferror(g_stream))
		return Nn_SetFileWriteError();

	return NN_OK;
}


/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteBinUnit                                                  */
/* Purpose:  Writes a unit section to the NNFF file                           */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteBinUnit  (const NN_PUNIT pUnit)
{
        NN_UNIT_ATTRIB ua;
	NN_STATUS nns;

	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Write the unit section header to the NNFF file */
	nns = Nn_WriteBinHeader(*(int*)NN_UNIT_SECTION_ID, NN_UNIT_SECTION_SIZE);
	if (nns != NN_OK)
		return nns;

	/* Write the unit section to the NNFF file */
	ua = pUnit->ua;
	if (eo_endian_order() != BIG_ENDIAN) 
		eo_swap_unit_attrib(&ua);
	fwrite(&ua, NN_UNIT_SECTION_SIZE, 1, g_stream);
	if (ferror(g_stream))
		return Nn_SetFileWriteError();

	/* If the unit has incoming connections */
	if (pUnit->ua.nNumConns > 0 && pUnit->aConns != NULL)
	{
		/* Write all connections directly after the unit section: */
		nns =  Nn_WriteBinConns(pUnit);
		if (nns != NN_OK)
			return nns;
	}

	/* If the unit has a matrix defined */
	if (pUnit->ua.nNumConns > 0 && pUnit->ppfMatrix != NULL)
	{
		/* Write the matrix definitiondirectly after the connection section: */
		nns =  Nn_WriteBinMatrix(pUnit);
		if (nns != NN_OK)
			return nns;
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteBinConns                                                 */
/* Purpose:  Writes all incoming connections to the NNFF file                 */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteBinConns (const NN_PUNIT pUnit)
{
	NN_CONN_ATTRIB ca;
	NN_STATUS nns;
	NN_PCONN  pConn;
	short     iC;

	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Write the connection section header to the NNFF file */
	nns = Nn_WriteBinHeader(*(int*)NN_CONN_SECTION_ID, NN_CONN_ENTRY_SIZE);
	if (nns != NN_OK)
		return nns;

	/* Write all connections directly after the unit section: */
	/* For all connections                                   */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		/* Get the connection at the given position */
		pConn = Nn_GetConnAt(pUnit, iC);

		/* Write the connection to the NNFF file */
                ca = pConn->ca;
		if (eo_endian_order() != BIG_ENDIAN) 
			eo_swap_conn_attrib(&ca);
		fwrite(&ca, 
			   NN_CONN_ENTRY_SIZE, 
			   1, 
			   g_stream);
		if (ferror(g_stream))
			return Nn_SetFileWriteError();
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteBinMatrix                                                */
/* Purpose:  Writes a unit matrix to the NNFF file                            */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteBinMatrix  (const NN_PUNIT pUnit)
{
	NN_STATUS nns;
	NN_FLOAT* pfRow;
	short     iC;
	NN_FLOAT* pfBuf;
	size_t    nBuf;       
	
        

	assert(pUnit != NULL);
	assert(g_stream != NULL);

	/* Write the unit section header to the NNFF file */
	nns = Nn_WriteBinHeader(*(int*)NN_MATRIX_SECTION_ID, NN_MATRIX_ENTRY_SIZE);
	if (nns != NN_OK)
		return nns;

	nBuf  = pUnit->ua.nNumConns * NN_MATRIX_ENTRY_SIZE;
	pfBuf = malloc(nBuf);

	/* Write the matrix row by row directly after the connections */
	/* For all matrix rows (the size of the matrix is pUnit->ua.nNumConns ^ 2) */
	for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
	{
		/* Get the matrix row at the given position */
		pfRow = Nn_GetMatrixRowAt(pUnit, iC);


		/* Write the row to the NNFF file */
		memcpy(pfBuf, pfRow, nBuf);
		if (eo_endian_order() != BIG_ENDIAN) 
			eo_swap_double_n(pfBuf, pUnit->ua.nNumConns);
		fwrite(pfBuf, 
			   NN_MATRIX_ENTRY_SIZE * pUnit->ua.nNumConns,
			   1,
			   g_stream);
		if (ferror(g_stream))
		{
			free(pfBuf);
			return Nn_SetFileReadError();
		}
	}

	free(pfBuf);
	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/

void eo_swap_net_attrib(NN_NET_ATTRIB* pna)
{
	eo_swap_short_n(pna->anVersion, 2);
	eo_swap_short_n(&(pna->nNumLayers), 1);
	eo_swap_short_n(&(pna->iInpLayer), 1);
	eo_swap_short_n(&(pna->iOutLayer), 1);
	eo_swap_short_n(&(pna->nPrecision), 1);
}

void eo_swap_layer_attrib(NN_LAYER_ATTRIB* pla) 
{
	eo_swap_short_n(&(pla->iLayer), 1);
	eo_swap_short_n(&(pla->nNumUnits), 1);
	eo_swap_short_n(&(pla->nInpFnId), 1);
	eo_swap_short_n(&(pla->nActFnId), 1);
	eo_swap_short_n(&(pla->nOutFnId), 1);
	eo_swap_double_n(&(pla->fActSlope), 1);
	eo_swap_double_n(&(pla->fActThres), 1);
}

void eo_swap_unit_attrib(NN_UNIT_ATTRIB* pua) 
{
	eo_swap_short_n(&(pua->iLayer), 1);
	eo_swap_short_n(&(pua->iUnit), 1);
	eo_swap_short_n(&(pua->nNumConns), 1);
	eo_swap_short_n(&(pua->bHasMatrix), 1);
	eo_swap_double_n(&(pua->fInpBias), 1);
	eo_swap_double_n(&(pua->fInpScale), 1);
	eo_swap_double_n(&(pua->fOutBias), 1);
	eo_swap_double_n(&(pua->fOutScale), 1);
}

void eo_swap_conn_attrib(NN_CONN_ATTRIB* pca) 
{
	eo_swap_short_n(&(pca->iLayer), 1);
	eo_swap_short_n(&(pca->iUnit), 1);
	eo_swap_double_n(&(pca->fWeight), 1);
}


/*////////////////////////////////////////////////////////////////////////////*/

