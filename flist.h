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

#ifndef _FLIST_H
#define _FLIST_H

#include "fdefs.h"
#include "fbase.h"
#include "fcompare.h"
#include "flambda.h"
#include "ftuple.h"

namespace F
{

#ifdef LINUX
typedef int (*_SortCompare)(const void *, const void *, void *);
#else
typedef int (*_SortCompare)(void *, const void *, const void *);
#endif

/*
 * Low-level implementation.
 */
extern PURE Any _list_last(_Node *_xs);
extern PURE size_t _list_length(_Node *_xs);
extern PURE _Node *_list_append(_Node *_xs, _Node *_ys);
extern PURE _Node *_list_reverse(_Node *_xs);
extern PURE _Node *_list_zip(_Node *xs, _Node *ys);
extern PURE _Node *_list_sort(_Node *_xs, void *_data, _SortCompare _f);
extern PURE _Node *_list_take(_Node *_xs, size_t _len);
extern PURE _Node *_list_take_while(_Node *_xs, bool (*_f)(void *,Any),
    void *_data);
extern PURE Any _list_foldl(_Node *_xs, Any (*_f)(void *,Any,Any),
    Any _a, void *_data);
extern PURE Any _list_foldr(_Node *_xs, Any (*_f)(void *,Any,Any),
    Any _a, void *_data);
extern PURE _Node *_list_map(_Node *_xs,
    Any (*_f)(void *,Any), void *_data);
extern PURE _Node *_list_filter(_Node *_xs,
    bool (*f)(void *,Any), void *_data);
extern PURE int _list_compare(_Node *_xs, _Node *_ys,
    int (*_f)(Any, Any));
extern PURE String _list_show(_Node *_xs, String (*_f)(Any));

/*
 * Constructor.
 * O(1).
 */
template <typename _T>
inline PURE List<_T> list(void)
{
    List<_T> _xs = {nullptr};
    return _xs;
}

/*
 * Test if empty.
 * O(1).
 */
template <typename _T>
inline PURE bool is_empty(List<_T> _xs)
{
    return (_xs._impl == nullptr);
}

/*
 * Head.
 * O(1).
 */
template <typename _T>
inline PURE _T head(List<_T> _xs)
{
    check(_xs._impl != nullptr, "head []");
    return cast<_T>(_xs._impl->val);
}

/*
 * Tail.
 * O(1).
 */
template <typename _T>
inline PURE List<_T> tail(List<_T> _xs) 
{
    check(_xs._impl != nullptr, "tail []");
    List<_T> _ys = {_xs._impl->next};
    return _ys;
}

/*
 * Cons.
 * O(1).
 */
template <typename _T>
inline PURE List<_T> cons(_T _x, List<_T> _xs)
{
    _Node *_ys = (_Node *)gc_malloc(sizeof(_Node));
    _ys->val = cast<Any>(_x);
    _ys->next = _xs._impl;
    List<_T> _zs = {_ys};
    return _zs;
}

/*
 * Last.
 * O(n).
 */
template <typename _T>
inline PURE _T last(List<_T> _xs)
{
    check(_xs._impl != nullptr, "last []");
    return cast<_T>(_list_last(_xs._impl));
}

/*
 * Length.
 * O(n).
 */
template <typename _T>
inline PURE size_t length(List<_T> _xs)
{
    return _list_length(_xs._impl);
}

/*
 * Take.
 * O(n).
 */
template <typename _T>
inline PURE List<_T> take(List<_T> _xs, size_t _len)
{
    List<_T> _ys = {_list_take(_xs._impl, _len)};
    return _ys;
}

/*
 * Take while.
 * O(n).
 */
template <typename _T, typename _F>
inline PURE List<_T> take_while(List<_T> _xs, _F _test)
{
    bool (*_test_func_ptr)(void *,Any) =
        [](void *_test_0, Any _x) -> bool
    {
        _T _a = cast<_T>(_x);
        _F *_test_1 = (_F *)_test_0;
        return (*_test_1)(_a);
    };
    List<_T> _ys = {_list_take_while(_xs._impl, _test_func_ptr,
        (void *)&_test)};
    return _ys;
}

/*
 * Append.
 * O(n).
 */
template <typename _T>
inline PURE List<_T> append(List<_T> _xs, List<_T> _ys)
{
    List<_T> _zs = {_list_append(_xs._impl, _ys._impl)};
    return _zs;
}

/*
 * Reverse.
 * O(n).
 */
template <typename _T>
inline PURE List<_T> reverse(List<_T> _xs)
{
    List<_T> _ys = {_list_reverse(_xs._impl)};
    return _ys;
}

/*
 * Zip.
 * O(n).
 */
template <typename _T, typename _U>
inline PURE List<Tuple<_T, _U>> zip(List<_T> _xs, List<_U> _ys)
{
    List<Tuple<_T, _U>> _zs = {_list_zip(_xs._impl, _ys._impl)};
    return _zs;
}

/*
 * Sort.
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
        Any _a = *(Any *)_x;
        Any _b = *(Any *)_y;
        _F *_cmp_1 = (_F *)_cmp_0;
        return (*_cmp_1)(cast<_T>(_a), cast<_T>(_b));
    };
    List<_T> _ys = {_list_sort(_xs._impl, (void *)(&_cmp), _cmp_func_ptr)};
    return _ys;
}

/*
 * Sort.
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
        _T _a = cast<_T>(*(Any *)_x);
        _T _b = cast<_T>(*(Any *)_y);
        return compare(_a, _b);
    };
    List<_T> _ys = {_list_sort(_xs._impl, nullptr, _cmp_func_ptr)};
    return _ys;
}

/*
 * Fold left.
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldl(List<_T> _xs, _A _a, _F _func)
{
    Any (*_func_ptr)(void *, Any, Any) =
        [] (void *_func_0, Any _a0, Any _x0) -> Any
    {
        _A _a = cast<_A>(_a0);
        _T _x = cast<_T>(_x0);
        _F *_func_1 = (_F *)_func_0;
        _A _b = (*_func_1)(_a, _x);
        return cast<Any>(_b);
    };
    Any _r = _list_foldl(_xs._impl, _func_ptr, cast<Any>(_a), (void *)&_func);
    return cast<_A>(_r);
}

/*
 * Fold right.
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldr(List<_T> _xs, _A _a, _F _func)
{
    Any (*_func_ptr)(void *, Any, Any) =
        [] (void *_func_0, Any _a0, Any _x0) -> Any
    {
        _A _a = cast<_A>(_a0);
        _T _x = cast<_T>(_x0);
        _F *_func_1 = (_F *)_func_0;
        _A _b = (*_func_1)(_a, _x);
        return cast<Any>(_b);
    };
    Any _r = _list_foldr(_xs._impl, _func_ptr, cast<Any>(_a), (void *)&_func);
    return cast<_A>(_r);
}

/*
 * Map.
 * O(n).
 */
template <typename _U, typename _T, typename _F>
inline PURE List<_U> map(List<_T> _xs, _F _func)
{
    Any (*_func_ptr)(void *, Any) =
        [] (void *_func_0, Any _x0) -> Any
    {
        _T _x = cast<_T>(_x0);
        _F *_func_1 = (_F *)_func_0;
        _U _y = (*_func_1)(_x);
        return cast<Any>(_y);
    };
    List<_U> _ys = {_list_map(_xs._impl, _func_ptr, (void *)&_func)};
    return _ys;  
}

/*
 * Filter.
 * O(n).
 */
template <typename _T, typename _F>
inline PURE List<_T> filter(List<_T> _xs, _F _func)
{
    bool (*_func_ptr)(void *, Any) =
        [] (void *_func_0, Any _x0) -> bool
    {
        _T _x = cast<_T>(_x0);
        _F *_func_1 = (_F *)_func_0;
        return (*_func_1)(_x);
    };
    List<_T> _ys = {_list_filter(_xs._impl, _func_ptr, (void *)&_func)};
    return _ys;
}

/*
 * Compare.
 * O(n).
 */
template <typename _T>
inline PURE int compare(List<_T> _xs, List<_T> _ys)
{
    int (*_cmp_func_ptr)(Any, Any) =
        [] (Any _x, Any _y) -> int
    {
        _T _a = cast<_T>(_x);
        _T _b = cast<_T>(_y);
        return compare(_a, _b);
    };
    return _list_compare(_xs._impl, _ys._impl, _cmp_func_ptr);
}

/*
 * Show.
 * O(n).
 */
template <typename _T>
inline PURE String show(List<_T> _xs)
{
    String (*_func_ptr)(Any) =
        [] (Any _x0) -> String
    {
        _T _x = cast<_T>(_x0);
        return show(_x);
    };
    return _list_show(_xs._impl, _func_ptr);
}

}           /* namespace F */

#endif      /* _FLIST_H */
