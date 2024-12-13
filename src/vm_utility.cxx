#include <valarray>
#include "vm_utility.hxx"

namespace vm
{

std::optional<vm::program_code_t> load_program(const fs::path &programFile)
{
    program_code_t data;

    if (fs::is_regular_file(programFile))
    {
        auto sz = static_cast<std::streamsize>(fs::file_size(programFile));
        data.resize(sz);
        std::ifstream bin{programFile, std::ios::binary};

        if (bin.is_open())
        {
            bin.read(reinterpret_cast<char *>(data.data()), sz);
            if (bin.good() || bin.eof())
                return data;
        }
    }

    return std::nullopt;
}

std::optional<hex_file> parse_hex(const fs::path &hexFile)
{
    hex_file result;

    if (fs::is_regular_file(hexFile))
    {
        std::ifstream hex{hexFile};

        if (hex.is_open())
        {
            std::string tmp;

            while (std::getline(hex, tmp))
            {
                std::string_view line = tmp;
                constexpr size_t min_record_size = 1 + (1 + 2 + 1 + 1) * 2; // ':'<count><address><type><checksum>
                size_t record_size = min_record_size;
                auto pos = line.find(':');
                if (pos > line.size())
                {
                    continue;
                }
                if (record_size > line.size())
                {
                    return std::nullopt;
                }
                line = line.substr(pos);
                record_size += (from_hex(line[1]) << 4u) | from_hex(line[2]);

                if (record_size > line.size())
                {
                    return std::nullopt;
                }
                auto& record = result.emplace_back();
                record = parse_hex_record(line.substr(1)); // Note: ignore garbage at eol

                if (record.is_eof()) // skip data after EOF record
                    break;
            }
            return result;
        }
    }
    return std::nullopt;
}

hex_record parse_hex_record(std::string_view line)
{
    hex_record record;
    const auto bytes = from_hex(line);

    record.sum_actual = hex_checksum(&bytes.front(), &bytes.back());
    record.count = bytes[0];
    record.offset = (bytes[1] << 8u) | bytes[2]; // should int16_t be used?
    record.type = bytes[3];
    record.data.resize(record.count);
    size_t idx = 4;
    for (auto& byte: record.data)
    {
        byte = bytes[idx];
        ++idx;
    }
    record.sum_expected = bytes.back();

    return record;
}

static constexpr bool in_range(char c, char lo, char hi)
{
    return (lo <= c) && (c <= hi);
}
static constexpr std::uint8_t from_hex_impl(char hex)
{
    if (in_range(hex, '0', '9')) return hex - '0';
    if (in_range(hex, 'a', 'f')) return hex - 'a' + 10;
    if (in_range(hex, 'A', 'F')) return hex - 'A' + 10;
    return 0xff;
}

static_assert(from_hex_impl('0') == 0);
static_assert(from_hex_impl('9') == 9);
static_assert(from_hex_impl('a') == 10);
static_assert(from_hex_impl('f') == 15);
static_assert(from_hex_impl('A') == 10);
static_assert(from_hex_impl('F') == 15);
static_assert(from_hex_impl('x') == 0xff);

std::uint8_t from_hex(char hex)
{
    return from_hex_impl(hex);
}

std::vector<uint8_t> from_hex(std::string_view hex)
{
    std::vector<uint8_t> result;
    size_t size = hex.size() / 2;
    auto ptr = hex.data();
    result.resize(size);
    for (auto& byte : result)
    {
        auto hi = from_hex(ptr[0]);
        auto lo  = from_hex(ptr[1]);
        byte = (hi << 4u) | lo; // TODO: validate return values
        ptr += 2;
    }
    return result;
}

uint8_t hex_checksum(const uint8_t* first, const uint8_t* last)
{
    uint8_t sum = 0;

    while (first != last)
    {
        sum += *first;
        ++first;
    }
    sum = ~sum;

    return 1 + sum;
}

hex_record::record_type hex_record::get_type() const
{
    if (type < record_type::HEX_UNKNOWN)
    {
        return static_cast<record_type>(type);
    }
    return hex_record::HEX_UNKNOWN;
}

bool hex_record::is_valid() const
{
    return (sum_actual == sum_expected) && (count == data.size());
}

std::string_view hex_record::get_type_name() const
{
    switch (get_type())
    {
        case HEX_DATA: return "HEX_DATA";
        case HEX_EOF: return "HEX_EOF";
        case HEX_SEGMENT_EXTEND: return "HEX_SEGMENT_EXTEND";
        case HEX_SEGMENT_START: return "HEX_SEGMENT_START";
        case HEX_LINEAR_EXTEND: return "HEX_LINEAR_EXTEND";
        case HEX_LINEAR_START: return "HEX_LINEAR_START";
        case HEX_UNKNOWN: return "HEX_UNKNOWN";
    }
    return "IMPOSSIBLE";
}

uint32_t hex_record::get_extend() const
{
    if (data.size() == 2) [[likely]]
    {
        uint32_t value = ((data[0] << 8u) | (data[1] << 0u));
        if (is_extend_segment()) return value <<  4u;
        if (is_extend_linear() ) return value << 16u;
    }
    return 0;
}

uint32_t hex_record::get_start() const
{
    if (data.size() == 4) [[likely]]
    {
        if (is_start_linear() | is_start_segment()) [[likely]]
        {
            uint32_t value =  ((data[0] << 24u) | (data[1] << 16u) | (data[2] << 8u) | (data[3] << 0u));
            return value;
        }
    }
    return 0;
}

bool hex_record::is_data() const
{
    return get_type() == record_type::HEX_DATA;
}

bool hex_record::is_start_segment() const
{
    return type == HEX_SEGMENT_START;
}

bool hex_record::is_extend_segment() const
{
    return type == HEX_SEGMENT_EXTEND;
}

bool hex_record::is_eof() const
{
    return get_type() == record_type::HEX_EOF;
}

bool hex_record::is_start_linear() const
{
    return type == HEX_LINEAR_START;
}

bool hex_record::is_extend_linear() const
{
    return type == HEX_LINEAR_EXTEND;
}

} // namespace vm
