#include "Compression.h"
#include <chrono>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <memory>
using namespace std::chrono;


vector<POSITION_CHAR> tarSpec;
const int k = 12;
int lineWidth;
int maxLength = 0;

TAR_INFO targetSequenceExtraction(string strTarName)
{
    vector<POSITION_RANGE> tarLow, tarN;
    vector<POSITION_CHAR> tarSpec;
    ifstream inputFile(strTarName);
    stringstream buffer;
    buffer << inputFile.rdbuf();
    string tarSeq = buffer.str();

    // mičemo prvi red i sve praznine
    tarSeq.erase(remove(tarSeq.begin(), tarSeq.end(), '\r'), tarSeq.end());
    string tarId = tarSeq.substr(0, tarSeq.find('\n'));
    tarSeq = tarSeq.substr(tarSeq.find('\n') + 1);
    lineWidth = tarSeq.find('\n');
    tarSeq.erase(remove(tarSeq.begin(), tarSeq.end(), '\n'), tarSeq.end());

    bool flag = false; // flag je true ako smo na malom slovu
    int len = 0;
    POSITION_RANGE low;
    // mala slova
    for (int i = 0; i < tarSeq.length(); i++)
    {
        if (islower(tarSeq.at(i)))
        {
            if (!flag)
            {
                flag = true;
                low.begin = i;
            }
            tarSeq.replace(i, 1, 1, toupper(tarSeq.at(i)));
            len++;
            if (i == tarSeq.length() - 1)
            {
                low.length = len;
                tarLow.push_back(low);
            }
        }
        else if (flag)
        {
            low.length = len;
            tarLow.push_back(low);
            len = 0;
            flag = false;
        }
    }

    flag = false; // flag je true ako smo na N
    len = 0;
    POSITION_RANGE N;
    // N-ovi
    for (int i = 0; i < tarSeq.length(); i++)
    {
        if (tarSeq.at(i) == 'N')
        {
            if (!flag)
            {
                flag = true;
                N.begin = i;
            }
            len++;
            if (i == tarSeq.length() - 1)
            {
                N.length = len;
                tarN.push_back(low);
            }
        }
        else if (flag)
        {
            N.length = len;
            tarN.push_back(N);
            len = 0;
            flag = false;
        }
    }

    // specijalni znakovi
    POSITION_CHAR spec;
    for (int i = 0; i < tarSeq.length(); i++)
    {
        if (tarSeq[i] != 'A' && tarSeq[i] != 'C' && tarSeq[i] != 'G' && tarSeq[i] != 'T' && tarSeq[i] != 'N')
        {
            spec.begin = i;
            spec.c = tarSeq[i];
            tarSpec.push_back(spec);
        }
    }
    for (int i = 0; i < tarSeq.length(); i++)
    {
        if (tarSeq[i] != 'A' && tarSeq[i] != 'C' && tarSeq[i] != 'G' && tarSeq[i] != 'T' && tarSeq[i] != 'N')
        {
            tarSeq.erase(tarSeq.begin() + i, tarSeq.begin()+i+1);
            i--;
        }
    }
    tarSeq.erase(remove(tarSeq.begin(), tarSeq.end(), 'N'), tarSeq.end());

    TAR_INFO info;
    info.tarId = tarId;
    info.tarLow = tarLow;
    info.tarN = tarN;
    info.tarSpec = tarSpec;
    info.tarSeq = tarSeq;
    return info;
}

int integerCoding(char ch)
{ // encoding ACGT
    switch (toupper(ch))
    {
    case 'A':
        return 0;
    case 'C':
        return 1;
    case 'G':
        return 2;
    case 'T':
        return 3;
    default:
        return 4;
    }
}

// hashiranje se svodi na pretvorbu kodiranog k-mera iz baze 5 u bazu 10
// npr. hash vrijednost 3-mera (102) je 1*5^2 + 0*5^1 + 1*5^0 = 27
// zaključio sam da je ovo ipak okej jer njihov H zauzima isto memorije kao i ovaj moj za dani k
int hashKmer(string *seq, int kmerPos)
{
    int value = 0;
    for (int i = 0; i < k; i++)
    {
        value += integerCoding((*seq)[kmerPos + i]) * pow(5, (k - i - 1));
    }
    return value;
}

int hashTriple(FIRST_LEVEL_TRIPLE triple)
{
    int value = 0;
    int mismatchCode = 0;
    for (int i = 0; i < triple.mismatch.length(); i++)
    {
        mismatchCode += integerCoding(triple.mismatch[i]);
    }
    value = triple.begin * triple.length * mismatchCode;
    return (value) % (maxLength * 20);
}

bool compareTriple(FIRST_LEVEL_TRIPLE triple1, FIRST_LEVEL_TRIPLE triple2)
{
    return triple1.tarIndex < triple2.tarIndex;
}

bool equalTriple(FIRST_LEVEL_TRIPLE triple1, FIRST_LEVEL_TRIPLE triple2)
{
    if (triple1.begin == triple2.begin && triple1.length == triple2.length &&
        triple1.mismatch == triple2.mismatch)
        return true;
    return false;
}

