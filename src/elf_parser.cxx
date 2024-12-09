#include <iostream>
#include <elf_loader.hxx>

namespace fs = std::filesystem;
using program_code_t = elf_loader::binary_data;

std::optional<program_code_t> load_program(const fs::path &programFile);

static const auto field_sz = std::setw(15);

template<typename Type>
void print_hex(std::string_view name, Type value)
{
    std::cout
        << field_sz << name << ' '
        << std::hex << std::setw(sizeof(value) * 2) << std::setfill('0') << value
        << std::setfill(' ') << std::endl;
}
template<typename Type>
void print_raw(std::string_view name, Type value)
{
    std::cout << field_sz << name << ' ' << std::dec << value << std::endl;
}

template<typename ElfFile>
void print_file_header(ElfFile exe)
{
    auto header = exe.fileHeader();
    std::cout << "File header:" << std::endl;
    print_raw("e_type", header->e_type);
    print_raw("e_machine", header->e_machine);
    print_raw("e_version", header->e_version);
    print_hex("e_entry", header->e_entry);

    print_raw("program h off", header->e_phoff);
    print_raw("section h off", header->e_shoff);

    print_hex("e_flags", header->e_flags);
    print_raw("e_ehsize", header->e_ehsize);
    print_raw("fh size", sizeof(*header));

    print_raw("e_phentsize", header->e_phentsize);
    print_raw("e_phnum", header->e_phnum);

    print_raw("e_shentsize", header->e_shentsize);
    print_raw("e_shnum", header->e_shnum);

    print_raw("e_shstrndx", header->e_shstrndx);
}

int main(int argc, char** argv)
{
    using exe = elf_loader::executable;

    auto bin = load_program(argv[argc > 1]);

    if (bin->empty())
    {
        std::cerr << "Unable load file" << std::endl;
        return EXIT_FAILURE;
    }

    const auto& code = bin.value();
    auto parser = exe::parse(code);

    std::cout << field_sz << "elf " << std::boolalpha << parser->is_elf() << std::endl;
    std::cout << field_sz << "32 bit " << std::boolalpha << parser->is_32bit() << std::endl;
    std::cout << field_sz << "64 bit " << std::boolalpha << parser->is_64bit() << std::endl;

    std::cout << field_sz << "version " << std::hex << parser->get_version() << std::endl;
    std::cout << field_sz << "encoding " << std::hex << parser->get_encoding() << std::endl;
    std::cout << field_sz << "os_abi " << std::hex << parser->get_os_abi() << std::endl;
    std::cout << field_sz << "abi_version " << std::hex << parser->get_abi_version() << std::endl;

    if (auto exe32 = parser->to_32bit())
    {
        print_file_header(exe32.value());
    }
    else if (auto exe64 = parser->to_64bit())
    {
        print_file_header(exe64.value());
    }
    else
    {
        std::cout << "unknown file format";
    }

    return EXIT_SUCCESS;
}

std::optional<program_code_t> load_program(const fs::path &programFile)
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