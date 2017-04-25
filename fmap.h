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

#ifndef _FMAP_H
#define _FMAP_H

#include "fbase.h"
#include "fcompare.h"
#include "ftree.h"

#include "flist_defs.h"
#include "fmap_defs.h"
#include "fstring_defs.h"
#include "ftuple_defs.h"

namespace F
{

template <typename _K>
int _map_compare_wrapper(Value<Word> _k1, Value<Word> _k2)
{
    Tuple<_K, Value<Word>> _a = _bit_cast<Tuple<_K, Value<Word>>>(_k1);
    Tuple<_K, Value<Word>> _b = _bit_cast<Tuple<_K, Value<Word>>>(_k2);
    return compare(first(_a), first(_b));
}

/**
 * Construct an empty map.
 * O(1).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> map()
{
    Map<_K, _V> _m = {_tree_empty()};
    return _m;
}

/**
 * Test if a map is empty.
 * O(1).
 */
template <typename _K, typename _V>
inline PURE bool empty(Map<_K, _V> _m)
{
    return _tree_is_empty(_m._impl);
}

/**
 * Insert a key-value pair into a map.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> insert(Map<_K, _V> _m, Tuple<_K, _V> _k)
{
    Map<_K, _V> _m1 = {_tree_insert(_m._impl, _bit_cast<Value<Word>>(_k),
        _map_compare_wrapper<_K>)};
    return _m1;
}

/**
 * Find an entry in the map.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Optional<Tuple<_K, _V>> find(Map<_K, _V> _m, const _K &_k)
{
    Value<_K> _k1 = _k;
    Tuple<_K, _V> _key = {_bit_cast<Value<Word> *>(&_k1)};
    auto _entry = _tree_search(_m._impl, _bit_cast<Value<Word>>(_key),
        _map_compare_wrapper<_K>);
    Tuple<_K, _V> *_entry1 = (Tuple<_K, _V> *)_entry;
    return (_entry1 != nullptr? Optional<Tuple<_K, _V>>(*_entry1):
        Optional<Tuple<_K, _V>>());
}

/**
 * Remove an entry from the map.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> erase(Map<_K, _V> _m, const _K &_k)
{
    Value<_K> _k1 = _k;
    Tuple<_K, _V> _key = {_bit_cast<Value<Word> *>(&_k1)};
    Map<_K, _V> _m1 = {_tree_delete(_m._impl, _bit_cast<Value<Word>>(_key),
        _map_compare_wrapper<_K>)};
    return _m1;
}

/**
 * Map size.
 * O(1).
 */
template <typename _K, typename _V>
inline PURE size_t size(Map<_K, _V> _m)
{
    return _tree_size(_m._impl);
}

/**
 * Map fold left. ([](A a, Tuple<K, V> e) -> A).
 * O(n).
 */
template <typename _K, typename _V, typename _A, typename _F>
inline PURE _A foldl(Map<_K, _V> _m, const _A &_arg, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, Value<Word>) =
        [](void *_func_0, Value<Word> _a0, Value<Word> _k0) -> Value<Word>
    {
        Value<_A> _a = _bit_cast<Value<_A>>(_a0);
        Tuple<_K, _V> _entry = _bit_cast<Tuple<_K, _V>>(_k0);
        _F *_func_1 = (_F *)_func_0;
        Value<_A> _b = (*_func_1)(_a, _entry);
        return _bit_cast<Value<Word>>(_b);
    };
    Value<_A> _arg1 = _arg;
    Value<Word> _r = _tree_foldl(_m._impl, _bit_cast<Value<Word>>(_arg1),
        _func_ptr, (void *)&_func);
    return _bit_cast<Value<_A>>(_r);
}

/**
 * Map fold right. ([](A a, Tuple<K, V> e) -> A).
 * O(n).
 */
