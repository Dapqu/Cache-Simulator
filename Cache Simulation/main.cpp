#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <deque>
#include <math.h>
#include <algorithm>

using namespace std;

vector<string> addresses;

long long BinaryToDecimal(string binary);
string HexToBinary(string hex);
void ReadFile(string fileName, vector<string>& addresses);
double DirectMapped(double sets, double bytes, vector<string>& addresses);
double FullyAssociative(double blocks, double bytes, vector<string>& addresses, bool FIFO);
double SetAssociative(double sets, double blocks, double bytes, vector<string>& addresses, bool FIFO);

// Direct Mapped, n sets of 1 block.
double DirectMapped(double sets, double bytes, vector<string>& addresses) {
    double offset = log2(bytes);
    double lineID = log2(sets);

    vector<string> directMappedCache(sets);

    double hit = 0;

    for (auto & address : addresses) {
        string tag = address.substr(0, address.length() - lineID - offset);
        string lineBinary = address.substr(address.length() - lineID - offset, lineID);
        long long lineNumber = BinaryToDecimal(lineBinary);

        if (directMappedCache.at(lineNumber) == tag) {
            hit++;
        }
        else {
            directMappedCache.at(lineNumber) = tag;
        }
    }

    return hit / (double) addresses.size();
}
// Fully Associative, 1 set of n blocks.
double FullyAssociative(double blocks, double bytes, vector<string>& addresses, bool FIFO) {
    double offset = log2(bytes);

    deque<string> fullyAssociativeCache;

    double hit = 0;

    for (auto & address : addresses) {
        string tag = address.substr(0, address.length() - offset);

        bool found = false;

        if (FIFO) {
            deque<string>::iterator it = find(fullyAssociativeCache.begin(), fullyAssociativeCache.end(), tag);
            if (it != fullyAssociativeCache.end()) {
                hit++;
            }
            else {
                if (fullyAssociativeCache.size() >= blocks) {
                    fullyAssociativeCache.pop_front();
                }
                fullyAssociativeCache.push_back(tag);
            }
        }
        else {
            for (int i = 0; i < fullyAssociativeCache.size(); i++) {
                if (fullyAssociativeCache.at(i) == tag) {
                    hit++;
                    found = true;
                    fullyAssociativeCache.erase(fullyAssociativeCache.begin() + i);
                    fullyAssociativeCache.push_back(tag);
                    break;
                }
            }
            if (!found) {
                if (fullyAssociativeCache.size() >= blocks) {
                    fullyAssociativeCache.pop_front();
                }
                fullyAssociativeCache.push_back(tag);
            }
        }
    }

    return hit / (double)addresses.size();
}
// Set Associative, n sets of m blocks, each set is like a FA.
double SetAssociative(double sets, double blocks, double bytes, vector<string>& addresses, bool FIFO) {
    double offset = log2(bytes);
    double setID = log2(sets);

    vector<deque<string>>setAssociativeCache(sets);

    double hit = 0;

    for (auto & address : addresses) {
        string tag = address.substr(0, address.length() - setID - offset);
        string lineBinary = address.substr(address.length() - setID - offset, setID);
        long long setNumber = BinaryToDecimal(lineBinary);

        bool found = false;

        if (FIFO) {
            deque<string>::iterator it = find(setAssociativeCache.at(setNumber).begin(), setAssociativeCache.at(setNumber).end(), tag);
            if (it != setAssociativeCache.at(setNumber).end()) {
                hit++;
            }
            else {
                if (setAssociativeCache.at(setNumber).size() >= blocks) {
                    setAssociativeCache.at(setNumber).pop_front();
                }
                setAssociativeCache.at(setNumber).push_back(tag);
            }
        }
        else {
            for (int i = 0; i < setAssociativeCache.at(setNumber).size(); i++) {
                if (setAssociativeCache.at(setNumber).at(i) == tag) {
                    hit++;
                    found = true;
                    setAssociativeCache.at(setNumber).erase(setAssociativeCache.at(setNumber).begin() + i);
                    setAssociativeCache.at(setNumber).push_back(tag);
                    break;
                }
            }
            if (!found) {
                if (setAssociativeCache.at(setNumber).size() >= blocks) {
                    setAssociativeCache.at(setNumber).pop_front();
                }
                setAssociativeCache.at(setNumber).push_back(tag);
            }
        }
    }

    return hit / (double)addresses.size();
}

