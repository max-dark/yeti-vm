#include <iostream>

#include "yeti-vm/vm_memory.hxx"
#include "yeti-vm/vm_utility.hxx"

using range = vm::memory_block::params;

int main()
{
    {
        range a{0, 100};
        range b{50, 150};
        range c{50, 50};

        vm::ensure(a.is_overlap(a), "!a.is_overlap(a)");
        vm::ensure(a.is_overlap(b), "!a.is_overlap(b)");
        vm::ensure(a.is_overlap(c), "!a.is_overlap(c)");
        vm::ensure(b.is_overlap(a), "!b.is_overlap(a)");
        vm::ensure(b.is_overlap(c), "!b.is_overlap(c)");
        vm::ensure(c.is_overlap(b), "!c.is_overlap(b)");
    }

    {
        range a{100, 100};

        vm::ensure(a.in_range(100), "should be in range: ");
        vm::ensure(a.in_range(199), "should be in range");
        vm::ensure(!a.in_range(  0), "should not be in range");
        vm::ensure(!a.in_range(250), "should not be in range");

        vm::ensure(a.in_range(100, 100), "100, 100: should be in range");
        vm::ensure(a.in_range(110,  50), "110,  50: should be in range");
        vm::ensure(a.in_range(110,  90), "110,  90: should be in range");

        vm::ensure(!a.in_range(10, 100), "10, 100: should not be in range");
        vm::ensure(!a.in_range(110, 100), "110, 100: should not be in range");
    }

    {
        vm::memory_management_unit mmu;

        vm::ensure(!mmu.add_block<vm::generic_memory>(100, 0), "empty blocks is not allowed");

        vm::ensure(mmu.add_block<vm::generic_memory>(100, 100), "should return true");
        vm::ensure(mmu.add_block<vm::generic_memory>(250, 100), "should return true");

        vm::ensure(!mmu.add_block<vm::generic_memory>( 90, 100), "add overlapped region :should return false");
        vm::ensure(!mmu.add_block<vm::generic_memory>(190, 100), "add overlapped region :should return false");
        vm::ensure(!mmu.add_block<vm::generic_memory>(100, 100), "add overlapped region :should return false");
        vm::ensure(!mmu.add_block<vm::generic_memory>(250, 100), "add overlapped region :should return false");

        auto test_find = [&mmu](auto start, auto size, bool expected)
        {
            auto ptr = mmu.find_block(start, size);
            bool is_null = nullptr == ptr;
            vm::ensure(is_null == expected, expected ? "should be null" : "should be not null");
        };

        test_find(0, 10, true);
        test_find(100, 110, true);
        test_find(110, 100, true);
        test_find(200, 100, true);

        test_find(100, 100, false);
        test_find(100,  10, false);
        test_find(110,  10, false);
        test_find(110,  90, false);

        auto test_set_get = [&mmu](auto start, auto value, bool expectedNull = false)
        {
            using value_type = decltype(value);

            auto* load = mmu.find_block(start, sizeof(value));
            auto* store = mmu.find_block(start, sizeof(value));
            bool a_is_null = nullptr == load;
            bool b_is_null = nullptr == store;
            vm::ensure(a_is_null == expectedNull, expectedNull ? "a: should be null" : "a: should be not null");
            vm::ensure(b_is_null == expectedNull, expectedNull ? "b: should be null" : "b: should be not null");
            vm::ensure(load == store, "a & b should be same");

            if (expectedNull) return;

            auto load_ptr = load->template get_ro_ptr<value_type>(start); // Oo
            auto store_ptr = store->template get_rw_ptr<value_type>(start); // oO
            vm::ensure(load_ptr != nullptr, "ptr: should be not null");
            vm::ensure(load_ptr == store_ptr, "ptr: a & b should be same");
            vm::ensure(*load_ptr == *store_ptr, "value: a & b should be same");
        };

        test_set_get(100, int8_t{0x01});
        test_set_get(110, uint8_t{0x0f});
        test_set_get(120, int16_t{0xa1});
        test_set_get(130, uint16_t{0xb1});
        test_set_get(140, int16_t{0xc1});
        test_set_get(150, uint16_t{0xd1});
    }

    std::cout << "ok" << std::endl;
    return EXIT_SUCCESS;
}