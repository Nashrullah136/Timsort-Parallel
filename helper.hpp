#include "fstream"
#include "vector"
#include <filesystem>

std::vector<int> readFile(std::string dir)
{
    std::ifstream file(dir);
    std::string data;
    std::vector<int> result;
    while (std::getline(file, data))
    {
        int temp = std::stoi(data);
        result.push_back(temp);
    }
    return result;
}

bool is_sorted(std::vector<int> data)
{
    int size = data.size();
    int now = 0;
    for (auto &&i : data)
    {
        if (i != now)
        {
            return false;
        }
        now++;
    }
    return true;
}
