#include "load.hpp"

#include "parser.hpp"
#include "string.hpp"

#include <mll/env.hpp>
#include <mll/eval.hpp>
#include <mll/node.hpp>
#include <mll/print.hpp>

#include <cassert>
#include <fstream>
#include <iostream>

#if __has_include(<unistd.h>) // UNIX
#include <unistd.h>
#elif __has_include(<direct.h>) // WINDOWS
#include <direct.h>
#define getcwd _getcwd
#endif

namespace mlisp {

namespace {

auto constexpr LOAD_PATH_KEY = "mlisp:load-path";

std::string get_current_load_path(mll::Env const& env)
{
    std::string load_path;
    if (auto node = env.lookup(LOAD_PATH_KEY)) {
        if (auto str = mll::dynamic_node_cast<String>(*node)) {
            load_path = (*str).value();
        }
    }
    if (load_path.empty()) {
        std::unique_ptr<char, decltype(::free)*> cwd_ptr{::getcwd(NULL, 0), ::free};
        load_path = cwd_ptr.get();
    }
    return load_path;
}

bool is_absolute_path(std::string const& path)
{
    assert(!path.empty());
    return path[0] == '/' || (path.size() > 1 && path[1] == ':');
}

std::string make_absolute_filepath(std::string const& current_load_path, std::string const& filepath)
{
    std::string absolute_filepath;
    if (is_absolute_path(filepath)) {
        absolute_filepath = filepath;
    }
    else {
        absolute_filepath = current_load_path + "/" + filepath;
    }

    // normalize slashes
    for (auto& c : absolute_filepath) {
        if (c == '\\') {
            c = '/';
        }
    }
    return absolute_filepath;
}

std::string get_parent_path(std::string const& path)
{
    auto pos = path.find_last_of('/');
    assert(pos != std::string::npos);
    return path.substr(0, pos);
}

void set_load_path(mll::Env& env, std::string const& path)
{
    env.set(LOAD_PATH_KEY, String{path});
}
} // namespace

bool load_file(mll::Env& env, std::string const& filepath)
{
    auto const current_load_path = get_current_load_path(env);
    auto const absolute_filepath = make_absolute_filepath(current_load_path, filepath);

    if (std::ifstream ifs{absolute_filepath.c_str()}; ifs.is_open()) {
        set_load_path(env, get_parent_path(absolute_filepath));
        mlisp::Parser parser;
        try {
            while (true) {
                auto expr = parser.parse(ifs);
                if (!expr.has_value()) {
                    break;
                }
                eval(*expr, env);
            }
        }
        catch (mll::ParseError& e) {
            std::cerr << e.what() << '\n';
            return false;
        }
        catch (mll::EvalError& e) {
            std::cerr << e.what() << '\n';
            return false;
        }
        set_load_path(env, current_load_path);
        return parser.clean();
    }
    return false;
}

} // namespace mlisp