#include <cassert>
#include <fstream>
#include <sstream>

#include <unistd.h> // isatty

#include <readline/readline.h>
#include <readline/history.h>

#include "mll.hpp"

using namespace mll;

bool is_symbol(Node node)
{
    return !!to_symbol(node);
}

Pair to_pair(Node node, char const* cmd)
{
    auto pair = to_pair(node);
    if (!pair) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a pair.");
    }
    return *pair;
}

Number to_number(Node node, char const* cmd)
{
    auto number = to_number(node);
    if (!number) {
        throw EvalError(cmd + (": " + std::to_string(node)) + " is not a number.");
    }
    return *number;
}

Pair to_formal_args(Node node, char const* cmd)
{
    auto args = to_pair(node, cmd);

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

bool equal(Node n1, Node n2)
{
    auto num1 = to_number(n1);
    if (num1) {
        auto num2 = to_number(n2);
        if (num2) {
            return num1->value() == num2->value();
        }
        return false;
    }

    auto str1 = to_string(n1);
    if (str1) {
        auto str2 = to_string(n2);
        if (str2) {
            return str1->text() == str2->text();
        }
        return false;
    }

    auto sym1 = to_symbol(n1);
    if (sym1) {
        auto sym2 = to_symbol(n2);
        if (sym2) {
            return sym1->name() == sym2->name();
        }
        return false;
    }

    auto list1 = to_pair(n1);
    if (list1) {
        auto list2 = to_pair(n2);
        if (list2) {
            auto l1 = *list1;
            auto l2 = *list2;
            while (l1) {
                if (!l2) {
                    return false;
                }
                if (!equal(car(l1), car(l2))) {
                    return false;
                }
                l1 = cdr(l1);
                l2 = cdr(l2);
            }
            return !l2;
        }
        return false;
    }

    return false;
}

void set_arithmetic_operators(std::shared_ptr<Env> env)
{
    set(env, "+", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        auto result = 0.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result += to_number(arg, "+").value();
        }
        return make_number(result);
    }));

    set(env, "-", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (!args) {
            throw EvalError("-: too few parameters");
        }

        auto result = to_number(eval(car(args), env), "-").value();
        args = cdr(args);
        if (args) {
            while (args) {
                auto arg = eval(car(args), env);
                result -= to_number(arg, "-").value();
                args = cdr(args);
            }
        } else {
            // unary -
            result = -result;
        }

        return make_number(result);
    }));

    set(env, "*", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        auto result = 1.0;
        for (; args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result *= to_number(arg, "*").value();
        }

        return make_number(result);
    }));

    set(env, "/", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (!args || !cdr(args)) {
            throw EvalError("/: too few parameters");
        }

        auto result = to_number(eval(car(args), env), "/").value();
        for (args = cdr(args); args; args = cdr(args)) {
            auto arg = eval(car(args), env);
            result /= to_number(arg, "/").value();
        }

        return make_number(result);
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

    set(env, "symbol?", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (to_symbol(car(args))) {
            return Symbol{ "t" };
        }
        return {};
    }));

    set(env, "set", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (!args || !cdr(args)) {
            throw EvalError("set: too few parameters");
        }
        if (cdr(cdr(args))) {
            throw EvalError("set: too many parameters");
        }

        auto symbol = to_symbol(eval(car(args), env));
        if (!symbol) {
            throw EvalError("set: " + std::to_string(car(args)) +
                            " is not a symbol.");
        }

        auto value = eval(cadr(args), env);
        set(env, symbol->name(), value);

        return value;
    }));

    set(env, "do", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        env = make_env(env);
        Node result;
        while (args) {
            result = eval(car(args), env);
            args = cdr(args);
        }
        return result;
    }));

    set(env, "nil?", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (!eval(car(args), env)) {
            return Symbol{ "t" };
        }
        return {};
    }));

    set(env, "zero?", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (to_number(eval(car(args), env), "zero?").value() == 0) {
            return Symbol{ "t" };
        }
        return {};
    }));

    set(env, "if", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        auto cond = car(args);
        if (!cdr(args)) {
            throw EvalError("if: too few arguments");
        }

        auto body = cdr(args);
        auto then_arm = car(body);
        auto else_arm = cadr(body);

        if (eval(cond, env)) {
            return eval(then_arm, env);
        } else {
            return eval(else_arm, env);
        }
    }));

    set(env, "print", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        Node ret;
        auto first = true;
        while (args) {
            if (first) {
                first = false;
            } else {
                std::cout << " ";
            }
            std::cout << (ret = eval(car(args), env));
            args = cdr(args);
        }
        std::cout << std::endl;
        return ret;
    }));

    set(env, "equal?", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        if (!args) {
            throw EvalError("equal?: two few args");
        }

        auto obj = eval(car(args), env);
        for (args = cdr(args); args; args = cdr(args)) {
            if (!equal(obj, eval(car(args), env))) {
                return {};  // nil => false
            }
        }

        return make_symbol("t");
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
        return car(to_pair(eval(car(args), env), "car"));
    }));

    set(env, "cdr", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (cdr(args)) {
            throw EvalError("cdr: too many args given");
        }
        return cdr(to_pair(eval(car(args), env), "cdr"));
    }));

    set(env, "cons", make_proc([] (Pair args, std::shared_ptr<Env> env) {
        if (!cdr(args)) {
            throw EvalError("cons: not enough args");
        }

        auto head = eval(car(args), env);
        auto tail = to_pair(eval(cadr(args), env), "cons");

        return cons(head, tail);
    }));

    set(env, "cond", make_proc([] (Pair args, std::shared_ptr<Env> env) -> Node {
        while (args) {
            auto clause = to_pair(car(args), "cond");
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

        auto symbol = to_symbol(car(args));
        if (!symbol) {
            throw EvalError("label: " + std::to_string(car(args)) +
                            " is not a symbol.");
        }

        auto value = eval(cadr(args), env);
        set(env, symbol->name(), value);

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