long long BinaryToDecimal(string binary) {
    unsigned long long decimalOutput = 0;
    reverse(binary.begin(), binary.end());
    for (int i = 0; i < binary.size(); i++) {
        if (binary[i] == '1') {
            decimalOutput += pow(2, i);
        }
    }
    return decimalOutput;
}
string HexToBinary(string hex) {
    // index
    int i = 0;
    // Conversion table.
    map<char, string> table = {
            {'0', "0000"}, {'1', "0001"}, {'2', "0010"}, {'3', "0011"},
            {'4', "0100"}, {'5', "0101"}, {'6', "0110"}, {'7', "0111"},
            {'8', "1000"}, {'9', "1001"}, {'A', "1010"}, {'B', "1011"},
            {'C', "1100"}, {'D', "1101"}, {'E', "1110"}, {'F', "1111"},
    };
    // Binary string for the output.
    string binary = "";

    while (hex[i]) {
        binary += table[toupper(hex[i++])];
    }

    return binary;
}
// Read the File and convert hex data to binary and store them in vector.
void  ReadFile(string fileName, vector<string>& addresses) {
    ifstream traceFile(fileName);

    char los;   // Load or Store.
    string hex; // The hex data address.
    int number; // The number at the end.

    if (traceFile.is_open()) {
        while(traceFile >> los >> hex >> number) {
            addresses.push_back(HexToBinary(hex.substr(2, hex.length() - 2)));
        }
    }
}

