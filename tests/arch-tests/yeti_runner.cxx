#include <yeti-vm/vm_basic.hxx>

#include <iostream>
#include <cstring>

namespace vm::yeti_runner
{
struct Runner: protected vm::basic_vm
{
    bool initProgram(int testIdx, char ** argv)
    {
        bool isa_ok = init_isa();
        bool mem_ok = init_memory();
        mem_ok = mem_ok && add_memory(std::make_shared<DeviceMemory>(this));

        bool init_ok = isa_ok && mem_ok;
        init_ok = init_ok && initSysCalls();

        auto code = vm::parse_hex(argv[testIdx]);
        init_ok = init_ok && code.has_value();
        init_ok = init_ok && set_program(code.value());

        return init_ok;
    }
    bool initSysCalls()
    {
        syscall_should_throw(false);
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

    bool exec(bool debug = false)
    {
        enable_debugging(debug);
        start();
        try
        {
            run();
        }
        catch (std::exception& e)
        {
            std::cerr << std::endl << "Exception: " << e.what() << std::endl;
            dump_state(std::cerr);
            return false;
        }
        return !set_dev; // no failures
    }
protected:
    void debug() override
    {
        if (set_dev)
        {
            dump_state(std::cerr);
            auto fill_c = std::cerr.fill();
            std::cerr << std::dec;
            std::cerr << "set_dev == true " << std::endl;
            std::cerr << "DEV MEM: " << std::endl;
            for(auto v: dev_mem)
            {
                std::cerr << "\t" << std::hex << std::setfill('0') << std::setw(8) << v << std::endl;
            }
            std::cerr << "\t:DEV MEM" << std::endl;
            std::cerr << std::dec << std::setfill(fill_c);
            halt();
        }
        return basic_vm::debug();
    }
    void assert_set(uint32_t idx, uint32_t v)
    {
        dev_mem[idx] = v;
        set_dev = true;
    }
    bool set_dev = false;
    std::array<uint32_t, 4> dev_mem{};
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
            runner->set_dev = true;
            if (size != 4) return false;
            auto offset = (address - get_start_address()) / 4;
            if (offset >= runner->dev_mem.size()) return false;
            runner->assert_set(offset, *reinterpret_cast<const uint32_t*>(source));
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
    int numFails = 0;
    for (int testIdx = 1; testIdx < argc; ++testIdx)
    {
        vm::yeti_runner::Runner yetiVM;
        if (!yetiVM.initProgram(testIdx, argv))
        {
            std::cerr << "Unable init: " << std::dec << testIdx << " " << argv[testIdx] << std::endl;
            return EXIT_FAILURE;
        }
        if (!yetiVM.exec(argc == 2)) // single file - enable debug output
        {
            std::cerr << "Fail: " << std::dec << testIdx << " " << argv[testIdx] << std::endl;
            ++numFails;
        }
    }
    return numFails;
}
