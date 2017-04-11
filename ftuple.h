/*
 * Copyright (c) The National University of Singapore.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _FTUPLE_H
#define _FTUPLE_H

#include "fbase.h"
#include "fvalue.h"
#include "fshow.h"

#include "fstring_defs.h"
#include "ftuple_defs.h"

namespace F
{

inline void _tuple_init(Value<Word> *_end_ptr)
{
    ;
}

template <typename _T>
inline void _tuple_init(Value<Word> *_end_ptr, _T _arg)
{
    *_end_ptr = _bit_cast<Value<Word>>(Value<_T>(_arg));
}

template <typename _U, typename... _T>
inline void _tuple_init(Value<Word> *_start_ptr, _U _arg, _T... _args)
{
    *_start_ptr = _bit_cast<Value<Word>>(Value<_U>(_arg));
    _tuple_init<_T...>(_start_ptr + 1, _args...);
}

/**
 * Construct a tuple.
 * O(1).
 */
template <typename... _T>
inline PURE Tuple<_T...> tuple(_T... _args)
{
    Value<Word> *_impl = (Value<Word> *)gc_malloc(
        sizeof...(_args) * sizeof(Value<Word>));
    _tuple_init(_impl, _args...);
    Tuple<_T...> _t = {_impl};
    return _t;
}

/**
 * First element of a tuple.
 * O(1).
 */
template <typename _U, typename... _T>
inline PURE const _U &first(Tuple<_U, _T...> _t)
{
    return (Value<_U> &)_t._impl[0];
}

/**
 * Second element of a tuple.
 * O(1).
 */
template <typename _A, typename _U, typename... _T>
inline PURE const _U &second(Tuple<_A, _U, _T...> _t)
{
    return (Value<_U> &)_t._impl[1];
}

/**
 * Third element of a tuple.
 * O(1).
 */
template <typename _A, typename _B, typename _U, typename... _T>
inline PURE const _U &third(Tuple<_A, _B, _U, _T...> _t)
{
    return (Value<_U> &)_t._impl[2];
}

/**
 * Fourth element of a tuple.
 * O(1).
 */
template <typename _A, typename _B, typename _C, typename _U, typename... _T>
inline PURE const _U &fourth(Tuple<_A, _B, _C, _U, _T...> _t)
{
    return (Value<_U> &)_t._impl[3];
}

/**
 * Fifth element of a tuple.
 * O(1).
 */
template <typename _A, typename _B, typename _C, typename _D, typename _U, typename... _T>
inline PURE const _U &fifth(Tuple<_A, _B, _C, _D, _U, _T...> _t)
{
    return (Value<_U> &)_t._impl[4];
}

/**
 * Sixth element of a tuple.
 * O(1).
 */
template <typename _A, typename _B, typename _C, typename _D, typename _E, typename _U, typename... _T>
inline PURE const _U &sixth(Tuple<_A, _B, _C, _D, _E, _U, _T...> _t)
{
    return (Value<_U> &)_t._impl[5];
}

/**
 * Seventh element of a tuple.
 * O(1).
 */
template <typename _A, typename _B, typename _C, typename _D, typename _E, typename _U, typename _F, typename... _T>
inline PURE const _U &seventh(Tuple<_A, _B, _C, _D, _E, _F, _U, _T...> _t)
{
    return (Value<_U> &)_t._impl[6];
}

/**
 * Eighth element of a tuple.
 * O(1).
 */
template <typename _A, typename _B, typename _C, typename _D, typename _E, typename _U, typename _F, typename _G, typename... _T>
inline PURE const _U &eighth(Tuple<_A, _B, _C, _D, _E, _F, _G, _U, _T...> _t)
{
    return (Value<_U> &)_t._impl[7];
}

/**
 * Tuple size (a.k.a. tuple length).
 * O(1).
 */
template <typename... _T>
constexpr size_t size(Tuple<_T...> _t)
{
    return sizeof...(_T);
}

template <typename _T>
inline PURE int _tuple_compare(Tuple<_T> _t, Tuple<_T> _u)
{
    return compare(first(_t), first(_u));
}

template <typename _U, typename _V, typename ..._T>
inline PURE int _tuple_compare(Tuple<_U, _V, _T...> _t,
    Tuple<_U, _V, _T...> _u)
{
    int _cmp = compare(first(_t), first(_u));
    if (_cmp != 0)
        return _cmp;
    Tuple<_V, _T...> _x = {_t._impl + 1};
    Tuple<_V, _T...> _y = {_u._impl + 1};
    return _tuple_compare<_V, _T...>(_x, _y);
}

/**
 * Tuple compare.
 * O(n).
 */
template <typename... _T>
inline PURE int compare(Tuple<_T...> _t, Tuple<_T...> _u)
{
    return _tuple_compare<_T...>(_t, _u);
}

// Forward decls:
PURE String append(String _str0, String _str1);
PURE String append(String _str0, char32_t _c);
PURE String string(char32_t _c);

template <typename _T>
inline PURE String _tuple_show(Tuple<_T> _t, String _str0)
{
    String _str = show(first(_t));
    _str = append(_str0, _str);
    return _str;
}

template <typename _U, typename _V, typename ..._T>
inline PURE String _tuple_show(Tuple<_U, _V, _T...> _t, String _str0)
{
    String _str = show(first(_t));
    _str = append(_str0, _str);
    _str = append(_str, ',');
    Tuple<_V, _T...> _u = {_t._impl + 1};
    return _tuple_show<_V, _T...>(_u, _str);
}

/**
 * Tuple show.
 * O(n).
 */
template <typename... _T>
inline PURE String show(Tuple<_T...> _t)
{
    String _str = string('(');
    _str = _tuple_show<_T...>(_t, _str);
    _str = append(_str, ')');
    return _str;
}

}           /* namespace F */

#include "fstring.h"

#endif      /* _FTUPLE_H */
