#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>

namespace mll {

class Node;

class Env : public std::enable_shared_from_this<Env> {
public:
    static std::shared_ptr<Env> create();
    std::shared_ptr<Env> derive_new();

    void set(std::string const&, Node const&);
    bool deep_update(std::string const&, Node const&);
    bool shallow_update(std::string const&, Node const&);
    std::optional<Node> deep_lookup(std::string const&) const;
    std::optional<Node> shallow_lookup(std::string const&) const;

private:
    Env() = default;
    std::shared_ptr<Env> _base;
    std::map<std::string, Node> _vars;
};

} // namespace mll