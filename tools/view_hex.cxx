#include <iostream>

#include <vm_utility.hxx>
#include <format>

std::string to_hex(const vm::hex_record& r)
{
    std::string result;
    result.reserve(r.data.size() * 2);

    for (auto byte: r.data)
    {
        result += std::format("{:02X}", byte);
    }

    return result;
}

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
        << std::setw(10) << std::internal << "offset "
        << std::setw(10) << std::left << "is valid"
        << std::setw(10) << std::left << "r.data"
        << std::endl;
    size_t idx = 1;
    for (auto& r : records)
    {
        std::cout
                << std::dec
                << std::setw(5) << std::left << idx++
                << std::setw(20) << std::left << r.get_type_name()
                << std::setw(10) << std::right << std::uint32_t(r.count)
                << std::setw(10) << std::right << std::hex << std::uint32_t(r.offset) << ' '
                << std::setw(10) << std::left << r.is_valid()
                << std::setw(10) << std::left << to_hex(r)
                << std::endl;
    }

    return EXIT_SUCCESS;
}