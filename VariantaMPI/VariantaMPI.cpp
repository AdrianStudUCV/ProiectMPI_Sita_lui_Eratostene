// VariantaMPI.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <cmath>
#include <mpi.h>

int main(int argc, char** argv) {
    // Initializarea mediului MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Cine sunt eu? (0, 1, 2...)
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Cate procese sunt in total?

    long long n = 100000000; // Cautam primele pana la 10^8
    long long limit = std::sqrt(n);

    // Calculam portiunea de numere alocata acestui proces
    long long elements_per_proc = (n - 1) / size;
    long long low = 2 + rank * elements_per_proc;
    // Ultimul proces preia restul numerelor (pentru cazurile in care impartirea nu e exacta)
    long long high = (rank == size - 1) ? n : low + elements_per_proc - 1;
    long long local_size = high - low + 1;

    // Alocam memorie doar pentru bucata noastra
    std::vector<char> local_prime(local_size, 1);

    double start_time = 0.0;

    // --- START MASURARE TIMP (Folosim timer-ul nativ MPI) ---
    // Bariera asigura ca toate procesele incep cronometrarea in acelasi timp
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        std::cout << "Se foloseste MPI cu " << size << " procese pentru n = " << n << "...\n";
        start_time = MPI_Wtime();
    }

    long long current_prime = 2;

    // Folosim o bucla care se bazeaza pe valoarea primita prin broadcast
    while (true) {
        // Pasul 1: Procesul 0 trimite numarul prim curent (sau semnalul de oprire) tuturor
        MPI_Bcast(&current_prime, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

        // Toate procesele (inclusiv 0) verifica aici daca am depasit limita
        if (current_prime > limit) {
            break;
        }

        // Pasul 2: Calculam primul multiplu al lui 'current_prime' in intervalul [low, high]
        long long first_multiple = (low / current_prime) * current_prime;
        if (first_multiple < low) first_multiple += current_prime;
        if (first_multiple == current_prime) first_multiple += current_prime; // Nu ne taiem pe noi insine

        // Pasul 3: Taiem multiplii din vectorul local
        for (long long j = first_multiple; j <= high; j += current_prime) {
            local_prime[j - low] = 0;
        }

        // Pasul 4: Procesul 0 cauta urmatorul numar prim pentru iteratia urmatoare
        if (rank == 0) {
            current_prime++;
            while (current_prime <= limit && local_prime[current_prime - low] == 0) {
                current_prime++;
            }
            
        }
    }

    // Calculam cate numere prime a gasit acest proces
    long long local_count = 0;
    for (long long i = 0; i < local_size; ++i) {
        if (local_prime[i]) {
            local_count++;
        }
    }

    // Pasul 5: Adunam toate rezultatele locale la procesul 0
    long long global_count = 0;
    MPI_Reduce(&local_count, &global_count, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    // --- STOP MASURARE TIMP ---
    if (rank == 0) {
        double end_time = MPI_Wtime();
        std::cout << "Numere prime gasite: " << global_count << "\n";
        std::cout << "Timp de executie (MPI): " << end_time - start_time << " secunde\n";
    }

    // Inchidem mediul MPI
    MPI_Finalize();
    return 0;
}
