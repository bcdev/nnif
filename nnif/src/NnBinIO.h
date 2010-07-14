/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnBinIO.h                                                     */
/* Purpose:     Interface def. file for binary I/O routines for the NNFF      */
/* Remarks:     Implemented in NnBinIO.c                                      */
/* Author:      Norman Fomferra (SCICON)                                      */
/*              Tel:   +49 4152 1457 (tel)                                    */
/*              Fax:   +49 4152 1455 (fax)                                    */
/*              Email: Norman.Fomferra@gkss.de                                */
/*////////////////////////////////////////////////////////////////////////////*/

#ifdef __cplusplus
extern "C" {
#endif

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_CreateNetFromBinFile                                          */
/* Purpose:  Reads a neural net object from a binary NNFF file.               */
/* Remarks:  The function calls Nn_AssertSemanticIntegrity if the net object  */
/*           was succesfully read in.                                         */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_CreateNetFromBinFile
(
	PCSTR    pchFilePath,  /* Path to the binary NNFF file */
	int      nNumInpUnits, /* Size of the input vector    */
	int      nNumOutUnits, /* Size of the output vector   */
	NN_PNET* ppNet         /* The created neural net object */
);

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_WriteBinFile                                                  */
/* Purpose:  Writes a neural net object to a binary NNFF file                 */
/* Returns:  NN_OK (or zero) for success, an error code otherwise             */
/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_WriteNetToBinFile   
(
	PCSTR pchFilePath,  /* Path to the binary NNFF file */
	const NN_PNET pNet  /* The neural net object that shall be written */
);

#ifdef __cplusplus
}
#endif
/* EOF ///////////////////////////////////////////////////////////////////////*/
