/*
 * File:        NnMain.c
 * Purpose:     Implementation of the neural net file conversion tool
 * Author:      Norman Fomferra (Brockmann Consult GmbH)
 *              Tel:   +49 4152 889 303 (tel)
 *              Fax:   +49 4152 889 333 (fax)
 *              Email: norman.fomferra@brockmann-consult.de
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#include <NnBase.h>
#include <NnCheck.h>
#include <NnProc.h>
#include <NnMemIO.h>
#include <NnBinIO.h>
#include <NnAscIO.h>


#define NNFT_PROGRAM_NAME    "nnftool"
#define NNFT_COPYRIGHT_INFO  "Copyright (c) 1998-2010 by Brockmann Consult GmbH"

/*
 * @todo testNnfNet detectes wrong I/O vector size if the test file contains a single line
 */


/* V 1.2: Included Dr. Schiller's IMT network (nf)
 *
 * V 1.3: Added flag handling in writeFfbpFunc for the Dr. Schiller's IMT networks (nf)
 *
 * V 1.4: ??? - use diff to find out 
 *
 * V 1.4.1: openFile now logs the file being opened to stderr, added message to file-format-errors
 *
 * V 1.5: Added new option -ib to also privide per unit scaling offsets.  
 */
#define NNFT_VERSION_INFO    "Version 1.5"  

#define NUM_LAYERS_MAX  16

#define NN_BIN_EXT  ".nnf"
#define NN_ASC_EXT  ".nna"

#define NUM_FFBP_LINES_SKIP 27
#define LINE_LEN_MAX   1023

#define IO_VECTOR_SIZE_MAX 1024

#define PI 3.14159265359


#define ERR_LIMIT 1e-4

typedef enum 
{
	NNFTOOL_HELP,
	NNFTOOL_NNF2NNF,
	NNFTOOL_FFBP2NNF,
	NNFTOOL_FFBPX2NNF,
	NNFTOOL_TEST,
	NNFTOOL_CREATE
}
PRG_MODE;

typedef struct
{
	int       nNumInp;
	char**    ppchInpName;
	int*      pnInpFnId;
 	double*   pdInpMin;
	double*   pdInpMax;

	int       nNumOut;
	char**    ppchOutName;
	int*      pnOutFnId;
	double*   pdOutMin;
	double*   pdOutMax;
}
FFBP_TRANS;

static PRG_MODE g_nPrgMode                     = NNFTOOL_NNF2NNF;
static char     g_pchNnIFile  [NN_MAX_PATH+1]  = "";
static char     g_pchNnI2File [NN_MAX_PATH+1]  = "";
static char     g_pchNnOFile  [NN_MAX_PATH+1]  = "";
static char     g_pchPatIFile [NN_MAX_PATH+1]  = "";
static char     g_pchPatOFile [NN_MAX_PATH+1]  = "";
static char     g_pchFuncName [NN_MAX_PATH+1]  = "";
static BOOL     g_bLayerDump                   = FALSE;
static BOOL     g_bForceBinaryOut              = FALSE;
static BOOL     g_bForceMemoryCreat            = FALSE;
static int      g_nNumLinesSkip                = 0;
static int      g_nNumLayers                   = 0;
static int      g_anNumUnits  [NUM_LAYERS_MAX] = {0};
static BOOL     g_bInternalNormalising         = FALSE;
static BOOL     g_bInputScaling                = FALSE;
static BOOL     g_bOutputScaling               = FALSE;
static double   g_dThreshold                   = 0.0;
static double   g_dIBiases[IO_VECTOR_SIZE_MAX];
static double   g_dIScales[IO_VECTOR_SIZE_MAX];
static double   g_dOBiases[IO_VECTOR_SIZE_MAX];
static double   g_dOScales[IO_VECTOR_SIZE_MAX];

NN_PNET  readNnfNet     (const char* pchFile, BOOL bForceMemoryCreat);
NN_PNET  readFfbpNet(const char* pchFfbpFile, FFBP_TRANS* pFfbpTrans, BOOL bInternalNormalising, BOOL bInputScaling, BOOL bOutputScaling);
NN_PNET  createFfbpxNet (const NN_PNET pNet1, const FFBP_TRANS* pFfbpTrans1, const NN_PNET pNet2, const FFBP_TRANS* pFfbpTrans2, double threshold, BOOL bInternalNormalising);
NN_PNET  createNnfNet   (int nNumLayers, const int* pnNumUnits);
void     writeFfbpFunc  (const char* pchFuncName, const FFBP_TRANS* pFfbpTrans, BOOL bInternalNormalising, BOOL isIMTNet);
void     writeFfbpFuncDecl (FILE* ostream, const char* pchFunc, const FFBP_TRANS* pFfbpTrans, BOOL bInternalNormalising, BOOL isIMTNet);
void     testNnfNet     (NN_PNET pNet, const char* pszIFile, const char* pszOFile, int nNumLinesSkip, int bLayerDump);
void     copyNet        (NN_PNET sourceNet, NN_PNET targetNet, int layerOffset);
FILE* openFile(const char* pchFile, const char* pchMode);
void  closeFile(FILE* stream);
BOOL  isBinaryFile (const char* pchFile);
void  replaceFileExt(char* pchFile, const char* pchExt);
BOOL  overwriteExistingFile  (const char* pchFile);
BOOL  existsFile (const char* pchFile);
BOOL  startsWithString(const char* pchString, const char* pchToken);
BOOL  equalStrings(const char* pch1, const char* pch2);
BOOL  isOptionString(const char* pch);
BOOL  isEmptyString(const char* pch);
void  makeValidFunctionName(char* name);
void  printUsage ();
void  printProgramInfo();

char*  readLine(FILE* istream, int* piLine);
int    parseInt(char** ppchLine, int iLine);
double parseDouble(char** ppchLine, int iLine);
void   throwInvalidFileFormatException(int iLine, const char* pchMessage);
void   throwInvalidOptionArgumentException(const char* pchOption);
void   throwMissingOptionArgumentException(const char* pchOption);


