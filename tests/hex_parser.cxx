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

using record_type = vm::hex_record::record_type;

void test_parse_record(std::string_view hex, uint8_t expectedSum, uint8_t expectedType)
{
    ensure(hex.size() % 2 == 0, "input size should be even");
    const auto record = vm::parse_hex_record(hex);
    ensure(record.type == expectedType, std::format("wrong type = {:02x}, expected = {:02x}, data = {}", record.type, expectedType, hex));
    ensure(record.sum_calculated == expectedSum, std::format("wrong checksum = {:02x}, expected = {:02x}, data = {}", record.sum_calculated, expectedSum, hex));
}

int main(int argc, char** argv)
{
    test_parse_record("00000001FF", 0xFF, record_type::HEX_EOF);
    test_parse_record("020000021200EA", 0xEA, record_type::HEX_SEGMENT_EXTEND);
    test_parse_record("0400000300003800C1", 0xC1, record_type::HEX_SEGMENT_START);
    test_parse_record("020000040800F2", 0xF2, record_type::HEX_LINEAR_EXTEND);
    test_parse_record("04000005000000CD2A", 0x2A, record_type::HEX_LINEAR_START);
    test_parse_record("0B0010006164647265737320676170A7", 0xA7, record_type::HEX_DATA);
    return EXIT_SUCCESS;
}