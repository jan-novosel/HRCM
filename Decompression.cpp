#include "Compression.h"
#include <chrono>
using namespace std::chrono;

int lineWidth;

void reverseSecondLevel(vector<FIRST_LEVEL_STRUCT> &firstLevel, vector<SECOND_LEVEL_STRUCT> secondLevel) {
    for(int i=0; i<secondLevel.size(); i++) {
        FIRST_LEVEL_STRUCT fls;
        fls.tar = secondLevel.at(i).tar;
        for(int j=0; j<secondLevel.at(i).triples.size(); j++) {
            FIRST_LEVEL_TRIPLE flt;
            if(secondLevel.at(i).triples.at(j).isMismatch) {
                flt.begin = secondLevel.at(i).triples.at(j).begin;
                flt.length = secondLevel.at(i).triples.at(j).length;
                flt.mismatch = secondLevel.at(i).triples.at(j).mismatch;
                fls.triples.push_back(flt);
            } else {
                for(int k=0; k<secondLevel.at(i).triples.at(j).length; k++) {
                    int tarId = secondLevel.at(i).triples.at(j).id;
                    int tripleInd = secondLevel.at(i).triples.at(j).begin + k;
                    fls.triples.push_back(firstLevel.at(tarId).triples.at(tripleInd));
                }
            }
        }
        firstLevel.push_back(fls);
    }
}

string reverseFirstLevel(string refSeq, FIRST_LEVEL_STRUCT fls) {
    string tarSeq = "";
    for (int i = 0; i < fls.triples.size(); i++) {
        int pos = fls.triples.at(i).begin;
        int l = fls.triples.at(i).length;
        string mm = fls.triples.at(i).mismatch;
        tarSeq.append(refSeq.substr(pos, l));
        if (mm != " ") {
            tarSeq.append(mm);
        }
    }
    return tarSeq;
}

void restoreSeq(string *tarSeq, TAR_INFO tarInfo) {
    // specijalci
    for (int i = 0; i < tarInfo.tarSpec.size(); i++) {
        (*tarSeq).insert(tarInfo.tarSpec.at(i).begin, string(1, tarInfo.tarSpec.at(i).c));
    }

    // N-ovi
    for (int i = 0; i < tarInfo.tarN.size(); i++) {
        (*tarSeq).insert(tarInfo.tarN.at(i).begin, string(tarInfo.tarN.at(i).length, 'N'));
    }

    // mala slova
    for (int i = 0; i < tarInfo.tarLow.size(); i++) {
        int begin = tarInfo.tarLow.at(i).begin;
        int length = tarInfo.tarLow.at(i).length;
        for (int j = 0; j < length; j++) {
            (*tarSeq).replace(begin+j, 1, 1, tolower((*tarSeq).at(begin+j)));
        }
    }
}

