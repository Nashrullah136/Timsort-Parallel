#include <iostream>
#include <omp.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <timsort.hpp>

int main(int argc, char **argv)
{
    int n = 20;
    int dataPerThread = n/4;
    std::vector<int> data;
    bool isFinish[4] = {false, false, false, false};
    for (size_t i = 0; i < n; i++)
    {
        data.push_back(n-i);
    }
    auto rai = data.begin();
    #pragma omp parallel num_threads(4)
    {
        int id = omp_get_thread_num();
        gfx::timsort(rai+id*dataPerThread, rai+(id+1)*dataPerThread);
        for (size_t i = 0; i < log2(4); i++)
        {
            if(id%int(pow(2, i+1)) != 0){
                printf("Thread %d is finished \n", id);
                isFinish[id] = true;
                break;
            }
            while(!isFinish[id+int(pow(2, i))]){}
            // printf("start: %d \nstart1: %d\n", id*dataPerThread, (id+int(pow(2, i)))*dataPerThread);
            gfx::timmerge(rai+id*dataPerThread, rai+(id+int(pow(2, i)))*dataPerThread, rai+(id+int(pow(2, i+1)))*dataPerThread);
        }
    }

    for (size_t i = 0; i < n; i++)
    {
        printf("%d \n", data[i]);
    }
    return 0;
}