int main (int argc, char *argv[])
{
	int   iArg;
	int   nNumArgs;

    printProgramInfo();

	if (argc <= 1) 
    {
		printUsage();
		return -1;
	}

    {
        int i;
        for (i = 0; i < IO_VECTOR_SIZE_MAX; i++)
        {
            g_dIBiases[i] = 0.0;
            g_dIScales[i] = 1.0;
            g_dOBiases[i] = 0.0;
            g_dOScales[i] = 1.0;
        }
    }
	
	nNumArgs = 0;
	for (iArg = 1; iArg < argc; iArg++) 
    {
		if (isOptionString(argv[iArg])) 
        {
			char* pchOption = argv[iArg] + 1;
			if (equalStrings(pchOption, "?")) 
            {
				g_nPrgMode = NNFTOOL_HELP;
			}
			else if (equalStrings(pchOption, "nnf")) 
            {
				g_nPrgMode = NNFTOOL_NNF2NNF;
			}
			else if (equalStrings(pchOption, "ffbp")) 
            {
				g_nPrgMode = NNFTOOL_FFBP2NNF;
			}
			else if (equalStrings(pchOption, "ffbpx")) 
            {
				g_nPrgMode = NNFTOOL_FFBPX2NNF;
			}
			else if (equalStrings(pchOption, "test")) 
            {
				g_nPrgMode = NNFTOOL_TEST;
			}
			else if (equalStrings(pchOption, "create")) 
            {
				g_nPrgMode = NNFTOOL_CREATE;
			}
			else if (equalStrings(pchOption, "dump")) 
            {
				g_bLayerDump = TRUE;
			}
			else if (equalStrings(pchOption, "b")) 
            {
				g_bForceBinaryOut = TRUE;
			}
			else if (equalStrings(pchOption, "m")) 
            {
				g_bForceMemoryCreat = TRUE;
			}
			else if (equalStrings(pchOption, "n")) 
            {
				g_bInternalNormalising = TRUE;
			}
			else if ((pchOption[0] == 'i' || pchOption[0] == 'o') 
                     && (pchOption[1] == 's' || pchOption[1] == 'b') 
					 && isdigit(pchOption[2])) 
            {
				char* pch;
                int i1, i2 = 0;

                if (pchOption[0] == 'i')
                    g_bInputScaling = TRUE;
                if (pchOption[0] == 'o')
                    g_bOutputScaling = TRUE;

                i1 = strtol(pchOption+2, &pch, 0) - 1;
                if (i1 < 0 || i1 >= IO_VECTOR_SIZE_MAX) 
                    throwInvalidOptionArgumentException(pchOption);
                if (*pch == '-') {
                    i2 = strtol(pch + 1, &pch, 0) - 1;
                    if (i2 < i1 || i2 >= IO_VECTOR_SIZE_MAX) 
                        throwInvalidOptionArgumentException(pchOption);
                }
				if (iArg < argc-1 && !isOptionString(argv[iArg+1])) {
                    int i;
                    double value;
					iArg++;
					if (isEmptyString(argv[iArg]))
						throwInvalidOptionArgumentException(pchOption);
                    value = strtod(argv[iArg], &pch);
                    if (*pch != '\0') 
                        throwInvalidOptionArgumentException(pchOption);
                    if (*pch != '\0') 
                        throwInvalidOptionArgumentException(pchOption);
                    for (i = i1; i <= i2; i++)
                    {
						if (pchOption[0] == 'i') {
							if (pchOption[1] == 's') {
								g_dIScales[i] = value;
							} else {
								g_dIBiases[i] = value;
							}
						} else {
							if (pchOption[1] == 's') {
								g_dOScales[i] = value;
							} else {
								g_dOBiases[i] = value;
							}
						}
                    }
				}
				else
					throwMissingOptionArgumentException(pchOption);
			}
			else if (equalStrings(pchOption, "o")) 
            {
				if (iArg < argc-1 && !isOptionString(argv[iArg+1])) {
					iArg++;
					if (isEmptyString(argv[iArg]))
						throwInvalidOptionArgumentException(pchOption);
					strcpy(g_pchNnOFile, argv[iArg]);
					strcpy(g_pchPatOFile, argv[iArg]);
				}
				else
					throwMissingOptionArgumentException(pchOption);
			}
			else if (equalStrings(pchOption, "l")) 
            {
				if (iArg < argc-1 && !isOptionString(argv[iArg+1])) 
                {
					iArg++;
					if (isEmptyString(argv[iArg]))
						throwInvalidOptionArgumentException(pchOption);
					g_nNumLinesSkip = atoi(argv[iArg]);
					if (g_nNumLinesSkip <= 0)
						throwInvalidOptionArgumentException(pchOption);
				}
				else
					throwMissingOptionArgumentException(pchOption);
			}
			else if (isEmptyString(pchOption)) 
            {
				fprintf(stderr, "Missing option\n");
				return -1;
			}
			else  
            {
				fprintf(stderr, "Unknown option %s\n", pchOption);
				return -1;
			}
		}
		else 
        {
			if (isEmptyString(argv[iArg])) 
            {
				fprintf(stderr, "Invalid argument\n");
				return -1;
			}

			if (g_nPrgMode == NNFTOOL_NNF2NNF) 
            {
				if (nNumArgs == 0)
					strcpy(g_pchNnIFile, argv[iArg]);
			}
			else if (g_nPrgMode == NNFTOOL_FFBP2NNF) 
            {
				if (nNumArgs == 0)
					strcpy(g_pchNnIFile, argv[iArg]);
				else if (nNumArgs == 1) 
                {
					strcpy(g_pchFuncName, argv[iArg]);
                    makeValidFunctionName(g_pchFuncName);
				}
			}
			else if (g_nPrgMode == NNFTOOL_FFBPX2NNF) 
            {
				if (nNumArgs == 0)
					strcpy(g_pchNnIFile, argv[iArg]);
				else if (nNumArgs == 1)
					strcpy(g_pchNnI2File, argv[iArg]);
				else if (nNumArgs == 2) 
                {
					g_dThreshold = atof(argv[iArg]);
					if (g_dThreshold < 0.0) 
                    {
						fprintf(stderr, "Invalid threshold\n");
						return -1;
					}
				}
				else if (nNumArgs == 3) 
                {
					strcpy(g_pchFuncName, argv[iArg]);
                    makeValidFunctionName(g_pchFuncName);
				}
			}
			else if (g_nPrgMode == NNFTOOL_TEST) 
            {
				if (nNumArgs == 0)
					strcpy(g_pchNnIFile, argv[iArg]);
				else if (nNumArgs == 1)
					strcpy(g_pchPatIFile, argv[iArg]);
			}
			else if (g_nPrgMode == NNFTOOL_CREATE) 
            {
				if (nNumArgs < NUM_LAYERS_MAX) 
                {
					g_anNumUnits[nNumArgs] = atoi(argv[iArg]);
					if (g_anNumUnits[nNumArgs] <= 0) 
                    {
						fprintf(stderr, "Invalid number of units\n");
						return -1;
					}
					g_nNumLayers++;
				}
			}

			nNumArgs++;
		}
	}

	if (g_nPrgMode == NNFTOOL_NNF2NNF   && nNumArgs != 1 ||
		g_nPrgMode == NNFTOOL_FFBP2NNF  && nNumArgs < 1  ||
		g_nPrgMode == NNFTOOL_FFBPX2NNF && nNumArgs < 3  ||
		g_nPrgMode == NNFTOOL_TEST      && nNumArgs != 2 ||
		g_nPrgMode == NNFTOOL_CREATE    && (nNumArgs < 2 || nNumArgs >= NUM_LAYERS_MAX) ||
		g_nPrgMode == NNFTOOL_HELP      && nNumArgs != 0) 
	{
		fprintf(stderr, "Invalid number of arguments\n");
		return -1;
	}

    Nn_SetOutStream(stdout);


	if (g_nPrgMode == NNFTOOL_NNF2NNF ||
		g_nPrgMode == NNFTOOL_FFBP2NNF ||
		g_nPrgMode == NNFTOOL_FFBPX2NNF ||
		g_nPrgMode == NNFTOOL_CREATE)
	{
		NN_PNET pNet;
		NN_STATUS nns;

		if (g_nPrgMode == NNFTOOL_NNF2NNF) 
        {
			pNet = readNnfNet(g_pchNnIFile, g_bForceMemoryCreat);
		}
		else if (g_nPrgMode == NNFTOOL_FFBP2NNF) 
        {
    		FFBP_TRANS ffbpTrans; 
			pNet = readFfbpNet(g_pchNnIFile, &ffbpTrans, g_bInternalNormalising, g_bInputScaling, g_bOutputScaling);
			if (!isEmptyString(g_pchFuncName))
				writeFfbpFunc(g_pchFuncName, &ffbpTrans, g_bInternalNormalising, FALSE);
		}
		else if (g_nPrgMode == NNFTOOL_FFBPX2NNF) 
        {
    		FFBP_TRANS ffbpTrans1; 
	    	FFBP_TRANS ffbpTrans2; 
            NN_PNET pNet1 = readFfbpNet(g_pchNnIFile, &ffbpTrans1, g_bInternalNormalising, g_bInputScaling, g_bOutputScaling);
            NN_PNET pNet2 = readFfbpNet(g_pchNnI2File, &ffbpTrans2, FALSE, FALSE, FALSE);
			pNet = createFfbpxNet(pNet1, &ffbpTrans1, pNet2, &ffbpTrans2, g_dThreshold, g_bInternalNormalising);
			if (!isEmptyString(g_pchFuncName))
				writeFfbpFunc(g_pchFuncName, &ffbpTrans1, g_bInternalNormalising, TRUE);
		}
		else 
        {
			pNet = createNnfNet(g_nNumLayers, g_anNumUnits); 
		}

		if (g_bForceBinaryOut) 
        {
			if (isEmptyString(g_pchNnOFile)) 
            {
				strcpy(g_pchNnOFile, g_pchNnIFile);
				replaceFileExt(g_pchNnOFile, NN_BIN_EXT);
			}
			if (existsFile(g_pchNnOFile) && !overwriteExistingFile(g_pchNnOFile))
				return 0;
			nns = Nn_WriteNetToBinFile(g_pchNnOFile, pNet);
		}
		else 
        {
			if (isEmptyString(g_pchNnOFile)) 
            {
				strcpy(g_pchNnOFile, g_pchNnIFile);
				replaceFileExt(g_pchNnOFile, NN_ASC_EXT);
			}
			if (existsFile(g_pchNnOFile) && !overwriteExistingFile(g_pchNnOFile))
				return 0;
			nns = Nn_WriteNetToAscFile(g_pchNnOFile, pNet);
		}
		
		if (nns != NN_OK) 
        {
			fprintf(stderr, "NNF-Error: %s (NN_STATUS=%d)\n", Nn_GetErrMsg(), Nn_GetErrNo());
			exit(-1);
		}

		Nn_DeleteNet(pNet);
	}
	else if (g_nPrgMode == NNFTOOL_TEST) 
    {
		NN_PNET pNet = readNnfNet(g_pchNnIFile, g_bForceMemoryCreat);
		if (isEmptyString(g_pchPatOFile)) 
        {
			strcpy(g_pchPatOFile, g_pchPatIFile);
			replaceFileExt(g_pchPatOFile, "_res.txt");
		}
		testNnfNet(pNet, g_pchPatIFile, g_pchPatOFile, g_nNumLinesSkip, g_bLayerDump);
		Nn_DeleteNet(pNet);
	}
	else 
    {
		printUsage();
	}
	
	return 0;
}


