#include "app.h"
#include "stdlib.h"
#include "stdio.h"

#include "sim/computer.h"
#include "alg/algorithm.h"
#include "sim/page.h"
#include "sim/intset.h"

#include <vector>
#include <unordered_map>

// Build a flat future access sequence from an IntSet.
// Simple semantics:
//  - Each "new(pid,bytes)" creates a new internal ptrId = nextPtr++ and assigns sequential page ids.
//  - "use(ptrId)" appends all page ids for that pointer to the sequence.
//  - "delete(ptrId)" removes the mapping so future uses do nothing.
//  - "kill(pid)" removes all pointers that were created with that pid (we track owner on creation).
static std::vector<unsigned int> buildAccessSequenceFromIntSet(IntSet iset) {
    std::vector<unsigned int> seq;
    const Instruction *ins = nullptr;

    unsigned int pageCounter = 0;
    unsigned int nextPtrId = 1;
    std::unordered_map<unsigned int, std::vector<unsigned int>> ptrPages; // ptrId -> page ids
    std::unordered_map<unsigned int, unsigned int> ptrOwner; // ptrId -> pid

    // iterate instructions (IntSet::next() is const but advances its internal iterator)
    while ((ins = iset.next())) {
        switch (ins->type) {
            case newI: {
                unsigned int pid = ins->param1;
                size_t bytes = ins->param2;
                unsigned int pages = static_cast<unsigned int>(bytes / Page::pageSize);
                if (bytes % Page::pageSize != 0) ++pages;
                unsigned int ptrId = nextPtrId++;
                std::vector<unsigned int> pagesVec;
                pagesVec.reserve(pages);
                for (unsigned int p = 0; p < pages; ++p) {
                    pagesVec.push_back(pageCounter++);
                }
                ptrPages[ptrId] = std::move(pagesVec);
                ptrOwner[ptrId] = pid;
                break;
            }
            case useI: {
                unsigned int ptrRef = ins->param1;
                auto it = ptrPages.find(ptrRef);
                if (it != ptrPages.end()) {
                    // append all pages of that pointer (simulate an access to each page it points to)
                    seq.insert(seq.end(), it->second.begin(), it->second.end());
                }
                break;
            }
            case delI: {
                unsigned int ptrRef = ins->param1;
                ptrPages.erase(ptrRef);
                ptrOwner.erase(ptrRef);
                break;
            }
            case killI: {
                unsigned int pid = ins->param1;
                // remove all ptrs owned by pid
                std::vector<unsigned int> toErase;
                for (auto &kv : ptrOwner) if (kv.second == pid) toErase.push_back(kv.first);
                for (unsigned int id : toErase) { ptrPages.erase(id); ptrOwner.erase(id); }
                break;
            }
            default: break;
        }
    }
    return seq;
}

int main(int argc, char *argv[]) {
    printf("[OS]-[Proyecto 2] - test run\n");

    Computer computer;

    // 1) load IntSet file (relative path to project root)
    IntSet iset;
    std::string filepath = "./files/a"; // adjust if needed
    iset.loadSet(filepath);

    // 2) build future access sequence from the IntSet (local simulation)
    std::vector<unsigned int> futureSequence = buildAccessSequenceFromIntSet(iset);

    // print the future sequence summary
    printf("Future access sequence length: %zu\n", futureSequence.size());
    if (!futureSequence.empty()) {
        printf("First 20 entries: ");
        for (size_t i = 0; i < futureSequence.size() && i < 20; ++i) printf("%u ", futureSequence[i]);
        printf("\n");
    }

    // 3) init MMU algorithm (pass the future sequence so OPT can build its occurrences)
    computer.mmu.initAlgorithm(AlgType::OPT, futureSequence);

    // 4) run the IntSet instruction stream through the MMU to exercise the full integration
    //    Note: executeIntSet will call newPtr/usePtr/delPtr/kill and the MMU will notify the algorithm.
    //    We need to reset the IntSet iterator before passing it (IntSet::next() advanced it).
    //    Re-load to reset internal iterator, or call loadSet again.
    iset.loadSet(filepath); // reset internal iterator and instruction vector
    computer.mmu.executeIntSet(iset);

    // 5) final state
    computer.mmu.printState();

    return 0;
}