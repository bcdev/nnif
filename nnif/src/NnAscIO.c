/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnAscIO.c                                                     */
/* Purpose:     Implementation of the neural net ASCII I/O routines           */
/* Remarks:     Interface defined in NnAscIO.h                                */
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

#include "NnBase.h"
#include "NnCheck.h"
#include "NnAscIO.h"

/*////////////////////////////////////////////////////////////////////////////*/
static const NN_KWENT aKwEntSect[] = 
{
	{ NN_SECT_NET,   NN_NAME_NET   },
	{ NN_SECT_LAYER, NN_NAME_LAYER },
	{ NN_SECT_UNIT,  NN_NAME_UNIT  }
};

/*////////////////////////////////////////////////////////////////////////////*/
static const NN_KWENT aKwEntKey[] = 
{
	{ NN_KEY_CONNECTION,  NN_NAME_CONNECTION  },
	{ NN_KEY_NUM_CONNS,   NN_NAME_NUM_CONNS   },
	{ NN_KEY_INP_BIAS,    NN_NAME_INP_BIAS    },
	{ NN_KEY_INP_SCALE,   NN_NAME_INP_SCALE   },
	{ NN_KEY_OUT_BIAS,    NN_NAME_OUT_BIAS    },
	{ NN_KEY_OUT_SCALE,   NN_NAME_OUT_SCALE   },
	{ NN_KEY_MATRIX,      NN_NAME_MATRIX      },
	{ NN_KEY_NUM_UNITS,   NN_NAME_NUM_UNITS   },
	{ NN_KEY_INP_FNID,    NN_NAME_INP_FNID    },
	{ NN_KEY_ACT_FNID,    NN_NAME_ACT_FNID    },
	{ NN_KEY_OUT_FNID,    NN_NAME_OUT_FNID    },
	{ NN_KEY_ACT_SLOPE,   NN_NAME_ACT_SLOPE   },
	{ NN_KEY_ACT_THRES,   NN_NAME_ACT_THRES   },
	{ NN_KEY_NUM_LAYERS,  NN_NAME_NUM_LAYERS  },
	{ NN_KEY_MIN_VERSION, NN_NAME_MIN_VERSION },
	{ NN_KEY_MAJ_VERSION, NN_NAME_MAJ_VERSION },
	{ NN_KEY_INP_LAYER,   NN_NAME_INP_LAYER   },
	{ NN_KEY_OUT_LAYER,   NN_NAME_OUT_LAYER   },
	{ NN_KEY_PRECISION,   NN_NAME_PRECISION   }
};

/*////////////////////////////////////////////////////////////////////////////*/
static const NN_KWENT aKwEntPrec[] = 
{
	{ NN_PREC_SINGLE, NN_NAME_SINGLE },
	{ NN_PREC_DOUBLE, NN_NAME_DOUBLE }
};

/*////////////////////////////////////////////////////////////////////////////*/
static const NN_KWENT aKwEntInpFn[] = 
{
	{ NN_FUNC_ZERO,  NN_NAME_ZERO  },
	{ NN_FUNC_SUM_1, NN_NAME_SUM_1 },
	{ NN_FUNC_SUM_2, NN_NAME_SUM_2 }
};

/*////////////////////////////////////////////////////////////////////////////*/
static const NN_KWENT aKwEntActFn[] = 
{
	{ NN_FUNC_IDENTITY,   NN_NAME_IDENTITY   },
	{ NN_FUNC_THRESHOLD,  NN_NAME_THRESHOLD  },
	{ NN_FUNC_LINEAR,     NN_NAME_LINEAR     },
	{ NN_FUNC_SEMILINEAR, NN_NAME_SEMILINEAR },
	{ NN_FUNC_SIGMOID_1,  NN_NAME_SIGMOID_1  },
	{ NN_FUNC_SIGMOID_2,  NN_NAME_SIGMOID_2  },
	{ NN_FUNC_RBF_1,      NN_NAME_RBF_1      },
	{ NN_FUNC_RBF_2,      NN_NAME_RBF_2      }
}; 

/*////////////////////////////////////////////////////////////////////////////*/
static const NN_KWENT aKwEntOutFn[] = 
{
	{ NN_FUNC_IDENTITY,    NN_NAME_IDENTITY    },
	{ NN_FUNC_LINEAR,      NN_NAME_LINEAR      },
	{ NN_FUNC_QUADRATIC,   NN_NAME_QUADRATIC   },
	{ NN_FUNC_EXPONENTIAL, NN_NAME_EXPONENTIAL },
	{ NN_FUNC_LOGARITHMIC, NN_NAME_LOGARITHMIC }
}; 

/*////////////////////////////////////////////////////////////////////////////*/
/*static const size_t nEntSize = sizeof (NN_KWENT);                           */
#define nEntSize  (sizeof (NN_KWENT))

