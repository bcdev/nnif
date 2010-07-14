/*////////////////////////////////////////////////////////////////////////////*/
/* File:        NnAscIO.h                                                     */
/* Purpose:     Interface def. file for ASCII I/O routines for the NNFF       */
/* Remarks:     Implemented in NnAscIO.c                                      */
/* Author:      Norman Fomferra (SCICON)                                      */
/*              Tel:   +49 4152 1457 (tel)                                    */
/*              Fax:   +49 4152 1455 (fax)                                    */
/*              Email: Norman.Fomferra@gkss.de                                */
/*////////////////////////////////////////////////////////////////////////////*/

#ifdef __cplusplus
extern "C" {
#endif

#define NN_MAX_PATH   511
#define NN_MAX_LINE   511
#define NN_MAX_TOKEN  NN_MAX_LINE

#define NN_NAME_NET         "Net"   
#define NN_NAME_LAYER       "Layer" 
#define NN_NAME_UNIT        "Unit"  

#define NN_NAME_CONNECTION  "C"     
#define NN_NAME_NUM_CONNS   "NumConns"    
#define NN_NAME_INP_BIAS    "InpBias"     
#define NN_NAME_INP_SCALE   "InpScale"    
#define NN_NAME_OUT_BIAS    "OutBias"     
#define NN_NAME_OUT_SCALE   "OutScale"    
#define NN_NAME_ACTIVATION  "Activation"   
#define NN_NAME_MATRIX      "M"     
#define NN_NAME_NUM_UNITS   "NumUnits"    
#define NN_NAME_INP_FNID    "InpFunc"     
#define NN_NAME_ACT_FNID    "ActFunc"     
#define NN_NAME_OUT_FNID    "OutFunc"     
#define NN_NAME_ACT_SLOPE   "ActSlope"    
#define NN_NAME_ACT_THRES   "ActThres"    
#define NN_NAME_NUM_LAYERS  "NumLayers"   
#define NN_NAME_MIN_VERSION "MinVersion"  
#define NN_NAME_MAJ_VERSION "MajVersion"  
#define NN_NAME_INP_LAYER   "InpLayer"    
#define NN_NAME_OUT_LAYER   "OutLayer"    
#define NN_NAME_PRECISION   "Precision"   

#define NN_NAME_SINGLE      "Single" 
#define NN_NAME_DOUBLE      "Double" 

#define NN_NAME_ZERO        "Zero" 
#define NN_NAME_SUM_1       "Sum_1" 
#define NN_NAME_SUM_2       "Sum_2" 

#define NN_NAME_IDENTITY    "Identity"  
#define NN_NAME_THRESHOLD   "Threshold" 
#define NN_NAME_LINEAR      "Linear"    
#define NN_NAME_SEMILINEAR  "SemiLinear"
#define NN_NAME_QUADRATIC   "Quadratic"
#define NN_NAME_EXPONENTIAL "Exponential"
#define NN_NAME_LOGARITHMIC "Logarithmic"
#define NN_NAME_SIGMOID_1   "Sigmoid_1"   
#define NN_NAME_SIGMOID_2   "Sigmoid_2"   
#define NN_NAME_RBF_1       "Rbf_1"
#define NN_NAME_RBF_2       "Rbf_2"

/*////////////////////////////////////////////////////////////////////////////*/
typedef enum ENnSectId
{
	NN_SECT_NET, 
	NN_SECT_LAYER, 
	NN_SECT_UNIT
}
NN_SECTID;

/*////////////////////////////////////////////////////////////////////////////*/
typedef enum ENnKeyId
{
	NN_KEY_NUM_LAYERS, 
	NN_KEY_MIN_VERSION,
	NN_KEY_MAJ_VERSION,
	NN_KEY_INP_LAYER,  
	NN_KEY_OUT_LAYER,  
	NN_KEY_PRECISION,  
	NN_KEY_NUM_UNITS,  
	NN_KEY_INP_FNID,   
	NN_KEY_ACT_FNID,   
	NN_KEY_OUT_FNID,   
	NN_KEY_ACT_SLOPE,  
	NN_KEY_ACT_THRES,  
	NN_KEY_NUM_CONNS,  
	NN_KEY_INP_BIAS,   
	NN_KEY_INP_SCALE,  
	NN_KEY_OUT_BIAS,   
	NN_KEY_OUT_SCALE,  
	NN_KEY_ACTIVATION, 
	NN_KEY_CONNECTION, 
	NN_KEY_MATRIX
}
NN_KEYID;

/*////////////////////////////////////////////////////////////////////////////*/
typedef enum ENnToken
{
	NN_TOK_EOL,
	NN_TOK_INT,
	NN_TOK_FLOAT,
	NN_TOK_STRING,
	NN_TOK_NAME,
	NN_TOK_PUNCT
}
NN_TOKEN;

/*////////////////////////////////////////////////////////////////////////////*/
typedef struct SNnKwEnt
{
	const int   nId;
	const char* pchName;
}
NN_KWENT;

/*////////////////////////////////////////////////////////////////////////////*/
typedef struct SNnKwTab
{
	const int        nSize;
	const NN_KWENT*  pEntries; 
}
NN_KWTAB;


/*////////////////////////////////////////////////////////////////////////////*/
NN_STATUS Nn_CreateNetFromAscFile
(
	PCSTR    pchFilePath, 
	int      nNumInpUnits,
	int      nNumOutUnits,
	NN_PNET* ppNet
);

BOOL Nn_ParseNet(NN_PNET* ppNet);
BOOL Nn_ParseSectionHeader(NN_PNET pNet);
BOOL Nn_ParseSectionEntry(NN_PNET pNet);
BOOL Nn_ParseNetSectionEntry(NN_PNET pNet);
BOOL Nn_ParseLayerSectionEntry(NN_PNET pNet);
BOOL Nn_ParseUnitSectionEntry(NN_PNET pNet);
BOOL Nn_ParseConnEntryAssign(const NN_PNET pNet, NN_PUNIT pUnit);
BOOL Nn_ParseMatrixEntryAssign(const NN_PNET pNet, NN_PUNIT pUnit);
BOOL Nn_ParseKeywordAssign(const NN_KWTAB* aTab, short* pnFnId);
BOOL Nn_ParseFloatAssign(NN_FLOAT* pf);
BOOL Nn_ParseShortAssign(short* ps);
BOOL Nn_ParseIndexAssign(short sMax, short* ps);
BOOL Nn_ParseCountAssign(short sMin, short sMax, short* ps);
BOOL Nn_ParseFloat(NN_FLOAT* pf);
BOOL Nn_ParseShort(short* ps);
BOOL Nn_ParseIndex(short sMax, short* ps);
BOOL Nn_ParseCount(short sMin, short sMax, short* ps);

BOOL Nn_CheckLayerIndex(const NN_PNET pNet, int iL);
BOOL Nn_CheckUnitIndex(const NN_PLAYER pLayer, int iU);
BOOL Nn_CheckConnIndex(const NN_PUNIT pUnit, int iC);

int Nn_FindKeywordIdent(const NN_KWTAB* aKwTab, PCSTR pszName);
PCSTR Nn_FindKeywordName(const NN_KWTAB* aKwTab, int nKwId);
PCSTR Nn_GetPrintKeyword(const NN_KWTAB* pTab, int nKwId);
int Nn_CompareKw(PCSTR pstr1, PCSTR pstr2);

NN_STATUS Nn_OpenAscFileScanner (PCSTR pchFilePath);
void Nn_CloseAscFileScanner ();
BOOL Nn_ParsePunctuatorOpt (int ch);
BOOL Nn_ParsePunctuator (int ch);
BOOL Nn_ParseTokenOpt (NN_TOKEN nTokenId);
BOOL Nn_ParseToken (NN_TOKEN nTokenId);
NN_TOKEN Nn_ScanToken();
void Nn_ReadChar();
BOOL Nn_ReadLine (); 
PCSTR Nn_GetToken(char szToken[NN_MAX_TOKEN+1]);
PCSTR Nn_GetTokenName (NN_TOKEN nTokenId); 

NN_STATUS Nn_AscReadError (PCSTR pszFormat, ...);
int Nn_GetNumErrors();
long Nn_GetTokenValInt();
double Nn_GetTokenValFloat();

NN_STATUS Nn_WriteNetToAscFile (PCSTR pchFilePath, const NN_PNET pNet);
NN_STATUS Nn_WriteAscNet  (const NN_PNET pNet);
NN_STATUS Nn_AscWriteError ();

#ifdef __cplusplus
}
#endif
/* EOF ///////////////////////////////////////////////////////////////////////*/
