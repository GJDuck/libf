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

#ifndef _FLIST_H
#define _FLIST_H

#include "fbase.h"
#include "fcompare.h"
#include "flambda.h"
#include "ftuple.h"
#include "fvalue.h"

#include "flist_defs.h"
#include "fmap_defs.h"
#include "fset_defs.h"
#include "fstring_defs.h"

namespace F
{

#ifdef LINUX
typedef int (*_SortCompare)(const void *, const void *, void *);
#else
typedef int (*_SortCompare)(void *, const void *, const void *);
#endif

extern PURE List<char32_t> _string_list(_Seq s);

/*
 * Low-level implementation.
 */
extern PURE List<Word> _list_last(List<Word> _xs);
extern PURE size_t _list_length(List<Word> _xs);
extern PURE List<Word> _list_append(List<Word> _xs, List<Word> _ys);
extern PURE List<Word> _list_reverse(List<Word> _xs);
extern PURE List<Word> _list_zip(List<Word> xs, List<Word> ys);
extern PURE List<Word> _list_sort(List<Word> _xs, void *_data,
    _SortCompare _f);
extern PURE List<Word> _list_take(List<Word> _xs, size_t _len);
extern PURE List<Word> _list_take_while(List<Word> _xs,
    bool (*_f)(void *,Value<Word>), void *_data);
extern PURE Value<Word> _list_foldl(List<Word> _xs,
    Value<Word> (*_f)(void *,Value<Word>,Value<Word>), Value<Word> _a,
    void *_data);
extern PURE Value<Word> _list_foldr(List<Word> _xs,
    Value<Word> (*_f)(void *,Value<Word>,Value<Word>), Value<Word> _a,
    void *_data);
extern PURE List<Word> _list_map(List<Word> _xs,
    Value<Word> (*_f)(void *,Value<Word>), void *_data);
extern PURE List<Word> _list_filter(List<Word> _xs,
    bool (*f)(void *,Value<Word>), void *_data);
extern PURE int _list_compare(List<Word> _xs, List<Word> _ys,
    int (*_f)(Value<Word>, Value<Word>));
extern PURE String _list_show(List<Word> _xs, String (*_f)(Value<Word>));

/**
 * Construct an empty list.
 * O(1).
 */
template <typename _T>
inline PURE List<_T> list()
{
    return (Nil){};
}

/**
 * Constructor a list from a head and a tail.
 * O(1).
 */
template <typename _T>
inline PURE List<_T> list(const _T &_x, List<_T> _xs)
{
    return (Node<_T>){_x, _xs};
}

/**
 * Construct a list from a set.
 * O(n).
 */
template <typename _T>
inline PURE List<_T> list(Set<_T> _s)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>) =
        [] (void *_unused, Value<Word> _k) -> Value<Word>
    {
        return _k;
    };
    List<_T> _xs = _bit_cast<List<_T>>(_tree_to_list(_s._impl, _func_ptr,
        nullptr));
    return _xs;
}

/**
 * Construct a list from a map.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE List<Tuple<_K, _V>> list(Map<_K, _V> _m)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>) =
        [] (void *_unused, Value<Word> _k) -> Value<Word>
    {
        return _k;
    };
    List<Tuple<_K, _V>> _xs = _bit_cast<List<Tuple<_K, _V>>>(
        _tree_to_list(_m._impl, _func_ptr, nullptr));
    return _xs;
}

/**
 * Construct a list from a string.
 * O(n).
 */
inline PURE List<char32_t> list(String _s)
{
    return _string_list(_s._impl);
}

/**
 * Test if a list is empty.
 * O(1).
 */
template <typename _T>
inline PURE bool empty(List<_T> _xs)
{
    return (index(_xs) == NIL);
}

/**
 * Get the head of a list.
 * O(1).
 */
template <typename _T>
inline PURE const _T &head(List<_T> _xs)
{
    if (empty(_xs))
        error("head []");
    const Node<_T> &_node = _xs;
    return _node.elem;
}

/**
 * Get the tail of a list.
 * O(1).
 */
template <typename _T>
inline PURE List<_T> tail(List<_T> _xs) 
{
    if (empty(_xs))
        error("tail []");
    const Node<_T> &_node = _xs;
    return _node.next;
}

/**
 * Get the last element of a list.
 * O(n).
 */
template <typename _T>
inline PURE const _T &last(List<_T> _xs)
{
    _xs = _bit_cast<List<_T>>(_list_last(_bit_cast<List<Word>>(_xs)));
    const Node<_T> &_node = _xs;
    return _node.elem;
}

