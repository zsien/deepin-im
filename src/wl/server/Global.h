#ifndef WL_SERVER_GLOBAL_H
#define WL_SERVER_GLOBAL_H

#include "Server.h"

#include <memory>

namespace wl {
namespace server {

template<auto F>
struct GlobalCallbackWrapper;

template<typename C, typename R, typename Arg, typename... Args, R (C::*F)(Arg, Args...)>
struct GlobalCallbackWrapper<F>
{
    static R func(Arg farg, void *userdata, Args... args)
    {
        auto *p = reinterpret_cast<C *>(userdata);
        return (p->*F)(farg, args...);
    }
};

class Global
{
public:
    template<typename T>
    static std::shared_ptr<Global> create(const std::shared_ptr<Server> &server)
    {
        auto g = std::make_shared<Global>(server, T::interface);

        return g;
    }

private:
    std::unique_ptr<wl_global, Deleter<wl_global_destroy>> global_;

    Global(const std::shared_ptr<Server> &server, const struct wl_interface *interface);

    void onBind(struct wl_client *client, uint32_t version, uint32_t id);
    static void onDestroy();
};

} // namespace server
} // namespace wl

#endif // !WL_SERVER_GLOBAL_H
