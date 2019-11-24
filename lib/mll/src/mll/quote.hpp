#pragma once

#include <optional>
#include <string>

namespace mll {

class Symbol;

bool is_quote_token(std::string const& token);
const char* quote_token_from_symbol_name(std::string const& node);
std::optional<Symbol> quote_symbol_from_token(std::string const& token);

class Env;
void load_quote_procs(Env& env);

} // namespace mll