template <typename _K, typename _V, typename _A, typename _F>
inline PURE _A foldr(Map<_K, _V> _m, const _A &_arg, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, Value<Word>) =
        [](void *_func_0, Value<Word> _a0, Value<Word> _k0) -> Value<Word>
    {
        Value<_A> _a = _bit_cast<Value<_A>>(_a0);
        Tuple<_K, _V> _entry = _bit_cast<Tuple<_K, _V>>(_k0);
        _F *_func_1 = (_F *)_func_0;
        Value<_A> _b = (*_func_1)(_a, _entry);
        return _bit_cast<Value<Word>>(_b);
    };
    Value<_A> _arg1 = _arg;
    Value<Word> _r = _tree_foldr(_m._impl, _bit_cast<Value<Word>>(_arg1),
        _func_ptr, (void *)&_func);
    return _bit_cast<Value<_A>>(_r);
}

/**
 * Map map. ([](Tuple<K, V> e) -> Tuple<K, W>).
 * O(n).
 */
template <typename _W, typename _K, typename _V, typename _F>
inline PURE Map<_K, _W> map(Map<_K, _V> _m, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>) =
        [](void *_func_0, Value<Word> _k0) -> Value<Word>
    {
        Tuple<_K, _V> _entry = _bit_cast<Tuple<_K, _V>>(_k0);
        _F *_func_1 = (_F *)_func_0;
        _W _w = (*_func_1)(_entry);
        Tuple<_K, _W> _new_entry = tuple<_K, _W>(first(_entry), _w);
        return _bit_cast<Value<Word>>(_new_entry);
    };
    Map<_K, _W> _m1 = {_tree_map(_m._impl, _func_ptr, (void *)&_func)};
    return _m1;
}

/**
 * All map keys.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE List<_K> keys(Map<_K, _V> _m)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>) =
        [] (void *_unused, Value<Word> _k0) -> Value<Word>
    {
        Tuple<_K, _V> _k = _bit_cast<Tuple<_K, _V>>(_k0);
        return _bit_cast<Value<Word>>(first(_k));
    };
    List<_K> _xs = _bit_cast<List<_K>>(
        _tree_to_list(_m._impl, _func_ptr, nullptr));
    return _xs;
}

/**
 * All map Values.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE List<_V> values(Map<_K, _V> _m)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>) =
        [] (void *_unused, Value<Word> _k0) -> Value<Word>
    {
        Tuple<_K, _V> _k = _bit_cast<Tuple<_K, _V>>(_k0);
        return _bit_cast<Value<Word>>(second(_k));
    };
    List<_V> _xs = _bit_cast<List<_V>>(
        _tree_to_list(_m._impl, _func_ptr, nullptr));
    return _xs;
}

/**
 * Map split.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Result<Map<_K, _V>, Map<_K, _V>> split(Map<_K, _V> _m, _K _k)
{
    Tuple<_K, _V> _key = {_bit_cast<Value<Word> *>(&_k)};
    auto [_tl, _tr] = _tree_split(_m._impl,
        _bit_cast<Value<Word>>(_key), _map_compare_wrapper<_K>);
    Result<Map<_K, _V>, Map<_K, _V>> _r = {{_tl}, {_tr}};
    return _r;
}

/**
 * Map merge.
 * O(log(n) + log(m)).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> merge(Map<_K, _V> _ma, Map<_K, _V> _mb)
{
    Map<_K, _V> _m1 = {_tree_union(_ma._impl, _mb._impl,
        _map_compare_wrapper<_K>)};
    return _m1;
}

/**
 * Map verify.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE bool verify(Map<_K, _V> _m)
{
    return _tree_verify(_m._impl);
}

/**
 * Map compare.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE int compare(Map<_K, _V> _m1, Map<_K, _V> _m2)
{
    int (*_func_ptr)(void *, Value<Word>, Value<Word>) =
        [] (void *_unused, Value<Word> _k10, Value<Word> _k20) -> int
    {
        Tuple<_K, _V> _k1 = _bit_cast<Tuple<_K, _V>>(_k10);
        Tuple<_K, _V> _k2 = _bit_cast<Tuple<_K, _V>>(_k20);
        return compare(_k1, _k2);
    };
    return _tree_compare(_m1._impl, _m2._impl, nullptr, _func_ptr);
}

/**
 * Map show.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE String show(Map<_K, _V> _m)
{
    String (*_func_ptr)(Value<Word>) = 
        [] (Value<Word> _k0) -> String
    {
        Tuple<_K, _V> _k = _bit_cast<Tuple<_K, _V>>(_k0);
        return append(append(show(first(_k)), "->"), show(second(_k)));
    };
    return _tree_show(_m._impl, _func_ptr);
}

/**
 * Construct an iterator pointing to the least entry of a map.
 * O(1).
 */
