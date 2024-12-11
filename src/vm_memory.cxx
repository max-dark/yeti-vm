#include "vm_memory.hxx"

namespace vm
{

memory_block::~memory_block() = default;

bool memory_management_unit::add_block(memory_management_unit::value_type block)
{
    if (block->get_size() == 0)
        return false;

    const auto& k = block->get_params();
    for (const auto& pair: memory)
    {
        if (pair.first.is_overlap(k))
        {
            return false;
        }
    }
    auto [it, ok] = memory.try_emplace(block->get_params(), block);
    return ok;
}

memory_management_unit::pointer
memory_management_unit::find_block(memory_block::address_type address, memory_block::size_type size) const
{
    for (const auto& pair: memory)
    {
        if (pair.first.in_range(address, size))
        {
            return pair.second.get();
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
    storage_size start_offset = get_params().offset(address);
    auto dest_ptr = static_cast<storage_type::value_type*>(dest);
    std::copy_n(data.data() + start_offset, size, dest_ptr);
    return true;
}

bool generic_memory::store(memory_block::address_type address, const void *source, memory_block::size_type size)
{
    if (!get_params().in_range(address, size))
        return false;
    storage_size start_offset = get_params().offset(address);
    auto src_ptr = static_cast<storage_type::const_pointer>(source);
    std::copy_n(src_ptr, size, data.data() + start_offset);
    return true;
}

generic_memory::generic_memory(memory_block::address_type address, memory_block::size_type size)
        : memory_block(address, size)
        , data(size, 0)
{}

const void *generic_memory::get_ro(memory_block::address_type address, memory_block::size_type size) const
{
    if (!get_params().in_range(address, size))
        return nullptr;
    return data.data() + get_params().offset(address);
}

void *generic_memory::get_rw(memory_block::address_type address, memory_block::size_type size)
{
    if (!get_params().in_range(address, size))
        return nullptr;
    return data.data() + get_params().offset(address);
}

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
        auto start = offset(address);
        return (start + size) <= block_size;
    }
    return false;
}

memory_block::address_type memory_block::params::offset(memory_block::address_type address) const noexcept
{
    return address - block_start;
}
}//namespace vm
