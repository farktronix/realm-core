#include <stdlib.h>
#include <string.h>
#include <string>

#include <realm/util/features.h>
#include <realm/util/basic_system_errors.hpp>

using namespace realm::util;


namespace {

class system_category: public std::error_category {
    const char* name() const noexcept override;
    std::string message(int) const override;
};

system_category g_system_category;

const char* system_category::name() const noexcept
{
    return "realm.basic_system";
}

std::string system_category::message(int value) const
{
    const size_t max_msg_size = 256;
    char buffer[max_msg_size+1];

#ifdef _WIN32 // Windows version

    if (REALM_LIKELY(strerror_s(buffer, max_msg_size, value) == 0)) {
        return buffer; // Guaranteed to be NULL-terminated
    }

#elif defined __APPLE__ && defined __MACH__ // OSX, iOS and WatchOS version

    {
        const int result = strerror_r(value, buffer, max_msg_size);
        if (REALM_LIKELY(result == 0 || result == ERANGE || result == EINVAL)) {
            // On Apple platforms, strings generated by ERANGE cases are
            // guaranteed to be NULL-terminated, however, there is no formal
            // indication that this behaviour can be expected in EINVAL cases.
            // Even though the chances are slim, better safe than sorry.
            buffer[max_msg_size] = '\0';
            return buffer;
        }
    }

#elif ! REALM_ANDROID && _GNU_SOURCE // GNU specific version

    {
        char* msg = nullptr;
        if (REALM_LIKELY((msg = strerror_r(value, buffer, max_msg_size)) != nullptr)) {
            return msg; // Guaranteed to be NULL-terminated
        }
    }

#else // POSIX.1-2001 fallback version

    {
        const int result = strerror_r(value, buffer, max_msg_size);
        if (REALM_LIKELY(result == 0 || result == EINVAL)) {
            // POSIX doesn't make any guarantees that the string will be
            // NULL-terminated. Better safe than sorry.
            buffer[max_msg_size] = '\0';
            return buffer;
        }
    }

#endif

    return "Unknown error";
}

} // anonymous namespace


namespace realm {
namespace util {
namespace error {

std::error_code make_error_code(basic_system_errors err)
{
    return std::error_code(err, g_system_category);
}

} // namespace error
} // namespace util
} // namespace realm
