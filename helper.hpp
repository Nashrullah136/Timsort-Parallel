#include "fstream"
#include "vector"
#include <filesystem>

int *readFile(std::string dir)
{
    std::ifstream file(dir);
    std::string input;
    std::getline(file, input);
    int size = std::stoi(input);
    int *data = (int *)malloc(sizeof(int) * size);
    int now = 0;
    while (std::getline(file, input))
    {
        data[now++] = std::stoi(input);
    }
    return data;
}

int readDataCount(std::string dir)
{
    std::ifstream file(dir);
    std::string input;
    std::getline(file, input);
    int size = std::stoi(input);
    return size;
}

bool is_sorted(int *data, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (i != data[i])
        {
            return false;
        }
    }
    return true;
}
