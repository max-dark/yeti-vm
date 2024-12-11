#include <iostream>

#include <vm_memory.hxx>
#include <vm_utility.hxx>

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
            vm::ensure(is_null == expected, expected ? "should be null" : "should not be null");
        };

        test_find(0, 10, true);
        test_find(100, 110, true);
        test_find(110, 100, true);
        test_find(200, 100, true);

        test_find(100, 100, false);
        test_find(100,  10, false);
        test_find(110,  10, false);
        test_find(110,  90, false);
    }

    std::cout << "ok" << std::endl;
    return EXIT_SUCCESS;
}