NN_PNET readNnfNet(const char* pchNnfFile, BOOL bForceMemoryCreat)
{
	NN_PNET   pNet = NULL;
	NN_STATUS nns;
	FILE*     istream;

	istream = openFile(pchNnfFile, "rb");

	if (isBinaryFile(pchNnfFile)) 
    {
		if (bForceMemoryCreat) 
        {
			size_t nFileSize, nBytesRead;
			unsigned char* pMem;

			fseek(istream, 0, SEEK_END);
			nFileSize = ftell(istream);
			pMem = (unsigned char*) malloc(nFileSize);
			fseek(istream, 0, SEEK_SET);
			fread(pMem, 1, nFileSize, istream);
			fclose(istream);
			nns = Nn_CreateNetFromMemFile(pMem, nFileSize, &nBytesRead, &pNet, -1, -1);
			printf("Memory creation status: %d bytes file size, %d bytes converted\n", nFileSize, nBytesRead);
			free(pMem);
		}
		else 
        {
			closeFile(istream);
			nns = Nn_CreateNetFromBinFile(pchNnfFile, -1, -1, &pNet);
		}
	}
	else 
    {
		closeFile(istream);
		nns = Nn_CreateNetFromAscFile(pchNnfFile, -1, -1, &pNet);
	}

	if (nns != NN_OK) 
    {
		fprintf(stderr, "NNF-Error: %s (NN_STATUS=%d)\n", Nn_GetErrMsg(), Nn_GetErrNo());
		exit(-1);
	}

	return pNet;
}


NN_PNET readFfbpNet(const char* pchFfbpFile, FFBP_TRANS* pFfbpTrans, BOOL bInternalNormalising, BOOL bInputScaling, BOOL bOutputScaling)
{
	NN_PNET   pNet;
	NN_PLAYER pLayer;
	NN_PUNIT  pUnit;
	NN_PCONN  pConn;
	NN_STATUS nns;
	FILE*     istream;
	int       i, iLine, iL, iU, iC;
	char*     pchLine;
	int       nLayerIndex;
	int       nNumUnits, nNumUnits1, nNumUnits2;
	int       nNumInp, nNumOut;
	char      pchVarName[256];

	pFfbpTrans->nNumInp = 0;
	pFfbpTrans->pnInpFnId = NULL;
 	pFfbpTrans->pdInpMin = NULL;
	pFfbpTrans->pdInpMax = NULL;

	pFfbpTrans->nNumOut = 0;
	pFfbpTrans->pnOutFnId = NULL;
	pFfbpTrans->pdOutMin = NULL;
	pFfbpTrans->pdOutMax = NULL;

	istream = openFile(pchFfbpFile, "r");

	Nn_CreateNet(&pNet);
	iLine = 0;

    /*	
     *  while (iLine < NUM_FFBP_LINES_SKIP)
     *		pchLine = readLine(istream, &iLine);
     */

	pchLine = readLine(istream, &iLine); /* "problem: %s\n" */
	pchLine = readLine(istream, &iLine); /* "saved at %s %s %d %s %d\n" */
	pchLine = readLine(istream, &iLine); /* "\n" */
	pchLine = readLine(istream, &iLine); /* "trainings sample has total sum of error^2=%lf\n" */
	pchLine = readLine(istream, &iLine); /* "average of residues:\n" */
	pchLine = readLine(istream, &iLine); /* " training %s=%lf  test %s=%lf\n" */
	pchLine = readLine(istream, &iLine); /* " ratio avg.train/avg.test=%lf\n" */
	
	/* Read variable names for input vector
	 */
	pchLine = readLine(istream, &iLine); /* "\n" */
	pchLine = readLine(istream, &iLine); /* "the net has %d inputs:\n" */
	if (sscanf(pchLine, "the net has %d inputs:\n", &nNumInp) != 1 || nNumInp <= 0)
		throwInvalidFileFormatException(iLine, "missing or illegal net input specification");
	pFfbpTrans->nNumInp = nNumInp;
	pFfbpTrans->ppchInpName = (char**) malloc(nNumInp * sizeof (char*));
	pFfbpTrans->pnInpFnId = (int*) malloc(nNumInp * sizeof (int));
	for (i = 0; i < nNumInp; i++) 
    {
		pchLine = readLine(istream, &iLine);
		if (sscanf(pchLine, "input %*d is %s in [%*lf,%*lf]\n", pchVarName) != 1 || isEmptyString(pchVarName))
			throwInvalidFileFormatException(iLine, "missing or illegal net input parameter specification");
		pFfbpTrans->ppchInpName[i] = (char*) malloc(strlen(pchVarName) + 1);
		strcpy(pFfbpTrans->ppchInpName[i], pchVarName);
		if (startsWithString(pchVarName, "exp("))
			pFfbpTrans->pnInpFnId[i] = NN_FUNC_EXPONENTIAL;
		else if (startsWithString(pchVarName, "log("))
			pFfbpTrans->pnInpFnId[i] = NN_FUNC_LOGARITHMIC;
		else if (strchr(pchVarName, '(') == NULL)
			pFfbpTrans->pnInpFnId[i] = NN_FUNC_IDENTITY;
		else
			throwInvalidFileFormatException(iLine, "missing or illegal net input function specification");
	}

	/* Read variable names for input vector
	 */
	pchLine = readLine(istream, &iLine); /* "\n" */
	pchLine = readLine(istream, &iLine); /* "the net has %d outputs:\n" */
	if (sscanf(pchLine, "the net has %d outputs:\n", &nNumOut) != 1 || nNumOut <= 0)
		throwInvalidFileFormatException(iLine, "missing or illegal net output specification");
	pFfbpTrans->nNumOut = nNumOut;
	pFfbpTrans->ppchOutName = (char**) malloc(nNumOut * sizeof (char*));
	pFfbpTrans->pnOutFnId = (int*) malloc(nNumOut * sizeof (int));
	for (i = 0; i < nNumOut; i++) 
    {
		pchLine = readLine(istream, &iLine);
		if (sscanf(pchLine, "output %*d is %s in [%*lf,%*lf]\n", pchVarName) != 1 || isEmptyString(pchVarName))
			throwInvalidFileFormatException(iLine, "missing or illegal net output parameter specification");
		pFfbpTrans->ppchOutName[i] = (char*) malloc(strlen(pchVarName) + 1);
		strcpy(pFfbpTrans->ppchOutName[i], pchVarName);
		if (startsWithString(pchVarName, "exp("))
			pFfbpTrans->pnOutFnId[i] = NN_FUNC_EXPONENTIAL;
		else if (startsWithString(pchVarName, "log("))
			pFfbpTrans->pnOutFnId[i] = NN_FUNC_LOGARITHMIC;
		else if (strchr(pchVarName, '(') == NULL)
			pFfbpTrans->pnOutFnId[i] = NN_FUNC_IDENTITY;
		else
			throwInvalidFileFormatException(iLine, "missing or illegal net output function specification");
	}

	pchLine = readLine(istream, &iLine); /* "\n" */
	pchLine = readLine(istream, &iLine); /* "ranges repeated for easier input\n" */
	pchLine = readLine(istream, &iLine); /* "#\n" */
	if (*pchLine != '#') 
		throwInvalidFileFormatException(iLine, "missing '#' character");

	/* Read the size of the input vector
	 */
	pchLine = readLine(istream, &iLine);
	if (sscanf(pchLine, "%d\n", &nNumInp) != 1 || nNumInp != pFfbpTrans->nNumInp)
		throwInvalidFileFormatException(iLine, "missing or illegal number of input neurons");
	pFfbpTrans->pdInpMin = (double*) malloc(nNumInp * sizeof (double));
	pFfbpTrans->pdInpMax = (double*) malloc(nNumInp * sizeof (double));

	/* Read MIN/MAX for input vector
	 */
	for (i = 0; i < nNumInp; i++) 
    {
		pchLine = readLine(istream, &iLine);
		pFfbpTrans->pdInpMin[i] = parseDouble(&pchLine, iLine);
		pFfbpTrans->pdInpMax[i] = parseDouble(&pchLine, iLine);
	}

	/* Read the size of the output vector
	 */
	pchLine = readLine(istream, &iLine);
	if (sscanf(pchLine, "%d\n", &nNumOut) != 1 || nNumOut != pFfbpTrans->nNumOut)
		throwInvalidFileFormatException(iLine, "missing or illegal number of output neurons");
	pFfbpTrans->pdOutMin = (double*) malloc(nNumOut * sizeof (double));
	pFfbpTrans->pdOutMax = (double*) malloc(nNumOut * sizeof (double));

	/* Read MIN/MAX for input vector
	 */
	for (i = 0; i < nNumOut; i++) 
    {
		pchLine = readLine(istream, &iLine);
		pFfbpTrans->pdOutMin[i] = parseDouble(&pchLine, iLine);
		pFfbpTrans->pdOutMax[i] = parseDouble(&pchLine, iLine);
	}

	/* Read section '$'
	 */
	pchLine = readLine(istream, &iLine); /* "$\n" */
	if (*pchLine != '$') 
		throwInvalidFileFormatException(iLine, "missing '$' character");

	/* Read number of layers
	 */
	pchLine = readLine(istream, &iLine); /* "#planes=%d %d %d %d...\n" */
	if (*pchLine != '#') 
		throwInvalidFileFormatException(iLine, "missing '#' character");
	pchLine = strchr(pchLine, '=');
	if (pchLine == NULL) 
		throwInvalidFileFormatException(iLine, "missing '=' character in planes specification");
	pchLine++;
	pNet->na.nNumLayers = (short)parseInt(&pchLine, iLine);
	if (pNet->na.nNumLayers <= 1)
		throwInvalidFileFormatException(iLine, "illegal number of layers, should be > 1");
	Nn_CreateLayers(pNet);

	/* Read number of units for each layer
	 */
	for (iL = 0; iL < pNet->na.nNumLayers; iL++) 
    {
		pLayer = Nn_GetLayerAt(pNet, iL);
		
		pLayer->la.nNumUnits = (short)parseInt(&pchLine, iLine);
		if (pLayer->la.nNumUnits <= 0)
			throwInvalidFileFormatException(iLine, "illegal number of units, should be > 0");
		
		/* Overwrite function type defaults
		 * FFBP nets do not have the sigmoid-activation in the input layer!
		 * If Min/max scaling is included in the net, output functions of the input
		 * and output layer are set to a linear function
		 */
		if (iL == 0) 
        {
            if (pLayer->la.nNumUnits > IO_VECTOR_SIZE_MAX)
    			throwInvalidFileFormatException(iLine, "maximum number of units exceeded");

			pLayer->la.nActFnId = NN_FUNC_IDENTITY;
			pLayer->la.nOutFnId = NN_FUNC_LINEAR;
		}
		else if (iL == pNet->na.nNumLayers - 1) 
        {
            if (pLayer->la.nNumUnits > IO_VECTOR_SIZE_MAX)
    			throwInvalidFileFormatException(iLine, "maximum number of units exceeded");

			pLayer->la.nOutFnId = NN_FUNC_LINEAR;
		}

		Nn_CreateUnits(pLayer);

		for (iU = 0; iU < pLayer->la.nNumUnits; iU++) 
        {
			pUnit = Nn_GetUnitAt(pLayer, (short)iU);

			if (iL == 0) 
            {
				double a = 1.0;
				double b = 0.0;
				double c = 1.0;
				double d = 0.0;
				if (bInputScaling) 
				{
				    a = g_dIScales[iU];
				    b = g_dIBiases[iU];
				}
                if (bInternalNormalising) 
                {
					double dx = pFfbpTrans->pdInpMax[iU] - pFfbpTrans->pdInpMin[iU];
					c = 1.0 / dx;
					d = -pFfbpTrans->pdInpMin[iU] / dx;
				}
				pUnit->ua.fOutScale = c * a;
				pUnit->ua.fOutBias  = c * b + d;
            }
			else if (iL == pNet->na.nNumLayers - 1) 
            {
				double a = 1.0;
				double b = 0.0;
				double c = 1.0;
				double d = 0.0;
				if (bInputScaling) 
				{
				    a = g_dOScales[iU];
				    b = g_dOBiases[iU];
				}
                if (bInternalNormalising) 
                {
					double dx = pFfbpTrans->pdOutMax[iU] - pFfbpTrans->pdOutMin[iU];
					c = 1.0 / dx;
					d = -pFfbpTrans->pdOutMin[iU] / dx;
				}
				pUnit->ua.fOutScale = 1.0 / (c * a);
				pUnit->ua.fOutBias  = -(c * b + d) / (c * a);
            }

			if (iL > 0) 
            {
				pUnit->ua.nNumConns = Nn_GetLayerAt(pNet, iL-1)->la.nNumUnits;
				Nn_CreateConns(pUnit);
				for (iC = 0; iC < pUnit->ua.nNumConns; iC++) 
                {
					pConn = Nn_GetConnAt(pUnit, (short)iC);
					pConn->ca.iLayer  = (short)(iL-1);
					pConn->ca.iUnit   = (short)iC;
					pConn->ca.fWeight = 0.0;
				}
			}
		}
	}

	/* Read bias list
	 */
	for (iL = 0; iL < pNet->na.nNumLayers-1; iL++) 
    {
		pchLine = readLine(istream, &iLine); /* "bias %d %d\n" */
		if (sscanf(pchLine, "bias %d %d\n", &nLayerIndex, &nNumUnits) != 2) 
			throwInvalidFileFormatException(iLine, "missing or illegal 'bias' specification");
		pLayer = Nn_GetLayerAt(pNet, nLayerIndex);
		if (pLayer->la.nNumUnits != nNumUnits)
			throwInvalidFileFormatException(iLine, "illegal 'bias' specification: unexpected number of units");
		for (iU = 0; iU < nNumUnits; iU++) 
        {
			pchLine = readLine(istream, &iLine); /* "%lf\n" */
			pUnit = Nn_GetUnitAt(pLayer, (short)iU);
			pUnit->ua.fInpBias = parseDouble(&pchLine, iLine);
		}
	}

	/* Read weight list
	 */
	for (iL = 0; iL < pNet->na.nNumLayers-1; iL++) 
    {
		pchLine = readLine(istream, &iLine); /* "wgt %d %d %d\n" */
		if (sscanf(pchLine, "wgt %d %d %d\n", &nLayerIndex, &nNumUnits1, &nNumUnits2) != 3) 
			throwInvalidFileFormatException(iLine, "missing or illegal 'wgt' specification");
		if (nLayerIndex >= pNet->na.nNumLayers-1)
			throwInvalidFileFormatException(iLine, "illegal 'wgt' specification: layer index out of bounds");
		pLayer = Nn_GetLayerAt(pNet, nLayerIndex+1);
		if (pLayer->la.nNumUnits != nNumUnits2)
			throwInvalidFileFormatException(iLine, "illegal 'wgt' specification: unexpected number of units");
		for (iU = 0; iU < nNumUnits2; iU++) 
        {
			pUnit = Nn_GetUnitAt(pLayer, (short)iU);
			if (pUnit->ua.nNumConns != nNumUnits1)
				throwInvalidFileFormatException(iLine, "illegal 'wgt' specification: unexpected number of connections");
			for (iC = 0; iC < nNumUnits1; iC++) 
            {
				pchLine = readLine(istream, &iLine); /* "%lf\n" */
				pConn = Nn_GetConnAt(pUnit, (short)iC);
				pConn->ca.fWeight = parseDouble(&pchLine, iLine);
			}
		}
	}

	closeFile(istream);

	nns = Nn_AssertSemanticIntegrity(pNet, -1, -1);
	if (nns != NN_OK) 
    {
		fprintf(stderr, "NNF-Error: %s (NN_STATUS=%d)\n", Nn_GetErrMsg(), Nn_GetErrNo());
		exit(-1);
	}

	return pNet;
}


