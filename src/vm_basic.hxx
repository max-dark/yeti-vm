/// basic implementation of rv32 VM
#pragma once

#include <vm_base_types.hxx>
#include <vm_interface.hxx>
#include <vm_handler.hxx>
#include <vm_syscall.hxx>

namespace vm
{

/**
 * basic implementation of rv32 VM
 */
struct basic_vm: public vm_interface
{
    /// readonly memory
    using code_memory_t = std::vector<std::uint8_t>;
    /// rw(data) memory
    using data_memory_t = std::vector<std::uint8_t>;

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
    const void* get_ptr(address_t address) const;

    /// get pointer to memory
    [[nodiscard]]
    void* get_ptr(address_t address);

    /// set ro memory size
    void set_ro_size(size_t size);

    /// set rw memory size
    void set_rw_size(size_t size);

    /// set start of RO memory
    void set_ro_base(address_t base);

    /// set start of RW memory
    void set_rw_base(address_t base);

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
    void init_isa();

    /// resize memory to default values
    void init_memory();

    /// set memory size
    void init_memory(size_t code_size, size_t data_size);

    /// load program into ro memory
    bool set_program(const program_code_t& bin);

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

    registry opcodes;
    syscall_registry syscalls;

    /// registers container
    register_file registers{};
    code_memory_t code; // ro memory
    data_memory_t data; // rw memory
    /// RO memory offset
    address_t code_base = def_code_base;
    /// RW memory offset
    address_t data_base = def_data_base;

    /// running flag
    bool running = false;

    /// debug
    bool debugging = false;
};

} // namespace vm
