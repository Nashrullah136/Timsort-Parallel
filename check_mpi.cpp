#include <stdio.h>
#include <iostream>
#include <mpi.h>
#include <math.h>
#include <timsort.hpp>
#include <helper.hpp>

int calc_buffer_size(int id, int number_of_thread, int size)
{
    if (id == 0)
    {
        return size;
    }
    int multiply = 1;
    while (id % 2 == 0)
    {
        multiply *= 2;
        id /= 2;
    }
    return size / number_of_thread * multiply;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int number_of_threads;
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_threads);
    int id;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    int size = 0;
    int *input = nullptr;
    if (id == 0)
    {
        if (argc < 2)
        {
            std::cout<<argc<<std::endl;
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        std::string filename = argv[1];
        size = readDataCount(filename);
        input = readFile(filename);
    }
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int buffer_size = calc_buffer_size(id, number_of_threads, size);
    int *data = (int *)malloc(sizeof(int) * buffer_size);
    int data_filled = size / number_of_threads;
    MPI_Scatter(input, data_filled, MPI_INT, data, data_filled, MPI_INT, 0, MPI_COMM_WORLD);
    TimSort<>::sort(data, data + data_filled);
    for (size_t i = 0; i < log2(number_of_threads); i++)
    {
        int modifier = int(pow(2, i + 1));
        int diff_target = int(pow(2, i));
        if (id % modifier != 0)
        {
            MPI_Send(data, data_filled, MPI_INT, id - diff_target, 0, MPI_COMM_WORLD);
            break;
        }
        else
        {
            MPI_Recv(data + data_filled, data_filled, MPI_INT, id + diff_target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            TimSort<>::merge(data, data + data_filled, data + data_filled * 2);
            data_filled *= 2;
        }
    }
    if (id == 0)
    {
        bool result = is_sorted(data, size);
        std::cout << "MPI," << size << "," << result << std::endl;
    }
    MPI_Finalize();
    return 0;
}
