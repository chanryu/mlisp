#include <mll/quote.hpp>

namespace mll {

namespace {
char const* const TOKEN_QUOTE = "'";
char const* const TOKEN_QUASIQUOTE = "`";
char const* const TOKEN_UNQUOTE = ",";
char const* const TOKEN_UNQUOTE_SPLICING = ",@";
} // namespace

char const* const SYMBOL_QUOTE = "quote";
char const* const SYMBOL_QUASIQUOTE = "quasiquote";
char const* const SYMBOL_UNQUOTE = "unquote";
char const* const SYMBOL_UNQUOTE_SPLICING = "unquote-splicing";

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

const char* quote_symbol_name_from_token(std::string const& token)
{
    if (token == TOKEN_QUOTE) {
        return SYMBOL_QUOTE;
    }
    if (token == TOKEN_QUASIQUOTE) {
        return SYMBOL_QUASIQUOTE;
    }
    if (token == TOKEN_UNQUOTE) {
        return SYMBOL_UNQUOTE;
    }
    if (token == TOKEN_UNQUOTE_SPLICING) {
        return SYMBOL_UNQUOTE_SPLICING;
    }
    return nullptr;
}

} // namespace mll