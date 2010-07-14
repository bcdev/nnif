/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnProc.h                                                      */
/* Purpose:     Interface def. file for neural net processing routines        */
/* Remarks:     Implemented in NnProc.c                                       */
/* Author:      Norman Fomferra (SCICON)                                      */
/*              Tel:   +49 4152 1457 (tel)                                    */
/*              Fax:   +49 4152 1455 (fax)                                    */
/*              Email: Norman.Fomferra@gkss.de                                */
/*////////////////////////////////////////////////////////////////////////////*/

#ifdef __cplusplus
extern "C" {
#endif

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_ProcessNet                                                    */
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
);

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
);


#ifdef __cplusplus
}
#endif
/* EOF ///////////////////////////////////////////////////////////////////////*/

