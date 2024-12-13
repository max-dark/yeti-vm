#include <iostream>

#include <vm_utility.hxx>
#include <format>
#include <sstream>


inline void ensure(bool ok, const std::string& message)
{
    if (!ok) [[unlikely]]
    {
        throw std::domain_error(message);
    }
}

using record_type = vm::hex_record::record_type;

void test_parse_record(std::string_view hex, uint8_t expectedSum, uint8_t expectedType, uint8_t expectedCount)
{
    ensure(hex.size() % 2 == 0, "input size should be even");
    const auto record = vm::parse_hex_record(hex);
    ensure(record.type == expectedType, std::format("wrong type = {:02x}, expected = {:02x}, data = {}", record.type, expectedType, hex));
    ensure(record.sum_actual == expectedSum, std::format("wrong checksum = {:02x}, expected = {:02x}, data = {}", record.sum_actual, expectedSum, hex));
    ensure(record.sum_expected == expectedSum, std::format("wrong checksum = {:02x}, expected = {:02x}, data = {}", record.sum_expected, expectedSum, hex));

    ensure(record.count == expectedCount, std::format("wrong count = {:02x}, expected = {:02x}, data = {}", record.count, expectedCount, hex));
    ensure(record.data.size() == expectedCount, std::format("wrong data.size = {:02x}, expected = {:02x}, data = {}", record.data.size(), expectedCount, hex));
}

void test_parser()
{
    std::stringstream stream;

    stream <<
R"(
:10010000214601360121470136007EFE09D2190140
:100110002146017E17C20001FF5F16002148011928
:10012000194E79234623965778239EDA3F01B2CAA7
:100130003F0156702B5E712B722B732146013421C7
:00000001FF
)"
    ;
    // try parse example form Wikipedia(https://en.wikipedia.org/wiki/Intel_HEX)
    auto result = vm::parse_hex(stream);

    ensure(result.has_value(), "should return parsed data");
    auto &data = result.value();
    ensure(data.size() == 5, "should size() == 5");
    ensure(data.front().is_data(), "should front().is_data() == true");
    ensure(data.back().is_eof(), "should back().is_eof() == true");
}

int main(int argc, char** argv)
{
    test_parse_record("00000001FF", 0xFF, record_type::HEX_EOF, 0x00);
    test_parse_record("0A03F800696E7075742E747874003D", 0x3D, record_type::HEX_DATA, 0x0A);
    test_parse_record("020000021200EA", 0xEA, record_type::HEX_SEGMENT_EXTEND, 0x02);
    test_parse_record("0400000300003800C1", 0xC1, record_type::HEX_SEGMENT_START, 0x04);
    test_parse_record("020000040800F2", 0xF2, record_type::HEX_LINEAR_EXTEND, 0x02);
    test_parse_record("04000005000000CD2A", 0x2A, record_type::HEX_LINEAR_START, 0x04);
    test_parse_record("0B0010006164647265737320676170A7", 0xA7, record_type::HEX_DATA, 0x0B);

    test_parser();

    return EXIT_SUCCESS;
}