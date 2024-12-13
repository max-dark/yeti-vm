/// basic implementation of rv32 VM
#pragma once

#include <vm_base_types.hxx>
#include <vm_interface.hxx>
#include <vm_handler.hxx>
#include <vm_syscall.hxx>
#include <vm_memory.hxx>
#include <vm_utility.hxx>

#include <exception>
#include <stdexcept>

namespace vm
{

/**
 * basic implementation of rv32 VM
 */
struct basic_vm: public vm_interface
{
    struct unknown_instruction: std::domain_error {
        explicit unknown_instruction(const std::string& message): std::domain_error{message} {}
    };
    struct unknown_syscall: std::domain_error {
        explicit unknown_syscall(const std::string& message): std::domain_error{message} {}
    };
    struct code_access_error: std::domain_error {
        explicit code_access_error(const std::string& message): std::domain_error{message} {}
    };
    struct data_access_error: std::domain_error {
        explicit data_access_error(const std::string& message): std::domain_error{message} {}
    };

    /// stop VM
    void halt() final;

    /// jump to absolute address
    void jump_abs(address_t dest) override;

    /// jump by offset
    void jump_to(offset_t value) override;

    /// conditional jump by offset
    void jump_if(bool condition, offset_t value) override;

    /// conditional jump
    void jump_if_abs(bool condition, address_t value) override;

    /// system call
    void syscall() override;

    /// debug break
    void debug() override;

    /// CSR operations
    void control() override;

    /// memory barrier
    void barrier() override;

    /// read(load) value from memory
    /// size should be eq 1,2 or 4
    void read_memory(address_t from, uint8_t size, register_t& value) override;

    /// write(store) value to memory
    /// size should be eq 1,2 or 4
    void write_memory(address_t from, uint8_t size, register_t value) override;

    /// set register value
    void set_register(register_no r, register_t value) override;

    /// get register value
    [[nodiscard]]
    register_t get_register(register_no r) const override;

    /// get PC register value
    [[nodiscard]]
    register_t get_pc() const override;
    /// set PC register value
    void set_pc(register_t value);
    /// increment PC value
    void inc_pc();

    /// get pointer to memory
    [[nodiscard]]
    memory_block *get_ptr_rw(address_t address, uint8_t size);

    /// get pointer to memory
    [[nodiscard]]
    const memory_block *get_ptr_ro(address_t address, uint8_t size) const;

    /// set ro memory size
    [[nodiscard]]
    bool set_ro_size(size_t size);

    /// set rw memory size
    [[nodiscard]]
    bool set_rw_size(size_t size);

    /// set start of RO memory
    [[nodiscard]]
    bool set_ro_base(address_t base);

    /// set start of RW memory
    [[nodiscard]]
    bool set_rw_base(address_t base);

    /// single emulation step
    void run_step();

    /// run emulation cycle
    void run();

    /// init VM: clear memory/registers
    void start();

    /// VM is running
    [[nodiscard]]
    bool is_running() const;

    /// enable RV32I + RV32M extension
    [[nodiscard]]
    bool init_isa();

    /// resize memory to default values
    [[nodiscard]]
    bool init_memory();

    /// set memory size
    [[nodiscard]]
    bool init_memory(size_t code_size, size_t data_size);

    [[nodiscard]]
    bool add_memory(address_t address, size_t size);

    [[nodiscard]]
    bool add_memory(memory_block::ptr ptr);

    [[nodiscard]]
    bool add_code_block(address_t address, size_t size);

    [[nodiscard]]
    bool add_data_block(address_t address, size_t size);

    /// load program into ro memory
    [[nodiscard]]
    bool set_program(const program_code_t &bin, address_t pc_value);

    /// load program into ro memory
    [[nodiscard]]
    bool set_program(const hex_file &hex);

    [[nodiscard]]
    bool init_pc(address_t address);

    /// default start of RO memory block
    static constexpr address_t def_code_base = 0;
    /// default start of RW memory block
    static constexpr address_t def_data_base = 4 * 1024 * 1024;

    /// default size of RO memory block
    static constexpr size_t def_code_size = 4 * 1024 * 1024;

    /// default size of RW memory block
    static constexpr size_t def_data_size = 4 * 1024 * 1024;

    /// check that program can be started
    [[nodiscard]]
    bool is_initialized() const;

    [[nodiscard]]
    bool is_debugging_enabled() const;

    void enable_debugging(bool enable);

    syscall_registry& get_syscalls();
private:
    /// get pointer to current instruction
    [[nodiscard]]
    const opcode::OpcodeBase* get_current() const;

    using init_flags_t = std::uint8_t;
    enum InitFlag: init_flags_t
    {
        HAVE_CODE_BLOCK = 1 << 0,
        HAVE_DATA_BLOCK = 1 << 1,
        ISA_INITIALIZED = 1 << 2,
        PC_INITIALIZED  = 1 << 3,

        ALL_FLAGS_MASK
            = HAVE_DATA_BLOCK
            | HAVE_CODE_BLOCK
            | ISA_INITIALIZED
            | PC_INITIALIZED
            ,
    };

    init_flags_t initFlags = 0;

    [[nodiscard]]
    bool is_flag_set(InitFlag flag) const;
    void set_flag(InitFlag flag);
    void clear_flag(InitFlag flag);

    [[nodiscard]]
    bool have_code_block() const;
    [[nodiscard]]
    bool have_data_block() const;

    registry opcodes;
    syscall_registry syscalls;
    memory_management_unit mmu;

    /// registers container
    register_file registers{};

    size_t ro_size = def_code_size;
    size_t rw_size = def_data_size;

    /// RO memory offset
    address_t code_base = def_code_base;
    /// RW memory offset
    address_t data_base = def_data_base;

    address_t initial_pc = 0;

    /// running flag
    bool running = false;

    /// debug
    bool debugging = false;
};

} // namespace vm