static const NN_KWTAB g_tabSect  = { sizeof aKwEntSect  / nEntSize, aKwEntSect };
static const NN_KWTAB g_tabKey   = { sizeof aKwEntKey   / nEntSize, aKwEntKey };
static const NN_KWTAB g_tabInpFn = { sizeof aKwEntInpFn / nEntSize, aKwEntInpFn };
static const NN_KWTAB g_tabActFn = { sizeof aKwEntActFn / nEntSize, aKwEntActFn };
static const NN_KWTAB g_tabOutFn = { sizeof aKwEntOutFn / nEntSize, aKwEntOutFn };
static const NN_KWTAB g_tabPrec  = { sizeof aKwEntPrec  / nEntSize, aKwEntPrec };

/*////////////////////////////////////////////////////////////////////////////*/
static char  g_pchFilePath [NN_MAX_PATH+1];
static FILE* g_stream;

/*////////////////////////////////////////////////////////////////////////////*/
/*                                                                            */
/*  Nn_ReadAscFile implementation                                             */
/*                                                                            */
/*////////////////////////////////////////////////////////////////////////////*/

/*////////////////////////////////////////////////////////////////////////////*/
NN_STATUS Nn_CreateNetFromAscFile
(
	PCSTR    pchFilePath, 
	int      nNumInpUnits,
	int      nNumOutUnits,
	NN_PNET* ppNet
)
{
	NN_STATUS nns;

	assert(pchFilePath != NULL && *pchFilePath != '\0');
	assert(ppNet != NULL);

	*ppNet = NULL;

	Nn_ClearError();

	nns = Nn_OpenAscFileScanner(pchFilePath);
	if (nns == NN_OK)
	{
		if (Nn_ParseNet(ppNet))
			nns = NN_OK;
		else
			nns = Nn_Error(NN_FILE_READ_ERROR, 
				NN_ERR_PREFIX "%d errors in file '%s'", 
				Nn_GetNumErrors(), 
				pchFilePath);

		Nn_CloseAscFileScanner();
	}

	if (nns == NN_OK)
		nns = Nn_AssertSemanticIntegrity(*ppNet, nNumInpUnits, nNumOutUnits);

	return nns;
}

/*////////////////////////////////////////////////////////////////////////////*/
static int   g_nSection;
static int   g_nKey;
static short g_iL;
static short g_iU;

