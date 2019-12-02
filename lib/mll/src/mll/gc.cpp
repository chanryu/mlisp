#include <mll/env.hpp>
#include <mll/gc.hpp>

#include <algorithm>

namespace mll {

namespace {
static std::vector<Collectable*> all_collectables;
}

Collectable::Collectable()
{
    all_collectables.push_back(this);
}

bool Collectable::is_reachable() const
{
    return _reachable;
}

void Collectable::mark_reachables()
{
    _reachable = true;
}

void GC::collect(Env& root_env)
{
    // Clear
    for (auto collectable : all_collectables) {
        collectable->_reachable = false;
    }

    // Mark
    root_env.mark_reachables();

    // Sweep
}

} // namespace mll