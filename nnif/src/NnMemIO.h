/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnMemIO.h                                                     */
/* Purpose:     Interface def. file for binary memory I/O routines for the NNFF */
/* Remarks:     Implemented in NnMemIO.c                                       */
/* Author:      Norman Fomferra (SCICON)                                       */
/*              Tel:   +49 4152 1457 (tel)                                     */
/*              Fax:   +49 4152 1455 (fax)                                     */
/*              Email: Norman.Fomferra@gkss.de                                 */
/*//////////////////////////////////////////////////////////////////////////// */

#ifdef __cplusplus
extern "C" {
#endif

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
	size_t*  pnBytesRead,   /* Number of bytes read (pointer can be NULL) */
	NN_PNET* ppNet,         /* The resulting neural net object           */
	int      nNumInpUnits,  /* Number of input units                     */
	int      nNumOutUnits   /* Number of output units                    */
);

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
);
  
/*////////////////////////////////////////////////////////////////////////////*/

/*////////////////////////////////////////////////////////////////////////////*/
/* Type:     NN_MSTREAM                                                       */
/* Purpose:  Structure that represents a memory stream                        */
/*////////////////////////////////////////////////////////////////////////////*/

typedef struct SMStream
{
	PMEM   pMemBase;
	size_t nCurrPos;
	size_t nLastPos;
	int    nOpenMode;
	int    nErrNo;
}
NN_MSTREAM;

/*////////////////////////////////////////////////////////////////////////////*/
/* Memory stream routines                                                     */

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MOpen                                                         */
/* Purpose:  Standard library 'fopen' equivalent for a memory stream          */
/* Returns:  The memory stream if it can be created, NULL otherwise           */
/*////////////////////////////////////////////////////////////////////////////*/

NN_MSTREAM* Nn_MOpen (PMEM pMem, size_t nMemSize, PCSTR pchMode);

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MClose                                                        */
/* Purpose:  Standard library 'fclose' equivalent for a memory stream         */
/* Returns:  Zero if the stream was successfully closed                       */
/*////////////////////////////////////////////////////////////////////////////*/

int Nn_MClose (NN_MSTREAM* pMStream);

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
);

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
);

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MPos                                                          */
/* Purpose:  Standard library 'fpos' equivalent for a memory stream           */
/* Returns:  The actual read/write position within the memory stream          */
/*////////////////////////////////////////////////////////////////////////////*/

/* Prototype: size_t Nn_MPos (NN_MSTREAM* pMStream); */
#define Nn_MPos(pMStream) ((pMStream)->nCurrPos)

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MError                                                        */
/* Purpose:  Standard library 'ferror' equivalent for a memory stream         */
/* Returns:  If no error has occurred on stream, Nn_MError returns 0.         */
/*           Otherwise, it returns a nonzero value                            */
/*////////////////////////////////////////////////////////////////////////////*/

/* Prototype: int Nn_MError (NN_MSTREAM* pMStream); */
#define Nn_MError(pMStream) ((pMStream)->nErrNo)

/*////////////////////////////////////////////////////////////////////////////*/
/* Function: Nn_MEof                                                          */
/* Purpose:  Standard library 'feof' equivalent for a memory stream           */
/* Returns:  The feof function returns a nonzero value after the first read   */
/*           operation that attempts to read past the end of the file. It     */
/*           returns 0 if the current position is not end of file. There is no */
/*           error return.                                                    */
/*////////////////////////////////////////////////////////////////////////////*/

/* Prototype: int Nn_MEof (NN_MSTREAM* pMStream) */
#define Nn_MEof(pMStream) ((pMStream)->nCurrPos >= (pMStream)->nLastPos)

#ifdef __cplusplus
}
#endif
/* EOF ///////////////////////////////////////////////////////////////////////*/

