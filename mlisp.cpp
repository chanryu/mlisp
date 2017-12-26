#include <cassert>
#include <fstream>
#include <sstream>

#include <unistd.h> // isatty

#include <readline/readline.h>
#include <readline/history.h>

#include "mll.hpp"

using namespace mll;


int eval_file(std::shared_ptr<Env> env, const char* filename);

bool is_symbol(Node node)
{
    return !!to_symbol(node);
}

size_t length(Pair list)
{
    size_t l = 0;
    while (list) {
        l += 1;
        list = cdr(list);
    }
    return l;
}

void assert_argc(Pair args, size_t count, char const *cmd)
{
    if (length(args) != count) {
        throw EvalError(cmd + (" expects " + std::to_string(count)) + " argument(s).");
    }
}

void assert_argc_range(Pair args, size_t min, size_t max, char const *cmd)
{
    auto len = length(args);

    if (len >= min && len <= max) {
        throw EvalError(cmd + (" expects " + std::to_string(min)) + " ~ " +
                        std::to_string(max) + " argument(s).");
    }
}

Pair to_pair_or_throw(Node node, char const* cmd)
{
    auto pair = to_pair(node);
    if (!pair) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a pair.");
    }
    return *pair;
}

Number to_number_or_throw(Node node, char const* cmd)
{
    auto num = to_number(node);
    if (!num) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a number.");
    }
    return *num;
}

String to_string_or_throw(Node node, char const* cmd)
{
    auto str = to_string(node);
    if (!str) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a string.");
    }
    return *str;
}

Symbol to_symbol_or_throw(Node node, char const* cmd)
{
    auto sym = to_symbol(node);
    if (!sym) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a symbol.");
    }
    return *sym;
}

Pair to_formal_args(Node node, char const* cmd)
{
    auto args = to_pair_or_throw(node, cmd);

    // validate args (must be list of symbols)
    for (auto c = args; c; c = cdr(c)) {
        if (!is_symbol(car(c))) {
            throw EvalError(cmd + (": " + std::to_string(car(c))) + " is not a symbol");
        }
    }

    return args;
}

Node cadr(Pair pair)
{
    return car(cdr(pair));
}

Node caddr(Pair pair)
{
    return car(cdr(cdr(pair)));
}

void set_arithmetic_operators(std::shared_ptr<Env> env)
{
    set(env, "+", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        auto result = 0.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result += to_number_or_throw(arg, "+").value();
        }
        return make_number(result);
    }));

    set(env, "-", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (!args) {
            throw EvalError("-: too few parameters");
        }

        auto result = to_number_or_throw(eval(car(args), env), "-").value();
        args = cdr(args);
        if (args) {
            while (args) {
                auto arg = eval(car(args), env);
                result -= to_number_or_throw(arg, "-").value();
                args = cdr(args);
            }
        }
        else {
            // unary minus
            result = -result;
        }

        return make_number(result);
    }));

    set(env, "*", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        auto result = 1.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result *= to_number_or_throw(arg, "*").value();
        }

        return make_number(result);
    }));

    set(env, "/", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (!args || !cdr(args)) {
            throw EvalError("/: too few parameters");
        }

        auto result = to_number_or_throw(eval(car(args), env), "/").value();
        for (args = cdr(args); args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result /= to_number_or_throw(arg, "/").value();
        }

        return make_number(result);
    }));
}

void set_typesupport_operators(std::shared_ptr<Env> env)
{
    set(env, "number?", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (to_number(eval(car(args), env))) {
            return Symbol{ "t" };
        }
        return {};
    }));

    set(env, "number-equal?", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (length(args) != 2) {
            throw EvalError("number-equal?: must be given 2 parameters");
        }

        auto num1 = to_number_or_throw(eval(car(args), env), "number-equal?");
        auto num2 = to_number_or_throw(eval(cadr(args), env), "number-equal?");

        if (num1.value() == num2.value()) {
            return Symbol{ "t" };
        }
        return {};
    }));

    set(env, "string?", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (to_string(eval(car(args), env))) {
            return Symbol{ "t" };
        }
        return {};
    }));

    set(env, "string-equal?", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (length(args) != 2) {
            throw EvalError("string-equal?: must be given 2 parameters");
        }

        auto str1 = to_string_or_throw(eval(car(args), env), "string-equal?");
        auto str2 = to_string_or_throw(eval(cadr(args), env), "string-equal?");

        if (str1.text() == str2.text()) {
            return Symbol{ "t" };
        }
        return {};
    }));

    set(env, "symbol?", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (to_symbol(eval(car(args), env))) {
            return Symbol{ "t" };
        }
        return {};
    }));
}

