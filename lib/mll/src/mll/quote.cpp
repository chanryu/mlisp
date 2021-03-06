#include <mll/quote.hpp>

#include <mll/env.hpp>
#include <mll/eval.hpp>
#include <mll/list.hpp>
#include <mll/proc.hpp>
#include <mll/symbol.hpp>

namespace mll {

namespace {
char const* const TOKEN_QUOTE = "'";
char const* const TOKEN_QUASIQUOTE = "`";
char const* const TOKEN_UNQUOTE = ",";
char const* const TOKEN_UNQUOTE_SPLICING = ",@";
char const* const SYMBOL_QUOTE = "quote";
char const* const SYMBOL_QUASIQUOTE = "quasiquote";
char const* const SYMBOL_UNQUOTE = "unquote";
char const* const SYMBOL_UNQUOTE_SPLICING = "unquote-splicing";
} // namespace

bool is_quote_token(std::string const& token)
{
    return token == TOKEN_QUOTE || token == TOKEN_QUASIQUOTE || token == TOKEN_UNQUOTE ||
           token == TOKEN_UNQUOTE_SPLICING;
}

const char* quote_token_from_symbol_name(std::string const& symbol_name)
{
    if (symbol_name == SYMBOL_QUOTE) {
        return TOKEN_QUOTE;
    }
    if (symbol_name == SYMBOL_QUASIQUOTE) {
        return TOKEN_QUASIQUOTE;
    }
    if (symbol_name == SYMBOL_UNQUOTE) {
        return TOKEN_UNQUOTE;
    }
    if (symbol_name == SYMBOL_UNQUOTE_SPLICING) {
        return TOKEN_UNQUOTE_SPLICING;
    }
    return nullptr;
}

std::optional<Symbol> quote_symbol_from_token(std::string const& token)
{
    const char* name = nullptr;
    if (token == TOKEN_QUOTE) {
        name = SYMBOL_QUOTE;
    }
    else if (token == TOKEN_QUASIQUOTE) {
        name = SYMBOL_QUASIQUOTE;
    }
    else if (token == TOKEN_UNQUOTE) {
        name = SYMBOL_UNQUOTE;
    }
    else if (token == TOKEN_UNQUOTE_SPLICING) {
        name = SYMBOL_UNQUOTE_SPLICING;
    }
    if (name) {
        return Symbol{name};
    }
    return std::nullopt;
}

namespace {
Node unquote_list(List list, Env& env)
{
    return car(map(list, [&env](Node const& node) { return eval(node, env); }));
}

Node quasiquote_list(List list, Env& env)
{
    std::stack<Node> stack;
    auto add_to_stack = [&env, &stack](Node const& node) {
        if (auto lst = dynamic_node_cast<List>(node)) {
            if (auto s = dynamic_node_cast<Symbol>(car(*lst))) {
                if (s->name() == SYMBOL_QUOTE) {
                    stack.push(node);
                    return;
                }
                if (s->name() == SYMBOL_UNQUOTE) {
                    stack.push(eval(*lst, env));
                    return;
                }
                if (s->name() == SYMBOL_UNQUOTE_SPLICING) {
                    auto result = eval(*lst, env);
                    if (auto lst2 = dynamic_node_cast<List>(result)) {
                        while (!lst2->empty()) {
                            stack.push(car(*lst2));
                            *lst2 = cdr(*lst2);
                        }
                    }
                    else {
                        stack.push(result);
                    }
                    return;
                }
            }
            stack.push(quasiquote_list(*lst, env));
            return;
        }
        stack.push(node);
    };

    while (!list.empty()) {
        add_to_stack(car(list));
        list = cdr(list);
    }
    while (!stack.empty()) {
        list = cons(stack.top(), list);
        stack.pop();
    }
    return list;
}

} // namespace

void load_quote_procs(Env& env)
{
    auto defun = [&env](char const* cmd, Func func) { env.set(cmd, Proc{cmd, std::move(func)}); };

    defun(SYMBOL_QUOTE, [](List const& args, Env& /*env*/) { return car(args); });

    defun(SYMBOL_QUASIQUOTE, [](List const& args, Env& env) {
        auto node = car(args);
        if (auto list = dynamic_node_cast<List>(node)) {
            node = quasiquote_list(*list, env);
        }
        return node;
    });

    defun(SYMBOL_UNQUOTE, [](List const& args, Env& env) { return unquote_list(args, env); });

    defun(SYMBOL_UNQUOTE_SPLICING, [](List const& args, Env& env) { return unquote_list(args, env); });
}

} // namespace mll