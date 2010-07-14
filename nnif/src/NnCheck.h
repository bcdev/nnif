/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnCheck.h                                                     */
/* Purpose:     Interface def. file for neural net checking routines          */
/* Remarks:     Implemented in NnCheck.c                                      */
/* Author:      Norman Fomferra (SCICON)                                      */
/*              Tel:   +49 4152 1457 (tel)                                    */
/*              Fax:   +49 4152 1455 (fax)                                    */
/*              Email: Norman.Fomferra@gkss.de                                */
/*////////////////////////////////////////////////////////////////////////////*/

#ifdef __cplusplus
extern "C" {
#endif

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
	NN_PNET pNet,            /* The neural net object to be checked */
	int     nNumInpUnits,    /* The size of the net input vector   */
	int     nNumOutUnits     /* The size of the net output vector  */
);

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_PrintLayerOutputs                                             */
/* Purpose:  Prints the outputs of all units of all layers                    */
/* Returns:  No return value                                                  */
/*////////////////////////////////////////////////////////////////////////////*/

void Nn_PrintLayerOutputs(NN_PNET pNet, /* the net */
                          FILE* ostream, /* file output stream */
                          const char * format); /* fprintf output format string 
                                                  for a single unit value */

#ifdef __cplusplus
}
#endif
/* EOF ///////////////////////////////////////////////////////////////////////*/
