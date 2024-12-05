#pragma once

#include <vm_base_types.hxx>
#include <vm_interface.hxx>
#include <vm_handler.hxx>

namespace vm
{

struct basic_vm: public registry
        , public vm_interface
{
    using code_memory_t = std::vector<std::uint8_t>;
    using data_memory_t = std::vector<std::uint8_t>;

    void halt();

    void jump_abs(address_t dest) override;

    void jump_to(offset_t value) override;

    void jump_if(bool condition, offset_t value) override;

    void jump_if_abs(bool condition, address_t value) override;

    void syscall() override;

    void debug() override;

    void control() override;

    void barrier() override;

    void read_memory(address_t from, uint8_t size, register_t& value) override;

    void write_memory(address_t from, uint8_t size, register_t value) override;

    void set_register(register_no r, register_t value) override;

    [[nodiscard]]
    register_t get_register(register_no r) const override;

    [[nodiscard]]
    register_t get_pc() const override;
    void set_pc(register_t value);
    void inc_pc();

    [[nodiscard]]
    const void* get_ptr(address_t address) const;

    [[nodiscard]]
    void* get_ptr(address_t address);

    void set_ro_size(size_t size);
    void set_rw_size(size_t size);

    void set_ro_base(address_t base);
    void set_rw_base(address_t base);

    void run_step();

    void run();

    void start();

    [[nodiscard]]
    bool is_running() const;

    void init_memory();

    void init_memory(size_t code_size, size_t data_size);

    bool set_program(const program_code_t& bin);

    static constexpr address_t def_code_base = 0;
    static constexpr address_t def_data_base = 4 * 1024 * 1024;

    static constexpr size_t def_code_size = 4 * 1024 * 1024;
    static constexpr size_t def_data_size = 4 * 1024 * 1024;

    [[nodiscard]]
    bool is_initialized() const;
private:
    [[nodiscard]]
    const opcode::OpcodeBase* get_current() const;
    register_file registers{};
    code_memory_t code; // ro memory
    data_memory_t data; // rw memory
    address_t code_base = def_code_base;
    address_t data_base = def_data_base;

    bool running = false;
};

} // namespace vm
