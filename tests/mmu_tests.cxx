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

    std::cout << "ok" << std::endl;
    return EXIT_SUCCESS;
}