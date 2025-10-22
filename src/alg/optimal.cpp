#include "alg/optimal.h"
#include <cstdio>
#include <vector>
#include <iostream>

#define MAX_FRAMES 4

// Constructor
Optimal::Optimal(std::vector<Page> &ram, const std::vector<unsigned int> &sequence) : IAlgorithm(ram), mAccessSequence(sequence) {}

static void v_print(const std::vector<unsigned int> &vec, int index) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if ((int)i == index) std::cout << "*";
        std::cout << vec[i];
        if (i + 1 < vec.size()) std::cout << ", ";
    }
}

// Execute the Optimal algorithm over the provided access sequence.
// This implementation mimics the standalone version but updates mRam
// at the end so Pages reflect the final frame contents.
FrameResult Optimal::execute(const std::vector<unsigned int> &accesses) {
    std::vector<unsigned int> frame; // logical page numbers currently loaded
    int index = 0;
    FrameResult lastRes{InFrameE::Miss, 0};

    for (size_t step = 0; step < accesses.size(); ++step) {
        unsigned int page = accesses[step];

        // Search if page is already in frame
        int pagePos = 0;
        bool isInPage = false;
        for (pagePos = 0; pagePos < (int)frame.size(); ++pagePos) {
            if (frame[pagePos] == page) { isInPage = true; break; }
        }

        if (isInPage) {
            lastRes.state = InFrameE::Hit;
            lastRes.pagePos = pagePos;
        } else {
            // Miss
            if (frame.size() < MAX_FRAMES) {
                frame.push_back(page);
                lastRes.state = InFrameE::Miss;
                lastRes.pagePos = (unsigned int)(frame.size() - 1);
            } else {
                // Find which frame entry will be used farthest in the future (or never)
                std::vector<unsigned int> founded(frame.begin(), frame.end());

                for (size_t i = step + 1; i < accesses.size(); ++i) {
                    for (size_t j = 0; j < founded.size(); ++j) {
                        if (founded[j] == accesses[i]) {
                            founded.erase(founded.begin() + j);
                            break;
                        }
                    }
                    if (founded.size() == 1) break;
                }

                // The remaining founded[0] is the victim to replace
                int victimPos = 0;
                while (frame[victimPos] != founded[0]) ++victimPos;
                frame[victimPos] = page;
                lastRes.state = InFrameE::Miss;
                lastRes.pagePos = victimPos;
            }
        }

        // Debug print similar to original
        std::cout << "index = " << step << " { ";
        v_print(accesses, step);
        std::cout << " }\n";
        std::cout << "      frame =  { ";
        v_print(frame, lastRes.pagePos);
        std::cout << " }    " << (lastRes.state == InFrameE::Hit ? "Hit" : "Miss") << std::endl << std::endl;
    }

    // Optionally update mRam to reflect final frame contents (so other components can inspect)
    mRam.clear();
    for (unsigned int pg : frame) {
        Page p;
        p.setPhysicalDir(pg);
        p.setInRealMem(true);
        mRam.push_back(p);
    }

    return lastRes;
}


/*
    #include <iostream>
    #include <vector>

    #define MAX_FRAME 4

    enum class InFrame {
        Hit,
        Miss
    };

    struct FrameResult {
        InFrame state;
        int pagePos;
    };

    FrameResult optimalAlgorithm(std::vector<int> buf, int *index, std::vector<int> *frame) {
        int page = buf.at((*index)++), pagePos;
        bool isInPage = false;
        for (pagePos = 0; pagePos < (*frame).size(); ++pagePos) if ((*frame).at(pagePos) == page) {isInPage = true; break;}

        if (isInPage) return {InFrame::Hit, pagePos};
        if ((*frame).size() < MAX_FRAME) {
            (*frame).push_back(page);
            return {InFrame::Miss, pagePos};
        }
        
        std::vector<int> foundedPage((*frame).begin(), (*frame).end());

        for (int i = *index; i < buf.size(); ++i) {
            for (int j = 0; j < foundedPage.size(); ++j) {
                if (foundedPage.at(j) == buf.at(i)) {
                    foundedPage.erase(foundedPage.begin() + j);
                    break;
                }
            }

            if (foundedPage.size() == 1) break;
        }

        pagePos = 0;
        while (foundedPage.at(0) != (*frame).at(pagePos)) ++pagePos;

        (*frame).at(pagePos) = page;
        return {InFrame::Miss, pagePos};
    }

    void v_print(std::vector<int> vec, int index) {
        int i = 0;

        while (i < vec.size()) {
            if (i == index) std::cout << "*";
            std::cout << vec.at(i);
            i++;
            if (i >= vec.size()) break;
            std::cout << ", ";
        }
    }

    void testAlgorithm(std::vector<int> buf) {
        std::vector<int> frame;
        int index = 0, hits = 0;

        for (int i = 0; i < buf.size(); ++i) {
            FrameResult res = optimalAlgorithm(buf, &index, &frame);
            std::cout << "index = " << index - 1 << " { ";
            v_print(buf, index - 1);
            std::cout << " }     " << std::endl;
            std::cout << "      frame =  { ";
            v_print(frame, res.pagePos);
            std::cout << " }    " << (res.state == InFrame::Hit ? "Hit" : "Miss") << std::endl << std::endl;

            if (res.state == InFrame::Hit) hits++;
        }

        std::cout << "Hits: " << hits << std::endl << std::endl;
    }

    int main() {
        std::vector<int> buf = {0, 1, 2, 1, 3, 5, 8, 20, 13, 1, 2, 4, 5, 15, 0, 15};
        // std::vector<int> buf = {1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4};

        testAlgorithm(buf);

        return 0;
    }
*/