/**
 * List size (a.k.a. list length).
 * O(n).
 */
template <typename _T>
inline PURE size_t size(List<_T> _xs)
{
    return _list_length(_bit_cast<List<Word>>(_xs));
}

/**
 * List take `len' elements.
 * O(n).
 */
template <typename _T>
inline PURE List<_T> take(List<_T> _xs, size_t _len)
{
    return _bit_cast<List<_T>>(_list_take(_bit_cast<List<Word>>(_xs), _len));
}

/**
 * List take while `test' is true. ([](T elem) -> bool).
 * O(n).
 */
template <typename _T, typename _F>
inline PURE List<_T> take_while(List<_T> _xs, _F _test)
{
    bool (*_test_func_ptr)(void *, Value<Word>) =
        [](void *_test_0, Value<Word> _x) -> bool
    {
        Value<_T> _a = _bit_cast<Value<_T>>(_x);
        _F *_test_1 = (_F *)_test_0;
        return (*_test_1)(_a);
    };
    return _bit_cast<List<_T>>(_list_take_while(_bit_cast<List<Word>>(_xs),
        _test_func_ptr, (void *)&_test));
}

/**
 * List append.
 * O(n).
 */
template <typename _T>
inline PURE List<_T> append(List<_T> _xs, List<_T> _ys)
{
    return _bit_cast<List<_T>>(_list_append(_bit_cast<List<Word>>(_xs),
        _bit_cast<List<Word>>(_ys)));
}

/**
 * List reverse.
 * O(n).
 */
template <typename _T>
inline PURE List<_T> reverse(List<_T> _xs)
{
    return _bit_cast<List<_T>>(_list_reverse(_bit_cast<List<Word>>(_xs)));
}

/**
 * List zip.
 * O(n).
 */
template <typename _T, typename _U>
inline PURE List<Tuple<_T, _U>> zip(List<_T> _xs, List<_U> _ys)
{
    return _bit_cast<List<Tuple<_T, _U>>>(_list_zip(_bit_cast<List<Word>>(_xs),
        _bit_cast<List<Word>>(_ys)));
}

/**
 * List sort with comparison `cmp'. ([](_T a, _T b) -> int).
 * O(n^2).
 */
template <typename _T, typename _F>
inline PURE List<_T> sort(List<_T> _xs, _F _cmp)
{
    _SortCompare _cmp_func_ptr =
#ifdef LINUX
        [] (const void *_x, const void *_y, void *_cmp_0) -> int
#else
        [] (void *_cmp_0, const void *_x, const void *_y) -> int
#endif
    {
        const _T &_a = *(Value<_T> *)_x;
        const _T &_b = *(Value<_T> *)_y;
        _F *_cmp_1 = (_F *)_cmp_0;
        return (*_cmp_1)(_a, _b);
    };
    return _bit_cast<List<_T>>(_list_sort(_bit_cast<List<Word>>(_xs),
        (void *)(&_cmp), _cmp_func_ptr));
}

/**
 * List sort.
 * O(n^2).
 */
template <typename _T>
inline PURE List<_T> sort(List<_T> _xs)
{
    _SortCompare _cmp_func_ptr =
#ifdef LINUX
        [] (const void *_x, const void *_y, void *_unused) -> int
#else
        [] (void *_unused, const void *_x, const void *_y) -> int
#endif
    {
        const _T &_a = *(Value<_T> *)_x;
        const _T &_b = *(Value<_T> *)_y;
        return compare(_a, _b);
    };
    return _bit_cast<List<_T>>(_list_sort(_bit_cast<List<Word>>(_xs), nullptr,
        _cmp_func_ptr));
}

/**
 * List fold left. ([](A a, T x) -> A).
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldl(List<_T> _xs, const _A &_a, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, Value<Word>) =
        [] (void *_func_0, Value<Word> _a0, Value<Word> _x0) -> Value<Word>
    {
        Value<_A> _a = _bit_cast<Value<_A>>(_a0);
        Value<_T> _x = _bit_cast<Value<_T>>(_x0);
        _F *_func_1 = (_F *)_func_0;
        Value<_A> _b = (*_func_1)(_a, _x);
        return _bit_cast<Value<Word>>(_b);
    };
    Value<_A> _b = _a;
    return _bit_cast<Value<_A>>(_list_foldl(_bit_cast<List<Word>>(_xs),
        _func_ptr, _bit_cast<Value<Word>>(_b), (void *)&_func));
}

/**
 * List fold right. ([](A a, T x) -> A).
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldr(List<_T> _xs, const _A &_a, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, Value<Word>) =
        [] (void *_func_0, Value<Word> _a0, Value<Word> _x0) -> Value<Word>
    {
        Value<_A> _a = _bit_cast<Value<_A>>(_a0);
        Value<_T> _x = _bit_cast<Value<_T>>(_x0);
        _F *_func_1 = (_F *)_func_0;
        Value<_A> _b = (*_func_1)(_a, _x);
        return _bit_cast<Value<Word>>(_b);
    };
    Value<_A> _b = _a;
    return _bit_cast<Value<_A>>(_list_foldr(_bit_cast<List<Word>>(_xs),
        _func_ptr, _bit_cast<Value<Word>>(_b), (void *)&_func));
}

/**
 * List map. ([](T x) -> U).
 * O(n).
 */