void writeFfbpFunc(const char* pchFunc, 
                   const FFBP_TRANS* pFfbpTrans, 
                   BOOL bInternalNormalising, 
                   BOOL isIMTNet)
{
	FILE*     ostream;
	int       i;
	char      pchHFile[NN_MAX_PATH+1];
	char      pchCFile[NN_MAX_PATH+1];

	sprintf(pchHFile, "%s.h", pchFunc);
	sprintf(pchCFile, "%s.c", pchFunc);

	if (existsFile(pchHFile) && !overwriteExistingFile(pchHFile))
		return;
	if (existsFile(pchCFile) && !overwriteExistingFile(pchCFile))
		return;

	ostream = openFile(pchHFile, "w");
    fprintf(ostream, 
        "#ifndef %s_H_INCL\n"
        "#define %s_H_INCL\n"
        "\n"
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n"
        "\n"
        "/* Forward declaration for neural net structure */\n"
        "struct SNnNet;\n"
	    "\n"
        "/* Pointer to neural net structure */\n"
        "typedef struct SNnNet* NN_PNET;\n"
	    "\n",
        pchFunc, pchFunc);

    writeFfbpFuncDecl(ostream, pchFunc, pFfbpTrans, bInternalNormalising, isIMTNet);
	fprintf(ostream, ";\n");
	fprintf(ostream, 
        "\n"
        "#ifdef __cplusplus\n"
        "}\n"
        "#endif\n"
        "\n"
        "#endif /* %s_H_INCL */\n",
        pchFunc);
	closeFile(ostream);

	ostream = openFile(pchCFile, "w");
	
    fprintf(ostream, 
	    "#include <stdio.h>\n"
	    "#include <stdlib.h>\n"
	    "#include <math.h>\n"
	    "\n"
	    "#include <NnBase.h>\n"
	    "#include <NnProc.h>\n"
	    "#include \"%s\"\n"
	    "\n",
         pchHFile);

	if (!bInternalNormalising) 
    {
        fprintf(ostream, 
            "/**\n"
            " * Array containing the ranges for input vector normalisation in the form:<p>\n"
            " * <code>{{ MIN_0, MAX_0}, {MIN_1, MAX_1}, {MIN_2, MAX_2}, ... }</code>\n"
            " */\n"
            "static const double adInpRange[%d][2] =\n"
            "{\n",
            pFfbpTrans->nNumInp);
		for (i = 0; i < pFfbpTrans->nNumInp; i++) 
        {
			fprintf(ostream, 
                "\t{ %.8g, %.8g }%s\n",
				pFfbpTrans->pdInpMin[i],
                pFfbpTrans->pdInpMax[i], i < pFfbpTrans->nNumInp-1 ? "," : "");
		}
        fprintf(ostream, 
            "};\n"
            "\n");
    
        fprintf(ostream, 
            "/**\n"
            " * Array containing the ranges for output vector normalisation in the form:<p>\n"
            " * <code>{{ MIN_0, MAX_0}, {MIN_1, MAX_1}, {MIN_2, MAX_2}, ... }</code>\n"
            " */\n"
            "static const double adOutRange[%d][2] =\n"
            "{\n",
            pFfbpTrans->nNumOut);
		for (i = 0; i < pFfbpTrans->nNumOut; i++) 
        {
			fprintf(ostream, 
                    "\t{ %.8g, %.8g }%s\n",
					pFfbpTrans->pdOutMin[i],
                    pFfbpTrans->pdOutMax[i], i < pFfbpTrans->nNumOut-1 ? "," : "");
		}
        fprintf(ostream, 
            "};\n"
            "\n");
    }

    writeFfbpFuncDecl(ostream, pchFunc, pFfbpTrans, bInternalNormalising, isIMTNet);
	fprintf(ostream, 
        "\n{\n"
	    "\tdouble adInp[%2d];\n"
        "\n",
        pFfbpTrans->nNumInp, 
        pFfbpTrans->nNumOut);

	for (i = 0; i < pFfbpTrans->nNumInp; i++) 
    {
		if (!bInternalNormalising) 
        {
			if (pFfbpTrans->pnInpFnId[i] == NN_FUNC_EXPONENTIAL)
			    fprintf(ostream, 
                    "\tadInp[%2d] = (exp(pdInp[%2d]) - adInpRange[%2d][0])\n"
                    "\t            / (adInpRange[%2d][1] - adInpRange[%2d][0]);\n", 
                    i, i, i, i, i);
			else if (pFfbpTrans->pnInpFnId[i] == NN_FUNC_LOGARITHMIC)
			    fprintf(ostream, 
                    "\tadInp[%2d] = (log(pdInp[%2d]) - adInpRange[%2d][0])\n"
                    "\t            / (adInpRange[%2d][1] - adInpRange[%2d][0]);\n", 
                    i, i, i, i, i);
            else
			    fprintf(ostream, 
                    "\tadInp[%2d] = (pdInp[%2d] - adInpRange[%2d][0])\n"
                    "\t            / (adInpRange[%2d][1] - adInpRange[%2d][0]);\n", 
                    i, i, i, i, i);
		}
		else 
        {
			if (pFfbpTrans->pnInpFnId[i] == NN_FUNC_EXPONENTIAL)
                fprintf(ostream, "\tadInp[%2d] = exp(pdInp[%2d]);\n", i, i);
			else if (pFfbpTrans->pnInpFnId[i] == NN_FUNC_LOGARITHMIC)
                fprintf(ostream, "\tadInp[%2d] = log(pdInp[%2d]);\n", i, i);
            else
                fprintf(ostream, "\tadInp[%2d] = pdInp[%2d];\n", i, i);
        }
	}

	fprintf(ostream, 
        "\n"
        "\tNn_ProcessNet(pNet, adInp, pdOut);\n"
        "\n");
	
    for (i = 0; i < pFfbpTrans->nNumOut; i++) 
    {
		if (!bInternalNormalising) 
        {
			if (pFfbpTrans->pnOutFnId[i] == NN_FUNC_EXPONENTIAL)
			    fprintf(ostream,
                    "\tpdOut[%2d] = log(pdOut[%2d] * (adOutRange[%2d][1] - adOutRange[%2d][0])\n"
                    "\t                + adOutRange[%2d][0]);\n", 
                    i, i, i, i, i);
			else if (pFfbpTrans->pnOutFnId[i] == NN_FUNC_LOGARITHMIC)
			    fprintf(ostream,
                    "\tpdOut[%2d] = exp(pdOut[%2d] * (adOutRange[%2d][1] - adOutRange[%2d][0])\n"
                    "\t                + adOutRange[%2d][0]);\n", 
                    i, i, i, i, i);
            else
			    fprintf(ostream,
                    "\tpdOut[%2d] = pdOut[%2d] * (adOutRange[%2d][1] - adOutRange[%2d][0])\n"
                    "\t            + adOutRange[%2d][0];\n", 
                    i, i, i, i, i);
		}
		else 
        {
			if (pFfbpTrans->pnOutFnId[i] == NN_FUNC_EXPONENTIAL)
                fprintf(ostream, "\tpdOut[%2d] = log(pdOut[%2d]);\n", i, i);
			else if (pFfbpTrans->pnOutFnId[i] == NN_FUNC_LOGARITHMIC)
                fprintf(ostream, "\tpdOut[%2d] = exp(pdOut[%2d]);\n", i, i);
		}
	}
    
	fprintf(ostream, "}\n");

	closeFile(ostream);
}


