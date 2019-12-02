#pragma once

#include <vector>

namespace mll {

class Env;
class GC;

class Collectable {
public:
    Collectable();
    virtual ~Collectable() = default;

    bool is_reachable() const;
    virtual void mark_reachables();

private:
    friend class GC;
    bool _reachable = false;
};

class GC {
public:
    void collect(Env& root_env);
};
} // namespace mll