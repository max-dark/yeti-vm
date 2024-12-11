#include <iostream>

#include <vm_memory.hxx>
#include <vm_utility.hxx>

using range = vm::memory_block::params;

int main()
{
    {
        range a{0, 100};
        range b{50, 150};

        vm::ensure(a.is_overlap(b), "!a.is_overlap(b)");
        vm::ensure(b.is_overlap(a), "!b.is_overlap(a)");

    }
    return EXIT_SUCCESS;
}