int parseLine(char* line) {
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int physicalMemoryUsed(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

vector<FIRST_LEVEL_TRIPLE> firstLevelMatching(string refSeq, string tarSeq)
{
    vector<FIRST_LEVEL_TRIPLE> triples;

    // inicijalizacija polja H
    long unsigned int hashTableLen = pow(5, k); // znakovi se kodiraju u 0,1,2,3,4 pa trebamo 5^k različitih hash vrijednosti
    int *H = new int[hashTableLen];

    for (int i = 0; i < hashTableLen; i++)
    {
        H[i] = -1;
    }


    // inicijalizacija polja L
    int refSteps = refSeq.length() - k + 1;
    int *L = new int[refSteps];

    // punjenje polja H i L
    for (int i = 0; i < refSteps; i++)
    {
        int value = hashKmer(&refSeq, i);
        L[i] = H[value];
        H[value] = i;
    }

    // fist-level matching
    string tarSeqCopy = tarSeq;

    while (true)
    {
        int tarSteps = tarSeqCopy.length() - k + 1;
        bool found = false;
        for (int i = 0; i < tarSteps; i++)
        {
            if (H[hashKmer(&tarSeqCopy, i)] != -1)
            {
                found = true;
                break;
            }
        }
        if (!found)
            break;

        int l_max = 0;
        int pos_max = 0;
        int i_max;

        for (int i = 0; i < tarSteps; i++)
        {
            int value = hashKmer(&tarSeqCopy, i);
            int pos = H[value];

            if (pos != -1)
            {
                while (pos != -1)
                {
                    int l = k;
                    int p = pos;
                    while (tarSeqCopy[i + l] == refSeq[p + l]/* && tarSeqCopy[i + l] != '\0'*/)
                    {
                        l += 1;
                    }
                    if (l_max < l)
                    {
                        l_max = l;
                        pos_max = p;
                        i_max = i;
                    }
                    pos = L[pos]; // pos postaje -1 ako smo došli do prvog takvog k-mera u tarSeqCopy
                }
            }
        }
        if (l_max == 0)
            break;
        FIRST_LEVEL_TRIPLE triple;
        triple.tarIndex = i_max;
        triple.begin = pos_max;
        triple.length = l_max;
        triples.push_back(triple);
        string substring = tarSeqCopy.substr(i_max, l_max);
        tarSeqCopy.replace(i_max, l_max, string(l_max, ' '));
        int cnt = count(tarSeqCopy.begin(), tarSeqCopy.end(), ' ');
    }

    sort(triples.begin(), triples.end(), compareTriple);


    for (int i = 0; i < triples.size(); i++)
    {
        if (tarSeqCopy[triples.at(i).tarIndex + triples.at(i).length] == ' ')
        {
            triples.at(i).mismatch = " ";
        }
        else
        {
            int l = 1;
            while (tarSeqCopy[triples.at(i).tarIndex + triples.at(i).length + l] != ' ' && tarSeqCopy[triples.at(i).tarIndex + triples.at(i).length + l] != '\0')
                l++;
            triples.at(i).mismatch = tarSeqCopy.substr(triples.at(i).tarIndex + triples.at(i).length, l);
        }
    }
    cout << "Utrošak fizičke memorije: " << physicalMemoryUsed() << " KB" << endl;
    delete[] H;
    delete[] L;
    return triples;
}

vector<SECOND_LEVEL_STRUCT> secondLevelMatching(vector<FIRST_LEVEL_STRUCT> firstLevel)
{
    int **H = new int *[firstLevel.size() - 1];
    for (int i = 0; i < firstLevel.size() - 1; ++i)
    {
        H[i] = new int[maxLength * 20];
    }
    for (int i = 0; i < firstLevel.size() - 1; i++)
    {
        for (int j = 0; j < maxLength * 20; j++)
        {
            H[i][j] = -1;
        }
    }

    for (int i = 0; i < firstLevel.size() - 1; i++)
    { // ide od prvog do predzadnjeg tara
        for (int j = 0; j < firstLevel.at(i).triples.size(); j++)
        { // ide po trojkama
            int hash = hashTriple(firstLevel.at(i).triples.at(j));
            H[i][hash] = j;
        }
    }

    vector<SECOND_LEVEL_STRUCT> secondLevel;
    for (int i = 1; i < firstLevel.size(); i++)
    {                                                                      // i je index trenutnog tara
        vector<FIRST_LEVEL_TRIPLE> triplesCopy = firstLevel.at(i).triples; // trojke iz first levela za trenutni target

        SECOND_LEVEL_STRUCT sls;
        sls.tar = firstLevel.at(i).tar;

        while (!triplesCopy.empty())
        { // j je index trenutne trojke
            int hashValue = hashTriple(triplesCopy.at(0));
            SECOND_LEVEL_TRIPLE triple;
            triple.length = 0;

            for (int m = 0; m < i; m++)
            { // m je index trenutnog referentnog
                int len = 0;
                int pos = H[m][hashValue];
                if (pos != -1)
                {
                    while ((len) != triplesCopy.size() && equalTriple(triplesCopy.at(len), firstLevel.at(m).triples.at(pos + len)))
                    {
                        len++;
                    }
                    if (triple.length < len)
                    {
                        triple.isMismatch = false;
                        triple.id = m;
                        triple.begin = pos;
                        triple.length = len;
                    }
                }
            }
            if (triple.length == 0)
            {
                triple.isMismatch = true;
                triple.begin = triplesCopy.at(0).begin;
                triple.length = triplesCopy.at(0).length;
                triple.mismatch = triplesCopy.at(0).mismatch;
            }

            sls.triples.push_back(triple);
            if (triple.isMismatch)
            {
                triplesCopy.erase(triplesCopy.begin(), triplesCopy.begin() + 1);
            }
            else
                triplesCopy.erase(triplesCopy.begin(), triplesCopy.begin() + triple.length);
        }
        secondLevel.push_back(sls);
    }

    return secondLevel;
}

int main(int argc, char *argv[])
{
    auto start = high_resolution_clock::now();

    string strRefName;
    vector<string> strTarNames;

    for (int i = 1; i < argc; i++)
    {
        if ((string)argv[i] == "-r")
        {
            strRefName = argv[i + 1];
        }
        else if ((string)argv[i] == "-t")
        {
            int numTarFiles = argc - 3;
            for (int j = 1; j < numTarFiles; j++)
            {
                strTarNames.push_back(argv[i + j]);
            }
        }
    }

    // strRefName = "r.txt";
    // strTarNames.push_back("t1.txt");
    // strTarNames.push_back("t2.txt");
    // strTarNames.push_back("t3.txt");
    // strTarNames.push_back("t4.txt");
    // strTarNames.push_back("t5.txt");

    REF_INFO refInfo = referenceSequenceExtraction(strRefName);
    vector<TAR_INFO> tarInfos;

    for (string s : strTarNames)
    {
        TAR_INFO tarInfo = targetSequenceExtraction(s);
        tarInfos.push_back(tarInfo);
    }

    // first-level matching
    // ovdje se tona memorije zauzme
    vector<FIRST_LEVEL_STRUCT> firstLevel;
    for (TAR_INFO ti : tarInfos)
    {
        vector<FIRST_LEVEL_TRIPLE> triples = firstLevelMatching(refInfo.refSeq, ti.tarSeq);
        // tražimo najveći broj tripleta koji je nađen
        if (triples.size() > maxLength)
            maxLength = triples.size();
        FIRST_LEVEL_STRUCT fls;
        fls.tar = ti;
        fls.triples = triples;
        firstLevel.push_back(fls);
        // for (FIRST_LEVEL_TRIPLE t : triples) {
        //     cout << t.begin << "," << t.length << "," << t.mismatch << "; ";
        // }
        // cout << endl;
    }


    ofstream MyFile("compress.txt");
    MyFile << lineWidth << endl;


    //podaci o prvom taru    
    MyFile << firstLevel.at(0).tar.tarId << endl;
    for(POSITION_RANGE pr: firstLevel.at(0).tar.tarLow) {
        MyFile << pr.begin << "," << pr.length << ";";
    }
    MyFile << endl;
    for(POSITION_RANGE pr: firstLevel.at(0).tar.tarN) {
        MyFile << pr.begin << "," << pr.length << ";";
    }
    MyFile << endl;
    for(POSITION_CHAR pc: firstLevel.at(0).tar.tarSpec) {
        MyFile << pc.begin << "," << pc.c << ";";
    }
    MyFile << endl;
    //first level trojke prvog tara
    for(FIRST_LEVEL_TRIPLE flt: firstLevel.at(0).triples) {
        MyFile << flt.begin << "," << flt.length << "," << flt.mismatch << ";";
    }
    MyFile << endl;


    // second-level matching
    if (firstLevel.size() > 1)
    {
        vector<SECOND_LEVEL_STRUCT> secondLevel = secondLevelMatching(firstLevel);

        for(SECOND_LEVEL_STRUCT sls: secondLevel) {
            MyFile << sls.tar.tarId << endl;
            for(POSITION_RANGE pr: sls.tar.tarLow) {
                MyFile << pr.begin << "," << pr.length << ";";
            }
            MyFile << endl;
            for(POSITION_RANGE pr: sls.tar.tarN) {
                MyFile << pr.begin << "," << pr.length << ";";
            }
            MyFile << endl;
            for(POSITION_CHAR pc: sls.tar.tarSpec) {
                MyFile << pc.begin << "," << pc.c << ";";
            }
            MyFile << endl;
            for(SECOND_LEVEL_TRIPLE slt: sls.triples) {
                if(slt.isMismatch) {
                    MyFile << 1 << "," << slt.begin << "," << slt.length << "," << slt.mismatch << ";";
                } else MyFile << 0 << "," << slt.id << "," << slt.begin << "," << slt.length << ";";
            }
            MyFile << endl;
        }
    }

    system("zip compress.zip compress.txt");
    system("rm compress.txt");

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(stop - start);
 
    cout << "Vrijeme kompresije: " << duration.count() << " ms" << endl;

    return 0;
}