BOOL Nn_ParseNet (NN_PNET* ppNet)
{
	*ppNet     = NULL;
	g_nSection = -1;
	g_nKey     = -1;
	g_iL       = -1;
	g_iU       = -1;
	
	if (Nn_CreateNet(ppNet) != NN_OK)
		return FALSE;
	
	while (Nn_ReadLine())
	{
		if (Nn_ParseTokenOpt(NN_TOK_EOL))
			continue;

		if (Nn_ParsePunctuatorOpt('['))
		{
			if (Nn_ParseSectionHeader(*ppNet))
			{
				if (Nn_ParsePunctuator(']'))
					Nn_ParseToken(NN_TOK_EOL);
			}
		}
		else
		{
			Nn_ParseSectionEntry(*ppNet);
		}
	}
	
	if (Nn_GetNumErrors() > 0)
	{
		Nn_DeleteNet(*ppNet);
		*ppNet = NULL;
		return FALSE;
	}

	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseSectionHeader(NN_PNET pNet)
{
	NN_PLAYER pLayer;
	char szName[NN_MAX_TOKEN+1];
	short iL, iU;

	if (!Nn_ParseToken(NN_TOK_NAME))
		return FALSE;

	Nn_GetToken(szName);
	g_nSection = Nn_FindKeywordIdent(&g_tabSect, szName);
	if (g_nSection == -1)
	{
		Nn_AscReadError("unknown section [%s]", szName);
		return FALSE;
	}

	switch (g_nSection)
	{
	case NN_SECT_NET:
		return TRUE;
		break;

	case NN_SECT_LAYER:
		g_iL = -1;
		if (!Nn_ParsePunctuator('('))
			return FALSE;
		if (!Nn_ParseIndex(-1, &iL))
			return FALSE;
		if (!Nn_ParsePunctuator(')'))
			return FALSE;

		if (!Nn_LayersCreated(pNet))
		{
			if (Nn_CreateLayers(pNet) != NN_OK)
				return FALSE;
		}

		if (!Nn_CheckLayerIndex(pNet, iL))
			return FALSE;
		
		g_iL = iL;
		break;

	case NN_SECT_UNIT:
		if (!Nn_ParsePunctuator('('))
			return FALSE;
		if (!Nn_ParseIndex(-1, &iL))
			return FALSE;
		if (!Nn_ParsePunctuator(','))
			return FALSE;
		if (!Nn_ParseIndex(-1, &iU))
			return FALSE;
		if (!Nn_ParsePunctuator(')'))
			return FALSE;

		if (!Nn_LayersCreated(pNet))
		{
			if (Nn_CreateLayers(pNet) != NN_OK)
				return FALSE;
		}
		if (!Nn_CheckLayerIndex(pNet, iL))
			return FALSE;
		pLayer = Nn_GetLayerAt(pNet, iL);

		if (!Nn_UnitsCreated(pLayer))
		{
			if (Nn_CreateUnits(pLayer) != NN_OK)
				return FALSE;
		}
		if (!Nn_CheckUnitIndex(pLayer, iU))
			return FALSE;

		g_iL = iL;
		g_iU = iU;
		break;
	}

	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseSectionEntry(NN_PNET pNet)
{
	char szName[NN_MAX_TOKEN+1];

	if (!Nn_ParseToken(NN_TOK_NAME))
		return FALSE;

	Nn_GetToken(szName);
	g_nKey = Nn_FindKeywordIdent(&g_tabKey, szName);
	if (g_nKey == -1)
	{
		Nn_AscReadError("unknown key '%s'", szName);
		return FALSE;
	}

	switch (g_nSection)
	{
	case NN_SECT_NET:
		return Nn_ParseNetSectionEntry(pNet);
	case NN_SECT_LAYER:
		if (g_iL >= 0)
			return Nn_ParseLayerSectionEntry(pNet);
		else
			return FALSE;
	case NN_SECT_UNIT:
		if (g_iL >= 0 && g_iU >= 0)
			return Nn_ParseUnitSectionEntry(pNet);
		else
			return FALSE;
	}

	return FALSE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseNetSectionEntry(NN_PNET pNet)
{
	switch (g_nKey)
	{
	case NN_KEY_NUM_LAYERS:
		return Nn_ParseCountAssign(1, 256, &pNet->na.nNumLayers);
	case NN_KEY_MAJ_VERSION:
		return Nn_ParseCountAssign(1, 10, &pNet->na.anVersion[0]);
	case NN_KEY_MIN_VERSION:
		return Nn_ParseCountAssign(0, 10, &pNet->na.anVersion[1]);
	case NN_KEY_INP_LAYER:
		return Nn_ParseIndexAssign(pNet->na.nNumLayers, &pNet->na.iInpLayer);
	case NN_KEY_OUT_LAYER:  
		return Nn_ParseIndexAssign(pNet->na.nNumLayers, &pNet->na.iOutLayer);
	case NN_KEY_PRECISION: 
		return Nn_ParseKeywordAssign(&g_tabPrec, &pNet->na.nPrecision);
	default:
		Nn_AscReadError("key is not allowed here");
	}

	return FALSE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseLayerSectionEntry(NN_PNET pNet)
{
	NN_PLAYER pLayer = Nn_GetLayerAt(pNet, g_iL);

	switch (g_nKey)
	{
	case NN_KEY_NUM_UNITS:
		return Nn_ParseCountAssign(0, 32000, &pLayer->la.nNumUnits);
	case NN_KEY_INP_FNID:
		return Nn_ParseKeywordAssign(&g_tabInpFn, &pLayer->la.nInpFnId);
	case NN_KEY_ACT_FNID:
		return Nn_ParseKeywordAssign(&g_tabActFn, &pLayer->la.nActFnId);
	case NN_KEY_OUT_FNID:
		return Nn_ParseKeywordAssign(&g_tabOutFn, &pLayer->la.nOutFnId);
	case NN_KEY_ACT_SLOPE:
		return Nn_ParseFloatAssign(&pLayer->la.fActSlope);
	case NN_KEY_ACT_THRES:
		return Nn_ParseFloatAssign(&pLayer->la.fActThres);
	default:
		Nn_AscReadError("key is not allowed here");
	}

	return FALSE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseUnitSectionEntry(NN_PNET pNet)
{
	NN_PLAYER pLayer = Nn_GetLayerAt(pNet, g_iL);
	NN_PUNIT  pUnit  = Nn_GetUnitAt(pLayer, g_iU);

	switch (g_nKey)
	{
	case NN_KEY_NUM_CONNS:
		return Nn_ParseCountAssign(0, 32000, &pUnit->ua.nNumConns);
	case NN_KEY_INP_BIAS:
		return Nn_ParseFloatAssign(&pUnit->ua.fInpBias);
	case NN_KEY_INP_SCALE:
		return Nn_ParseFloatAssign(&pUnit->ua.fInpScale);
	case NN_KEY_OUT_BIAS:
		return Nn_ParseFloatAssign(&pUnit->ua.fOutBias);
	case NN_KEY_OUT_SCALE:
		return Nn_ParseFloatAssign(&pUnit->ua.fOutScale);
	case NN_KEY_CONNECTION:
		return Nn_ParseConnEntryAssign(pNet, pUnit);
	case NN_KEY_MATRIX:
		return Nn_ParseMatrixEntryAssign(pNet, pUnit);
	default:
		Nn_AscReadError("key is not allowed here");
	}

	return FALSE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseConnEntryAssign(const NN_PNET pNet, NN_PUNIT pUnit)
{
	short iC;
	NN_CONN c;

	if (!Nn_ConnsCreated(pUnit))
	{
		if (Nn_CreateConns(pUnit) != NN_OK)
			return FALSE;
	}

	if (!Nn_ParsePunctuator('('))
		return FALSE;
	if (!Nn_ParseIndex(-1, &iC))
		return FALSE;
	if (!Nn_ParsePunctuator(')'))
		return FALSE;
	if (!Nn_ParsePunctuator('='))
		return FALSE;
	if (!Nn_ParseIndex(-1, &c.ca.iLayer))
		return FALSE;
	if (!Nn_ParsePunctuator(','))
		return FALSE;
	if (!Nn_ParseIndex(-1, &c.ca.iUnit))
		return FALSE;
	if (!Nn_ParsePunctuator(','))
		return FALSE;
	if (!Nn_ParseFloat(&c.ca.fWeight))
		return FALSE;
	
	if (!Nn_CheckConnIndex(pUnit, iC))
		return FALSE;
	if (!Nn_CheckLayerIndex(pNet, c.ca.iLayer))
		return FALSE;
	if (!Nn_CheckUnitIndex(Nn_GetLayerAt(pNet, c.ca.iLayer), c.ca.iUnit))
		return FALSE;

	c.pUnit = Nn_GetUnitAt(Nn_GetLayerAt(pNet, c.ca.iLayer), c.ca.iUnit);
	Nn_SetConnAt(pUnit, iC, &c);
	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseMatrixEntryAssign(const NN_PNET pNet, NN_PUNIT pUnit)
{
	short iC1, iC2;
	NN_FLOAT fM;

	if (!Nn_MatrixCreated(pUnit))
	{
		if (Nn_CreateMatrix(pUnit) != NN_OK)
			return FALSE;
	}

	if (!Nn_ParsePunctuator('('))
		return FALSE;
	if (!Nn_ParseIndex(pUnit->ua.nNumConns, &iC1))
		return FALSE;
	if (!Nn_ParsePunctuator(','))
		return FALSE;
	if (!Nn_ParseIndex(pUnit->ua.nNumConns, &iC2))
		return FALSE;
	if (!Nn_ParsePunctuator(')'))
		return FALSE;
	if (!Nn_ParsePunctuator('='))
		return FALSE;
	if (!Nn_ParseFloat(&fM))
		return FALSE;

	if (!Nn_CheckConnIndex(pUnit, iC1))
		return FALSE;
	if (!Nn_CheckConnIndex(pUnit, iC2))
		return FALSE;

	Nn_SetMatrixElemAt(pUnit, iC1, iC2, fM);
	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseKeywordAssign(const NN_KWTAB* pTab, short* pnKwId)
{
	int nKwId;

	if (!Nn_ParsePunctuator('='))
		return FALSE;

	if (Nn_ParseTokenOpt(NN_TOK_NAME))
	{
		char szName[NN_MAX_LINE+1];
		Nn_GetToken(szName);

		nKwId = Nn_FindKeywordIdent(pTab, szName);
		if (nKwId == -1)
		{
			Nn_AscReadError("invalid keyword '%s'", szName);
			return FALSE;
		}
	}
	else if (Nn_ParseToken(NN_TOK_INT))
	{
		nKwId = Nn_GetTokenValInt();
		if (Nn_FindKeywordName(pTab, nKwId) == NULL)
		{
			Nn_AscReadError("invalid identifier (ID=%d)", nKwId);
			return FALSE;
		}
	}
	else
		return FALSE;

	*pnKwId = (short) nKwId;
	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseFloat(NN_FLOAT* pf)
{
	if (!Nn_ParseTokenOpt(NN_TOK_INT))
	{
		if (!Nn_ParseToken(NN_TOK_FLOAT))
			return FALSE;
		*pf = (NN_FLOAT) Nn_GetTokenValFloat();
	}
	else
		*pf = (NN_FLOAT) Nn_GetTokenValInt();
	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseFloatAssign(NN_FLOAT* pf)
{
	if (!Nn_ParsePunctuator('='))
		return FALSE;
	return Nn_ParseFloat(pf);
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseShort(short* ps)
{
	if (!Nn_ParseToken(NN_TOK_INT))
		return FALSE;
	*ps = (short) Nn_GetTokenValInt();
	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseShortAssign(short* ps)
{
	if (!Nn_ParsePunctuator('='))
		return FALSE;
	return Nn_ParseShort(ps);
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseCount(short sMin, short sMax, short* ps)
{
	if (!Nn_ParseShort(ps))
		return FALSE;
	if (sMin >= 0 && *ps < sMin)
	{
		Nn_AscReadError("integer must be less than %d", sMin);
		return FALSE;
	}
	else if (sMax >= 0 && *ps > sMax)
	{
		Nn_AscReadError("integer must be greater than %d", sMax);
		return FALSE;
	}
	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseCountAssign(short sMin, short sMax, short* ps)
{
	if (!Nn_ParsePunctuator('='))
		return FALSE;
	return Nn_ParseCount(sMin, sMax, ps);
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseIndex(short sMax, short* ps)
{
	if (!Nn_ParseCount(1, sMax, ps))
		return FALSE;
	(*ps)--;
	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseIndexAssign(short sMax, short* ps)
{
	if (!Nn_ParsePunctuator('='))
		return FALSE;
	return Nn_ParseIndex(sMax, ps);
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_CheckLayerIndex(const NN_PNET pNet, int iL)
{	
	assert(pNet != NULL);

	if (pNet->na.nNumLayers <= 0 || pNet->aLayers == NULL)
	{
		Nn_AscReadError("Layer(%d): missing layer definition", iL+1);
		return FALSE;
	}
	
	if (iL < 0 || iL >= pNet->na.nNumLayers)
	{
		Nn_AscReadError("Layer(%d): layer index out of range", iL+1);
		return FALSE;
	}

	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_CheckUnitIndex(const NN_PLAYER pLayer, int iU)
{	
	assert(pLayer != NULL);

	if (pLayer->la.nNumUnits <= 0 || pLayer->aUnits == NULL)
	{
		Nn_AscReadError("Unit(%d): missing unit definition", iU+1);
		return FALSE;
	}
	
	if (iU < 0 || iU >= pLayer->la.nNumUnits)
	{
		Nn_AscReadError("Unit(%d): unit index out of range", iU+1);
		return FALSE;
	}

	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_CheckConnIndex(const NN_PUNIT pUnit, int iC)
{	
	assert(pUnit != NULL);

	if (pUnit->ua.nNumConns <= 0 || pUnit->aConns == NULL)
	{
		Nn_AscReadError("C(%d): missing connection definition", iC+1);
		return FALSE;
	}
	
	if (iC < 0 || iC >= pUnit->ua.nNumConns)
	{
		Nn_AscReadError("C(%d): connection index out of range", iC+1);
		return FALSE;
	}

	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
int Nn_FindKeywordIdent(const NN_KWTAB* pTab, PCSTR pchName)
{
	int i;
	for (i = 0; i < pTab->nSize; i++)
	{
		if (Nn_CompareKw(pTab->pEntries[i].pchName, pchName) == 0)
			return pTab->pEntries[i].nId;
	}
	return -1;
}

/*////////////////////////////////////////////////////////////////////////////*/
PCSTR Nn_FindKeywordName(const NN_KWTAB* pTab, int nKwId)
{
	int i;
	for (i = 0; i < pTab->nSize; i++)
	{
		if (pTab->pEntries[i].nId == nKwId)
			return pTab->pEntries[i].pchName;
	}
	return NULL;
}

/*////////////////////////////////////////////////////////////////////////////*/
PCSTR Nn_GetPrintKeyword(const NN_KWTAB* pTab, int nKwId)
{
	static char pchName[256];
	int i;
	for (i = 0; i < pTab->nSize; i++)
	{
		if (pTab->pEntries[i].nId == nKwId) 
		{
			sprintf(pchName, "%s ; ID = %d", pTab->pEntries[i].pchName, nKwId);
			return pchName;
		}
	}
	sprintf(pchName, "%d ; no keyword found for ID=%d!", nKwId, nKwId);
	return pchName;
}

/*////////////////////////////////////////////////////////////////////////////*/
int Nn_CompareKw(PCSTR pstr1, PCSTR pstr2)
{
	for (;pstr1[0] && pstr2[0]; pstr1++, pstr2++)
	{
		if (toupper(pstr1[0]) != toupper(pstr2[0]))
			break;
	}
	return pstr1[0] - pstr2[0];
}

/*////////////////////////////////////////////////////////////////////////////*/
static char      g_pchLine [NN_MAX_LINE+1];
static PCSTR     g_pchCur;
static PCSTR     g_pchToken;
static int       g_nTokenLen;
static NN_TOKEN  g_nTokenId;
static BOOL      g_bTokenConsumed;
static double    g_dTokenVal;
static long      g_lTokenVal;
static int       g_nLineNo;
static int       g_nNumErrors;

#define Nn_ConsumeToken() (g_bTokenConsumed = TRUE)
#define Nn_GetTokenId()   (g_nTokenId)
#define Nn_PeekChar()     (*g_pchCur)
#define Nn_ConsumeChar()  (++g_pchCur)
#define Nn_PeekBinary()   (*g_pchCur >= 0 && *g_pchCur < 32 || *g_pchCur == 127)

long   Nn_GetTokenValInt ()   { return g_lTokenVal; }
double Nn_GetTokenValFloat () { return g_dTokenVal; }

/*////////////////////////////////////////////////////////////////////////////*/
NN_STATUS Nn_OpenAscFileScanner (PCSTR pchFilePath)
{
	strncpy(g_pchFilePath, pchFilePath, NN_MAX_PATH);
	g_pchFilePath[NN_MAX_PATH] = '\0';

	g_stream = fopen(pchFilePath, "r");
	if (g_stream == NULL)
		return Nn_Error(NN_CANT_OPEN_FILE, NN_ERR_PREFIX "can't open file '%s'", pchFilePath);

	g_pchLine[0]     = '\0';
	g_pchCur         = g_pchLine;
	g_pchToken       = g_pchLine;
	g_nTokenLen      = 0;
	g_nTokenId       = NN_TOK_EOL;
	g_bTokenConsumed = TRUE;
	g_dTokenVal      = 0.0;
	g_lTokenVal      = 0;
	g_nLineNo        = 0;
	g_nNumErrors     = 0;

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/
void Nn_CloseAscFileScanner ()
{
	fclose(g_stream);
	g_stream = NULL;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParsePunctuatorOpt (int ch)
{
	if (Nn_ScanToken() == NN_TOK_PUNCT && g_pchToken[0] == ch)
	{
		Nn_ConsumeToken();
		return TRUE;
	}
	else
		return FALSE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParsePunctuator (int ch)
{
	if (Nn_ScanToken() == NN_TOK_PUNCT && g_pchToken[0] == ch)
	{
		Nn_ConsumeToken();
		return TRUE;
	}
	else
	{
		if (Nn_GetTokenId() == NN_TOK_EOL)
		{
			Nn_AscReadError("'%c' expected, but found %s", 
				ch, 
				Nn_GetTokenName(Nn_GetTokenId()));
		}
		else
		{
			char szToken[NN_MAX_TOKEN+1];
			Nn_GetToken(szToken);
			Nn_AscReadError("'%c' expected, but found %s '%s'", 
				ch, 
				Nn_GetTokenName(Nn_GetTokenId()), 
				szToken);
		}
		Nn_ConsumeToken();
		return FALSE;
	}
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseTokenOpt (NN_TOKEN nTokenId)
{
	if (Nn_ScanToken() == nTokenId)
	{
		Nn_ConsumeToken();
		return TRUE;
	}
	else
		return FALSE;
}

/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ParseToken (NN_TOKEN nTokenId)
{
	if (Nn_ScanToken() == nTokenId)
	{
		Nn_ConsumeToken();
		return TRUE;
	}
	else
	{
		if (Nn_GetTokenId() == NN_TOK_EOL)
		{
			Nn_AscReadError("%s expected, but found %s", 
				Nn_GetTokenName(nTokenId), 
				Nn_GetTokenName(Nn_GetTokenId()));
		}
		else
		{
			char szToken[NN_MAX_TOKEN+1];
			Nn_GetToken(szToken);
			Nn_AscReadError("%s expected, but found %s '%s'", 
				Nn_GetTokenName(nTokenId), 
				Nn_GetTokenName(Nn_GetTokenId()), 
				szToken);
		}
		Nn_ConsumeToken();
		return FALSE;
	}
}


/*////////////////////////////////////////////////////////////////////////////*/
NN_TOKEN Nn_ScanToken()
{
	if (!g_bTokenConsumed)
		return g_nTokenId;

	/* Go to the next token... */
	Nn_ReadChar();
	g_pchToken  = g_pchCur;
	/* Initialize the new token string length to zero: */
	g_nTokenLen = 0;
	/* The next token is not yet consumed */
	g_bTokenConsumed = FALSE;
	/* Set the token type to undefined */
	g_nTokenId  = NN_TOK_EOL;

	/* Alphabetic or underscore character gives notice of */
	/* the beginning of a name:                          */
	/*                                                   */
	if (isalpha(Nn_PeekChar()) || Nn_PeekChar() == '_')
	{
		g_nTokenId = NN_TOK_NAME;

		do 
		{
			Nn_ConsumeChar();
		} 
		while (isalnum(Nn_PeekChar()) || Nn_PeekChar() == '_');
	}

	/* A digit indicates the beginning of a either an integer */
	/* or NN_FLOATing point constant                         */
	/*                                                       */
	else if (
		isdigit(Nn_PeekChar()) || 
		Nn_PeekChar() == '.' || 
		Nn_PeekChar() == '+' || 
		Nn_PeekChar() == '-')
	{
		PSTR pchD, pchL;

		g_dTokenVal = strtod(g_pchCur, &pchD);
		g_lTokenVal = strtol(g_pchCur, &pchL, 0);

		if (pchD > g_pchCur && pchD > pchL)
		{
			g_nTokenId = NN_TOK_FLOAT;
			g_pchCur   = pchD;
		}
		else if (pchL > g_pchCur && pchL >= pchD)
		{
			g_nTokenId = NN_TOK_INT;
			g_pchCur   = pchL;
		}
		else
		{
			g_nTokenId = NN_TOK_PUNCT;
			Nn_ConsumeChar();
		}
	}

#if 0
	/* String literal delimitter */
	/*                          */
	else if (Nn_PeekChar() == '\"')
	{
		g_nTokenId = NN_TOK_STRING;
		g_sTokenVal.Empty();

		/* Scan the complete name until the end of file is repched */
		/* or the next string delimitter is found:                 */
		for (;;)
		{
			Nn_ConsumeChar();
			g_lTokenVal = Nn_PeekChar();
			if (g_lTokenVal == '\"')
			{
				Nn_ConsumeChar();
				break;
			}
			else if (g_lTokenVal == '\0')
			{
				Nn_AscReadError("missing string literal delimitter");
				break;
			}

			g_sTokenVal += (TCHAR) g_lTokenVal;
		} 
	}
#endif

	else if (Nn_PeekChar() == '\n')
	{
		g_nTokenId = NN_TOK_EOL;
		Nn_ConsumeChar();
	}

	else if (Nn_PeekBinary())
	{
		g_nTokenId = NN_TOK_EOL;
	}

	else
	{
		g_nTokenId = NN_TOK_PUNCT;
		Nn_ConsumeChar();
	}

	g_nTokenLen = g_pchCur - g_pchToken;
	return g_nTokenId;
}


/*////////////////////////////////////////////////////////////////////////////*/
void Nn_ReadChar()
{
	BOOL bComment = FALSE;
	BOOL bContinue;
	do 
	{
		bContinue = TRUE; 
		switch (Nn_PeekChar())
		{
		case '\r':
		case '\t':
		case ' ' : 
			Nn_ConsumeChar();
			break;
		case ';' : 
			Nn_ConsumeChar();
			bComment  = TRUE; 
			break;
		case '\n': 
		case '\0': 
			bContinue = FALSE; 
			bComment  = FALSE; 
			break;
		default:
			bContinue = bComment;
			if (bComment)
				Nn_ConsumeChar();
		}
	}
	while (bContinue);
}


/*////////////////////////////////////////////////////////////////////////////*/
BOOL Nn_ReadLine () 
{
	g_pchCur = g_pchLine;
	if (fgets(g_pchLine, NN_MAX_LINE, g_stream) != NULL)
		g_nLineNo++;
	else
	{
		g_pchLine[0] = '\0';
		if (ferror(g_stream))
			Nn_Error(NN_FILE_READ_ERROR, NN_ERR_PREFIX "reading from '%s' failed!", g_pchFilePath);
		return FALSE;
	}
	return TRUE;
}

/*////////////////////////////////////////////////////////////////////////////*/
PCSTR Nn_GetToken(char szToken[NN_MAX_TOKEN+1])
{
	if (g_pchToken == NULL || g_nTokenLen == 0)
		szToken[0] = '\0';
	else
	{
		strncpy(szToken, g_pchToken, g_nTokenLen);
		szToken[g_nTokenLen] = '\0';
	}
	return szToken;
}

/*////////////////////////////////////////////////////////////////////////////*/
PCSTR Nn_GetTokenName (NN_TOKEN nTokenId) 
{
	if (nTokenId == NN_TOK_EOL)
		return "end of line";
	else if (nTokenId == NN_TOK_STRING)
		return "string literal";
	else if (nTokenId == NN_TOK_FLOAT)
		return "floating point constant";
	else if (nTokenId == NN_TOK_INT)
		return "integer constant";
	else if (nTokenId == NN_TOK_NAME)
		return "identifier";
	else if (nTokenId == NN_TOK_PUNCT)
		return "punctuator";

	return "token";
}

/*////////////////////////////////////////////////////////////////////////////*/
NN_STATUS Nn_AscReadError (PCSTR pchFormat, ...)
{
	static char szBuffer[1024];

	va_list pArgList;
	va_start(pArgList, pchFormat);
	vsprintf(szBuffer, pchFormat, pArgList);
	va_end(pArgList);

	g_nNumErrors++;
	return Nn_Error(NN_FILE_READ_ERROR, NN_ERR_PREFIX "%s(%d): %s", g_pchFilePath, g_nLineNo, szBuffer);
}


/*////////////////////////////////////////////////////////////////////////////*/
/*                                                                            */
/*  Nn_WriteAscFile implementation                                            */
/*                                                                            */
/*////////////////////////////////////////////////////////////////////////////*/


NN_STATUS Nn_WriteNetToAscFile(PCSTR pchFilePath, const NN_PNET pNet)
{
	NN_STATUS nns;

	assert(pchFilePath != NULL);
	assert(pNet != NULL);

	Nn_ClearError();

	strncpy(g_pchFilePath, pchFilePath, NN_MAX_PATH);
	g_pchFilePath[NN_MAX_PATH] = '\0';

	g_stream = fopen(pchFilePath, "w");
	if (g_stream != NULL)
	{
		Nn_WriteAscNet(pNet);
		nns = Nn_GetErrNo();

		fclose(g_stream);
		g_stream = NULL;
	}
	else
		nns = Nn_Error(NN_CANT_OPEN_FILE, 
			NN_ERR_PREFIX "can't open file '%s' for write", 
			pchFilePath);

	return nns;
}

/*////////////////////////////////////////////////////////////////////////////*/
NN_STATUS Nn_WriteAscNet(const NN_PNET pNet)
{
	short iL, iU, iC, iC1, iC2;
	NN_PLAYER pLayer;
	NN_PUNIT  pUnit;
	NN_PCONN  pConn;

	fprintf(g_stream, "; Definition of the neural net\n");
	fprintf(g_stream, "; \n");
	fprintf(g_stream, "[ %s ]\n", NN_NAME_NET);
	fprintf(g_stream, "%s = %d\n", NN_NAME_MAJ_VERSION, pNet->na.anVersion[0]);
	fprintf(g_stream, "%s = %d\n", NN_NAME_MIN_VERSION, pNet->na.anVersion[1]);
	fprintf(g_stream, "%s = %d\n", NN_NAME_NUM_LAYERS,  pNet->na.nNumLayers);  
	fprintf(g_stream, "%s = %d\n", NN_NAME_INP_LAYER,   pNet->na.iInpLayer+1); 
	fprintf(g_stream, "%s = %d\n", NN_NAME_OUT_LAYER,   pNet->na.iOutLayer+1); 
	fprintf(g_stream, "%s = %s\n", NN_NAME_PRECISION,   Nn_GetPrintKeyword(&g_tabPrec, pNet->na.nPrecision));  

	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		pLayer = Nn_GetLayerAt(pNet, iL);
		
		fprintf(g_stream, "  \n");
		fprintf(g_stream, "; Definition of layer %d\n", iL+1);
		fprintf(g_stream, "; \n");
		fprintf(g_stream, "[ %s(%d) ]\n", NN_NAME_LAYER, iL+1);
		fprintf(g_stream, "%s = %d\n", NN_NAME_NUM_UNITS, pLayer->la.nNumUnits);
		fprintf(g_stream, "%s = %s\n", NN_NAME_INP_FNID,  Nn_GetPrintKeyword(&g_tabInpFn, pLayer->la.nInpFnId)); 
		fprintf(g_stream, "%s = %s\n", NN_NAME_ACT_FNID,  Nn_GetPrintKeyword(&g_tabActFn, pLayer->la.nActFnId)); 
		fprintf(g_stream, "%s = %s\n", NN_NAME_OUT_FNID,  Nn_GetPrintKeyword(&g_tabOutFn, pLayer->la.nOutFnId)); 
		fprintf(g_stream, "%s = %.10g\n", NN_NAME_ACT_SLOPE, pLayer->la.fActSlope);
		fprintf(g_stream, "%s = %.10g\n", NN_NAME_ACT_THRES, pLayer->la.fActThres);
		if (ferror(g_stream))
			return Nn_AscWriteError();
	}

	for (iL = 0; iL < pNet->na.nNumLayers; iL++)
	{
		pLayer = Nn_GetLayerAt(pNet, iL);

		for (iU = 0; iU < pLayer->la.nNumUnits; iU++)
		{
			pUnit = Nn_GetUnitAt(pLayer, iU);

			fprintf(g_stream, "  \n");
			fprintf(g_stream, "; Definition of unit %d of layer %d\n", iU+1, iL+1);
			fprintf(g_stream, "; \n");
			fprintf(g_stream, "[ %s(%d,%d) ]\n", NN_NAME_UNIT, iL+1, iU+1);
			fprintf(g_stream, "%s = %d\n", NN_NAME_NUM_CONNS,  pUnit->ua.nNumConns);
			fprintf(g_stream, "%s = %.10g\n", NN_NAME_INP_BIAS,   pUnit->ua.fInpBias);
			fprintf(g_stream, "%s = %.10g\n", NN_NAME_INP_SCALE,  pUnit->ua.fInpScale);
			fprintf(g_stream, "%s = %.10g\n", NN_NAME_OUT_BIAS,   pUnit->ua.fOutBias);
			fprintf(g_stream, "%s = %.10g\n", NN_NAME_OUT_SCALE,  pUnit->ua.fOutScale);
			if (ferror(g_stream))
				return Nn_AscWriteError();

			if (pUnit->ua.nNumConns == 0)
				fprintf(g_stream, "; No incoming connections defined!\n");
			else
			{
				fprintf(g_stream, "; Definition of the incoming connections:\n");
				fprintf(g_stream, "; Form:\n");
				fprintf(g_stream, "; \t%s(iC) = iL, iU, fW\n", NN_NAME_CONNECTION);
				fprintf(g_stream, "; with\n");
				fprintf(g_stream, "; \tiC: Connection index (1...%d)\n", pUnit->ua.nNumConns);
				fprintf(g_stream, "; \tiL: Source layer index\n");
				fprintf(g_stream, "; \tiU: Source unit index\n");
				fprintf(g_stream, "; \tfW: Weight or RBF centre point co-ordinate value\n");
				fprintf(g_stream, "; \n");
				for (iC = 0; iC < pUnit->ua.nNumConns; iC++)
				{
					pConn = Nn_GetConnAt(pUnit, iC);
					fprintf(g_stream, "%s(%d) = %d,%d, %.10g\n", 
						NN_NAME_CONNECTION, 
						iC+1, 
						pConn->ca.iLayer+1, 
						pConn->ca.iUnit+1, 
						pConn->ca.fWeight);
					if (ferror(g_stream))
						return Nn_AscWriteError();
				}

				if (pUnit->ppfMatrix != NULL)
				{
					fprintf(g_stream, "; Definition of the RBF inverse co-variance matrix:\n");
					fprintf(g_stream, "; Entry form:\n");
					fprintf(g_stream, "; \t%s(iC1,iC2) = fM\n", NN_NAME_MATRIX);
					fprintf(g_stream, "; with\n");
					fprintf(g_stream, "; \tiC1: Connection index (1...%d)\n", pUnit->ua.nNumConns);
					fprintf(g_stream, "; \tiC2: Connection index (1...%d)\n", pUnit->ua.nNumConns);
					fprintf(g_stream, "; \tfM:  Matrix entry value\n");
					fprintf(g_stream, "; \n");
					for (iC1 = 0; iC1 < pUnit->ua.nNumConns; iC1++)
					{
						for (iC2 = 0; iC2 < pUnit->ua.nNumConns; iC2++)
						{
							fprintf(g_stream, "%s(%d,%d) = %.10g\n", 
								NN_NAME_MATRIX, 
								iC1+1, 
								iC2+1, 
								pUnit->ppfMatrix[iC1][iC2]);
							if (ferror(g_stream))
								return Nn_AscWriteError();
						}
					}
				}
			}
		}
	}

	return NN_OK;
}

/*////////////////////////////////////////////////////////////////////////////*/

NN_STATUS Nn_AscWriteError ()
{
	return Nn_Error(NN_FILE_WRITE_ERROR, NN_ERR_PREFIX "can't write to ASCII file '%s'", g_pchFilePath);
}

/*////////////////////////////////////////////////////////////////////////////*/
