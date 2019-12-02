#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>

#include <mll/gc.hpp>

namespace mll {

class Node;

class Env : public std::enable_shared_from_this<Env>, public Collectable {
public:
    static std::shared_ptr<Env> create();
    std::shared_ptr<Env> derive_new();

    void set(std::string const&, Node const&);
    bool deep_update(std::string const&, Node const&);
    bool shallow_update(std::string const&, Node const&);
    std::optional<Node> deep_lookup(std::string const&) const;
    std::optional<Node> shallow_lookup(std::string const&) const;

public:
    // Collectable overrides
    void mark_reachables() final;

private:
    Env() = default;
    std::shared_ptr<Env> _base;
    std::map<std::string, Node> _vars;
};

} // namespace mll