void set_complementary_operators(std::shared_ptr<Env> env)
{
    set(env, "list", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        std::vector<Node> objs;
        for (; args; args = cdr(args)) {
            objs.push_back(eval(car(args), env));
        }

        Pair list;
        while (!objs.empty()) {
            list = cons(objs.back(), list);
            objs.pop_back();
        }
        return list;
    }));

    set(env, "set", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        assert_argc(args, 2, "set");

        auto symbol = to_symbol_or_throw(eval(car(args), env), "set");
        auto value = eval(cadr(args), env);
        set(env, symbol.name(), value);
        return value;
    }));

    set(env, "if", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        assert_argc_range(args, 2, 3, "if");

        auto cond = car(args);
        auto body = cdr(args);
        auto then_arm = car(body);
        auto else_arm = cadr(body);
        if (eval(cond, env)) {
            return eval(then_arm, env);
        }
        return eval(else_arm, env);
    }));

    set(env, "print", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        Node ret;
        auto first = true;
        while (args) {
            if (first) {
                first = false;
            }
            else {
                std::cout << " ";
            }
            std::cout << (ret = eval(car(args), env));
            args = cdr(args);
        }
        std::cout << std::endl;
        return ret;
    }));

    set(env, "load", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        assert_argc(args, 1, "load");
        auto filename = to_string_or_throw(car(args), "load");
        if (!eval_file(env, filename.text().c_str())) {
            return make_symbol("t");
        }
        return {};
    }));
}

void set_primitive_operators(std::shared_ptr<Env> env)
{
    // "quote" is already built into the Parser/eval()
    //set(env, "quote", make_proc([](List args, Env) {
    //    return car(args);
    //}));

    set(env, "atom", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        auto pair = to_pair(eval(car(args), env));
        if (!pair || !*pair) {
            return make_symbol("t");
        }
        return {};
    }));

    set(env, "eq", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (!car(args) || !cadr(args)) {
            throw EvalError("eq: too few args given");
        }
        else if (caddr(args)) {
            throw EvalError("eq: too many args given");
        }
        if (eval(car(args), env) == eval(cadr(args), env)) {
            return make_symbol("t");
        }
        return {};
    }));

    set(env, "car", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (cdr(args)) {
            throw EvalError("car: too many args given");
        }
        return car(to_pair_or_throw(eval(car(args), env), "car"));
    }));

    set(env, "cdr", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (cdr(args)) {
            throw EvalError("cdr: too many args given");
        }
        return cdr(to_pair_or_throw(eval(car(args), env), "cdr"));
    }));

    set(env, "cons", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (!cdr(args)) {
            throw EvalError("cons: not enough args");
        }

        auto head = eval(car(args), env);
        auto tail = to_pair_or_throw(eval(cadr(args), env), "cons");

        return cons(head, tail);
    }));

    set(env, "cond", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        while (args) {
            auto clause = to_pair_or_throw(car(args), "cond");
            auto pred = car(clause);
            if (eval(pred, env)) {
                Node result;
                for (auto expr = cdr(clause); expr; expr = cdr(expr)) {
                    result = eval(car(expr), env);
                }
                return result;
            }
            args = cdr(args);
        }
        return {};
    }));

    set(env, "lambda", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (cdr(cdr(args))) {
            throw EvalError("lambda: too many args");
        }

        auto formal_args = to_formal_args(car(args), "lambda");
        auto lambda_body = cadr(args);
        auto creator_env = env;

        return make_proc([formal_args, lambda_body, creator_env] (Pair args, std::shared_ptr<Env> env) {
            auto lambda_env = make_env(creator_env);

            auto syms = formal_args;
            while (syms) {
                if (!args) {
                    EvalError("Proc: too few args");
                }

                assert(is_symbol(car(syms)));

                auto sym = to_symbol(car(syms));
                auto val = eval(car(args), env);
                set(lambda_env, sym->name(), val);
                syms = cdr(syms);
                args = cdr(args);
            }

            if (args) {
                EvalError("Proc: too many args");
            }

            return eval(lambda_body, lambda_env);
        });
    }));

    set(env, "label", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (!args || !cdr(args)) {
            throw EvalError("label: too few parameters");
        }
        if (cdr(cdr(args))) {
            throw EvalError("label: too many parameters");
        }

        auto symbol = to_symbol_or_throw(car(args), "label");
        auto value = eval(cadr(args), env);
        set(env, symbol.name(), value);

        return value;
    }));

    set(env, "defun", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        auto name = car(args);
        auto body = cons(make_symbol("lambda"), cdr(args));
        return eval(cons(make_symbol("label"), cons(name, cons(body, {}))), env);
    }));
}

