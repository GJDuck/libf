/*
 * Copyright (c) 2017 The National University of Singapore.
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

#include "fbase.h"
#include "flist.h"
#include "fseq.h"

#include "flist_defs.h"
#include "fstring_defs.h"

#include <cstring>

namespace F
{

extern PURE Value<Word> _string_frag_foldl(size_t _idx, _Frag _frag,
    Value<Word> _arg,
    Value<Word> (*_f)(void *, Value<Word>, size_t, char32_t), void *_data);
extern PURE Value<Word> _string_frag_foldr(size_t _idx, _Frag _frag,
    Value<Word> _arg,
    Value<Word> (*_f)(void *, Value<Word>, size_t, char32_t), void *_data);
extern PURE _Frag _string_frag_map(size_t _idx, _Frag _frag,
    char32_t (*_f)(void *, size_t, char32_t), void *_data);
extern PURE Optional<_Frag> _string_frag_filter_map(size_t _idx, _Frag _frag,
    Optional<char32_t> (*_f)(void *, size_t, char32_t), void *_data);
extern PURE _Seq _string_init(const char *_str0);
extern PURE _Seq _string_init_with_char(char32_t c);
extern PURE char *_string_cstring(_Seq _s);
extern PURE List<char32_t> _string_list(_Seq s);
extern PURE char32_t _string_lookup(_Seq _s, size_t _idx);
extern PURE char32_t _string_frag_lookup(_Frag frag, size_t idx);
extern PURE char32_t _string_search(_Seq _s, size_t _idx);
extern PURE _Seq _string_append_char(_Seq _s, char32_t _c);
extern PURE _Seq _string_append_cstring(_Seq _s, const char *_str);
extern PURE Result<_Seq, _Seq> _string_split(_Seq _s, size_t _idx);
extern PURE _Seq _string_left(_Seq _s, size_t _idx);
extern PURE _Seq _string_right(_Seq _s, size_t _idx);
extern PURE _Seq _string_between(_Seq _s, size_t _lidx, size_t _ridx);
extern PURE _Seq _string_insert(_Seq _s, size_t _idx, _Seq _t);
extern PURE _Seq _string_delete(_Seq _s, size_t _lidx, size_t _ridx);
extern PURE int _string_frag_compare(void *, _Frag _a, size_t _idx1,
    _Frag _b, size_t _idx2);

/**
 * Construct the empty string.
 * O(1).
 */
inline PURE String string()
{
    String _str = {_seq_empty()};
    return _str;
}

/**
 * Construct a string from C-string `cstr'.
 * O(n).
 */
inline PURE String string(const char *_cstr)
{
    String _str = {_string_init(_cstr)};
    return _str;
}

/**
 * Construct a singleton string from a character `c'.
 * O(1).
 */
inline PURE String string(char32_t _c)
{
    String _str = {_string_init_with_char(_c)};
    return _str;
}

/**
 * Test if a string is empty.
 * O(1).
 */
inline PURE bool empty(String _str)
{
    return _seq_is_empty(_str._impl);
}

/**
 * Convert a string into a C-string.
 * O(n).
 */
inline PURE const char *c_str(String _str)
{
    return _string_cstring(_str._impl);
}

/**
 * Append strings.
 * O(min(log(n), log(m))).
 */
inline PURE String append(String _str0, String _str1)
{
    String _str = {_seq_append(_str0._impl, _str1._impl)};
    return _str;
}

/**
 * Append character `c'.
 * O(1).
 */
inline PURE String append(String _str0, char32_t _c)
{
    String _str = {_string_append_char(_str0._impl, _c)};
    return _str;
}

/**
 * Append C-string `cstr'.
 * O(n), n = len(cstr).
 */
inline PURE String append(String _str0, const char *_cstr)
{
    String _str = {_string_append_cstring(_str0._impl, _cstr)};
    return _str;
}

/**
 * Alias of `append'.
 * O(min(log(n), log(m))).
 */
inline PURE String operator+(String _str0, String _str1)
{
    String _str = {_seq_append(_str0._impl, _str1._impl)};
    return _str;
}

/**
 * Alias of `append'.
 * O(1).
 */
inline PURE String operator+(String _str0, char32_t _c)
{
    String _str = {_string_append_char(_str0._impl, _c)};
    return _str;
}

/**
 * Alias of `append'.
 * O(n), n = len(cstr).
 */
