#pragma once

#include "vm_shared.hxx"
#include <concepts>

namespace vm
{

template<typename T>
concept standard_layout = std::is_standard_layout_v<T>;

/**
 * Memory region
 */
struct memory_block
{
    using ptr = std::shared_ptr<memory_block>;
    using address_type = std::uint32_t;
    using size_type = std::uint32_t;

    /// block identify
    struct params
    {
        /// start address
        address_type block_start;
        /// block size in bytes
        size_type    block_size;

        bool operator<(const params& rhs) const noexcept
        {
            return block_start < rhs.block_start;
        }

        [[nodiscard]]
        address_type offset(address_type address) const noexcept;
        [[nodiscard]]
        bool in_range(address_type address) const noexcept;
        [[nodiscard]]
        bool in_range(address_type address, size_type size) const noexcept;
        [[nodiscard]]
        bool is_overlap(address_type address, size_type size) const noexcept;

        [[nodiscard]]
        bool is_overlap(const params& other) const noexcept
        {
            return is_overlap(other.block_start, other.block_size);
        }
    };

    [[nodiscard]]
    const params& get_params() const;

    [[nodiscard]]
    address_type get_start_address() const noexcept
    {
        return get_params().block_start;
    }

    [[nodiscard]]
    size_type get_size() const noexcept
    {
        return get_params().block_size;
    }

    /**
     * read value
     * @param address absolute address
     * @param dest pointer to buffer
     * @param size size of buffer
     * @return true on success
     */
    [[nodiscard]]
    virtual bool load(address_type address, void * dest, size_type size) const = 0;

    /**
     * write value
     * @param address absolute address
     * @param source pointer to buffer
     * @param size size of buffer
     * @return true on success
     */
    [[nodiscard]]
    virtual bool store(memory_block::address_type address, const void * source, memory_block::size_type size) = 0;

    template<standard_layout Type>
    [[nodiscard]]
    const Type * get_ro_ptr(address_type address) const
    {
        return static_cast<const Type*>(get_ro(address, sizeof(Type)));
    }

    template<standard_layout Type>
    [[nodiscard]]
    Type * get_rw_ptr(address_type address)
    {
        return static_cast<Type*>(get_rw(address, sizeof(Type)));
    }

    virtual ~memory_block();
protected:
    explicit memory_block(address_type address, size_type size);

    [[nodiscard]]
    virtual const void * get_ro(address_type address, size_type size) const = 0;
    [[nodiscard]]
    virtual void * get_rw(address_type address, size_type size) = 0;
protected:
    params block_params;
};

struct memory_management_unit
{
    using key_type = memory_block::params;
    using value_type = memory_block::ptr;
    using pointer = memory_block *;
    using memory_blocks =  std::map<key_type, value_type>;

    template<typename BlockType, typename... Args>
    [[nodiscard]]
    bool add_block(Args... args)
    {
        static_assert(std::is_base_of_v<memory_block, BlockType>, "BlockType is not MemoryBlock");
        return add_block(std::make_shared<BlockType>(std::forward<Args>(args)...));
    }

    [[nodiscard]]
    bool add_block(value_type block);
    [[nodiscard]]
    pointer find_block(memory_block::address_type address, memory_block::size_type size) const;

private:
    memory_blocks memory;
};

struct generic_memory: public memory_block
{
    generic_memory(address_type address, size_type size);

    bool load(address_type address, void *dest, size_type size) const override;

    bool store(memory_block::address_type address, const void *source, memory_block::size_type size) override;

protected:
    [[nodiscard]]
    const void * get_ro(address_type address, size_type size) const override;
    [[nodiscard]]
    void * get_rw(address_type address, size_type size) override;
private:
    using storage_type = std::vector<std::uint8_t>;
    using storage_size = storage_type::size_type;
    storage_type data;
};

}//namespace vm