int repl(std::shared_ptr<Env> env)
{
    static bool once = true;
    if (once) {
        once = false;
        // By default readline does filename completion. We disable this
        // by asking readline to just insert the TAB character itself.
        rl_bind_key('\t', rl_insert);
    }

    auto get_line = [] (char const* prompt, std::string& line) -> bool {
        auto buf = readline(prompt);
        if (buf) {
            line = buf;
            free(buf);
            if (!line.empty()) {
                add_history(line.c_str());
            }
            line.push_back('\n');
            return true;
        }
        return false;
    };

    auto parser = Parser{};

    while (true) {
        char const* prompt;
        if (parser.clean()) {
            prompt = "mlisp> ";
        }
        else {
            prompt = "...... ";
        }

        std::string line;
        if (!get_line(prompt, line)) {
            break;
        }

        auto is = std::istringstream(line);

        while (!is.eof()) {
            try {
                auto expr = parser.parse(is);
                if (!expr) {
                    break;
                }
                std::cout << eval(*expr, env) << std::endl;
            }
            catch (ParseError& e) {
                std::cout << e.what() << std::endl;
            }
            catch (EvalError& e) {
                std::cout << e.what() << std::endl;
            }
        }

        if (std::cin.eof()) {
            break;
        }
    }

    std::cout << std::endl;
    std::cout << "Bye." << std::endl;

    return parser.clean() ? 0 : -1;
}

int eval_stream(std::shared_ptr<Env> env, std::istream& is, std::ostream& os)
{
    try {
        auto parser = Parser{};

        while (true) {
            auto expr = parser.parse(is);
            if (!expr) {
                break;
            }
            os << eval(*expr, env) << std::endl;
        }
    }
    catch (ParseError& e) {
        std::cout << e.what() << std::endl;
        return -1;
    }
    catch (EvalError& e) {
        std::cout << e.what() << std::endl;
        return -1;
    }

    return is.eof() ? 0 : -1;
}

int eval_file(std::shared_ptr<Env> env, const char* filename)
{
    auto ifs = std::ifstream{ filename };
    if (!ifs.is_open()) {
        return -1;
    }

    // throw-away stream
    std::ostringstream oss;
    oss.setstate(std::ios_base::badbit);

    return eval_stream(env, ifs, oss);
}

int main(int argc, char *argv[])
{
    auto env = make_env(nullptr);

    set_primitive_operators(env);
    set_typesupport_operators(env);
    set_complementary_operators(env);

    for (int i = 1; i < argc; ++i) {
        int ret = eval_file(env, argv[i]);
        if (ret != 0) {
            return ret;
        }
    }

    auto cin_piped = []() {
        return !isatty(fileno(stdin));
    }();

    if (cin_piped) {
        return eval_stream(env, std::cin, std::cout);
    }

    if (argc > 1) {
        return 0;
    }

    return repl(env);
}
