#include <iostream>
#include <vector>
#include <timsort.hpp>
#include <helper.hpp>
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
    std::string filename;
    if (argc > 2)
    {
        filename = argv[2];
        minRun = std::stoi(argv[1]);
    }
    else
    {
        filename = argv[1];
    }
    int size = readDataCount(filename);
    int *data = readFile(filename);
    if (minRun == 0)
    {
        minRun = minRunLength(size);
    }
    auto start = std::chrono::high_resolution_clock::now();
    TimSort<>::sort(data, data + size, minRun);
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << size << "," << minRun << "," << std::setprecision(9) << duration / 1000000000 << std::endl;
    return 0;
}
