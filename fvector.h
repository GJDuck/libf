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

#ifndef _FVECTOR_H
#define _FVECTOR_H

#include "fdefs.h"
#include "fbase.h"
#include "fseq.h"
#include "fstring.h"

namespace F
{

extern PURE _Seq _vector_push_back(_Seq _s, size_t _size, Any _elem);
extern PURE _Seq _vector_pop_back(_Seq _s, size_t _size);
extern PURE _Seq _vector_push_front(_Seq _s, size_t _size, Any _elem);
extern PURE _Seq _vector_pop_front(_Seq _s, size_t _size);
extern PURE Any _vector_lookup(_Seq _s, size_t _size, size_t _idx);
extern PURE Result<_Seq, _Seq> _vector_split(_Seq _s, size_t _size,
    size_t _idx);
extern PURE Any _vector_frag_foldl(_Frag _frag, size_t _size, Any _arg,
    Any (*_f)(void *, Any, Any), void *_data);
extern PURE Any _vector_frag_foldr(_Frag _frag, size_t _size, Any _arg,
    Any (*_f)(void *, Any, Any), void *_data);
extern PURE _Frag _vector_frag_map(_Frag _frag, size_t _size_0,
    size_t _size_1, Any (*_f)(void *, Any), void *_data);
extern PURE _Frag _vector_frag_filter_map(_Frag frag, size_t _size_0,
    size_t _size_1, Result<bool, Any> (*_f)(void *, Any), void *_data);
extern PURE int _vector_frag_compare(void *_data, _Frag _frag1, size_t _idx1,
    _Frag _frag2, size_t _idx2);

/*
 * Constructor.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> vector(void)
{
    Vector<_T> _v = {_SEQ_EMPTY};
    return _v;
}

/*
 * Length.
 * O(1).
 */
template <typename _T>
inline PURE size_t length(Vector<_T> _v)
{
    return _seq_length(_v._impl);
}

/*
 * Push back.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> push_back(Vector<_T> _v0, _T _elem)
{
    Vector<_T> _v = {_vector_push_back(_v0._impl, sizeof(_T),
        cast<Any>(_elem))};
    return _v;
}

/*
 * Pop back.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> pop_back(Vector<_T> _v0)
{
    Vector<_T> _v = {_vector_pop_back(_v0._impl, sizeof(_T))};
    return _v;
}

/*
 * Push front.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> push_front(Vector<_T> _v0, _T _elem)
{
    Vector<_T> _v = {_vector_push_front(_v0._impl, sizeof(_T),
        cast<Any>(_elem))};
    return _v;
}

/*
 * Pop front.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> pop_front(Vector<_T> _v0)
{
    Vector<_T> _v = {_vector_pop_front(_v0._impl, sizeof(_T))};
    return _v;
}

/*
 * Append.
 * O(min(log(n), log(m))).
 */
template <typename _T>
inline PURE Vector<_T> append(Vector<_T> _v0, Vector<_T> _v1)
{
    Vector<_T> _v = {_seq_append(_v0._impl, _v1._impl)};
    return _v;
}

/*
 * Lookup.
 * O(log(n)).
 */
template <typename _T>
inline PURE _T lookup(Vector<_T> _v, size_t _idx)
{
    Any _elem = _vector_lookup(_v._impl, sizeof(_T), _idx);
    return cast<_T>(_elem);
}

/*
 * Split.
 * O(log(n)).
 */
template <typename _T>
inline PURE Result<Vector<_T>, Vector<_T>> split(Vector<_T> _v, size_t _idx)
{
    Result<_Seq, _Seq> _r = _vector_split(_v._impl, sizeof(_T), _idx);
    Vector<_T> _vl = {_r.fst};
    Vector<_T> _vr = {_r.snd};
    return {_vl, _vr};
}

/*
 * Fold left.
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldl(Vector<_T> _v, _A _arg, _F _func)
{
    Any (*_func_ptr)(void *, Any, _Frag) =
        [](void *_func_0, Any _a0, _Frag _k0) -> Any
    {
        Any (*_func_ptr_1)(void *, Any, Any) =
            [](void *_func_0, Any _a0, Any _c0) -> Any
        {
            _F *_func_1 = (_F *)_func_0;
            _A _a = cast<_A>(_a0);
            _T _c = cast<_T>(_c0);
            _A _b = (*_func_1)(_a, _c);
            return cast<Any>(_b);
        };
        return _vector_frag_foldl(_k0, sizeof(_T), _a0, _func_ptr_1,
            _func_0);
    };
    Any _r = _seq_foldl(_v._impl, cast<Any>(_arg), _func_ptr, (void *)&_func);
    return cast<_A>(_r);
}

/*
 * Fold right.
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldr(Vector<_T> _v, _A _arg, _F _func)
{
    Any (*_func_ptr)(void *, Any, _Frag) =
        [](void *_func_0, Any _a0, _Frag _k0) -> Any
    {
        Any (*_func_ptr_1)(void *, Any, Any) =
            [](void *_func_0, Any _a0, Any _c0) -> Any
        {
            _F *_func_1 = (_F *)_func_0;
            _A _a = cast<_A>(_a0);
            _T _c = cast<_T>(_c0);
            _A _b = (*_func_1)(_a, _c);
            return cast<Any>(_b);
        };
        return _vector_frag_foldr(_k0, sizeof(_T), _a0, _func_ptr_1,
            _func_0);
    };
    Any _r = _seq_foldr(_v._impl, cast<Any>(_arg), _func_ptr, (void *)&_func);
    return cast<_A>(_r);
}

/*
 * Map.
 * O(n).
 */
template <typename _U, typename _T, typename _F>
inline PURE Vector<_U> map(Vector<_T> _v, _F _func)
{
    _Frag (*_func_ptr)(void *, _Frag) =
        [](void *_func_0, _Frag _k0) -> _Frag
    {
        Any (*_func_ptr_1)(void *, Any) =
            [](void *_func_0, Any _a0) -> Any
        {
            _F *_func_1 = (_F *)_func_0;
            _T _a = cast<_T>(_a0);
            _U _b = (*_func_1)(_a);
            return cast<Any>(_b);
        };
        return _vector_frag_map(_k0, sizeof(_T), sizeof(_U), _func_ptr_1,
            _func_0);
    };
    Vector<_U> _r = {_seq_map(_v._impl, _func_ptr, (void *)&_func)};
    return _r;
}

/*
 * Filter.
 * O(n).
 */
template <typename _T, typename _F>
inline PURE Vector<_T> filter(Vector<_T> _v, _F _func)
{
    Any (*_func_ptr)(void *, Any, _Frag) =
        [](void *_func_0, Any _a0, _Frag _k0) -> Any
    {
        Result<bool, Any> (*_func_ptr_1)(void *, Any) =
            [](void *_func_0, Any _a0) -> Result<bool, Any>
        {
            _F *_func_1 = (_F *)_func_0;
            _T _a1 = cast<_T>(_a0);
            return {(*_func_1)(_a1), _a0};
        };
        _Frag _k = _vector_frag_filter_map(_k0, sizeof(_T), sizeof(_T),
            _func_ptr_1, _func_0);
        if (_k == nullptr)
            return _a0;
        _Seq _a = cast<_Seq>(_a0);
        _Seq _a1 = _seq_push_back(_a, _k);
        return cast<Any>(_a1);
    };
    Any _r = _seq_foldl(_v._impl, cast<Any>(_SEQ_EMPTY), _func_ptr,
        (void *)&_func);
    Vector<_T> _r1 = {cast<_Seq>(_r)};
    return _r1;
}

/*
 * Show.
 * O(n).
 */
template <typename _T>
inline PURE String show(Vector<_T> _v)
{
    String (*_func_ptr)(String, _T) =
        [] (String _s, _T _x0) -> String
    {
        _T _x = cast<_T>(_x0);
        if (length(_s) != 1)
            _s = append(_s, ',');
        String _t = show(_x);
        _s = append(_s, _t);
        return _s;
    };
    String _r = foldl(_v, string('<'), _func_ptr);
    _r = append(_r, '>');
    return _r;
}

/*
 * Compare.
 * O(n).
 */
template <typename _T>
inline PURE int compare(Vector<_T> _v, Vector<_T> _u)
{
    int (*_func_ptr)(Any, Any) =
        [] (Any _x, Any _y) -> int
    {
        _T _a = cast<_T>(_x);
        _T _b = cast<_T>(_y);
        return compare(_a, _b);
    };
    Result<int (*)(Any, Any), size_t> _info = {_func_ptr, sizeof(_T)};
    return _seq_compare(_v._impl, _u._impl, (void *)&_info,
        _vector_frag_compare);
}

/*
 * Verify.
 * O(n).
 */
template <typename _T>
inline PURE bool verify(Vector<_T> _v)
{
    return _seq_verify(_v._impl);
}

}               /* namespace F */

#endif          /* _FVECTOR_H */
