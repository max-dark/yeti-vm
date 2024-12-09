#include "elf_loader.hxx"

#include <algorithm>
#include <ranges>

#include <elf.h>

namespace elf_loader
{

namespace detail
{

struct elf_parser: public executable
{

    explicit elf_parser(binary_view bin): view(bin) {}
    explicit elf_parser(const binary_data& bin): view(bin.data(), bin.size()) {}

    [[nodiscard]]
    const void* raw_at(size_type pos) const override
    {
        return view.data() + pos;
    }

    [[nodiscard]]
    binary_view get_identify() const override
    {
        return view.subspan(0, EI_NIDENT);
    }

    [[nodiscard]]
    binary_view get_magic() const override
    {
        return view.subspan(0, SELFMAG);
    }
private:
    binary_view view;
};

} // namespace detail

executable::~executable() = default;

executable::ptr executable::parse(const binary_data &bin)
{
    auto ptr = std::make_shared<detail::elf_parser>(bin);
    return ptr;
}

bool executable::is_elf() const
{
    static byte ELF_MAGIC[SELFMAG] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3 };
    auto file_magic = get_magic();
    return std::ranges::equal(ELF_MAGIC, file_magic);
}

bool executable::is_32bit() const
{
    auto ident = get_identify();
    return ELFCLASS32 == ident[EI_CLASS];
}

bool executable::is_64bit() const
{
    auto ident = get<byte>(0);
    return ELFCLASS64 == ident[EI_CLASS];
}

word executable::get_encoding() const
{
    auto ident = get<byte>(0);
    return ident[EI_DATA];
}

word executable::get_os_abi() const
{
    auto ident = get<byte>(0);
    return ident[EI_OSABI];
}

word executable::get_version() const
{
    auto ident = get<byte>(0);
    return ident[EI_VERSION];
}

word executable::get_abi_version() const
{
    auto ident = get<byte>(0);
    return ident[EI_ABIVERSION];
}

bool executable::is_le() const
{
    return get_encoding() == ELFDATA2LSB;
}

bool executable::is_be() const
{
    return get_encoding() == ELFDATA2MSB;
}

std::optional<executable::elf32> executable::to_32bit() const
{
    if (!is_32bit()) return std::nullopt;
    return elf_loader::executable::elf32(this);
}

std::optional<executable::elf64> executable::to_64bit() const
{
    if (!is_64bit()) return std::nullopt;
    return elf_loader::executable::elf64(this);
}

} //namespace elf_loader