inline PURE String operator+(String _str0, const char *_cstr)
{
    String _str = {_string_append_cstring(_str0._impl, _cstr)};
    return _str;
}

/**
 * Destructively append strings.
 * O(min(log(n), log(m))).
 */
inline String &operator+=(String &_str0, String _str1)
{
    String _str = {_seq_append(_str0._impl, _str1._impl)};
	_str0 = _str;
    return _str0;
}

/**
 * Destructively append character `c'.
 * O(1).
 */
inline String &operator+=(String &_str0, char32_t _c)
{
    String _str = {_string_append_char(_str0._impl, _c)};
	_str0 = _str;
    return _str0;
}

/**
 * Destructively append C-string `cstr'.
 * O(n), n = len(cstr).
 */
inline String &operator+=(String &_str0, const char *_cstr)
{
    String _str = {_string_append_cstring(_str0._impl, _cstr)};
    _str0 = _str;
	return _str0;
}

/**
 * String size (a.k.a. string length).
 * O(1).
 */
inline PURE size_t size(String _str)
{
    return _seq_length(_str._impl);
}

/**
 * Get the character at index `idx'.
 * O(log(n)).
 */
inline PURE Optional<char32_t> at(String _s, size_t _idx)
{
    char32_t _c = _string_search(_s._impl, _idx);
	return (_c == 0? Optional<char32_t>(): Optional<char32_t>(_c));
}

/**
 * Get the character at index `idx'.
 * O(log(n)).
 */
inline PURE char32_t lookup(String _s, size_t _idx)
{
    Optional<char32_t> _r = at(_s, _idx);
    if (empty(_r))
        error("string lookup out-of-bounds");
    return char32_t(_r);
}

/**
 * Find the first occurence of a sub-string.
 * O(n.m.log(m)), m = len(t).
 */
extern PURE Optional<size_t> find(String _s, String _t, size_t _pos = 0);

/**
 * Find the first occurence of a C-sub-string.
 * O(n.m), m = len(t).
 */
extern PURE Optional<size_t> find(String _s, const char *_t, size_t _pos = 0);

/**
 * Find the first occurence of the character `c'.
 * O(n).
 */
extern PURE Optional<size_t> find(String _s, char32_t _c, size_t _pos = 0);

/**
 * Replace the first occurence of sub-string `t' with string `r'.
 */
extern PURE Result<String, Optional<size_t>> replace(String _s, String _t, String _r, size_t _pos = 0); 

/**
 * Replace the first occurence of C-sub-string `t' with string `r'.
 */
extern PURE Result<String, Optional<size_t>> replace(String _s, const char *_t, String _r, size_t _pos = 0); 

/**
 * Replace all occurences of sub-string `t' with string `r'.
 */
extern PURE String replace_all(String _s, String _t, String _r, size_t _pos = 0);

/**
 * Replace all occurences of C-sub-string `t' with string `r'.
 */
extern PURE String replace_all(String _s, const char *_t, String _r, size_t _pos = 0);

/**
 * Split.
 * O(log(n)).
 */
inline PURE Result<String, String> split(String _s, size_t _idx)
{
    auto [_sl0, _sr0] = _string_split(_s._impl, _idx);
    String _sl = {_sl0};
    String _sr = {_sr0};
    return {_sl, _sr};
}

/**
 * Left split.
 * O(log(n)).
 */
inline PURE String left(String _s, size_t _idx)
{
    String _sl = {_string_left(_s._impl, _idx)};
    return _sl;
}

/**
 * Right split.
 * O(log(n)).
 */
inline PURE String right(String _s, size_t _idx)
{
    String _sr = {_string_right(_s._impl, _idx)};
    return _sr;
}

/**
 * Extract the sub-string at `idx' with `count' characters.
 * O(log(n)).
 */
inline PURE String between(String _s, size_t _idx, size_t _count)
{
    String _sb = {_string_between(_s._impl, _idx, _idx + _count)};
    return _sb;
}

/**
 * Insert the string `t' at index `idx'.
 * O(log(n)).
 */
inline PURE String insert(String _s, size_t _idx, String _t)
{
    String _sr = {_string_insert(_s._impl, _idx, _t._impl)};
    return _sr;
}

/**
 * Erase the sub-string at `idx' with `count' characters.
 * O(log(n)).
 */
inline PURE String erase(String _s, size_t _idx, size_t _count = 1)
{
    String _s2 = {_string_delete(_s._impl, _idx, _idx + _count)};
    return _s2;
}

