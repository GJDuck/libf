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

#ifndef _FSTRING_H
#define _FSTRING_H

#include "fdefs.h"
#include "fbase.h"
#include "flist.h"
#include "fseq.h"

#include <string.h>

namespace F
{

extern PURE Any _string_frag_foldl(_Frag _frag, Any _arg,
    Any (*_f)(void *, Any, char32_t), void *_data);
extern PURE Any _string_frag_foldr(_Frag _frag, Any _arg,
    Any (*_f)(void *, Any, char32_t), void *_data);
extern PURE _Frag _string_frag_map(_Frag _frag,
    char32_t (*_f)(void *, char32_t), void *_data);
extern PURE _Frag _string_frag_filter_map(_Frag _frag,
    Result<bool, char32_t> (*_f)(void *, char32_t), void *_data);
extern PURE _Seq _string_init(const char *_str0);
extern PURE _Seq _string_init_with_char(char32_t c);
extern PURE char *_string_cstring(_Seq _s);
extern PURE List<char32_t> _string_list(_Seq s);
extern PURE char32_t _string_lookup(_Seq _s, size_t _idx);
extern PURE _Seq _string_append_char(_Seq _s, char32_t _c);
extern PURE _Seq _string_append_cstring(_Seq _s, const char *_str);
extern PURE Result<_Seq, _Seq> _string_split(_Seq _s, size_t _idx);
extern PURE _Seq _string_left(_Seq _s, size_t _idx);
extern PURE _Seq _string_right(_Seq _s, size_t _idx);
extern PURE _Seq _string_between(_Seq _s, size_t _lidx, size_t _ridx);
extern PURE _Seq _string_insert(_Seq _s, size_t _idx, _Seq _t);
extern PURE int _string_frag_compare(void *, _Frag _a, size_t _idx1,
    _Frag _b, size_t _idx2);

/*
 * str = string()
 * Constructs the empty string `str'.
 * O(1).
 */
inline PURE String string(void)
{
    String _str = {_SEQ_EMPTY};
    return _str;
}

/*
 * str = string(cstr)
 * Constructs a string `str' from C-string `cstr'.
 * O(n).
 */
inline PURE String string(const char *_str0)
{
    String _str = {_string_init(_str0)};
    return _str;
}

/*
 * str = string(c)
 * Constructs a singleton string `str' from character `c'.
 * O(1).
 */
inline PURE String string(char32_t _c)
{
    String _str = {_string_init_with_char(_c)};
    return _str;
}

/*
 * Convert to C-string.
 * O(n).
 */
inline PURE const char *cstr(String _str)
{
    return _string_cstring(_str._impl);
}

/*
 * Append.
 * O(min(log(n), log(m))).
 */
inline PURE String append(String _str0, String _str1)
{
    String _str = {_seq_append(_str0._impl, _str1._impl)};
    return _str;
}

/*
 * Append char.
 * O(1).
 */
inline PURE String append(String _str0, char32_t _c)
{
    String _str = {_string_append_char(_str0._impl, _c)};
    return _str;
}

/*
 * Append C-string.
 * O(n), n = len(cstr).
 */
inline PURE String append(String _str0, const char *_cstr)
{
    String _str = {_string_append_cstring(_str0._impl, _cstr)};
    return _str;
}

/*
 * Length.
 * O(1).
 */
inline PURE size_t length(String _str)
{
    return _seq_length(_str._impl);
}

/*
 * Lookup.
 * O(log(n)).
 */
inline PURE char32_t lookup(String _s, size_t _idx)
{
    return _string_lookup(_s._impl, _idx);
}

/*
 * Split.
 * O(log(n)).
 */
inline PURE Result<String, String> split(String _s, size_t _idx)
{
    Result<_Seq, _Seq> _r = _string_split(_s._impl, _idx);
    String _sl = {_r.fst};
    String _sr = {_r.snd};
    return {_sl, _sr};
}

/*
 * Left split.
 * O(log(n)).
 */
inline PURE String left(String _s, size_t _idx)
{
    String _sl = {_string_left(_s._impl, _idx)};
    return _sl;
}

/*
 * Right split.
 * O(log(n)).
 */
inline PURE String right(String _s, size_t _idx)
{
    String _sr = {_string_right(_s._impl, _idx)};
    return _sr;
}

/*
 * Sub-string between.
 * O(log(n)).
 */
inline PURE String between(String _s, size_t _lidx, size_t _ridx)
{
    String _sb = {_string_between(_s._impl, _lidx, _ridx)};
    return _sb;
}

/*
 * Insert sub-string.
 * O(log(n)).
 */
inline PURE String insert(String _s, size_t _idx, String _t)
{
    String _sr = {_string_insert(_s._impl, _idx, _t._impl)};
    return _sr;
}

/*
 * Constructor.
 * O(n).
 */
inline PURE List<char32_t> list(String _s)
{
    return _string_list(_s._impl);
}

/*
 * String show.
 * O(n).
 */
extern PURE String show(String _s);

/*
 * String compare.
 * O(n).
 */
inline PURE int compare(String _s, String _t)
{
    return _seq_compare(_s._impl, _t._impl, nullptr, _string_frag_compare);
}

/*
 * String fold left.
 * O(n).
 */
template <typename _T, typename _F>
inline PURE _T foldl(String _str, _T _arg, _F _func)
{
    Any (*_func_ptr)(void *, Any, _Frag) =
        [](void *_func_0, Any _a0, _Frag _k0) -> Any
    {
        Any (*_func_ptr_1)(void *, Any, char32_t) =
            [](void *_func_0, Any _a0, char32_t _c) -> Any
        {
            _F *_func_1 = (_F *)_func_0;
            _T _a = cast<_T>(_a0);
            _T _b = (*_func_1)(_a, _c);
            return cast<Any>(_b);
        };
        return _string_frag_foldl(_k0, _a0, _func_ptr_1, _func_0);
    };
    Any _r = _seq_foldl(_str._impl, cast<Any>(_arg), _func_ptr, (void *)&_func);
    return cast<_T>(_r);
}

/*
 * String fold right.
 * O(n).
 */
template <typename _T, typename _F>
inline PURE _T foldr(String _str, _T _arg, _F _func)
{
    Any (*_func_ptr)(void *, Any, _Frag) =
        [](void *_func_0, Any _a0, _Frag _k0) -> Any
    {
        Any (*_func_ptr_1)(void *, Any, char32_t) =
            [](void *_func_0, Any _a0, char32_t _c) -> Any
        {
            _F *_func_1 = (_F *)_func_0;
            _T _a = cast<_T>(_a0);
            _T _b = (*_func_1)(_a, _c);
            return cast<Any>(_b);
        };
        return _string_frag_foldr(_k0, _a0, _func_ptr_1, _func_0);
    };
    Any _r = _seq_foldr(_str._impl, cast<Any>(_arg), _func_ptr, (void *)&_func);
    return cast<_T>(_r);
}

/*
 * String map.
 * O(n).
 */
template <typename _F>
inline PURE String map(String _str, _F _func)
{
    _Frag (*_func_ptr)(void *, _Frag) =
        [](void *_func_0, _Frag _k0) -> _Frag
    {
        char32_t (*_func_ptr_1)(void *, char32_t) =
            [](void *_func_0, char32_t _a0) -> char32_t
        {
            _F *_func_1 = (_F *)_func_0;
            return (*_func_1)(_a0);
        };
        return _string_frag_map(_k0, _func_ptr_1, _func_0);
    };
    String _r = {_seq_map(_str._impl, _func_ptr, (void *)*_func)};
    return _r;
}

/*
 * String filter.
 * O(n).
 */
template <typename _F>
inline PURE String filter(String _str, _F _func)
{
    Any (*_func_ptr)(void *, Any, _Frag) =
        [](void *_func_0, Any _a0, _Frag _k0) -> Any
    {
        Result<bool, char32_t> (*_func_ptr_1)(void *, char32_t) =
            [](void *_func_0, char32_t _a0) -> Result<bool, char32_t>
        {
            _F *_func_1 = (_F *)_func_0;
            return {(*_func_1)(_a0), _a0};
        };
        _Frag _k = _string_frag_filter_map(_k0, _func_ptr_1, _func_0);
        if (_k == nullptr)
            return _a0;
        _Seq _a = cast<_Seq>(_a0);
        _Seq _a1 = _seq_push_back(_a, _k);
        return cast<Any>(_a1);
    };
    Any _r = _seq_foldl(_str._impl, cast<Any>(_SEQ_EMPTY), _func_ptr,
        (void *)&_func);
    String _r1 = {cast<_Seq>(_r)};
    return _r1;
}

/*
 * String filter map.
 * O(n).
 */
template <typename _F>
inline PURE String filter_map(String _str, _F _func)
{
    Any (*_func_ptr)(void *, Any, _Frag) =
        [](void *_func_0, Any _a0, _Frag _k0) -> Any
    {
        Result<bool, char32_t> (*_func_ptr_1)(void *, char32_t) =
            [](void *_func_0, char32_t _a0) -> Result<bool, char32_t>
        {
            _F *_func_1 = (_F *)_func_0;
            return (*_func_1)(_a0);
        };
        _Frag _k = _string_frag_filter_map(_k0, _func_ptr_1, _func_0);
        if (_k == nullptr)
            return _a0;
        _Seq _a = cast<_Seq>(_a0);
        _Seq _a1 = _seq_push_back(_a, _k);
        return cast<Any>(_a1);
    };
    Any _r = _seq_foldl(_str._impl, cast<Any>(_SEQ_EMPTY), _func_ptr,
        (void *)&_func);
    String _r1 = {cast<_Seq>(_r)};
    return _r1;
}

/*
 * Verify.
 * O(n).
 */
inline PURE bool verify(String _s)
{
    return _seq_verify(_s._impl);
}

}           /* namespace F */

#endif      /* _FSTRING_H */
