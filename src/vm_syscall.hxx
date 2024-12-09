#pragma once

#include <vm_interface.hxx>
#include <functional>

namespace vm
{
struct syscall_interface
{
    using ptr = std::shared_ptr<syscall_interface>;

    virtual void exec(vm_interface* vm) = 0;

    [[nodiscard]]
    virtual register_t get_id() const = 0;

    [[nodiscard]]
    virtual std::string_view get_name() const = 0;

    virtual ~syscall_interface();
};

struct syscall_functor final: public syscall_interface
{
    using callback_type = std::function<void(vm_interface* vm)>;

    syscall_functor(register_t id, std::string name, callback_type callback)
        : id{id}
        , name{std::move(name)}
        , callback{std::move(callback)}
    {}

    static ptr create(register_t id, std::string name, callback_type callback)
    {
        return std::make_shared<syscall_functor>(id, std::move(name), std::move(callback));
    }

    void exec(vm_interface* vm) final
    {
        callback(vm);
    }

    [[nodiscard]]
    register_t get_id() const final
    {
        return id;
    }

    [[nodiscard]]
    std::string_view get_name() const final
    {
        return name;
    }
private:
    callback_type callback;
    std::string name;
    register_t id;
};

struct syscall_registry
{
    using interface = syscall_interface;
    using handler_ptr = syscall_interface*;
    using key_type = register_t;
    using syscall_id = register_t;
    using handler_map = std::map<key_type, interface::ptr>;

    /**
     * register handler by type
     * @tparam Handler
     */
    template<typename Handler>
    inline bool register_handler()
    {
        static_assert(std::is_base_of_v<interface, Handler>, "Wrong type of Handler");
        return register_handler(std::make_shared<Handler>());
    }
    /// register handler by pointer
    bool register_handler(interface::ptr handler);

    /// find handler by ID
    [[nodiscard]]
    handler_ptr find_handler(syscall_id id) const;
    /// get syscall handler ID
    syscall_id get_syscall_id(const vm_interface* vm) const;

    handler_map handlers;
};
} // namespace vm