/**
 * String show.
 * O(n).
 */
extern PURE String show(String _str);

/**
 * String compare.
 * O(n).
 */
inline PURE int compare(String _s, String _t)
{
    return _seq_compare(_s._impl, _t._impl, nullptr, _string_frag_compare);
}

/**
 * String fold left. ([](T, size_t idx, char32_t c) -> T).
 * O(n).
 */
template <typename _T, typename _F>
inline PURE _T foldl(String _str, const _T &_arg, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, size_t, _Frag) =
        [](void *_func_0, Value<Word> _a0, size_t _idx, _Frag _k0) ->
            Value<Word>
    {
        Value<Word> (*_func_ptr_1)(void *, Value<Word>, size_t, char32_t) =
            [](void *_func_0, Value<Word> _a0, size_t _idx, char32_t _c) ->
                Value<Word>
        {
            _F *_func_1 = (_F *)_func_0;
            Value<_T> _a = _bit_cast<Value<_T>>(_a0);
            Value<_T> _b = (*_func_1)(_a, _idx, _c);
            return _bit_cast<Value<Word>>(_b);
        };
        return _string_frag_foldl(_idx, _k0, _a0, _func_ptr_1, _func_0);
    };
    Value<_T> _arg1 = _arg;
    Value<Word> _r = _seq_foldl(_str._impl, _bit_cast<Value<Word>>(_arg1),
        _func_ptr, (void *)&_func);
    return _bit_cast<Value<_T>>(_r);
}

/**
 * String fold right. ([](T, size_t idx, char32_t c) -> T).
 * O(n).
 */
template <typename _T, typename _F>
inline PURE _T foldr(String _str, const _T &_arg, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, size_t, _Frag) =
        [](void *_func_0, Value<Word> _a0, size_t _idx, _Frag _k0) ->
            Value<Word>
    {
        Value<Word> (*_func_ptr_1)(void *, Value<Word>, size_t, char32_t) =
            [](void *_func_0, Value<Word> _a0, size_t _idx, char32_t _c) ->
                Value<Word>
        {
            _F *_func_1 = (_F *)_func_0;
            Value<_T> _a = _bit_cast<Value<_T>>(_a0);
            Value<_T> _b = (*_func_1)(_a, _idx, _c);
            return _bit_cast<Value<Word>>(_b);
        };
        return _string_frag_foldr(_idx, _k0, _a0, _func_ptr_1, _func_0);
    };
    Value<_T> _arg1 = _arg;
    Value<Word> _r = _seq_foldr(_str._impl, _bit_cast<Value<Word>>(_arg1),
        _func_ptr, (void *)&_func);
    return _bit_cast<Value<_T>>(_r);
}

/**
 * String map. ([](size_t idx, char32_t c) -> char32_t).
 * O(n).
 */
template <typename _F>
inline PURE String map(String _str, _F _func)
{
    _Frag (*_func_ptr)(void *, size_t, _Frag) =
        [](void *_func_0, size_t _idx, _Frag _k0) -> _Frag
    {
        char32_t (*_func_ptr_1)(void *, size_t, char32_t) =
            [](void *_func_0, size_t _idx, char32_t _a0) -> char32_t
        {
            _F *_func_1 = (_F *)_func_0;
            return (*_func_1)(_idx, _a0);
        };
        return _string_frag_map(_idx, _k0, _func_ptr_1, _func_0);
    };
    String _r = {_seq_map(_str._impl, _func_ptr, (void *)*_func)};
    return _r;
}

/**
 * String filter. ([](size_t idx, char32_t c) -> bool).
 * O(n).
 */
template <typename _F>
inline PURE String filter(String _str, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, size_t, _Frag) =
        [](void *_func_0, Value<Word> _a0, size_t _idx, _Frag _k0) ->
            Value<Word>
    {
        Optional<char32_t> (*_func_ptr_1)(void *, size_t, char32_t) =
            [](void *_func_0, size_t _idx, char32_t _a0) -> Optional<char32_t>
        {
            _F *_func_1 = (_F *)_func_0;
            return ((*_func_1)(_idx, _a0)?
                Optional<char32_t>(_a0): Optional<char32_t>());
        };
        Optional<_Frag> _k = _string_frag_filter_map(_idx, _k0, _func_ptr_1,
            _func_0);
        if (empty(_k))
            return _a0;
        Value<_Seq> _a = _bit_cast<Value<_Seq>>(_a0);
        Value<_Seq> _a1 = _seq_push_back(_a, _k);
        return _bit_cast<Value<Word>>(_a1);
    };
    Value<_Seq> _s0 = _seq_empty();
    Value<Word> _r = _seq_foldl(_str._impl, _bit_cast<Value<Word>>(_s0),
        _func_ptr, (void *)&_func);
    _Seq _s1 = _bit_cast<Value<_Seq>>(_r);
    String _r1 = {_s1};
    return _r1;
}

