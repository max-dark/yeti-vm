#pragma once

#include <vm_stdlib.hxx>

#include <elf.h>

namespace elf_loader
{
using byte = std::uint8_t;
using half = std::uint16_t;
using word = std::uint32_t;
using dword = std::uint64_t;

using binary_data = std::vector<byte>;
using binary_view = std::span<const binary_data::value_type>;

struct executable
{
    using ptr = std::shared_ptr<executable>;
    using size_type = std::size_t;

    template
    <
        typename FileHeaderT,
        typename ProgramHeaderT,
        typename SectionHeaderT
    >
    struct elf
    {
        using FileHeader = const FileHeaderT;
        using ProgramHeader = const ProgramHeaderT;
        using SectionHeader = const SectionHeaderT;

        auto fileHeader() const
        {
            return exe->get<FileHeader>(0);
        }

        auto fileType() const
        {
            return fileHeader()->e_type;
        }

        explicit elf(const executable* exe): exe{exe} {}
    private:
        const executable* exe = nullptr;
    };

    using elf32 = elf<Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr>;
    using elf64 = elf<Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr>;

    [[nodiscard]]
    std::optional<elf32> to_32bit() const;
    [[nodiscard]]
    std::optional<elf64> to_64bit() const;

    [[nodiscard]]
    virtual binary_view get_identify() const = 0;

    [[nodiscard]]
    virtual binary_view get_magic() const = 0;

    [[nodiscard]]
    bool is_elf() const;

    [[nodiscard]]
    bool is_32bit() const;
    [[nodiscard]]
    bool is_64bit() const;

    [[nodiscard]]
    word get_encoding() const;

    [[nodiscard]]
    bool is_le() const;
    [[nodiscard]]
    bool is_be() const;

    [[nodiscard]]
    word get_version() const;

    [[nodiscard]]
    word get_abi_version() const;

    [[nodiscard]]
    word get_os_abi() const;

    static executable::ptr parse(const binary_data& bin);
    virtual ~executable();

protected:
    executable() = default;

    [[nodiscard]]
    virtual const void* raw_at(size_type pos) const = 0;

    template<typename Type>
    [[nodiscard]]
    const Type* get(size_type pos) const
    {
        return static_cast<const Type*>(raw_at(pos));
    }
};

} //namespace elf_loader
