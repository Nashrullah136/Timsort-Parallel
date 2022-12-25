#include <iostream>
#include <vector>
#include <timsort.hpp>
#include <omp.h>
#include <math.h>
#include <algorithm>
#include <helper.hpp>
#include <string>

int main(int argc, char **argv)
{
    if(argc < 3){
        std::cout<<"Number of threads and input directory argument needed"<<std::endl;
        return -1;
    }
    std::string filename = argv[2];
    int size = readDataCount(filename);
    int* data = readFile(filename);
    int num_of_thread = std::stoi((std::string)argv[1]);
    int dataPerThread = size / num_of_thread;
    bool isFinish[num_of_thread] = {};
    auto start = data;
    #pragma omp parallel num_threads(num_of_thread)
    {
        int id = omp_get_thread_num();
        TimSort<>::sort(start + id * dataPerThread, start + (id + 1) * dataPerThread);
        for (size_t i = 0; i < log2(num_of_thread); i++)
        {
            if (id % int(pow(2, i + 1)) != 0)
            {
                isFinish[id] = true;
                break;
            }
            while (!isFinish[id + int(pow(2, i))]){}
            TimSort<>::merge(start + id * dataPerThread, start + (id + int(pow(2, i))) * dataPerThread, start + (id + int(pow(2, i + 1))) * dataPerThread);
        }
    }
    bool result = is_sorted(data, size);
    std::cout<<"OpenMP,"<<size<<","<<result<<std::endl;
    return 0;
}