/**
 * String filter map. ([](size_t idx, char32_t c) -> Optional<char32_t>).
 * O(n).
 */
template <typename _F>
inline PURE String filter_map(String _str, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, size_t, _Frag) =
        [](void *_func_0, Value<Word> _a0, size_t _idx, _Frag _k0) ->
            Value<Word>
    {
        Optional<char32_t> (*_func_ptr_1)(void *, size_t, char32_t) =
            [](void *_func_0, size_t _idx, char32_t _a0) -> Optional<char32_t>
        {
            _F *_func_1 = (_F *)_func_0;
            return (*_func_1)(_idx, _a0);
        };
        Optional<_Frag> _k = _string_frag_filter_map(_idx, _k0, _func_ptr_1,
            _func_0);
        if (empty(_k))
            return _a0;
        Value<_Seq> _a = _bit_cast<Value<_Seq>>(_a0);
        Value<_Seq> _a1 = _seq_push_back(_a, _k);
        return _bit_cast<Value<Word>>(_a1);
    };
    Value<_Seq> _s0 = _seq_empty();
    Value<Word> _r = _seq_foldl(_str._impl, _bit_cast<Value<Word>>(_s0),
        _func_ptr, (void *)&_func);
    _Seq _s1 = _bit_cast<Value<_Seq>>(_r);
    String _r1 = {_s1};
    return _r1;
}

/**
 * Verify.
 * O(n).
 */
inline PURE bool verify(String _s)
{
    return _seq_verify(_s._impl);
}

/**
 * Construct an iterator pointing to the start of a string.
 * O(1).
 */
inline PURE StringItr begin(String _s)
{
    StringItr _itr;
    _itr._seq_itr = begin(_s._impl);
    return _itr;
}

/**
 * Construct an iterator pointing to the end of a string.
 * O(1).
 */
inline PURE StringItr end(String _s)
{
    StringItr _itr;
    _itr._seq_itr = end(_s._impl);
    return _itr;
}

/**
 * String iterator increment.
 * O(1).
 */
inline StringItr &operator ++(StringItr &_i)
{
    ++_i._seq_itr;
    return _i;
}

/**
 * String iterator decrement.
 * O(1).
 */
inline StringItr &operator --(StringItr &_i)
{
    --_i._seq_itr;
    return _i;
}

/**
 * String iterator add offset.
 * O(log(offset)).
 */
inline StringItr &operator +(StringItr &_i, ssize_t _offset)
{
    _i._seq_itr += _offset;
    return _i;
}

/**
 * String iterator substract offset.
 * O(log(offset))
 */
inline StringItr &operator -(StringItr &_i, ssize_t _offset)
{
    _i._seq_itr -= _offset;
    return _i;
}

/**
 * String iterator add offset.
 * O(log(offset))
 */
inline StringItr &operator +=(StringItr &_i, ssize_t _offset)
{
    _i._seq_itr += _offset;
    return _i;
}

/**
 * String iterator subtract offset.
 * O(log(offset))
 */
inline StringItr &operator -=(StringItr &_i, ssize_t _offset)
{
    _i._seq_itr -= _offset;
    return _i;
}

/**
 * String iterator dereference.
 * O(log(delta)), where delta is distance to last dereference.
 */
inline PURE char32_t operator *(StringItr &_i)
{
    size_t _idx;
    _Frag _frag = _seq_itr_get(&_i._seq_itr, &_idx);
    char32_t _r = _string_frag_lookup(_frag, _idx);
    return _r;
}

/**
 * String iterator same offset.
 * O(1).
 */
inline PURE bool operator ==(const StringItr &_i, const StringItr &_j)
{
    return (_i._seq_itr == _j._seq_itr);
}

/**
 * String iterator different offset.
 * O(1).
 */
inline PURE bool operator !=(const StringItr &_i, const StringItr &_j)
{
    return (_i._seq_itr != _j._seq_itr);
}

}           /* namespace F */

#include "flist.h"

#endif      /* _FSTRING_H */
