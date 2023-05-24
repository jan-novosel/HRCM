#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdbool.h>
#include <vector>
#include <cctype>
#include <cmath>
#include <map>

using namespace std;

typedef struct
{
    int begin;
    int length;
} POSITION_RANGE;

typedef struct
{
    int begin;
    char c;
} POSITION_CHAR;

typedef struct
{
    int tarIndex = 0;
    int begin = 0;
    int length = 0;
    string mismatch = "";
} FIRST_LEVEL_TRIPLE;

typedef struct
{
    bool isMismatch;
    int id;
    int begin;
    int length;
    string mismatch;
} SECOND_LEVEL_TRIPLE;

typedef struct
{
    string refSeq;
    string refId;
    vector<POSITION_RANGE> refLow;
} REF_INFO;

typedef struct
{
    string tarSeq;
    string tarId;
    vector<POSITION_RANGE> tarLow;
    vector<POSITION_RANGE> tarN;
    vector<POSITION_CHAR> tarSpec;
} TAR_INFO;

typedef struct
{
    TAR_INFO tar;
    vector<FIRST_LEVEL_TRIPLE> triples;
} FIRST_LEVEL_STRUCT;

typedef struct
{
    TAR_INFO tar;
    vector<SECOND_LEVEL_TRIPLE> triples;
} SECOND_LEVEL_STRUCT;

REF_INFO referenceSequenceExtraction(string strRefName)
{
    vector<POSITION_RANGE> refLow;
    ifstream inputFile(strRefName);
    stringstream buffer;
    buffer << inputFile.rdbuf();
    string refSeq = buffer.str();

    // mičemo prvi red i sve praznine
    string refId = refSeq.substr(0, refSeq.find('\n'));
    refSeq = refSeq.substr(refSeq.find('\n') + 1);
    refSeq.erase(remove(refSeq.begin(), refSeq.end(), '\n'), refSeq.end());
    refSeq.erase(remove(refSeq.begin(), refSeq.end(), '\r'), refSeq.end());

    bool flag = false; // flag je true ako smo na malom slovu
    int len = 0;
    POSITION_RANGE low;
    // mala slova
    for (int i = 0; i < refSeq.length(); i++)
    {
        if (islower(refSeq.at(i)))
        {
            if (!flag)
            {
                flag = true;
                low.begin = i;
            }
            refSeq.replace(i, 1, 1, toupper(refSeq.at(i)));
            len++;
            if (i == refSeq.length() - 1)
            {
                low.length = len;
                refLow.push_back(low);
            }
        }
        else if (flag)
        {
            low.length = len;
            refLow.push_back(low);
            len = 0;
            flag = false;
        }
    }

    REF_INFO info;
    info.refId = refId;
    info.refLow = refLow;
    info.refSeq = refSeq;
    return info;
}