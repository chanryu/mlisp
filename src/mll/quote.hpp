#pragma once

#include <string>

namespace mll {

extern char const* const SYMBOL_QUOTE;            // "quote"
extern char const* const SYMBOL_QUASIQUOTE;       // "quasiquote"
extern char const* const SYMBOL_UNQUOTE;          // "unquote"
extern char const* const SYMBOL_UNQUOTE_SPLICING; // "unquote-splicing"

bool is_quote_token(std::string const& token);
const char* quote_token_from_symbol_name(std::string const& node);
const char* quote_symbol_name_from_token(std::string const& token);

} // namespace mll