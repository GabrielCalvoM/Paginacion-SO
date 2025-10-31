#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <random>
#include <sstream>

#include "alg/algorithm.h"
#include "sim/intset.h"
#include "sim/page.h"

namespace {
    typedef struct {
        bool exists;
        std::vector<unsigned int> pointers;
    } process_t;
}

static inline void insertInstruction(IntSet *set, std::string &buffer, IntTypeE instruction, unsigned int param);
static inline void insertInstruction(IntSet *set, std::string &buffer, IntTypeE instruction, unsigned int param1, unsigned int param2);

////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<unsigned int> IntSet::getAccessSequence() const {
    // vars
    std::vector<unsigned int> sequence;
    unsigned int nextPtrId = 1; // ocurrence of "new"
    std::unordered_map<unsigned int, unsigned int> filetoPtr; // file-index -> ptrId
    std::unordered_map<unsigned int, std::vector<unsigned int>> ptrToPages; // ptrId -> pageId
    unsigned int pageCounter = 1;

    //
    for (const auto &instr : mVec) { switch (instr.type) {
        case newI: {
            // define pages
            size_t bytes = instr.param2;
            unsigned int pages = static_cast<unsigned int>(std::ceil(bytes / (double)Page::pageSize));
            unsigned int ptrId = nextPtrId++;
            
            // assign sequential ids
            std::vector<unsigned int> pagesVec;
            pagesVec.reserve(pages);
            for (unsigned int p = 0; p < pages; ++p) pagesVec.push_back(pageCounter++);
            
            // store mapping
            ptrToPages[ptrId] = pagesVec;

            auto it = ptrToPages.find(ptrId);

            // add to sequence
            if (it != ptrToPages.end()) {
                sequence.insert(sequence.end(), it->second.begin(), it->second.end());
            } 

            break;
        }

        case useI: {
            // retrieve ptr
            unsigned int ptrRef = instr.param1;
            auto it = ptrToPages.find(ptrRef);

            // add to sequence
            if (it != ptrToPages.end()) {
                sequence.insert(sequence.end(), it->second.begin(), it->second.end());
            }
            break;
        }

        case delI: {
            // remove from struct
            unsigned int ptrRef = instr.param1;
            ptrToPages.erase(ptrRef);
            break;
        }

        case killI: {
            // retrieve pid
            unsigned int pid = instr.param1;
            
            // delete owner
            for (auto it : ptrToPages) {
                // TODO
            }
            break;
        }

    }}

    return sequence;
}
////////////////////////////////////////////////////////////////////////////////////////////
std::string IntSet::generateInstructions(unsigned int seed, unsigned int nProc, unsigned int nOp) {
    std::string buffer;

    if (nOp == 0 || nProc == 0) return buffer;
    emptyVec();

    std::vector<process_t> processes(nProc, (process_t){true, {}});
    std::vector<bool> pointers;

    const IntTypeE instrList[] = {newI, useI, delI, killI};
    IntTypeE instr = newI;

    std::mt19937 genSeed(seed);
    std::uniform_int_distribution<> genSpace(1, 2e4);
    std::uniform_int_distribution<> genPtr;
    std::uniform_int_distribution<> genProc(0, nProc - 1);
    std::discrete_distribution<> genInstr({0.3, 0.5, 0.15, 0.05});

    unsigned int instrI, ptrI, processI, pointersExists = 0, processesExists = nProc;

    for (unsigned int i = 0; i < nOp; ++i) {
        while ((instrI = i <= nProc / 10 || pointersExists == 0 ? 0                         // crear punteros si no hay o si no han pasado más de nProc/10 instrucciones
            : genInstr(genSeed)) == 3
            && i / (nProc - processesExists + 1) < std::ceil((double)nOp / (double)nProc)); // no hacer kill antes de nOp/nProc instrucciones

        instr = instrList[instrI];
        
        // generar use o delete
        if (instrI == 1 || instrI == 2) {
            while (!pointers[ptrI = genPtr(genSeed, decltype(genPtr)::param_type(0, pointers.size() - 1))]);    // solo obtener punteros válidos
            insertInstruction(this, buffer, instr, ptrI + 1);
            if (instrI == 2) { pointers[ptrI] = false; --pointersExists; }                                      // invalidar si es delete
            continue;
        }

        while (!processes[processI = genProc(genSeed)].exists); // solo obtener procesos corriendo
        
        // generar kill
        if (instrI == 3) {
            insertInstruction(this, buffer, instr, processI + 1);
            std::vector ptrId = processes[processI].pointers;
            for (unsigned int i = 0; i < ptrId.size(); ++i) { pointers[ptrId[i]] = false; --pointersExists; }   // matar todos los punteros del proceso
            processes[processI].exists = false;                                                                 // matar el proceso
            --processesExists;
            continue;
        }

        // generar new
        unsigned int size = genSpace(genSeed);
        insertInstruction(this, buffer, instr, processI + 1, size);
        processes[processI].pointers.push_back(pointers.size());
        pointers.push_back(true); ++pointersExists;             // agregar puntero
    }

    return buffer;
}

std::string IntSet::loadSet(const std::string filepath) {
    emptyVec();
    
    std::ifstream file(filepath);
    std::string buffer, line, intStr, param;
    IntTypeE intType;

    while (std::getline(file, line, ')')) {
        if (line.empty()) continue;

        std::replace(line.begin(), line.end(), '(', ' ');
        std::replace(line.begin(), line.end(), ',', ' ');
        std::stringstream ss(line);
        while (ss && !std::isalnum(ss.peek())) ss.get();

        if (!ss) continue;

        ss >> intStr;

        bool isInstruction = false;
        for (const auto& [key, _] : stringToIntType) if (intStr.compare(key) == 0) { isInstruction = true; break; }
        if (!isInstruction) continue;

        intType = stringToIntType.at(intStr);

        std::vector<unsigned int> args;

        while (ss >> param) args.push_back(std::stoi(param));
        std::ostringstream oss;

        if (intType == newI) { 
            pushInstruction({intType, args[0], args[1]});
            oss << intTypeString.at(intType) << "(" << args[0] << "," << args[1] << ")\n";
            buffer.append(oss.str());
        }
        else {
            pushInstruction({intType, args[0]});
            oss << intTypeString.at(intType) << "(" << args[0] << ")\n";
            buffer.append(oss.str());
        }
    }

    return buffer;
}

void IntSet::saveSet(const std::string filepath) const {
    std::ofstream file(filepath);
    decltype(mVec.begin()) it = mVec.begin();

    do {
        Instruction instr = *it;
        file << intTypeString.at(instr.type) << "(" << instr.param1;
        if (instr.type == newI) file << "," << instr.param2;
        file << "),";
    } while (++it != mVec.end());
}

bool IntSet::pushInstruction(const Instruction ins) {
    mVec.push_back(ins);
    return true;
}

static inline void insertInstruction(IntSet *set, std::string &buffer, IntTypeE instruction, unsigned int param) {
    std::stringstream ss;
    Instruction instr(instruction, param);
    set->pushInstruction(instr);
    ss << intTypeString.at(instruction) << "(" << param << ")\n";
    buffer += ss.str();
}

static inline void insertInstruction(IntSet *set, std::string &buffer, IntTypeE instruction, unsigned int param1, unsigned int param2) {
    std::stringstream ss;
    Instruction instr(instruction, param1, param2);
    set->pushInstruction(instr);
    ss << intTypeString.at(instruction) << "(" << param1 << "," << param2 << ")\n";
    buffer += ss.str();
}