template <typename _U, typename _T, typename _F>
inline PURE List<_U> map(List<_T> _xs, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>) =
        [] (void *_func_0, Value<Word> _x0) -> Value<Word>
    {
        Value<_T> _x = _bit_cast<Value<_T>>(_x0);
        _F *_func_1 = (_F *)_func_0;
        Value<_U> _y = (*_func_1)(_x);
        return _bit_cast<Value<Word>>(_y);
    };
    return _bit_cast<List<_U>>(_list_map(_bit_cast<List<Word>>(_xs),
        _func_ptr, (void *)&_func));
}

/**
 * List filter. ([](T x) -> bool).
 * O(n).
 */
template <typename _T, typename _F>
inline PURE List<_T> filter(List<_T> _xs, _F _func)
{
    bool (*_func_ptr)(void *, Value<Word>) =
        [] (void *_func_0, Value<Word> _x0) -> bool
    {
        Value<_T> _x = _bit_cast<Value<_T>>(_x0);
        _F *_func_1 = (_F *)_func_0;
        return (*_func_1)(_x);
    };
    return _bit_cast<List<_T>>(_list_filter(_bit_cast<List<Word>>(_xs),
        _func_ptr, (void *)&_func));
}

/**
 * List compare.
 * O(n).
 */
template <typename _T>
inline PURE int compare(List<_T> _xs, List<_T> _ys)
{
    int (*_cmp_func_ptr)(Value<Word>, Value<Word>) =
        [] (Value<Word> _x, Value<Word> _y) -> int
    {
        Value<_T> _a0 = _bit_cast<Value<_T>>(_x);
        Value<_T> _b0 = _bit_cast<Value<_T>>(_y);
        const _T &_a = _a0;
        const _T &_b = _b0;
        return compare(_a, _b);
    };
    return _list_compare(_bit_cast<List<Word>>(_xs),
        _bit_cast<List<Word>>(_ys), _cmp_func_ptr);
}

/**
 * List show.
 * O(n).
 */
template <typename _T>
inline PURE String show(List<_T> _xs)
{
    String (*_func_ptr)(Value<Word>) =
        [] (Value<Word> _x0) -> String
    {
        Value<_T> _x1 = _bit_cast<Value<_T>>(_x0);
        const _T &_x = _x1;
        return show(_x);
    };
    return _list_show(_bit_cast<List<Word>>(_xs), _func_ptr);
}

/**
 * Construct an iterator pointing to the start of a list.
 * O(1).
 */
template <typename _T>
inline PURE ListItr<_T> begin(List<_T> _xs)
{
    ListItr<_T> _itr = {_xs};
    return _itr;
}

/**
 * Construct an iterator pointing to the end of a list.
 * O(1).
 */
template <typename _T>
inline PURE ListItr<_T> end(List<_T> _xs)
{
    ListItr<_T> _itr = {list<_T>()};
    return _itr;
}

/**
 * List iterator increment.
 * O(1).
 */
template <typename _T>
inline ListItr<_T> &operator++(ListItr<_T> &_i)
{
    _i._list = tail(_i._list);
    return _i;
}

/**
 * List iterator dereference.
 * O(1).
 */
template <typename _T>
inline PURE const _T &operator*(ListItr<_T> &_i)
{
    return head(_i._list);
}

template <typename _T>
inline bool operator==(const ListItr<_T> &_i, const ListItr<_T> &_j)
{
    return ((empty(_i._list) && empty(_j._list)) ||
            (!empty(_i._list) && !empty(_j._list)));
}

template <typename _T>
inline bool operator!=(const ListItr<_T> &_i, const ListItr<_T> &_j)
{
    return ((!empty(_i._list) && empty(_j._list)) ||
            (empty(_i._list) && !empty(_j._list)));
}

}           /* namespace F */

#include "fmap.h"
#include "fset.h"
#include "fstring.h"

#endif      /* _FLIST_H */
