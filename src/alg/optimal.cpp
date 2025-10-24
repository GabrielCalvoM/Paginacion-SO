#include "alg/optimal.h"
#include <cstdio>
#include <vector>
#include <iostream>


// Constructor
Optimal::Optimal(std::vector<Page> &ram, const std::vector<unsigned int> &sequence) : IAlgorithm(ram), mAccessSequence(sequence) {}

static void v_print(const std::vector<unsigned int> &vec, int index) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if ((int)i == index) std::cout << "*";
        std::cout << vec[i];
        if (i + 1 < vec.size()) std::cout << ", ";
    }
}

// Execute OPT
std::vector<unsigned int> Optimal::execute(const std::vector<Page> &bufRAM, unsigned int pages)
{
    printf("\n [OPT]-Start \n");
    std::vector<unsigned int> evicted;
    if (bufRAM.empty() || pages == 0) return evicted;

    unsigned int count = std::min<unsigned int>(pages, static_cast<unsigned int>(bufRAM.size()));
    for (unsigned int i = 0; i < count; ++i) evicted.push_back(i);
    
    printf(" [OPT]-Evicting Pages: ");
    for (unsigned int idx : evicted) {  
        std::cout << idx << " ";
    }
    std::cout << "\n";
    return evicted;
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
