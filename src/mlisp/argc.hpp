#include <cstddef>

namespace mll {
class List;
}

namespace mlisp {
void assert_argc(mll::List const& args, size_t count, char const* cmd);
void assert_argc_min(mll::List const& args, size_t min, char const* cmd);
} // namespace mlisp