void writeFfbpFuncDecl(FILE* ostream, 
                       const char* pchFunc, 
                       const FFBP_TRANS* pFfbpTrans, 
                       BOOL bInternalNormalising,
                       BOOL isIMTNet)
{
	int i;

    fprintf(ostream, 
        "/**\n"
        " * The %s function processes a neural net which was converted from the\n"
        " * GKSS-FFBP format to the NNF format used by the MERIS level 2 processor.\n"
        " * <p>\n"
        " * The original FFBP net was trained with input vectors having the following definition:"
        " * <p>\n", 
        pchFunc);

    for (i = 0; i < pFfbpTrans->nNumInp; i++)
    {
		fprintf(ostream, " *   %3d: %s in [%.10g, %.10g] <br>\n", 
		        i + 1, 
				pFfbpTrans->ppchInpName[i],
				pFfbpTrans->pdInpMin[i],
				pFfbpTrans->pdInpMax[i]);
    }
	fprintf(ostream, 
        " * <p>\n"
        " * The original FFBP net was trained with output vectors having the following definition:\n"
        " * <p>\n");
	
    for (i = 0; i < pFfbpTrans->nNumOut; i++)
    {
		fprintf(ostream, " *   %3d: %s in [%.10g, %.10g] <br>\n", 
		        i + 1, 
				pFfbpTrans->ppchOutName[i],
				pFfbpTrans->pdOutMin[i],
				pFfbpTrans->pdOutMax[i]);
    }

    fprintf(ostream,
        " * <p>\n"
        " * The ranges are used to normalize the in- and output vectors to values\n"
        " * in the range [0, 1].\n");

    if (bInternalNormalising) {
        fprintf(ostream,
            " * This normalisation must be part of the neural net\n"
            " * given by <code>pNet</code> and is not performed within the %s\n"
            " * function.\n",
            pchFunc);
    }
    else {
        fprintf(ostream,
            " * This normalisation must not be part of the neural net\n"
            " * given by <code>pNet</code> because it is performed within the %s\n"
            " * function.\n",
            pchFunc);
    }

    if (isIMTNet) {
        fprintf(ostream, 
            " * \n"
            " * @param pNet the neural net\n"
            " * @param pdInp input vector, points to an array of at least %d double values\n"
            " * @param pdOut output vector, points to an array of at least %d double values\n"
            " *              the last element contains the out-of-scope flag\n"
            " *              having either the value 0.0 or 1.0\n"
            " */\n"
            "void %s(NN_PNET pNet, const double* pdInp, double* pdOut)", 
            pFfbpTrans->nNumInp, pFfbpTrans->nNumOut + 1, pchFunc);
    }
    else {
        fprintf(ostream, 
            " * \n"
            " * @param pNet the neural net\n"
            " * @param pdInp input vector, points to an array of at least %d double values\n"
            " * @param pdOut output vector, points to an array of at least %d double values\n"
            " */\n"
            "void %s(NN_PNET pNet, const double* pdInp, double* pdOut)", 
            pFfbpTrans->nNumInp, pFfbpTrans->nNumOut, pchFunc);
    }
}

/**
 * Creates Dr.Schiller's new CASE II net out of the given
 * forward net <code>net1</code> and an inverse net <code>net2</code>.
 */
