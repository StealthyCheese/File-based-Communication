#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <chrono>

#include "Driver.hpp"

// Example usage

int main() {
    ULONG processId = 1980;
    uintptr_t address = 0x500000;
    uint64_t buffer = NULL;

    const int iterations = 10;

    for (int i = 0; i < iterations; ++i) {
        auto start = std::chrono::high_resolution_clock::now(); // Start the timer
        if (ReadVirt(processId, address, &buffer)) {
            auto end = std::chrono::high_resolution_clock::now(); // End the timer
            std::chrono::duration<double> duration = end - start;
            std::cout << "Successfully Read Process Memory: " << buffer << ". Time taken: " << duration.count() << " seconds" << std::endl;
        }
        else {
            std::cout << "Error Reading Process Memory" << GetLastError() << std::endl;
        }
    }

    Sleep(5000);
    return 0;
}