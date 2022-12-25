#!/bin/sh
read -p 'Maximum threads: ' max_threads
echo "Compile optimum_minrun.cpp"
g++ optimum_minrun.cpp -o optimum_minrun.out -I .
echo "Compile check_openmp.cpp"
g++ check_openmp.cpp -o check_openmp.out -I . -fopenmp
echo "Compile check_mpi.cpp"
mpicxx check_mpi.cpp -o check_mpi.out -I .
echo "Compile bench_timsort.cpp"
g++ bench_timsort.cpp -o bench_timsort.out -I .
echo "Compile bench_openmp.cpp"
g++ bench_openmp.cpp -o bench_openmp.out -I . -fopenmp
echo "Compile bench_mpi.cpp"
mpicxx bench_mpi.cpp -o bench_mpi.out -I . -fopenmp
echo "" > optimum_minrun
echo "" > check
echo "" > seq
for f in ./test_data/*; do
    echo "-------------------$f-----------------"
    echo "./optimum_minrun.out $f"
    # ./optimum_minrun.out "$f" >> optimum_minrun
    # echo "mpiexec -n $max_threads ./check_mpi.out $f"
    mpiexec -n "$max_threads" ./check_mpi.out "$f" >> check
    echo "./check_openmp.out $max_threads $f"
    ./check_openmp.out "$max_threads" "$f" >> check
    echo "./bench_timsort.out $f"
    ./bench_timsort.out "$f" >> seq
    echo "mpiexec -n $max_threads ./bench_mpi.out $f"
    mpiexec -n "$max_threads" ./bench_mpi.out "$f" >> seq
    echo "./bench_openmp.out $max_threads $f"
    ./bench_openmp.out "$max_threads" "$f" >> seq
done
# echo "" > par
# for f in ./test_data/*; do
#     for ((i = 2; i <= max_threads; i*=2)); do
#         echo "mpiexec -n $i ./bench_mpi.out $f"
#         mpiexec -n "$i" ./bench_mpi.out "$f" >> par
#         echo "./bench_openmp.out $i $f"
#         ./bench_openmp.out "$i" "$f" >> par
#     done
# done
