#include <stdio.h>
#include <iostream>
#include <mpi.h>
#include <math.h>
#include <timsort.hpp>

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

void print_data(int *start, int len)
{
    printf("---------------------------------------------------\n");
    for (size_t i = 0; i < len; i++)
    {
        std::cout << start[i] << std::endl;
    }
}

int main(int argc, char const *argv[])
{
    MPI_Init(nullptr, nullptr);
    int number_of_threads;
    MPI_Comm_size(MPI_COMM_WORLD, &number_of_threads);
    int id;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    int size = 0;
    int *input = nullptr;
    if (id == 0)
    {
        std::cin >> size;
        input = (int *)malloc(sizeof(int) * size);
        int i = 0;
        while (std::cin >> input[i++])
        {
        }
    }
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int buffer_size = calc_buffer_size(id, number_of_threads, size);
    int *data = (int *)malloc(sizeof(int) * buffer_size);
    int data_filled = size / number_of_threads;
    MPI_Scatter(input, data_filled, MPI_INT, data, data_filled, MPI_INT, 0, MPI_COMM_WORLD);
    if (id == 0)
    {
        print_data(data, data_filled);
    }
    //TimSort<>::sort(data, data + data_filled);
    for (size_t i = 0; i < log2(number_of_threads); i++)
    {
        int modifier = int(pow(2, i + 1));
	int diff_target = int(pow(2, i));
        if (id % modifier != 0)
        {
	    //printf("Send %d -> %d (%d)\n", id, id - diff_target, data_filled);
            MPI_Send(data, data_filled, MPI_INT, id - diff_target, 0, MPI_COMM_WORLD);
	    break;
        }
        else
        {
	    //printf("Receive %d <- %d (%d)\n", id, id + diff_target, data_filled);
            MPI_Recv(data + data_filled, data_filled, MPI_INT, id + diff_target, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //TimSort<>::merge(data, data + data_filled, data + data_filled * 2);
            data_filled *= 2;
            if (id == 0)
            {
                print_data(data, data_filled);
            }
        }
    }
    MPI_Finalize();
    return 0;
}
