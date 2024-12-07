#pragma once
#include <vm_base_types.hxx>
#include <exception>
#include <stdexcept>

namespace vm
{

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
