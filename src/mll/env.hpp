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
    bool update(std::string const&, Node const&);
    std::optional<Node> lookup(std::string const&) const;
    std::optional<Node> shallow_lookup(std::string const&) const;

private:
    Env() = default;
    std::shared_ptr<Env> base_;
    std::map<std::string, Node> vars_;
};

} // namespace mll