int main(int argc, char *argv[])
{
    auto start = high_resolution_clock::now();

    string strRefName;
    string zipFile;

    // unzippaj i pročitaj unzippani txt


    for (int i = 1; i < argc; i++)
    {
        if ((string)argv[i] == "-r")
        {
            strRefName = argv[i + 1];
        }
        else if ((string)argv[i] == "-t")
        {
            zipFile = argv[i+1];
        }
    }

    string UNZIP_COMMAND = "unzip ";
    UNZIP_COMMAND.append(zipFile);
    system(UNZIP_COMMAND.c_str());

    REF_INFO refInfo = referenceSequenceExtraction(strRefName);

    vector<FIRST_LEVEL_STRUCT> firstLevel;
    vector<SECOND_LEVEL_STRUCT> secondLevel;

    fstream MyFile;

    MyFile.open("compress.txt", ios::in); 
    
    // Checking whether the file is open.
    if (MyFile.is_open()) { 
        string s;
        // spremi duljinu retka
        getline(MyFile, s);
        lineWidth = stoi(s);    

        getline(MyFile, s);
        FIRST_LEVEL_STRUCT fls;
        fls.tar.tarId = s;
        getline(MyFile, s);
        stringstream ssLow(s);
        string str;

        vector<POSITION_RANGE> tarLow;
        while (getline(ssLow, str, ';')) {
            POSITION_RANGE low;
            low.begin = stoi(str.substr(0, str.find(',')));
            str = str.substr(str.find(',')+1);
            low.length = stoi(str);
            tarLow.push_back(low);
        }

        getline(MyFile, s);
        stringstream ssN(s);

        vector<POSITION_RANGE> tarN;
        while (getline(ssN, str, ';')) {
            POSITION_RANGE N;
            N.begin = stoi(str.substr(0, str.find(',')));
            str = str.substr(str.find(',')+1);
            N.length = stoi(str);
            tarN.push_back(N);
        }

        getline(MyFile, s);
        stringstream ssSpec(s);

        vector<POSITION_CHAR> tarSpec;
        while (getline(ssSpec, str, ';')) {
            POSITION_CHAR spec;
            spec.begin = stoi(str.substr(0, str.find(',')));
            str = str.substr(str.find(',')+1);
            spec.c = str[0];
            tarSpec.push_back(spec);
        }

        getline(MyFile, s);
        stringstream ssTriple(s);

        vector<FIRST_LEVEL_TRIPLE> triples;
        while (getline(ssTriple, str, ';')) { 
            FIRST_LEVEL_TRIPLE triple;
            triple.begin = stoi(str.substr(0, str.find(',')));
            str = str.substr(str.find(',')+1);
            triple.length = stoi(str.substr(0, str.find(',')));
            str = str.substr(str.find(',')+1);
            triple.mismatch = str;
            triples.push_back(triple); 
        } 
        fls.tar.tarLow = tarLow;
        fls.tar.tarN = tarN;
        fls.tar.tarSpec = tarSpec;
        fls.triples = triples;
        firstLevel.push_back(fls);

        while(getline(MyFile, s)) {
            SECOND_LEVEL_STRUCT sls;
            sls.tar.tarId = s;
            getline(MyFile, s);
            stringstream ssLowS(s);
            string str;

            vector<POSITION_RANGE> tarLow;
            while (getline(ssLowS, str, ';')) {
                POSITION_RANGE low;
                low.begin = stoi(str.substr(0, str.find(',')));
                str = str.substr(str.find(',')+1);
                low.length = stoi(str);
                tarLow.push_back(low);
            }

            getline(MyFile, s);
            stringstream ssNS(s);

            vector<POSITION_RANGE> tarN;
            while (getline(ssNS, str, ';')) {
                POSITION_RANGE N;
                N.begin = stoi(str.substr(0, str.find(',')));
                str = str.substr(str.find(',')+1);
                N.length = stoi(str);
                tarN.push_back(N);
            }

            getline(MyFile, s);
            stringstream ssSpecS(s);

            vector<POSITION_CHAR> tarSpec;
            while (getline(ssSpecS, str, ';')) {
                POSITION_CHAR spec;
                spec.begin = stoi(str.substr(0, str.find(',')));
                str = str.substr(str.find(',')+1);
                spec.c = str[0];
                tarSpec.push_back(spec);
            }

            getline(MyFile, s);
            stringstream ssTripleS(s);

            vector<SECOND_LEVEL_TRIPLE> triples;
            while (getline(ssTripleS, str, ';')) { 
                SECOND_LEVEL_TRIPLE triple;
                triple.isMismatch = stoi(str.substr(0, str.find(',')));
                str = str.substr(str.find(',')+1);
                if(triple.isMismatch) {
                    triple.begin = stoi(str.substr(0, str.find(',')));
                    str = str.substr(str.find(',')+1);
                    triple.length = stoi(str.substr(0, str.find(',')));
                    str = str.substr(str.find(',')+1);
                    triple.mismatch = str;
                } else {
                    triple.id = stoi(str.substr(0, str.find(',')));
                    str = str.substr(str.find(',')+1);
                    triple.begin = stoi(str.substr(0, str.find(',')));
                    str = str.substr(str.find(',')+1);
                    triple.length = stoi(str);
                }
                triples.push_back(triple); 

            } 
            sls.tar.tarLow = tarLow;
            sls.tar.tarN = tarN;
            sls.tar.tarSpec = tarSpec;
            sls.triples = triples;
            secondLevel.push_back(sls);
        }
        MyFile.close(); 
    }
    system("rm compress.txt");
    reverseSecondLevel(firstLevel, secondLevel);



    for (int i = 0; i < firstLevel.size(); i++) {
        string tarSeq = reverseFirstLevel(refInfo.refSeq, firstLevel.at(i));
        restoreSeq(&tarSeq, firstLevel.at(i).tar);
        string fileName = firstLevel.at(i).tar.tarId.substr(1);
        fileName = fileName.append(".fa");
        ofstream MyFile(fileName);
        MyFile << firstLevel.at(i).tar.tarId << endl;
        while(tarSeq.size() >= lineWidth) {
            MyFile << tarSeq.substr(0, lineWidth) << endl;
            tarSeq = tarSeq.substr(lineWidth);
        }
        MyFile << tarSeq;
    }

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<milliseconds>(stop - start);
 
    cout << "Vrijeme dekompresije: " << duration.count() << " ms" << endl;

    return 0;
}