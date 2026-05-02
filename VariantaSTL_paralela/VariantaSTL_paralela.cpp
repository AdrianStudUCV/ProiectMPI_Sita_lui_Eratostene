// VariantaSTL_paralela.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <execution> // Necesara pentru std::execution::par

int main() {
    long long n = 100000000; // Cautam primele pana la 10^8

    std::cout << "Se foloseste STL Paralel (C++17) pentru n = " << n << "...\n";

    // --- START MASURARE TIMP ---
    auto start_time = std::chrono::high_resolution_clock::now();

    // Din nou, folosim std::vector<char> obligatoriu pentru a evita data races la nivel de bit
    std::vector<char> is_prime(n + 1, 1);
    is_prime[0] = is_prime[1] = 0;

    long long limit = std::sqrt(n);
    std::vector<long long> base_primes;

    // Pasul 1: Sita secventiala pana la sqrt(n)
    for (long long p = 2; p <= limit; ++p) {
        if (is_prime[p]) {
            base_primes.push_back(p);
            for (long long j = p * p; j <= limit; j += p) {
                is_prime[j] = 0;
            }
        }
    }

    // Pasul 2: Pregatirea segmentelor pentru STL Paralel
    long long chunk_size = 10000; // Dimensiunea unui segment de memorie prelucrat
    std::vector<long long> chunk_starts;

    for (long long i = limit + 1; i <= n; i += chunk_size) {
        chunk_starts.push_back(i);
    }

    // Pasul 3: Executia paralela a algoritmului folosind std::for_each
    std::for_each(std::execution::par, chunk_starts.begin(), chunk_starts.end(),
        [&](long long start) {
            long long end = std::min(start + chunk_size - 1, n);

            for (long long p : base_primes) {
                // Gaseste primul multiplu al lui p in segmentul [start, end]
                long long first_multiple = (start / p) * p;
                if (first_multiple < start) first_multiple += p;
                if (first_multiple == p) first_multiple += p;

                // Taie multiplii din acest segment
                for (long long j = first_multiple; j <= end; j += p) {
                    is_prime[j] = 0;
                }
            }
        }
    );

    // --- STOP MASURARE TIMP ---
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    // Verificam cate numere prime am gasit
    long long primes_count = 0;
    for (long long p = 2; p <= n; ++p) {
        if (is_prime[p]) primes_count++;
    }

    std::cout << "Numere prime gasite: " << primes_count << "\n";
    std::cout << "Timp de executie (Sita + STL Paralel): " << duration.count() << " secunde\n";

    return 0;
}
