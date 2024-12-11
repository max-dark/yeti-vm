#include <iostream>

#include <vm_utility.hxx>
#include <format>


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << std::format("Usage: {} <hex file>", argv[0]) << std::endl;
        return EXIT_FAILURE;
    }

    auto hex = vm::parse_hex(argv[1]);

    if (!hex)
    {
        std::cout << std::format("unable load {}", argv[1]) << std::endl;
        return EXIT_FAILURE;
    }
    auto& records = hex.value();

    std::cout << std::boolalpha
        << std::setw(5) << std::left << "no"
        << std::setw(20) << std::left << "type"
        << std::setw(10) << std::internal << "count"
        << std::setw(10) << std::internal << "offset"
        << std::setw(10) << std::internal << "is valid"
        << std::setw(10) << std::internal << "calculated"
        << std::setw(10) << std::internal << "expected"
        << std::endl;
    size_t idx = 1;
    for (auto& r : records)
    {
        std::cout
                << std::dec
                << std::setw(5) << std::left << idx++
                << std::setw(20) << std::left << r.get_type_name()
                << std::setw(10) << std::internal << std::uint32_t(r.count)
                << std::setw(10) << std::internal << std::uint32_t(r.offset)
                << std::setw(10) << std::internal << r.is_valid()
                << std::hex
                << std::setw(10) << std::internal << uint32_t (r.sum_calculated)
                << std::setw(10) << std::internal << uint32_t (r.sum_expected)
                << std::endl;
    }

    return EXIT_SUCCESS;
}