NN_PNET createFfbpxNet(const NN_PNET pNet1, 
                       const FFBP_TRANS* pFfbpTrans1,
                       const NN_PNET pNet2, 
                       const FFBP_TRANS* pFfbpTrans2,
                       double threshold,
                       BOOL bInternalNormalising)
{
    NN_PNET pNet; /* The new net to be created */

    int nL1; /* Number of layers in 1st net */    
    int nL2; /* Number of layers in 2nd net */    

    short iLI1; /* Index of input layer in 1st net */      
    short iLO1; /* Index of output layer in 1st net  */     
    short iLI2; /* Index of input layer in 2nd net   */    
    short iLO2; /* Index of output layer in 2nd net  */     
    
    int nUI1; /* Number of units in input layer in 1st net  */     
    int nUO1; /* Number of units in output layer in 1st net   */    
    int nUI2; /* Number of units in input layer in 2nd net  */    
    int nUO2; /* Number of units in output layer in 2nd net */      

    NN_PLAYER pLI1; /* Input layer in 1st net   */
    NN_PLAYER pLO1; /* Output layer in 1st net  */
    NN_PLAYER pLI2; /* Input layer in 2nd net   */
    NN_PLAYER pLO2; /* Input layer in 2nd net   */
    
    nL1 = pNet1->na.nNumLayers; /* Number of layers in 1st net   */  
    nL2 = pNet2->na.nNumLayers; /* Number of layers in 2nd net  */   

    iLI1 = 0; /* Index of input layer in 1st net   */    
    iLO1 = nL1 - 1; /* Index of output layer in 1st net     */  
    iLI2 = nL1; /* Index of input layer in 2nd net       */
    iLO2 = nL1 + nL2 - 1; /* Index of output layer in 2nd net  */     
    
    Nn_CreateNet(&pNet);
    pNet->na.nNumLayers = nL1 + nL2 + 3;
    pNet->na.iInpLayer  = 0;
    pNet->na.iOutLayer  = pNet->na.nNumLayers - 1;
    
    Nn_CreateLayers(pNet);
    pLI1 = Nn_GetLayerAt(pNet, iLI1); /* Input layer in 1st net   */
    pLO1 = Nn_GetLayerAt(pNet, iLO1); /* Output layer in 1st net  */
    pLI2 = Nn_GetLayerAt(pNet, iLI2); /* Input layer in 2nd net  */ 
    pLO2 = Nn_GetLayerAt(pNet, iLO2); /* Output layer in 2nd net  */

    /* Copy layers of 1st net into the new one.
     */
    copyNet(pNet1, pNet, 0);

    /* Copy layers of 2nd net into the new one.
     */
    copyNet(pNet2, pNet, iLI2);

    nUI1 = pLI1->la.nNumUnits; /* Number of units in input layer in 1st net */      
    nUO1 = pLO1->la.nNumUnits; /* Number of units in output layer in 1st net */      
    nUI2 = pLI2->la.nNumUnits; /* Number of units in input layer in 2nd net */      
    nUO2 = pLO2->la.nNumUnits; /* Number of units in output layer in 2nd net  */     

    /* Connect the 1st layer of the 2nd net with the input and output layers
     * of the 1st net.
     */
    {
        NN_PUNIT pU;
        NN_PCONN pC;
        short iU;

        pLI2->la.nInpFnId  = NN_FUNC_SUM_1;
        pLI2->la.nOutFnId  = NN_FUNC_LINEAR;

        for (iU = 0; iU < nUI2; iU++)
        {
            /* Consider next unit pU of the input layer of the 2nd net
             */
            pU = Nn_GetUnitAt(pLI2, iU);

            pU->ua.nNumConns = 1;
            pU->ua.fOutScale = 1.0;
            pU->ua.fOutBias  = 0.0;

            /* Create connections for current unit pU
             */
            Nn_CreateConns(pU);

            pC = Nn_GetConnAt(pU, 0);
            pC->ca.fWeight = 1.0;

            if (iU < (nUI2 - nUO1))
            {
                pC->ca.iLayer = iLI1;
                pC->ca.iUnit  = iU;
            }
            else
            {
                /* unit index in output layer of 1st net 
                 */
                int iUO1 = iU - (nUI2 - nUO1); 

                /* If the bInternalNormalising flag is on, the 1st net produces
                 * physical units as output. So these have to be re-normalized (again).
                 */
                if (bInternalNormalising)
                {
                    double offs  = pFfbpTrans1->pdOutMin[iUO1];
                    double scale = pFfbpTrans1->pdOutMax[iUO1] - pFfbpTrans1->pdOutMin[iUO1];
                    pU->ua.fOutScale =    1.0 / scale;
                    pU->ua.fOutBias  = - offs / scale;            
                }
                pC->ca.iLayer = iLO1;
                pC->ca.iUnit  = iUO1;
            }
        }
    }

    /* Create a new layer that summates the negated output of output layer of
     * the 2nd net and a part of the input layer of the 1st net and computes the 
     * squares.
     */
    {
        NN_PLAYER pL;
        NN_PUNIT  pU;
        NN_PCONN  pC;
        short iU1;
        short iU2, nU2;
        NN_FLOAT a1, b1;
        NN_FLOAT a2, b2;

        /* Consider the first layer pL following the output layer of the 2nd net
         */
        pL = Nn_GetLayerAt(pNet, iLO2 + 1);
        pL->la.nNumUnits = nUO2;
        pL->la.nActFnId  = NN_FUNC_IDENTITY;
        pL->la.nOutFnId  = NN_FUNC_QUADRATIC;
        Nn_CreateUnits(pL);

        /* for all units of pL
         */
        nU2 = pL->la.nNumUnits;
        for (iU2 = 0; iU2 < nU2; iU2++)
        {
            /* Index of the corresponding unit in the very first layer of 
             * the 1st net
             */
            iU1 = iU2 + (nUI1 - nUO2);

            /* Consider next unit pU of the first layer following the output
             * layer of the 2nd net
             */
            pU = Nn_GetUnitAt(pL, iU2);
            pU->ua.nNumConns = 2;

            /* Init linear transformation with a = max-min and b = min
             */
            assert(iU1 >= 0 && iU1 < pFfbpTrans1->nNumInp);
            assert(iU2 >= 0 && iU2 < pFfbpTrans2->nNumOut);
            b1 = pFfbpTrans1->pdInpMin[iU1];
            a1 = pFfbpTrans1->pdInpMax[iU1] - b1;
            b2 = pFfbpTrans2->pdOutMin[iU2];
            a2 = pFfbpTrans2->pdOutMax[iU2] - b2;

            /* Combine offsets of both layers to a single input bias
             */
            pU->ua.fInpScale = 1.0;
            pU->ua.fInpBias  = b1 - b2; 

            /* Create connections for current unit pU
             */
            Nn_CreateConns(pU);
            
            /* Connect a part of the input layer of 1st net
             * positive scale
             */
            pC = Nn_GetConnAt(pU, 0);
            pC->ca.fWeight = +a1; 
            pC->ca.iLayer = iLI1;
            pC->ca.iUnit  = iU1;

            /* Connect to output layer of 2nd net and weight with 
             * negative scale
             */
            pC = Nn_GetConnAt(pU, 1);
            pC->ca.fWeight = -a2; 
            pC->ca.iLayer = iLO2;
            pC->ca.iUnit  = iU2;
        }
    }

    /* Create a new layer with a single unit which summates the quadric output
     * of the previous layer to compute a single flag with a threshold 
     * activation function.
     */
    {
        NN_PLAYER pL;
        NN_PUNIT  pU;
        NN_PCONN  pC;
        short iC, nC;

        /* Consider the second layer pL following the output layer of the 2nd net
         */
        pL = Nn_GetLayerAt(pNet, iLO2 + 2);
        pL->la.nNumUnits = 1;
        pL->la.nActFnId  = NN_FUNC_THRESHOLD;
        pL->la.fActThres = threshold;
        Nn_CreateUnits(pL);

        /* Consider next unit pU of the first layer following the output layer of the 2nd net
         */
        pU = Nn_GetUnitAt(pL, 0);
        pU->ua.nNumConns = Nn_GetLayerAt(pNet, iLO2 + 1)->la.nNumUnits;
        /* Create connections for current unit pU
         */
        Nn_CreateConns(pU);

        nC = pU->ua.nNumConns;
        for (iC = 0; iC < nC; iC++)
        {
            /* Connect with each unit of previous layer
             */
            pC = Nn_GetConnAt(pU, iC);
            pC->ca.fWeight = 1.0; 
            pC->ca.iLayer = iLO2 + 1; 
            pC->ca.iUnit  = iC;
        }
    }

    /* Create the output layer of the new net which takes the output of the
     * 1st net and the flag computed in the previous layer.
     */
    {
        NN_PLAYER pL;
        NN_PUNIT  pU;
        NN_PCONN  pC;
        short iU, nU;

        pL= Nn_GetLayerAt(pNet, iLO2 + 3);
        pL->la.nNumUnits = nUO1 + 1;
        pL->la.nActFnId  = NN_FUNC_IDENTITY;
        Nn_CreateUnits(pL);

        nU = pL->la.nNumUnits;
        for (iU = 0; iU < nU; iU++)
        {
            pU = Nn_GetUnitAt(pL, iU);
            pU->ua.nNumConns = 1;
            /* Create connections for current unit pU*/
            Nn_CreateConns(pU);
            
            /* Connect to the units of output layer of 1st net*/
            pC = Nn_GetConnAt(pU, 0);
            pC->ca.fWeight = 1.0; 
            pC->ca.iLayer = (iU < nU-1) ? iLO1 : iLO2 + 2;
            pC->ca.iUnit  = (iU < nU-1) ? iU : 0;
        }
    }



    {
        NN_STATUS nns = Nn_AssertSemanticIntegrity(pNet, nUI1, nUO1 + 1);
	    if (nns != NN_OK) {
		    fprintf(stderr, "NNF-Error: %s (NN_STATUS=%d)\n", Nn_GetErrMsg(), Nn_GetErrNo());
		    exit(-1);
	    }
    }

    return pNet;
}



