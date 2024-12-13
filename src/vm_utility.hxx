#pragma once
#include <vm_base_types.hxx>
#include <exception>
#include <stdexcept>

namespace vm
{

namespace fs = std::filesystem;

/**
 * load binary file
 * @param programFile
 * @return
 */
std::optional<vm::program_code_t> load_program(const fs::path& programFile);

/**
 * @see https://en.wikipedia.org/wiki/Intel_HEX
 *
 * IntelHEX file record
 *
 * format: ':' <count> <address> <type> [data] <checksum>
 */
struct hex_record
{
    /// known record types
    enum record_type : uint8_t
    {
        /// data
        HEX_DATA = 0x00,
        /// end of file
        HEX_EOF = 0x01,
        /// extended segment address
        HEX_SEGMENT_EXTEND = 0x02,
        /// start segment address
        HEX_SEGMENT_START = 0x03,
        /// extended linear address
        HEX_LINEAR_EXTEND = 0x04,
        /// start linear address
        HEX_LINEAR_START = 0x05,
        /// unknown record type
        HEX_UNKNOWN,
    };
    uint8_t count;
    uint8_t type;
    uint8_t sum_expected;
    uint8_t sum_actual;
    uint16_t offset;

    /// convert raw type to record_type
    [[nodiscard]]
    record_type get_type() const;

    ///@return true if it is EOF record
    [[nodiscard]]
    bool is_eof() const;

    [[nodiscard]]
    std::string_view get_type_name() const;

    ///@return true if checksums and sizes is same
    [[nodiscard]]
    bool is_valid() const;

    ///@return address extend
    [[nodiscard]]
    uint32_t get_extend() const;
    ///@return start address
    [[nodiscard]]
    uint32_t get_start() const;

    ///@return true if it is DATA record
    [[nodiscard]]
    bool is_data() const;

    ///@return true if it is HEX_SEGMENT_START record
    [[nodiscard]]
    bool is_start_segment() const;
    ///@return true if it is HEX_SEGMENT_EXTEND record
    [[nodiscard]]
    bool is_extend_segment() const;

    ///@return true if it is HEX_LINEAR_START record
    [[nodiscard]]
    bool is_start_linear() const;
    ///@return true if it is HEX_LINEAR_EXTEND record
    [[nodiscard]]
    bool is_extend_linear() const;

    /// record payload data
    std::vector<uint8_t> data;
};

/// records from hex file
using hex_file = std::vector<hex_record>;

/**
 * parse hex digit
 * @param hex hex digit
 * @return 0xff if char is not hex
 */
std::uint8_t from_hex(char hex);

/**
 * convert hex string to byte array
 * @param hex hex string
 * @return byte array
 */
std::vector<uint8_t> from_hex(std::string_view hex);

/**
 * convert hex string to byte array
 * @param hex hex string
 * @return byte array
 */
inline std::vector<uint8_t> from_hex(const std::string& hex)
{
    return from_hex(std::string_view{hex});
}

/// calculate checksum for hex file record
uint8_t hex_checksum(const uint8_t* first, const uint8_t* last);

/**
 * parse hex file
 * @param hexFile file to parse
 * @return list of parsed records
 */
std::optional<hex_file> parse_hex(const fs::path& hexFile);

hex_record parse_hex_record(std::string_view line);

/// interpret value as signed
inline signed_t to_signed(unsigned_t value)
{
    return std::bit_cast<signed_t>(value);
}

/// interpret value as unsigned
inline unsigned_t to_unsigned(signed_t value)
{
    return std::bit_cast<unsigned_t>(value);
}

inline void ensure(bool ok, std::string_view message)
{
    if (!ok) [[unlikely]]
    {
        throw std::domain_error(std::string{message});
    }
}

} // namespace vm
