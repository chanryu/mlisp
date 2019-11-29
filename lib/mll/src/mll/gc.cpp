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

void GC::collect()
{
    // Clear
    for (auto collectable : all_collectables) {
        collectable->_reachable = false;
    }

    // Mark
}

} // namespace mll