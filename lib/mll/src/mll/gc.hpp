#pragma once

#include <vector>

namespace mll {

class GC;

class Collectable {
public:
    Collectable();
    virtual ~Collectable() = default;
    virtual void get_collectables(std::vector<Collectable*>& collectables) const = 0;

private:
    friend class GC;
    bool _reachable = false;
};

class GC {
public:
    void collect();
};
} // namespace mll