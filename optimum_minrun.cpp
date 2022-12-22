#include <iostream>
#include <vector>
#include <timsort.hpp>
#include <string>
#include <chrono>
#include <iomanip>

int minRunLength(int n)
{

    int r = 0;
    while (n >= 2 * 32)
    {
        r |= (n & 1);
        n >>= 1;
    }
    return n + r;
}

int main(int argc, char **argv)
{
    int minRun = 0;
    if(argc > 1){
        minRun = std::stoi(argv[1]);
    }
    std::ios_base::sync_with_stdio(false);
    std::vector<int> data;
    int input;
    while (std::cin >> input)
    {
        data.push_back(input);
    }
    if(minRun == 0){
        minRun = minRunLength(data.size());
    }
    auto start = std::chrono::high_resolution_clock::now();
    TimSort<>::sort(data.begin(), data.end(), minRun);
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << data.size() << "," << minRun << "," << std::setprecision(9) << duration / 1000000000 << std::endl;
    return 0;
}
