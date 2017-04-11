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

#ifndef _FSET_H
#define _FSET_H

#include "fbase.h"
#include "fcompare.h"
#include "fshow.h"
#include "ftree.h"
#include "fvalue.h"

#include "flist_defs.h"
#include "fset_defs.h"
#include "fstring_defs.h"

namespace F
{

template <typename _T>
int _set_compare_wrapper(Value<Word> _k1, Value<Word> _k2)
{
    Value<_T> _a0 = _bit_cast<Value<_T>>(_k1);
    Value<_T> _b0 = _bit_cast<Value<_T>>(_k2);
    const _T &_a = _a0;
    const _T &_b = _b0;
    return compare(_a, _b);
}

/**
 * Construct the empty set.
 * O(1).
 */
template <typename _T>
inline PURE Set<_T> set(void)
{
    Set<_T> _s = {_tree_empty()};
    return _s;
}

/**
 * Construct a set from a list.
 * O(n * log(n)).
 */
template <typename _T>
inline PURE Set<_T> set(List<_T> _xs)
{
    Set<_T> _s = {_tree_from_list(_bit_cast<List<Value<Word>>>(_xs),
        _set_compare_wrapper<_T>)};
    return _s;
}

/**
 * Test if a set is empty.
 * O(1).
 */
template <typename _T>
inline PURE bool empty(Set<_T> _xs)
{
    return _tree_is_empty(_xs._impl);
}

/**
 * Test if an element is a member of a set.
 * O(log(n)).
 */
template <typename _T>
inline PURE bool find(Set<_T> _s, const _T &_k)
{
    Value<_T> _k1 = _k;
    auto _entry = _tree_search(_s._impl, _bit_cast<Value<Word>>(_k1),
        _set_compare_wrapper<_T>);
    return (_entry != nullptr);
}

/**
 * Set insert.
 * O(log(n)).
 */
template <typename _T>
inline PURE Set<_T> insert(Set<_T> _s, const _T &_k)
{
    Value<_T> _k1 = _k;
    Set<_T> _s1 = {_tree_insert(_s._impl, _bit_cast<Value<Word>>(_k1),
            _set_compare_wrapper<_T>)};
    return _s1;
}

/**
 * Set remove.
 * O(log(n)).
 */
template <typename _T>
inline PURE Set<_T> erase(Set<_T> _s, const _T &_k)
{
    Value<_T> _k1 = _k;
    Set<_T> _s1 = {_tree_delete(_s._impl, _bit_cast<Value<Word>>(_k1),
        _set_compare_wrapper<_T>)};
    return _s1;
}

/**
 * Set union (merge).
 * O(log(n) + log(m)).
 */
template <typename _T>
inline PURE Set<_T> merge(Set<_T> _s, Set<_T> _t)
{
    Set<_T> _u = {_tree_union(_s._impl, _t._impl, _set_compare_wrapper<_T>)};
    return _u;
}

/**
 * Set intersect.
 * O(log(n) + log(m)).
 */
template <typename _T>
inline PURE Set<_T> intersect(Set<_T> _s, Set<_T> _t)
{
    Set<_T> _u = {_tree_intersect(_s._impl, _t._impl,
        _set_compare_wrapper<_T>)};
    return _u;
}

/**
 * Set difference.
 * O(log(n) + log(m)).
 */
template <typename _T>
inline PURE Set<_T> diff(Set<_T> _s, Set<_T> _t)
{
    Set<_T> _u = {_tree_diff(_s._impl, _t._impl, _set_compare_wrapper<_T>)};
    return _u;
}

/**
 * Set size.
 * O(n).
 */
template <typename _T>
inline PURE size_t size(Set<_T> _s)
{
    return _tree_size(_s._impl);
}

/**
 * Set fold left. ([](A a, T x) -> A).
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldl(Set<_T> _s, const _A &_arg, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, Value<Word>) =
        [](void *_func_0, Value<Word> _a0, Value<Word> _k0) -> Value<Word>
    {
        Value<_A> _a = _bit_cast<Value<_A>>(_a0);
        Value<_T> _k = _bit_cast<Value<_T>>(_k0);
        _F *_func_1 = (_F *)_func_0;
        Value<_A> _b = (*_func_1)(_a, _k);
        return _bit_cast<Value<Word>>(_b);
    };
    Value<_A> _arg1 = _arg;
    Value<Word> _r = _tree_foldl(_s._impl, _bit_cast<Value<Word>>(_arg1),
        _func_ptr, (void *)&_func);
    return _bit_cast<Value<_A>>(_r);
}

/**
 * Set fold right. ([](A a, T x) -> A).
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldr(Set<_T> _s, const _A &_arg, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, Value<Word>) =
        [](void *_func_0, Value<Word> _a0, Value<Word> _k0) -> Value<Word>
    {
        Value<_A> _a = _bit_cast<Value<_A>>(_a0);
        Value<_T> _k = _bit_cast<Value<_T>>(_k0);
        _F *_func_1 = (_F *)_func_0;
        Value<_A> _b = (*_func_1)(_a, _k);
        return _bit_cast<Value<Word>>(_b);
    };
    Value<_A> _arg1 = _arg;
    Value<Word> _r = _tree_foldr(_s._impl, _bit_cast<Value<Word>>(_arg1),
        _func_ptr, (void *)&_func);
    return _bit_cast<Value<_A>>(_r);
}

/**
 * Set compare.
 * O(n).
 */
template <typename _T>
inline PURE int compare(Set<_T> _s, Set<_T> _t)
{
    int (*_func_ptr)(void *, Value<Word>, Value<Word>) =
        [](void *_unused, Value<Word> _ks0, Value<Word> _kt0) -> int
    {
        Value<_T> _ks1 = _bit_cast<Value<_T>>(_ks0);
        Value<_T> _kt1 = _bit_cast<Value<_T>>(_kt0);
        const _T &_ks = _ks1;
        const _T &_kt = _kt1;
        return compare(_ks, _kt);
    };
    return _tree_compare(_s._impl, _t._impl, nullptr, _func_ptr);
}

/**
 * Set show.
 * O(n).
 */
template <typename _T>
inline PURE String show(Set<_T> _s)
{
    String (*_func_ptr)(Value<Word>) =
        [] (Value<Word> _k0) -> String
    {
        Value<_T> _k1 = _bit_cast<Value<_T>>(_k0);
        const _T &_k = _k1;
        return show(_k);
    };
    return _tree_show(_s._impl, _func_ptr);
}

/**
 * Construct an iterator pointing to the least element of a set.
 * O(1).
 */
template <typename _T>
inline PURE SetItr<_T> begin(Set<_T> _s)
{
    SetItr<_T> _itr;
    _itr._tree_itr = begin(_s._impl);
    return _itr;
}

/**
 * Construct an iterator representing one past the greatest element of a set.
 * O(1).
 */
template <typename _T>
inline PURE SetItr<_T> end(Set<_T> _s)
{
    SetItr<_T> _itr;
    _itr._tree_itr = end(_s._impl);
    return _itr;
}

/**
 * Set iterator increment.
 * O(1).
 */
template <typename _T>
inline SetItr<_T> &operator++(SetItr<_T> &_i)
{
    ++_i._tree_itr;
    return _i;
}

/**
 * Set iterator decrement.
 * O(1).
 */
template <typename _T>
inline SetItr<_T> &operator--(SetItr<_T> &_i)
{
    --_i._tree_itr;
    return _i;
}

/**
 * Set iterator add offset.
 * O(1).
 */
template <typename _T>
inline SetItr<_T> &operator+(SetItr<_T> &_i, ssize_t _offset)
{
    _i._tree_itr += _offset;
    return _i;
}

/**
 * Set iterator substract offset.
 * O(1).
 */
template <typename _T>
inline SetItr<_T> &operator-(SetItr<_T> &_i, ssize_t _offset)
{
    _i._tree_itr -= _offset;
    return _i;
}

/**
 * Set iterator add offset.
 * O(1).
 */
template <typename _T>
inline SetItr<_T> &operator+=(SetItr<_T> &_i, ssize_t _offset)
{
    _i._tree_itr += _offset;
    return _i;
}

/**
 * Set iterator substract offset.
 * O(1).
 */
template <typename _T>
inline SetItr<_T> &operator-=(SetItr<_T> &_i, ssize_t _offset)
{
    _i._tree_itr -= _offset;
    return _i;
}

/**
 * Set iterator dereference.
 * O(log(delta)), where delta is distance to last dereference.
 */
template <typename _T>
inline PURE const _T &operator*(SetItr<_T> &_i)
{
    const Value<Word> &_k = *_i._tree_itr;
    return (Value<_T> &)_k;
}

/**
 * Set verify.
 * O(n).
 */
template <typename _T>
inline PURE bool verify(Set<_T> _s)
{
    return _tree_verify(_s._impl);
}

/**
 * Set iterator same offset.
 * O(1).
 */
template <typename _T>
inline PURE bool operator==(const SetItr<_T> &_i, const SetItr<_T> &_j)
{
    return (_i._tree_itr == _j._tree_itr);
}

/**
 * Set iterator different offset.
 * O(1).
 */
template <typename _T>
inline PURE bool operator!=(const SetItr<_T> &_i, const SetItr<_T> &_j)
{
    return (_i._tree_itr != _j._tree_itr);
}

}           /* namespace F */

#include "flist.h"
#include "fstring.h"

#endif      /* _FSET_H */
