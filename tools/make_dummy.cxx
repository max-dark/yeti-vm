/**
 * make_dummy - generate stub source file for RISC-V targets
 */

#include <iostream>
#include <ostream>
#include <fstream>
#include <format>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

std::string generate(const char* name)
{
    return std::format(
R"(// generated file
// do not edit
int {}_generated() {}
)", name, "{ return 42; }");
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << std::format("Usage: {} <path> <name>", argv[0]) << std::endl;
        return EXIT_FAILURE;
    }

    fs::path out_name = argv[1];

    std::ofstream out{out_name};

    if (!out.is_open())
    {
        std::cerr << std::format("Unable open {}", argv[1]) << std::endl;
        return EXIT_FAILURE;
    }

    out << generate(argv[2]);
    out.close();

    return EXIT_SUCCESS;
}