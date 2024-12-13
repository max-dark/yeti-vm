#include <iostream>

#include <vm_utility.hxx>
#include <format>


inline void ensure(bool ok, const std::string& message)
{
    if (!ok) [[unlikely]]
    {
        throw std::domain_error(message);
    }
}

void test_checksum(std::string_view hex, uint8_t expected)
{
    ensure(hex.size() % 2 == 0, "input size should be even");
    auto bytes = vm::from_hex(hex);
    ensure(bytes.size()*2 == hex.size(), "result size is invalid");
    auto sum = vm::hex_checksum(bytes.data(), bytes.data() + bytes.size());
    ensure(sum == expected, std::format("wrong result = {:02x}, expected = {:02x}, data = {}", sum, expected, hex));
}

int main(int argc, char** argv)
{
    test_checksum("00000001", 0xFF);
    test_checksum("020000021200", 0xEA);
    test_checksum("0400000300003800", 0xC1);
    test_checksum("020000040800", 0xF2);
    test_checksum("04000005000000CD", 0x2A);
    test_checksum("0B0010006164647265737320676170", 0xA7);
    return EXIT_SUCCESS;
}