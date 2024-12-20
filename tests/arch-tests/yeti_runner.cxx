#include <yeti-vm/vm_basic.hxx>

#include <iostream>
#include <cstring>

namespace vm::yeti_runner
{
struct Runner: protected vm::basic_vm
{
    bool initProgram(int argc, char ** argv)
    {
        if (argc != 2) return false;
        bool isa_ok = init_isa();
        bool mem_ok = init_memory();
        mem_ok = mem_ok && add_memory(std::make_shared<DeviceMemory>(this));

        bool init_ok = isa_ok && mem_ok;
        init_ok = init_ok && initSysCalls();

        auto code = vm::parse_hex(argv[1]);
        init_ok = init_ok && code.has_value();
        init_ok = init_ok && set_program(code.value());

        return init_ok;
    }
    bool initSysCalls()
    {
        using call = vm::syscall_functor;
        auto& sys = get_syscalls();
        bool ok = sys.register_handler(
                call::create(10, "exit"
                             , [this](vm::vm_interface* m)
                             { return do_exit(m); }));

        return ok;
    }
    void do_exit(vm::vm_interface*)
    {
        basic_vm::halt();
    }

    int exec()
    {
        start();
        try
        {
            run();
        }
        catch (std::exception& e)
        {
            std::cerr << std::endl << "Exception" << e.what() << std::endl;
            dump_state(std::cerr);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
protected:
    void debug() override
    {
        dump_state(std::cerr);
        halt();
        return basic_vm::debug();
    }
protected:
    struct DeviceMemory final: public vm::memory_block
    {
        explicit DeviceMemory(Runner* runner)
            : vm::memory_block{def_data_base + def_data_size, def_data_size}
            , runner{runner} {}

        [[nodiscard]]
        bool load(address_type address, void *dest, size_type size) const final
        {
            std::memset(dest, 0, size);
            return true;
        }

        [[nodiscard]]
        bool store(memory_block::address_type address, const void *source, memory_block::size_type size) final
        {
            return true;
        }

    protected:
        [[nodiscard]]
        const void *get_ro(address_type address, size_type size) const final
        {
            return nullptr;
        }

        [[nodiscard]]
        void *get_rw(address_type address, size_type size) final
        {
            return nullptr;
        }
    private:
        Runner* runner = nullptr;
    };
};
} // vm::yeti_runner

int main(int argc, char ** argv)
{
    vm::yeti_runner::Runner yetiVM;
    if (!yetiVM.initProgram(argc, argv)) return EXIT_FAILURE;
    return yetiVM.exec();
}
