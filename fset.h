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
#include "flist.h"
#include "ftree.h"

namespace F
{

template <typename _T>
int _set_compare_wrapper(Any _k1, Any _k2)
{
    _T _a = cast<_T>(_k1);
    _T _b = cast<_T>(_k2);
    return compare(_a, _b);
}

/*
 * Constructor.
 * O(1).
 */
template <typename _T>
inline PURE Set<_T> set(void)
{
    Set<_T> _s = {_TREE_EMPTY};
    return _s;
}

/*
 * Constructor.
 * O(n * log(n)).
 */
template <typename _T>
inline PURE Set<_T> set(List<_T> _xs)
{
    Set<_T> _s = {_tree_from_list(cast<List<Any>>(_xs),
        _set_compare_wrapper<_T>)};
    return _s;
}

/*
 * Membership.
 * O(log(n)).
 */
template <typename _T>
inline PURE bool member(Set<_T> _s, _T _k)
{
    return _tree_search(_s._impl, cast<Any>(_k), _set_compare_wrapper<_T>);
}

/*
 * Insert.
 * O(log(n)).
 */
template <typename _T>
inline PURE Set<_T> insert(Set<_T> _s, _T _k)
{
    int _flags = _TREE_INSERT_GROW_FLAG;
    Result<_Tree, Any *> _r0 = _tree_insert(_s._impl, cast<Any>(_k), _flags,
            _set_compare_wrapper<_T>);
    Set<_T> _s1 = {_r0.fst};
    return _s1;
}

/*
 * Removal.
 * O(log(n)).
 */
template <typename _T>
inline PURE Set<_T> remove(Set<_T> _s, _T _k)
{
    Result<_Tree, Any *> _r0 = _tree_delete(_s._impl, cast<Any>(_k),
        _set_compare_wrapper<_T>);
    Set<_T> _s1 = {_r0.fst};
    return _s1;
}

/*
 * Union (merge).
 * O(log(n) + log(m)).
 */
template <typename _T>
inline PURE Set<_T> merge(Set<_T> _s, Set<_T> _t)
{
    Set<_T> _u = {_tree_union(_s._impl, _t._impl, _set_compare_wrapper<_T>)};
    return _u;
}

/*
 * Intersect.
 * O(log(n) + log(m)).
 */
template <typename _T>
inline PURE Set<_T> intersect(Set<_T> _s, Set<_T> _t)
{
    Set<_T> _u = {_tree_intersect(_s._impl, _t._impl,
        _set_compare_wrapper<_T>)};
    return _u;
}

/*
 * Difference.
 * O(log(n) + log(m)).
 */
template <typename _T>
inline PURE Set<_T> diff(Set<_T> _s, Set<_T> _t)
{
    Set<_T> _u = {_tree_diff(_s._impl, _t._impl, _set_compare_wrapper<_T>)};
    return _u;
}

/*
 * Size.
 * O(n).
 */
template <typename _T>
inline PURE size_t size(Set<_T> _s)
{
    return _tree_size(_s._impl);
}

/*
 * Constructor.
 * O(n).
 */
template <typename _T>
inline PURE List<_T> list(Set<_T> _s)
{
    Any (*_func_ptr)(void *, Any) = [] (void *_unused, Any _k0) -> Any
    {
        return cast<Any>(_k0);
    };
    List<_T> _xs = cast<List<_T>>(_tree_to_list(_s._impl, _func_ptr, nullptr));
    return _xs;
}

/*
 * Fold left.
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldl(Set<_T> _s, _A _arg, _F _func)
{
    Any (*_func_ptr)(void *, Any, Any) =
        [](void *_func_0, Any _a0, Any _k0) -> Any
    {
        _A _a = cast<_A>(_a0);
        _T _k = cast<_T>(_k0);
        _F *_func_1 = (_F *)_func_0;
        _A _b = (*_func_1)(_a, _k);
        return cast<Any>(_b);
    };
    Any _r = _tree_foldl(_s._impl, cast<Any>(_arg), _func_ptr, (void *)&_func);
    return cast<_A>(_r);
}

/*
 * Fold right.
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldr(Set<_T> _s, _A _arg, _F _func)
{
    Any (*_func_ptr)(void *, Any, Any) =
        [](void *_func_0, Any _a0, Any _k0) -> Any
    {
        _A _a = cast<_A>(_a0);
        _T _k = cast<_T>(_k0);
        _F *_func_1 = (_F *)_func_0;
        _A _b = (*_func_1)(_a, _k);
        return cast<Any>(_b);
    };
    Any _r = _tree_foldr(_s._impl, cast<Any>(_arg), _func_ptr, (void *)&_func);
    return cast<_A>(_r);
}

/*
 * Compare.
 * O(n).
 */
template <typename _T>
inline PURE int compare(Set<_T> _s, Set<_T> _t)
{
    int (*_func_ptr)(void *, Any, Any) =
        [](void *_unused, Any _ks0, Any _kt0) -> int
    {
        _T _ks = cast<_T>(_ks0);
        _T _kt = cast<_T>(_kt0);
        return compare(_ks, _kt);
    };
    return _tree_compare(_s._impl, _t._impl, nullptr, _func_ptr);
}

/*
 * Show.
 * O(n).
 */
template <typename _T>
inline PURE String show(Set<_T> _s)
{
    String (*_func_ptr)(Any) =
        [] (Any _k0) -> String
    {
        _T _k = cast<_T>(_k0);
        return show(_k);
    };
    return _tree_show(_s._impl, _func_ptr);
}

}           /* namespace F */

#endif      /* _FSET_H */
