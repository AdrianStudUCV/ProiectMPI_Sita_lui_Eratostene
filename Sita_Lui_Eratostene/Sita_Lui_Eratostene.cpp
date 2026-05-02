// Sita_Lui_Eratostene.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <chrono>

// Functia executata de fiecare thread
void sieve_worker(long long start, long long end, const std::vector<long long>& base_primes, std::vector<char>& is_prime) {
    for (long long p : base_primes) {
        // Gaseste primul multiplu al lui p in intervalul [start, end]
        long long first_multiple = (start / p) * p;
        if (first_multiple < start) {
            first_multiple += p;
        }
        if (first_multiple == p) {
            first_multiple += p; // Nu taia numarul prim in sine
        }

        // Taie multiplii din bucata alocata acestui thread
        for (long long j = first_multiple; j <= end; j += p) {
            is_prime[j] = 0; // 0 inseamna ca nu este prim
        }
    }
}

int main() {
    long long n = 100000000; // Cautam primele pana la 10^8 (100 milioane)

    // Determinam cate thread-uri suporta procesorul fizic
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4; // Fallback in caz ca nu poate fi detectat

    std::cout << "Se folosesc " << num_threads << " thread-uri pentru n = " << n << "...\n";

    // --- START MASURARE TIMP ---
    auto start_time = std::chrono::high_resolution_clock::now();

    // Folosim std::vector<char> in loc de std::vector<bool>! 
    // Explicatie: std::vector<bool> comprima datele la nivel de bit, 
    // ceea ce cauzeaza race conditions cand thread-urile scriu biti vecini.
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

    // Pasul 2: Impartirea muncii pe thread-uri pentru restul numerelor
    std::vector<std::thread> threads;
    long long range_start = limit + 1;
    long long range_size = (n - range_start + 1) / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        long long thread_start = range_start + i * range_size;
        // Ultimul thread ia si restul numerelor, in caz ca impartirea nu e perfecta
        long long thread_end = (i == num_threads - 1) ? n : thread_start + range_size - 1;

        // Lansam thread-ul
        threads.emplace_back(sieve_worker, thread_start, thread_end, std::cref(base_primes), std::ref(is_prime));
    }

    // Asteptam ca toate thread-urile sa isi termine treaba (sincronizare)
    for (auto& t : threads) {
        t.join();
    }

    // --- STOP MASURARE TIMP ---
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    // Optional: Verificam cate numere prime am gasit
    long long primes_count = 0;
    for (long long p = 2; p <= n; ++p) {
        if (is_prime[p]) primes_count++;
    }

    std::cout << "Numere prime gasite: " << primes_count << "\n";
    std::cout << "Timp de executie (Sita + Threads): " << duration.count() << " secunde\n";

    return 0;
}
