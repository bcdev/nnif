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
#include "processCase2Net.h"

#define NUM_INP_UNITS 11
#define NUM_OUT_UNITS 4

void printUsage()
{
    fprintf(stderr, "Usage: case2 nnfFile inpFile outFile\n");
}

int main(int argc, char* argv[])
{
    FILE* istream;
    FILE* ostream;
    FILE* lstream = stdout;
    NN_PNET pNet;
    double inpVector[NUM_INP_UNITS];
    double outVector[NUM_OUT_UNITS];
    const char* netFile = NULL;
    const char* inpFile = NULL;
    const char* outFile = NULL;
    int i, iArg = 0;
    BOOL isEOF = FALSE;
    int numTestCases = 0;

    for (i = 1; i < argc; i++)
    {
        if (iArg == 0)
            netFile = argv[i];
        else if (iArg == 1)
            inpFile = argv[i];
        else if (iArg == 2)
            outFile = argv[i];
        else
        {
            fprintf(stderr, "to many arguments\n");
            printUsage();
            return -1;
        }
        iArg++;
    }

    if (netFile == NULL || inpFile == NULL || outFile == NULL)
    {
        printf("case2: error: to few arguments\n");
        printUsage();
        return -1;
    }


    fprintf(lstream, "loading neural net %s...\n", netFile);
    Nn_CreateNetFromBinFile(netFile, NUM_INP_UNITS, NUM_OUT_UNITS, &pNet);
    if (pNet == NULL)
    {
        fprintf(lstream, "%s\n", Nn_GetErrMsg());
        return 1;
    }
    fprintf(lstream, "neural net loaded\n");


    fprintf(lstream, "opening input file %s...\n", inpFile);
    istream = fopen(inpFile, "r");
    if (istream == NULL)
    {
        fprintf(lstream, "failed to open input file\n");
        return 2;
    }
    fprintf(lstream, "input file opened\n");


    fprintf(lstream, "opening output file %s...\n", outFile);
    ostream = fopen(outFile, "w");
    if (ostream == NULL)
    {
        fprintf(lstream, "failed to open output file\n");
        return 3;
    }
    fprintf(lstream, "output file opened\n");

    fprintf(lstream, "processing test cases...\n");
    for (;;) 
    {
        for (i = 0; i < NUM_INP_UNITS; i++)
        {
            if (fscanf(istream, "%lf", &inpVector[i]) != 1)
            {
                isEOF = TRUE;
                break;
            }
        }
        if (isEOF)
            break;

	numTestCases++;
        fprintf(stdout, "*");
        if (numTestCases % 50 == 0) 
           fprintf(stdout, "\n");
        fflush(stdout);

        processCase2Net(pNet, inpVector, outVector);

        for (i = 0; i < NUM_OUT_UNITS; i++)
            fprintf(ostream, "%f%s", outVector[i], (i < NUM_OUT_UNITS-1) ? "\t" : "");
        fprintf(ostream, "\n");
    }

    fprintf(lstream, "\n%d test cases processed\n", numTestCases);

    fclose(istream); 
    fclose(ostream); 
    if (lstream != stdout) 
        fclose(lstream);

    Nn_DeleteNet(pNet);
    return 0;
}