NN_PNET createNnfNet(int nNumLayers, const int* pnNumUnits)
{
	NN_PNET   pNet;
	NN_PLAYER pLayer;
	NN_PUNIT  pUnit;
	NN_PCONN  pConn;
	NN_STATUS nns;
	short     iL, iU, iC;

	Nn_CreateNet(&pNet);
	pNet->na.nNumLayers = (short)nNumLayers;

	Nn_CreateLayers(pNet);

	for (iL = 0; iL < nNumLayers; iL++)
	{
		pLayer = Nn_GetLayerAt(pNet, iL);
		pLayer->la.nNumUnits = (short)pnNumUnits[iL];
		Nn_CreateUnits(pLayer);
		for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
		{
			pUnit = Nn_GetUnitAt(pLayer, iU);
			if (iL == 0)
				continue;

			pUnit->ua.nNumConns = (short)pnNumUnits[iL-1];
			Nn_CreateConns(pUnit);
			for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
			{
				pConn = Nn_GetConnAt(pUnit, iC);
				pConn->ca.iLayer  = (short)(iL-1);
				pConn->ca.iUnit   = iC;
				pConn->ca.fWeight = (2.0 * rand()) / RAND_MAX - 1.0;
			}
		}
	}

	nns = Nn_AssertSemanticIntegrity(pNet, pnNumUnits[0], pnNumUnits[nNumLayers-1]);
	if (nns != NN_OK) {
		fprintf(stderr, "NNF-Error: %s (NN_STATUS=%d)\n", Nn_GetErrMsg(), Nn_GetErrNo());
		exit(-1);
	}

	return pNet;
}


BOOL getNextValue(FILE* istream, double* pdValue, BOOL* pbIsEOL) 
{
	BOOL bValOk = FALSE;
	*pdValue = 0.0;
	*pbIsEOL = FALSE;

	bValOk = fscanf(istream, "%lf", pdValue) == 1;
	
	while (TRUE)
	{
		int ch = getc(istream);
		if (ch == '\n' || ch == EOF)
		{
			*pbIsEOL = TRUE;
			if (ch == EOF)
				break;
		}
		else if (ch != ' ' && ch != '\t')
		{
			ungetc(ch, istream);
			break;
		}
	} 

	return bValOk;
}

void testNnfNet(NN_PNET pNet, 
                const char* pszIFile, 
                const char* pszOFile, 
                int nNumLinesSkip,
                int bLayerDump)
{		
	FILE*   istream = NULL;
	FILE*   ostream = NULL;
	int     nNumInpUnits = 0;
	int     nNumOutUnits = 0;
	int     nNumLines = 0;
	int     nNumRecords = 0;
	double* pdInpT = NULL;
	double* pdOutT = NULL;
	double* pdOutV = NULL;
	int     i = 0;
	int     ch = 0;
	BOOL    bIsEOL = FALSE;
	BOOL    bValOk = FALSE;
	
	istream = openFile(pszIFile, "r");
	ostream = openFile(pszOFile, "w");

	if (nNumLinesSkip > 0) 
	{
		while (nNumLines < nNumLinesSkip) 
		{
			ch = getc(istream);
			if (ch == '\n')
			{
				nNumLines++;
			}
			else if (ch == EOF)
			{
				break;
			}
		}
	}

	nNumInpUnits = Nn_GetInputLayer(pNet)->la.nNumUnits;
	nNumOutUnits = Nn_GetOutputLayer(pNet)->la.nNumUnits;

	pdInpT = (double*) malloc(nNumInpUnits * sizeof (double)); 
	pdOutT = (double*) malloc(nNumOutUnits * sizeof (double)); 
	pdOutV = (double*) malloc(nNumOutUnits * sizeof (double)); 

	for (; !feof(istream); nNumLines++)
	{
		for (i = 0; i < nNumInpUnits; i++) 
        {
			bValOk = getNextValue(istream, &pdInpT[i], &bIsEOL);
			/*printf("\nindex = %d, value = %g, status = %d\n", i, pdInpT[i], bValOk);
			*/
			if (bIsEOL) 
			{
				if (i == 0)
					break;
				fprintf(stderr, "Error: file %s, line %d: missing value for %d. input vector element\n", pszIFile, nNumLines+1, i+1);
				exit(-1);
			}
			if (!bValOk)
            {
				fprintf(stderr, "Error: file %s, line %d: invalid number format for %d. input vector element\n", pszIFile, nNumLines+1, i+1);
				exit(-1);
			}
		}
		if (feof(istream))
			break;

		for (i = 0; i < nNumOutUnits; i++) 
        {
			bValOk = getNextValue(istream, &pdOutT[i], &bIsEOL);
			if (bIsEOL) 
			{
				fprintf(stderr, "Error: file %s, line %d: missing value for %d. output vector element\n", pszIFile, nNumLines+1, i+1);
				exit(-1);
			}
			if (!bValOk)
            {
				fprintf(stderr, "Error: file %s, line %d: invalid number format for %d. output vector element\n", pszIFile, nNumLines+1, i+1);
				exit(-1);
			}
		}

		/* 
		 * skip all characters up to the end of line
		 * NOTE: This is a quick fix for FUB test files which have an extra column
		 * at the end of the test files.
		 */
		do
		{
			ch = getc(istream);
		} 
		while (ch != '\n' && ch != EOF);


		nNumRecords = nNumLines - nNumLinesSkip;


		Nn_ProcessNet(pNet, pdInpT, pdOutV);


		for (i = 0; i < nNumOutUnits; i++)
		{
			double dx = fabs(pdOutV[i] - pdOutT[i]);
			if (dx > ERR_LIMIT) 
			{
			    printf("WARNING: Significant deviation detected for %d. element of output vector:\n"
					   "         Value is %g, but should be %g, deviation is %g\n", 
					   i+1, pdOutV[i], pdOutT[i], dx);
			}
		}


        if (bLayerDump) 
        {
			fprintf(ostream, "\n*** Net dump for test record %d ***\n", nNumRecords+1);
            Nn_PrintLayerOutputs(pNet, ostream, NULL);
        }
        else 
        {
		    for (i = 0; i < nNumInpUnits; i++)
			    fprintf(ostream, " %g", pdInpT[i]);
		    for (i = 0; i < nNumOutUnits; i++)
			    fprintf(ostream, " %g", pdOutT[i]);
		    for (i = 0; i < nNumOutUnits; i++)
			    fprintf(ostream, " %g", pdOutV[i]);
		    fprintf(ostream, "\n");
        }
	}
	
	printf("File %s written, %d records processed\n", pszOFile, nNumRecords);
	
	closeFile(istream);
	closeFile(ostream);

	free(pdInpT);
	free(pdOutT);
	free(pdOutV);
}



void copyNet(NN_PNET sourceNet, NN_PNET targetNet, int layerOffset)
{
    NN_PLAYER pL1;
    NN_PLAYER pL2;
    NN_PUNIT  pU1;
    NN_PUNIT  pU2;
    NN_PCONN  pC1;
    NN_PCONN  pC2;

    short iL, nL;
    short iU, nU;
    short iC, nC;

    nL = sourceNet->na.nNumLayers;
    for (iL = 0; iL < nL; iL++) 
    {
        pL1 = Nn_GetLayerAt(sourceNet, iL);
        pL2 = Nn_GetLayerAt(targetNet, iL + layerOffset);

        memcpy(&pL2->la, &pL1->la, sizeof (NN_LAYER_ATTRIB));
        pL2->la.iLayer += layerOffset;
        Nn_CreateUnits(pL2);

        nU = pL1->la.nNumUnits;
        for (iU = 0; iU < nU; iU++)
        {
            pU1 = Nn_GetUnitAt(pL1, iU);
            pU2 = Nn_GetUnitAt(pL2, iU);

            memcpy(&pU2->ua, &pU1->ua, sizeof (NN_UNIT_ATTRIB));
            pU2->ua.iLayer += layerOffset;
            Nn_CreateConns(pU2);

            nC = pU1->ua.nNumConns;
            for (iC = 0; iC < nC; iC++)
            {
                pC1 = Nn_GetConnAt(pU1, iC);
                pC2 = Nn_GetConnAt(pU2, iC); 

                memcpy(&pC2->ca, &pC1->ca, sizeof (NN_CONN_ATTRIB));
                pC2->ca.iLayer += layerOffset;
            } /* next iC */
        } /* next iU */
    } /* next iL */
}