int main() {
    ReadFile("gcc.trace", addresses);
    cout << "gcc.trace, Fully Associative" << endl;
    cout << "Sets: 1   Blocks: 1    Bytes per block: 4    Replacement policy: FIFO   HIT RATE: " << FullyAssociative(1, 4, addresses, true) << endl;
    cout << "Sets: 1   Blocks: 1    Bytes per block: 4    Replacement policy: LRU    HIT RATE: " << FullyAssociative(1, 4, addresses,false) << endl;
    cout << "Sets: 1   Blocks: 16   Bytes per block: 4    Replacement policy: FIFO   HIT RATE: " << FullyAssociative(16, 4, addresses, true) << endl;
    cout << "Sets: 1   Blocks: 16   Bytes per block: 4    Replacement policy: LRU    HIT RATE: " << FullyAssociative(16, 4, addresses, false) << endl;
    cout << "Sets: 1   Blocks: 16   Bytes per block: 16   Replacement policy: FIFO   HIT RATE: " << FullyAssociative(16, 16, addresses, true) << endl;
    cout << "Sets: 1   Blocks: 16   Bytes per block: 16   Replacement policy: LRU    HIT RATE: " << FullyAssociative(16, 16, addresses, false) << endl;
    cout << "Sets: 1   Blocks: 64   Bytes per block: 16   Replacement policy: FIFO   HIT RATE: " << FullyAssociative(64, 16, addresses, true) << endl;
    cout << "Sets: 1   Blocks: 64   Bytes per block: 16   Replacement policy: LRU    HIT RATE: " << FullyAssociative(64, 16, addresses, false) << endl;
    cout << "Sets: 1   Blocks: 64   Bytes per block: 64   Replacement policy: FIFO   HIT RATE: " << FullyAssociative(64, 64, addresses, true) << endl;
    cout << "Sets: 1   Blocks: 64   Bytes per block: 64   Replacement policy: LRU    HIT RATE: " << FullyAssociative(64, 64, addresses, false) << endl;
    cout << "=============================================================" << endl;
    cout << "gcc.trace, Direct Mapped, NO REPLACEMENT POLICY" << endl;
    cout << "Sets: 16   Blocks: 1   Bytes per block: 4     HIT RATE: " << DirectMapped(16, 4, addresses) << endl;
    cout << "Sets: 16   Blocks: 1   Bytes per block: 16    HIT RATE: " << DirectMapped(16, 16, addresses) << endl;
    cout << "Sets: 64   Blocks: 1   Bytes per block: 16    HIT RATE: " << DirectMapped(64, 16, addresses) << endl;
    cout << "Sets: 64   Blocks: 1   Bytes per block: 64    HIT RATE: " << DirectMapped(64, 64, addresses) << endl;
    cout << "Sets: 256  Blocks: 1   Bytes per block: 64    HIT RATE: " << DirectMapped(256, 64, addresses) << endl;
    cout << "Sets: 256  Blocks: 1   Bytes per block: 256   HIT RATE: " << DirectMapped(256, 256, addresses) << endl;
    cout << "=============================================================" << endl;
    cout << "gcc.trace, Set Associative" << endl;
    cout << "Sets: 4   Blocks: 4    Bytes per block: 4     Replacement policy: FIFO   HIT RATE: " << SetAssociative(4, 4, 4, addresses, true) << endl;
    cout << "Sets: 4   Blocks: 4    Bytes per block: 4     Replacement policy: LRU    HIT RATE: " << SetAssociative(4, 4, 4, addresses, false) << endl;
    cout << "Sets: 4   Blocks: 4    Bytes per block: 16    Replacement policy: FIFO   HIT RATE: " << SetAssociative(4, 4, 16, addresses, true) << endl;
    cout << "Sets: 4   Blocks: 4    Bytes per block: 16    Replacement policy: LRU    HIT RATE: " << SetAssociative(4, 4, 16, addresses, false) << endl;
    cout << "Sets: 4   Blocks: 16   Bytes per block: 16    Replacement policy: FIFO   HIT RATE: " << SetAssociative(4, 16, 16, addresses, true) << endl;
    cout << "Sets: 4   Blocks: 16   Bytes per block: 16    Replacement policy: LRU    HIT RATE: " << SetAssociative(4, 16, 16, addresses, false) << endl;
    cout << "Sets: 4   Blocks: 16   Bytes per block: 64    Replacement policy: FIFO   HIT RATE: " << SetAssociative(4, 16, 64, addresses, true) << endl;
    cout << "Sets: 4   Blocks: 16   Bytes per block: 64    Replacement policy: LRU    HIT RATE: " << SetAssociative(4, 16, 64, addresses, false) << endl;
    cout << "Sets: 64  Blocks: 64   Bytes per block: 64    Replacement policy: FIFO   HIT RATE: " << SetAssociative(64, 64, 64, addresses, true) << endl;
    cout << "Sets: 64  Blocks: 64   Bytes per block: 64    Replacement policy: LRU    HIT RATE: " << SetAssociative(64, 64, 64, addresses, false) << endl;
    cout << "Sets: 64  Blocks: 64   Bytes per block: 256   Replacement policy: FIFO   HIT RATE: " << SetAssociative(64, 64, 256, addresses, true) << endl;
    cout << "Sets: 64  Blocks: 64   Bytes per block: 256   Replacement policy: LRU    HIT RATE: " << SetAssociative(64, 64, 256, addresses, false) << endl;
    cout << "Sets: 64  Blocks: 256  Bytes per block: 256   Replacement policy: FIFO   HIT RATE: " << SetAssociative(64, 256, 256, addresses, true) << endl;
    cout << "Sets: 64  Blocks: 256  Bytes per block: 256   Replacement policy: LRU    HIT RATE: " << SetAssociative(64, 256, 256, addresses, false) << endl;
    cout << "Sets: 64  Blocks: 256  Bytes per block: 1024  Replacement policy: FIFO   HIT RATE: " << SetAssociative(64, 256, 1024, addresses, true) << endl;
    cout << "Sets: 64  Blocks: 256  Bytes per block: 1024  Replacement policy: LRU    HIT RATE: " << SetAssociative(64, 256, 1024, addresses, false) << endl;
    return 0;
}