template <typename _K, typename _V>
inline PURE MapItr<_K, _V> begin(Map<_K, _V> _m)
{
    MapItr<_K, _V> _itr;
    _itr._tree_itr = begin(_m._impl);
    return _itr;
}

/**
 * Construct an iterator representing one past the greatest entry of a map.
 * O(1).
 */
template <typename _K, typename _V>
inline PURE MapItr<_K, _V> end(Map<_K, _V> _m)
{
    MapItr<_K, _V> _itr;
    _itr._tree_itr = end(_m._impl);
    return _itr;
}

/**
 * Map iterator increment.
 * O(1).
 */
template <typename _K, typename _V>
inline MapItr<_K, _V> &operator++(MapItr<_K, _V> &_i)
{
    ++_i._tree_itr;
    return _i;
}

/**
 * Map iterator decrement.
 * O(1).
 */
template <typename _K, typename _V>
inline MapItr<_K, _V> &operator--(MapItr<_K, _V> &_i)
{
    --_i._tree_itr;
    return _i;
}

/**
 * Map iterator add offset.
 * O(1).
 */
template <typename _K, typename _V>
inline MapItr<_K, _V> &operator+(MapItr<_K, _V> &_i, ssize_t _offset)
{
    _i._tree_itr += _offset;
    return _i;
}

/**
 * Map iterator substract offset.
 * O(1).
 */
template <typename _K, typename _V>
inline MapItr<_K, _V> &operator-(MapItr<_K, _V> &_i, ssize_t _offset)
{
    _i._tree_itr -= _offset;
    return _i;
}

/**
 * Map iterator add offset.
 * O(1).
 */
template <typename _K, typename _V>
inline MapItr<_K, _V> &operator+=(MapItr<_K, _V> &_i, ssize_t _offset)
{
    _i._tree_itr += _offset;
    return _i;
}

/**
 * Map iterator substract offset.
 * O(1).
 */
template <typename _K, typename _V>
inline MapItr<_K, _V> &operator-=(MapItr<_K, _V> &_i, ssize_t _offset)
{
    _i._tree_itr -= _offset;
    return _i;
}

/**
 * Map iterator dereference.
 * O(log(delta)), where delta is distance to last dereference.
 */
template <typename _K, typename _V>
inline PURE Tuple<_K, _V> operator*(MapItr<_K, _V> &_i)
{
    Value<Word> _k = *_i._tree_itr;
    return _bit_cast<Tuple<_K, _V>>(_k);
}

/**
 * Map iterator same offset.
 * O(1).
 */
template <typename _K, typename _V>
inline bool operator==(const MapItr<_K, _V> &_i, const MapItr<_K, _V> &_j)
{
    return (_i._tree_itr != _j._tree_itr);
}

/**
 * Map iterator different offset.
 * O(1).
 */
template <typename _K, typename _V>
inline bool operator!=(const MapItr<_K, _V> &_i, const MapItr<_K, _V> &_j)
{
    return (_i._tree_itr != _j._tree_itr);
}

}           /* namespace F */

#include "flist.h"
#include "fstring.h"
#include "ftuple.h"

#endif      /* _MAP_H */
