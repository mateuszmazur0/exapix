#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>
#include <functional>
#include <thread>
#include <memory>
#include <arpa/inet.h>
#include <exception>

extern "C"
{
    #include "pix.h"
    void pixtime(uint64_t __attribute__((unused)) t){}
}

const std::size_t maxN = 100*1000;
const std::size_t threads_num = 4;
const std::size_t test_interval_size = 4 * threads_num;

uint32_t fileTab[maxN + 1];
uint32_t pixTab[maxN + 1];

int main()
{
    std::cout << "Loading data file...\n";
    if(std::ifstream file{"pix-data-32m", std::ios::binary})
    {   
        if(file.read((char*)fileTab, (maxN + 1) * sizeof(uint32_t)))
            std::cout << "Read succesfully!\n";
        else
            return std::cout << "Read failed :C\n", 1;        
    }
    else
        return std::cout << "Couldnt open pix-data-32m!\n", 1;
   
    std::cout << "Starting testing...\n";
    
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<std::size_t> distribution(0, maxN);
    auto getRandom = std::bind(distribution, generator);

    for(std::size_t t = 0;;)
    {
        std::size_t curIntervalStart = getRandom();
        std::size_t curIntervalEnd = curIntervalStart + test_interval_size;
        
        if(curIntervalEnd > maxN + 1)
            continue; 
        
        std::cout << "Test #" << t << " on [" << curIntervalStart << ", " << curIntervalEnd << ")...\n";
        
        for(std::size_t i = curIntervalStart; i < curIntervalEnd; i++)
            pixTab[i] = 0;

        std::cout << "Running pix..." << std::endl;
        std::array<std::unique_ptr<std::thread>, threads_num> threads;
    
        uint64_t counter = curIntervalStart;

        for(auto& t : threads)
            t = std::make_unique<std::thread>([&]() { pix(pixTab, &counter, curIntervalEnd); });

        for(auto& t : threads)
            t->join();

        for(std::size_t i = curIntervalStart; i < curIntervalEnd; i++)
        {
            uint32_t converted = ntohl(fileTab[i]);

            if(converted != pixTab[i])
            {
                std::cout << "ERROR on index " << i << "\n";
                std::cout << "In file: " << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << converted << '\n';
                std::cout << "Pix did: " << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << pixTab[i] << '\n';
                return 1;
            }
        }

        std::cout << "OK\n";
        t++;
    }
}