FILE* openFile(const char* pchFile, const char* pchMode)
{
	FILE* stream;

	if (*pchMode == 'r' && !existsFile(pchFile)) 
    {
		fprintf(stderr, "Error: can not find file '%s'\n", pchFile);
		exit(-1);
	}

    if (*pchMode == 'r') 
    {
        fprintf(stderr, "Reading from file '%s'...\n", pchFile);
    }
    else 
    {
        fprintf(stderr, "Writing to file '%s'...\n", pchFile);
    }

	stream = fopen(pchFile, pchMode);
	if (stream == NULL) 
    {
		fprintf(stderr, "Error: can not open file '%s'\n", pchFile);
		exit(-1);
	}

	return stream;
}


void closeFile(FILE* stream)
{
	if (stream != NULL)
		fclose(stream);
}


BOOL existsFile (const char* pchPath)
{
	FILE* pStream = fopen(pchPath, "rb");
	if (pStream == NULL && errno == ENOENT)
		return FALSE;
	fclose(pStream);
	return TRUE;
}

BOOL overwriteExistingFile  (const char* pchPath)
{
	char pch[80];

	printf("The file '%s' already exists, overwrite? (y/n) ", pchPath);
	scanf(" %1s", pch);
	
	return *pch == 'y' || *pch == 'Y';
}


BOOL isBinaryFile (const char* pchPath)
{
	FILE* pStream = fopen(pchPath, "rb");
	BOOL bIsBinary = FALSE;
	int ch;
	
	if (pStream == NULL)
		return FALSE;

	do
	{
		ch = getc(pStream);
		if (ch >= 0 && ch < 32 && 
			!(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'))
		{
			bIsBinary = TRUE;
			break;
		}
	}
	while (ch != EOF);

	fclose(pStream);

	return bIsBinary;
}



char* readLine(FILE* istream, int* piLine)
{
	static char pchLine[LINE_LEN_MAX+1];
	if (fgets(pchLine, LINE_LEN_MAX, istream) == NULL)
		throwInvalidFileFormatException(*piLine, "unexpected end-of-file");
	(*piLine)++;
	return pchLine;
}


int parseInt(char** ppchLine, int iLine)
{
	char* pchL1 = *ppchLine;
	char* pchL2 = NULL;
	int n = (int) strtol(pchL1, &pchL2, 10);
	if (pchL2 == NULL || pchL2 <= pchL1) 
		throwInvalidFileFormatException(iLine, "'int'  number expected");
	*ppchLine = pchL2;
	return n;
}


double parseDouble(char** ppchLine, int iLine)
{
	char* pchL1 = *ppchLine;
	char* pchL2 = NULL;
	double d = strtod(pchL1, &pchL2);
	if (pchL2 == NULL || pchL2 <= pchL1) 
		throwInvalidFileFormatException(iLine, "'double' number expected");
	*ppchLine = pchL2;
	return d;
}


void replaceFileExt(char* pchFile, const char* pchExt)
{
	char* pchE = strrchr(pchFile, '.');
#ifdef WIN32
	char* pchD = strrchr(pchFile, '\\');
#else
	char* pchD = strrchr(pchFile, '/');
#endif
	if (pchE == NULL)
		strcat(pchFile, pchExt);
	else if (pchD == NULL || pchD < pchE)
		strcpy(pchE, pchExt);
}


BOOL startsWithString(const char* pchString, const char* pchToken)
{
	while (*pchString == *pchToken && *pchString && *pchToken) {
		pchString++;
		pchToken++;
	}
	return *pchToken == '\0';
}


BOOL equalStrings(const char* pch1, const char* pch2)
{
	return strcmp(pch1, pch2) == 0;
}


BOOL isOptionString(const char* pch)
{
	return *pch == '-' || *pch == '/';
}


BOOL isEmptyString(const char* pch)
{
	while (isspace(*pch))
		pch++;
	return *pch == '\0';
}


void throwInvalidFileFormatException(int iLine, const char* pchMessage)
{
	printf("File format error: line %d: %s\n", iLine, pchMessage);
	exit(-1);
}

void throwMissingOptionArgumentException(const char* pchOption)
{
	printf("Missing argument for option %s\n", pchOption);
	exit(-1);
}


void throwInvalidOptionArgumentException(const char* pchOption)
{
	printf("Invalid argument for option %s\n", pchOption);
	exit(-1);
}

/**
 * Prints the proigram name, version and copyrights.
 */
void printProgramInfo()
{
	printf("\n%s, %s\n%s\n\n", 
           NNFT_PROGRAM_NAME, 
           NNFT_VERSION_INFO, 
           NNFT_COPYRIGHT_INFO);
}

/**
 * Prints the nnftool's usage to the console.
 */
void printUsage()
{
	printf(
		"Usage:\n"
		"%s [-nnf] [-o file] [-b] [-m] file\n"
		"  -nnf     Switches to NNF ASCII/binary conversion mode (default mode)\n"
		"  -o file  Specifies a name for the NNF output file\n"
		"  -b       Forces creation of a binary NNF output file\n"
		"  -m       Forces in-memory creation of NNF net (for internal tests)\n"
		"  file     Name of a NNF input file (ASCII or binary)\n"
		"or\n"
		"%s -ffbp [-o file] [-b] [-i] [-<i|o><o|s><i1>[-<i2>] value] file [func]\n"
		"  -ffbp    Switches to FFBP conversion mode\n"
		"  -o file  Specifies a name for the NNF output file\n"
		"  -b       Forces creation of a binary NNF output file\n"
		"  -n       Includes input/output normalizing into the NNF file\n"
		"  -i<o|s>  Offset (o) or factor (s) for linear scaling of input units i1 to i2\n"
		"  -o<o|s>  Offset (o) or factor (s) for linear scaling of output units i1 to i2\n"
		"  file     Name of FFBP input file (ASCII)\n"
		"  func     Name of the C-function to be generated\n"
		"or\n"
		"%s -ffbpx [-o file] [-b] [-i] [-<i|o><o|s><i1>[-<i2>] value] file1 file2 thres [func]\n"
		"  -ffbp    Switches to FFBP conversion mode\n"
		"  -o file  Specifies a name for the NNF output file\n"
		"  -b       Forces creation of a binary NNF output file\n"
		"  -n       Includes input/output normalizing into the NNF file\n"
		"  -i<o|s>  Offset (o) or factor (s) for linear scaling of input units i1 to i2\n"
		"  -o<o|s>  Offset (o) or factor (s) for linear scaling of output units i1 to i2\n"
		"  file1    Name of the inverse FFBP input file (ASCII)\n"
		"  file2    Name of the forward FFBP input file (ASCII)\n"
		"  thres    Threshold for flag creation\n"
		"  func     Name of the C-function to be generated\n"
		"or\n"
		"%s -create [-o file] [-b] int1 int2 int3 ...\n"
		"  -create  Switches to multi-layer feedforward net creation mode\n"
		"  -o file  Specifies a name for the NNF output file\n"
		"  -b       Forces creation of a binary NNF output file\n"
		"  int{i}   Number of units in layer {i}, i=1: input, 1<i<n: hidden, i=n: output\n"
		"or\n"
		"%s -test [-l int] [-o file] [-m] file1 file2\n"
		"  -test    Switches to NNF test mode\n"
		"  -o file  Specifies a name for a pattern output file\n"
		"  -m       Forces in-memory creation of NNF net (for internal tests)\n"
		"  -l int   Specifies the number of lines to skip in input pattern file\n"
		"  file1    Name of a NNF input file (ASCII or binary)\n"
		"  file2    Name of a pattern input file\n"
		"\n",
		NNFT_PROGRAM_NAME,
		NNFT_PROGRAM_NAME,
		NNFT_PROGRAM_NAME,
		NNFT_PROGRAM_NAME,
		NNFT_PROGRAM_NAME
	);
}


/**
 * Makes a valid function name out of the given <code>name</code>
 * by replacing all invalid characters with the character '_'.
 */
void makeValidFunctionName(char* name) 
{ 
 	char* pch = name;
	if (!isalpha(*pch))
		*pch = '_';
	for (; *pch; pch++) {
		if (!isalnum(*pch))
			*pch = '_';
	}
}


