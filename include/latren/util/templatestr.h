// allows us to pass string literals to templates
// works using a clever string literal trick copied from https://stackoverflow.com/a/28209546

#pragma once

#include <utility>

template <char... Chars>
using TemplateString = std::integer_sequence<char, Chars...>;

template <typename T, T... Chars>
constexpr TemplateString<Chars...> operator""_tstr() { return { }; }

template <char... Chars>
inline const char* ReadTemplateString() {
    static constexpr char str[sizeof...(Chars) + 1] = { Chars..., '\0' };
    return str;
}