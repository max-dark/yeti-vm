#include "vm_memory.hxx"

namespace vm
{

memory_block::~memory_block() = default;

bool memory_management_unit::add_block(memory_management_unit::value_type block)
{
    auto [it, ok] = memory.try_emplace(block->get_params(), block);
    return ok;
}

memory_management_unit::pointer
memory_management_unit::find_block(memory_block::address_type address, memory_block::size_type size) const
{
    for (auto& [params, block]: memory)
    {
        if (params.in_range(address, size))
        {
            return block.get();
        }
    }

    return nullptr;
}

memory_block::memory_block(memory_block::address_type address, memory_block::size_type size)
    : block_params(address, size)
{}

const memory_block::params &memory_block::get_params() const
{
    return block_params;
}

bool generic_memory::load(memory_block::address_type address, void *dest, memory_block::size_type size) const
{
    if (!get_params().in_range(address, size))
        return false;
    storage_size start_offset = address - get_params().block_start;
    auto dest_ptr = static_cast<storage_type::value_type*>(dest);
    std::copy_n(data.data() + start_offset, size, dest_ptr);
    return true;
}

bool generic_memory::store(memory_block::address_type address, const void *source, memory_block::size_type size)
{
    if (!get_params().in_range(address, size))
        return false;
    storage_size start_offset = address - get_params().block_start;
    auto src_ptr = static_cast<storage_type::const_pointer>(source);
    std::copy_n(src_ptr, size, data.data() + start_offset);
    return true;
}

generic_memory::generic_memory(memory_block::address_type address, memory_block::size_type size)
        : memory_block(address, size)
        , data(size, 0)
{}

bool memory_block::params::is_overlap(memory_block::address_type address, memory_block::size_type size) const noexcept
{
    if (address < block_start)
    {
        auto end = address + size;
        return end >= block_start;
    }
    return in_range(address);
}

bool memory_block::params::in_range(memory_block::address_type address) const noexcept
{
    return (address >= block_start) && ((address - block_start) < block_size);
}

bool memory_block::params::in_range(memory_block::address_type address, memory_block::size_type size) const noexcept
{
    if (in_range(address))
    {
        auto start = address - block_start;
        return (start + size) <= block_size;
    }
    return false;
}
}//namespace vm
