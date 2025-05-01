#include <iostream>
#include "mclass.h"

int main(int argc, char* argv[])
{
    /*if (argc < 3)
    {
        std::cerr << "Too few parameters\n";
        return 0;
    }
    TMclass MXor(std::stoi(argv[2]), std::stoi(argv[1]));*/
    auto start = std::chrono::high_resolution_clock::now();
    TMclass MXor(4, 100000000);
    MXor.Run();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Mcs: " << duration.count() << "\n";
    return 0;
}
