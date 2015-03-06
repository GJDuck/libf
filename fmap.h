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

#ifndef _FMAP_H
#define _FMAP_H

#include <stdlib.h>

#include "fdefs.h"
#include "fbase.h"
#include "fcompare.h"
#include "flist.h"
#include "fmaybe.h"
#include "fstring.h"
#include "ftree.h"
#include "ftuple.h"

namespace F
{

template <typename _K>
int _map_compare_wrapper(Any _k1, Any _k2)
{
    Tuple<_K, Any> _a = cast<Tuple<_K, Any>>(_k1);
    Tuple<_K, Any> _b = cast<Tuple<_K, Any>>(_k2);
    return compare(fst(_a), fst(_b));
}

/*
 * Constructor.
 * O(1).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> map(void)
{
    Map<_K, _V> _m = {_TREE_EMPTY};
    return _m;
}

/*
 * Replace+Search.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Result<Map<_K, _V>, Maybe<_V>> replace_search(Map<_K, _V> _m,
    _K _k, _V _v)
{
    Tuple<_K, _V> _entry = tuple(_k, _v);
    int _flags = _TREE_INSERT_REPLACE_FLAG;
    Result<_Tree, Any *> _r0 = _tree_insert(_m._impl, cast<Any>(_entry),
        _flags, _map_compare_wrapper<_K>);
    Map<_K, _V> _m1 = {_r0.fst};
    Maybe<_V> _old_v = (_r0.snd != nullptr?
        maybe(snd_ptr(*cast<Tuple<_K, _V> *>(_r0.snd))):
        nothing<_V>());
    Result<Map<_K, _V>, Maybe<_V>> _r = {_m1, _old_v};
    return _r;
}

/*
 * Replace.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> replace(Map<_K, _V> _m, _K _k, _V _v)
{
    Tuple<_K, _V> _entry = tuple(_k, _v);
    int _flags = _TREE_INSERT_REPLACE_FLAG;
    Result<_Tree, Any *> _r0 = _tree_insert(_m._impl, cast<Any>(_entry),
        _flags, _map_compare_wrapper<_K>);
    check(_r0.snd != nullptr, "no entry exists");
    Map<_K, _V> _m1 = {_r0.fst};
    return _m1;
}

/*
 * Expand.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> expand(Map<_K, _V> _m, _K _k, _V _v)
{
    Tuple<_K, _V> _entry = tuple(_k, _v);
    int _flags = _TREE_INSERT_GROW_FLAG;
    Result<_Tree, Any *> _r0 = _tree_insert(_m._impl, cast<Any>(_entry),
        _flags, _map_compare_wrapper<_K>);
    check(_r0.snd == nullptr, "entry exists");
    Map<_K, _V> _m1 = {_r0.fst};
    return _m1;
}

/*
 * Insert+Search.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Result<Map<_K, _V>, Maybe<_V>> insert_search(Map<_K, _V> _m,
    _K _k, _V _v)
{
    Tuple<_K, _V> _entry = tuple(_k, _v);
    int _flags = _TREE_INSERT_REPLACE_FLAG | _TREE_INSERT_GROW_FLAG;
    Result<_Tree, Any *> _r0 = _tree_insert(_m._impl, cast<Any>(_entry),
        _flags, _map_compare_wrapper<_K>);
    Map<_K, _V> _m1 = {_r0.fst};
    Maybe<_V> _old_v = (_r0.snd != nullptr?
        maybe(snd_ptr(*cast<Tuple<_K, _V> *>(_r0.snd))):
        nothing<_V>());
    Result<Map<_K, _V>, Maybe<_V>> _r = {_m1, _old_v};
    return _r;
}

/*
 * Insert.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> insert(Map<_K, _V> _m, _K _k, _V _v)
{
    Tuple<_K, _V> _entry = tuple(_k, _v);
    int _flags = _TREE_INSERT_REPLACE_FLAG | _TREE_INSERT_GROW_FLAG;
    Result<_Tree, Any *> _r0 = _tree_insert(_m._impl, cast<Any>(_entry),
        _flags, _map_compare_wrapper<_K>);
    Map<_K, _V> _m1 = {_r0.fst};
    return _m1;
}

/*
 * Membership.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE bool member(Map<_K, _V> _m, _K _k)
{
    Tuple<_K, _V> _key = cast<Tuple<_K, _V>>(cast<Any *>(&_k));
    return _tree_search(_m._impl, cast<Any>(_key), _map_compare_wrapper<_K>);
}

/*
 * Search.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Maybe<_V> search(Map<_K, _V> _m, _K _k)
{
    Tuple<_K, _V> _key = cast<Tuple<_K, _V>>(cast<Any *>(&_k));
    Any *_r0 = _tree_search(_m._impl, cast<Any>(_key),
        _map_compare_wrapper<_K>);
    Maybe<_V> _r = (_r0 != nullptr?
        maybe<_V>(snd_ptr(*cast<Tuple<_K, _V> *>(_r0))):
        nothing<_V>());
    return _r;
}

/*
 * Lookup.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE _V lookup(Map<_K, _V> _m, _K _k)
{
    Tuple<_K, _V> _key = cast<Tuple<_K, _V>>(cast<Any *>(&_k));
    Any *_r = _tree_search(_m._impl, cast<Any>(_key),
        _map_compare_wrapper<_K>);
    check(_r != nullptr, "entry not found");
    return snd(*cast<Tuple<_K, _V> *>(_r));
}

/*
 * Remove.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> remove(Map<_K, _V> _m, _K _k)
{
    Tuple<_K, _V> _key = cast<Tuple<_K, _V>>(cast<Any *>(&_k));
    Result<_Tree, Any *> _r0 = _tree_delete(_m._impl, cast<Any>(_key),
        _map_compare_wrapper<_K>);
    Map<_K, _V> _m1 = {_r0.fst};
    return _m1;
}

/*
 * Remove+Search.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Result<Map<_K, _V>, Maybe<_V>> remove_search(Map<_K, _V> _m, _K _k)
{
    Tuple<_K, _V> _v0;
    Tuple<_K, _V> _key = cast<Tuple<_K, _V>>(cast<Any *>(&_k));
    Result<_Tree, Any *> _r0 = _tree_delete(_m._impl, cast<Any>(_key),
        _map_compare_wrapper<_K>);
    Map<_K, _V> _m1 = {_r0.fst};
    Maybe<_V> _old_v = (_r0.snd != nullptr?
        maybe(snd_ptr(*cast<Tuple<_K, _V> *>(_r0.snd))):
        nothing<_V>());
    Result<Map<_K, _V>, Maybe<_V>> _r = {_m1, _old_v};
    return _r;
}

/*
 * Size.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE size_t size(Map<_K, _V> _m)
{
    return _tree_size(_m._impl);
}

/*
 * Fold left.
 * O(n).
 */
template <typename _K, typename _V, typename _A, typename _F>
inline PURE _A foldl(Map<_K, _V> _m, _A _arg, _F _func)
{
    Any (*_func_ptr)(void *, Any, Any) =
        [](void *_func_0, Any _a0, Any _k0) -> Any
    {
        _A _a = cast<_A>(_a0);
        Tuple<_K, _V> _entry = cast<Tuple<_K, _V>>(_k0);
        _F *_func_1 = (_F *)_func_0;
        _A _b = (*_func_1)(_a, fst(_entry), snd(_entry));
        return cast<Any>(_b);
    };
    Any _r = _tree_foldl(_m._impl, cast<Any>(_arg), _func_ptr, (void *)&_func);
    return cast<_A>(_r);
}

/*
 * Fold right.
 * O(n).
 */
template <typename _K, typename _V, typename _A, typename _F>
inline PURE _A foldr(Map<_K, _V> _m, _A _arg, _F _func)
{
    Any (*_func_ptr)(void *, Any, Any) =
        [](void *_func_0, Any _a0, Any _k0) -> Any
    {
        _A _a = cast<_A>(_a0);
        Tuple<_K, _V> _entry = cast<Tuple<_K, _V>>(_k0);
        _F *_func_1 = (_F *)_func_0;
        _A _b = (*_func_1)(_a, fst(_entry), snd(_entry));
        return cast<Any>(_b);
    };
    Any _r = _tree_foldr(_m._impl, cast<Any>(_arg), _func_ptr, (void *)&_func);
    return cast<_A>(_r);
}

/*
 * Map.
 * O(n).
 */
template <typename _W, typename _K, typename _V, typename _F>
inline PURE Map<_K, _W> map(Map<_K, _V> _m, _F _func)
{
    Any (*_func_ptr)(void *, Any) =
        [](void *_func_0, Any _k0) -> Any
    {
        Tuple<_K, _V> _entry = cast<Tuple<_K, _V>>(_k0);
        _F *_func_1 = (_F *)_func_0;
        _W _w = (*_func_1)(fst(_entry), snd(_entry));
        Tuple<_K, _W> _new_entry = tuple<_K, _W>(fst(_entry), _w);
        return cast<Any>(_new_entry);
    };
    Map<_K, _W> _m1 = {_tree_map(_m._impl, _func_ptr, (void *)&_func)};
    return _m1;
}

/*
 * Constructor.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE List<Tuple<_K, _V>> list(Map<_K, _V> _m)
{
    Any (*_func_ptr)(void *, Any) = [] (void *_unused, Any _k0) -> Any
    {
        return cast<Any>(_k0);
    };
    List<Tuple<_K, _V>> _xs = 
        cast<List<Tuple<_K, _V>>>(_tree_to_list(_m._impl, _func_ptr,
            nullptr));
    return _xs;
}

/*
 * Keys.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE List<_K> keys(Map<_K, _V> _m)
{
    Any (*_func_ptr)(void *, Any) = [] (void *_unused, Any _k0) -> Any
    {
        Tuple<_K, _V> _k = cast<Tuple<_K, _V>>(_k0);
        return cast<Any>(fst(_k));
    };
    List<_K> _xs = cast<List<_K>>(_tree_to_list(_m._impl, _func_ptr, nullptr));
    return _xs;
}

/*
 * Values.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE List<_V> values(Map<_K, _V> _m)
{
    Any (*_func_ptr)(void *, Any) = [] (void *_unused, Any _k0) -> Any
    {
        Tuple<_K, _V> _k = cast<Tuple<_K, _V>>(_k0);
        return cast<Any>(snd(_k));
    };
    List<_V> _xs = cast<List<_V>>(_tree_to_list(_m._impl, _func_ptr, nullptr));
    return _xs;
}

/*
 * Split.
 * O(log(n)).
 */
template <typename _K, typename _V>
inline PURE Result<Map<_K, _V>, Map<_K, _V>> split(Map<_K, _V> _m, _K _k)
{
    Tuple<_K, _V> _key = cast<Tuple<_K, _V>>(cast<Any *>(&_k));
    Result<_Tree, _Tree> _r0 = _tree_split(_m._impl, cast<Any>(_key),
        _map_compare_wrapper<_K>);
    Result<Map<_K, _V>, Map<_K, _V>> _r = {{_r0.fst}, {_r0.snd}};
    return _r;
}

/*
 * Merge.
 * O(log(n) + log(m)).
 */
template <typename _K, typename _V>
inline PURE Map<_K, _V> merge(Map<_K, _V> _ma, Map<_K, _V> _mb)
{
    Map<_K, _V> _m1 = {_tree_union(_ma._impl, _mb._impl,
        _map_compare_wrapper<_K>)};
    return _m1;
}

/*
 * Verify.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE bool verify(Map<_K, _V> _m)
{
    return _tree_verify(_m._impl);
}

/*
 * Compare.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE int compare(Map<_K, _V> _m1, Map<_K, _V> _m2)
{
    int (*_func_ptr)(void *, Any, Any) =
        [] (void *_unused, Any _k10, Any _k20) -> int
    {
        Tuple<_K, _V> _k1 = cast<Tuple<_K, _V>>(_k10);
        Tuple<_K, _V> _k2 = cast<Tuple<_K, _V>>(_k20);
        return compare(_k1, _k2);
    };
    return _tree_compare(_m1._impl, _m2._impl, nullptr, _func_ptr);
}

/*
 * Show.
 * O(n).
 */
template <typename _K, typename _V>
inline PURE String show(Map<_K, _V> _m)
{
    String (*_func_ptr)(Any) = 
        [] (Any _k0) -> String
    {
        Tuple<_K, _V> _k = cast<Tuple<_K, _V>>(_k0);
        return append(append(show(fst(_k)), "->"), show(snd(_k)));
    };
    return _tree_show(_m._impl, _func_ptr);
}

}           /* namespace F */

#endif      /* _MAP_H */
