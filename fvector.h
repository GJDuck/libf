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

#include "fbase.h"
#include "fseq.h"

#include "flist_defs.h"
#include "fstring_defs.h"
#include "fvector_defs.h"

namespace F
{

extern PURE _Seq _vector_init(const void *_a, size_t _size, size_t _len);
extern PURE _Seq _vector_push_back(_Seq _s, size_t _size, Value<Word> _elem);
extern PURE _Seq _vector_pop_back(_Seq _s, size_t _size);
extern PURE _Seq _vector_push_front(_Seq _s, size_t _size, Value<Word> _elem);
extern PURE _Seq _vector_pop_front(_Seq _s, size_t _size);
extern PURE void *_vector_lookup(_Seq _s, size_t _size, size_t _idx);
extern PURE Result<_Seq, _Seq> _vector_split(_Seq _s, size_t _size,
    size_t _idx);
extern PURE _Seq _vector_left(_Seq _s, size_t _size, size_t _idx);
extern PURE _Seq _vector_right(_Seq _s, size_t _size, size_t _idx);
extern PURE _Seq _vector_between(_Seq _s, size_t _size, size_t _lidx,
    size_t _ridx);
extern PURE _Seq _vector_insert(_Seq _s, size_t _size, size_t _idx, _Seq _t);
extern PURE _Seq _vector_delete(_Seq _s, size_t _size, size_t _lidx,
    size_t _ridx);
extern PURE Value<Word> _vector_frag_lookup(_Frag _frag, size_t _size,
    size_t _idx);
extern PURE Value<Word> _vector_frag_foldl(_Frag _frag, size_t _size,
    Value<Word> _arg, size_t _idx,
    Value<Word> (*_f)(void *, Value<Word>, size_t, Value<Word>), void *_data);
extern PURE Value<Word> _vector_frag_foldr(_Frag _frag, size_t _size,
    Value<Word> _arg, size_t _idx,
    Value<Word> (*_f)(void *, Value<Word>, size_t, Value<Word>), void *_data);
extern PURE _Frag _vector_frag_map(_Frag _frag, size_t _size_0,
    size_t _size_1, size_t _idx, Value<Word> (*_f)(void *, size_t, Value<Word>),
    void *_data);
extern PURE Optional<_Frag> _vector_frag_filter_map(_Frag frag, size_t _size_0,
    size_t _size_1, size_t _idx,
    Optional<Word> (*_f)(void *, size_t, Value<Word>), void *_data);
extern PURE int _vector_frag_compare(void *_data, _Frag _frag1, size_t _idx1,
    _Frag _frag2, size_t _idx2);

inline PURE String string(char32_t);
inline PURE size_t size(String);
inline PURE String append(String, String);
inline PURE String append(String, char32_t);

/**
 * Construct the empty vector.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> vector()
{
    Vector<_T> _v = {_seq_empty()};
    return _v;
}

/**
 * Construct a vector from a C-array.
 * O(n).
 */
template <typename _T>
inline PURE Vector<_T> vector(const _T *_a, size_t _len)
{
    Vector<_T> _v = {_vector_init(_bit_cast<const void *>(_a), sizeof(_T),
        _len)};
    return _v;
}

/**
 * Construct a vector from a string.
 * O(n).
 */
extern PURE Vector<char32_t> vector(String _str);

/**
 * Construct a vector from a list.
 * O(n).
 */
template <typename _T>
inline PURE Vector<_T> vector(List<_T> _xs)
{
    Vector<_T> _v = vector<_T>();
    Vector<_T> (*_func_ptr)(Vector<_T>, _T) =
        [](Vector<_T> _v0, _T _x) -> Vector<_T>
    {
        Vector<_T> _v = {_vector_push_back(_v0._impl, sizeof(_T),
            _bit_cast<Value<Word>>(_x))};
        return _v;
    };
    return foldl(_xs, _v, _func_ptr);
}

/**
 * Test if a vector is empty.
 * O(1).
 */
template <typename _T>
inline PURE bool empty(Vector<_T> _v)
{
    return _seq_is_empty(_v._impl);
}

/**
 * Vector size (a.k.a. vector length).
 * O(1).
 */
template <typename _T>
inline PURE size_t size(Vector<_T> _v)
{
    return _seq_length(_v._impl);
}

/**
 * Push an element to the back of a vector.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> push_back(Vector<_T> _v0, _T _elem)
{
    Vector<_T> _v = {_vector_push_back(_v0._impl, sizeof(_T),
        _bit_cast<Value<Word>>(_elem))};
    return _v;
}

/**
 * Pop the last element from a vector.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> pop_back(Vector<_T> _v0)
{
    Vector<_T> _v = {_vector_pop_back(_v0._impl, sizeof(_T))};
    return _v;
}

/**
 * Push an element to the front of a vector.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> push_front(Vector<_T> _v0, _T _elem)
{
    Vector<_T> _v = {_vector_push_front(_v0._impl, sizeof(_T),
        _bit_cast<Value<Word>>(_elem))};
    return _v;
}

/**
 * Pop the first element from a vector.
 * O(1).
 */
template <typename _T>
inline PURE Vector<_T> pop_front(Vector<_T> _v0)
{
    Vector<_T> _v = {_vector_pop_front(_v0._impl, sizeof(_T))};
    return _v;
}

/**
 * Append vectors.
 * O(min(log(n), log(m))).
 */
template <typename _T>
inline PURE Vector<_T> append(Vector<_T> _v0, Vector<_T> _v1)
{
    Vector<_T> _v = {_seq_append(_v0._impl, _v1._impl)};
    return _v;
}

/**
 * Get the last element of a vector.
 * O(1).
 */
template <typename _T>
inline PURE const _T &back(Vector<_T> _v)
{
    void *_ptr = _vector_lookup(_v._impl, sizeof(_T), size(_v)-1);
    return *(const _T *)_ptr;
}

/**
 * Get the first element of a vector.
 * O(1).
 */
template <typename _T>
inline PURE const _T &front(Vector<_T> _v)
{
    void *_ptr = _vector_lookup(_v._impl, sizeof(_T), 0);
    return *(const _T *)_ptr;
}

/**
 * Get the element at `idx'.
 * O(log(n)).
 */
template <typename _T>
inline PURE const _T &at(Vector<_T> _v, size_t _idx)
{
    void *_ptr = _vector_lookup(_v._impl, sizeof(_T), _idx);
    return *(const _T *)_ptr;
}

/**
 * Vector split.
 * O(log(n)).
 */
template <typename _T>
inline PURE Result<Vector<_T>, Vector<_T>> split(Vector<_T> _v, size_t _idx)
{
    auto [_sl, _sr] = _vector_split(_v._impl, sizeof(_T), _idx);
    Vector<_T> _vl = {_sl};
    Vector<_T> _vr = {_sr};
    return {_vl, _vr};
}

/**
 * Vector split left.
 * O(long(n)).
 */
template <typename _T>
inline PURE Vector<_T> left(Vector<_T> _v, size_t _idx)
{
    Vector<_T> _vl = {_vector_left(_v._impl, sizeof(_T), _idx)};
    return _vl;
}

/**
 * Vector split right.
 * O(log(n)).
 */
template <typename _T>
inline PURE Vector<_T> right(Vector<_T> _v, size_t _idx)
{
    Vector<_T> _vr = {_vector_right(_v._impl, sizeof(_T), _idx)};
    return _vr;
}

/**
 * Vector between.
 * O(log(n)).
 */
template <typename _T>
inline PURE Vector<_T> between(Vector<_T> _v, size_t _idx, size_t _count)
{
    Vector<_T> _vr = {_vector_between(_v._impl, sizeof(_T), _idx,
        _idx + _count)};
    return _vr;
}

/**
 * Vector insert.
 * O(log(n)).
 */
template <typename _T>
inline PURE Vector<_T> insert(Vector<_T> _v, size_t _idx, Vector<_T> _u)
{
    Vector<_T> _vr = {_vector_insert(_v._impl, sizeof(_T), _idx, _u._impl)};
    return _vr;
}

/**
 * Vector erase.
 * O(log(n)).
 */
template <typename _T>
inline PURE Vector<_T> erase(Vector<_T> _v, size_t _idx, size_t _count = 1)
{
    Vector<_T> _vr = {_vector_delete(_v._impl, sizeof(_T), _idx,
        _idx + _count)};
    return _vr;
}

/**
 * Vector fold left. ([](A a, size_t idx, T elem) -> A).
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldl(Vector<_T> _v, const _A &_arg, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, size_t, _Frag) =
        [](void *_func_0, Value<Word> _a0, size_t _idx, _Frag _k0) ->
            Value<Word>
    {
        Value<Word> (*_func_ptr_1)(void *, Value<Word>, size_t, Value<Word>) =
            [](void *_func_0, Value<Word> _a0, size_t _idx, Value<Word> _c0) ->
                Value<Word>
        {
            _F *_func_1 = (_F *)_func_0;
            Value<_A> _a = _bit_cast<Value<_A>>(_a0);
            Value<_T> _c = _bit_cast<Value<_T>>(_c0);
            Value<_A> _b = (*_func_1)(_a, _idx, _c);
            return _bit_cast<Value<Word>>(_b);
        };
        return _vector_frag_foldl(_k0, sizeof(_T), _a0, _idx, _func_ptr_1,
            _func_0);
    };
    Value<_A> _arg1 = _arg;
    Value<Word> _r = _seq_foldl(_v._impl, _bit_cast<Value<Word>>(_arg1),
        _func_ptr, (void *)&_func);
    return _bit_cast<Value<_A>>(_r);
}

/**
 * Vector fold right. ([](A a, size_t idx, T elem) -> A).
 * O(n).
 */
template <typename _T, typename _A, typename _F>
inline PURE _A foldr(Vector<_T> _v, const _A &_arg, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, size_t, _Frag) =
        [](void *_func_0, Value<Word> _a0, size_t _idx, _Frag _k0) ->
            Value<Word>
    {
        Value<Word> (*_func_ptr_1)(void *, Value<Word>, size_t, Value<Word>) =
            [](void *_func_0, Value<Word> _a0, size_t _idx, Value<Word> _c0) ->
                Value<Word>
        {
            _F *_func_1 = (_F *)_func_0;
            Value<_A> _a = _bit_cast<Value<_A>>(_a0);
            Value<_T> _c = _bit_cast<Value<_T>>(_c0);
            Value<_A> _b = (*_func_1)(_a, _idx, _c);
            return _bit_cast<Value<Word>>(_b);
        };
        return _vector_frag_foldr(_k0, sizeof(_T), _a0, _idx, _func_ptr_1,
            _func_0);
    };
    Value<_A> _arg1 = _arg;
    Value<Word> _r = _seq_foldr(_v._impl, _bit_cast<Value<Word>>(_arg1),
        _func_ptr, (void *)&_func);
    return _bit_cast<Value<_A>>(_r);
}

/**
 * Vector map. ([](size_t idx, T elem) -> U).
 * O(n).
 */
template <typename _U, typename _T, typename _F>
inline PURE Vector<_U> map(Vector<_T> _v, _F _func)
{
    _Frag (*_func_ptr)(void *, size_t, _Frag) =
        [](void *_func_0, size_t _idx, _Frag _k0) -> _Frag
    {
        Value<Word> (*_func_ptr_1)(void *, size_t, Value<Word>) =
            [](void *_func_0, size_t _idx, Value<Word> _a0) -> Value<Word>
        {
            _F *_func_1 = (_F *)_func_0;
            Value<_T> _a = _bit_cast<Value<_T>>(_a0);
            Value<_U> _b = (*_func_1)(_idx, _a);
            return _bit_cast<Value<Word>>(_b);
        };
        return _vector_frag_map(_k0, sizeof(_T), sizeof(_U), _idx, _func_ptr_1,
            _func_0);
    };
    Vector<_U> _r = {_seq_map(_v._impl, _func_ptr, (void *)&_func)};
    return _r;
}

/**
 * Vector filter. ([](size_t idx, T elem) -> bool).
 * O(n).
 */
template <typename _T, typename _F>
inline PURE Vector<_T> filter(Vector<_T> _v, _F _func)
{
    Value<Word> (*_func_ptr)(void *, Value<Word>, size_t, _Frag) =
        [](void *_func_0, Value<Word> _a0, size_t _idx, _Frag _k0) ->
            Value<Word>
    {
        Optional<Word> (*_func_ptr_1)(void *, size_t, Value<Word>) =
            [](void *_func_0, size_t _idx, Value<Word> _a0) -> Optional<Word>
        {
            _F *_func_1 = (_F *)_func_0;
            Value<_T> _a1 = _bit_cast<Value<_T>>(_a0);
            return ((*_func_1)(_idx, _a1)?
                Optional<Word>(_a0): Optional<Word>());
        };
        Optional<_Frag> _k = _vector_frag_filter_map(_k0, sizeof(_T),
            sizeof(_T), _idx, _func_ptr_1, _func_0);
        if (empty(_k))
            return _a0;
        _Seq _a = _bit_cast<_Seq>(_a0);
        _Seq _a1 = _seq_push_back(_a, _k);
        return _bit_cast<Value<Word>>(_a1);
    };
    Value<Word> _r = _seq_foldl(_v._impl, _bit_cast<Value<Word>>(_seq_empty()),
        _func_ptr, (void *)&_func);
    Vector<_T> _r1 = {_bit_cast<_Seq>(_r)};
    return _r1;
}

/**
 * Vector show.
 * O(n).
 */
template <typename _T>
inline PURE String show(Vector<_T> _v)
{
    String (*_func_ptr)(String, size_t, Value<_T>) =
        [] (String _s, size_t _idx, Value<_T> _x) -> String
    {
        if (size(_s) != 1)
            _s = append(_s, ',');
        String _t = show(_x);
        _s = append(_s, _t);
        return _s;
    };
    String _r = foldl(_v, string('<'), _func_ptr);
    _r = append(_r, '>');
    return _r;
}

/**
 * Vector compare.
 * O(n).
 */
template <typename _T>
inline PURE int compare(Vector<_T> _v, Vector<_T> _u)
{
    int (*_func_ptr)(Value<Word>, Value<Word>) =
        [] (Value<Word> _x, Value<Word> _y) -> int
    {
        Value<_T> _a = _bit_cast<Value<_T>>(_x);
        Value<_T> _b = _bit_cast<Value<_T>>(_y);
        return compare(_a, _b);
    };
    Result<int (*)(Value<Word>, Value<Word>), size_t> _info =
        {_func_ptr, sizeof(_T)};
    return _seq_compare(_v._impl, _u._impl, (void *)&_info,
        _vector_frag_compare);
}

/**
 * Vector verify.
 * O(n).
 */
template <typename _T>
inline PURE bool verify(Vector<_T> _v)
{
    return _seq_verify(_v._impl);
}

/**
 * Construct an iterator pointing to the start of a vector.
 * O(1).
 */
template <typename _T>
inline VectorItr<_T> begin(Vector<_T> _v)
{
    VectorItr<_T> _itr;
    _itr._seq_itr = begin(_v._impl);
    return _itr;
}

/**
 * Construct an iterator pointing to the end of a vector.
 * O(1).
 */
template <typename _T>
inline VectorItr<_T> end(Vector<_T> _v)
{
    VectorItr<_T> _itr;
    _itr._seq_itr = end(_v._impl);
    return _itr;
}

/**
 * Vector iterator increment.
 * O(1).
 */
template <typename _T>
inline VectorItr<_T> &operator ++(VectorItr<_T> &_i)
{
    ++_i._seq_itr;
    return _i;
}

/**
 * Vector iterator decrement.
 * O(1).
 */
template <typename _T>
inline VectorItr<_T> &operator --(VectorItr<_T> &_i)
{
    --_i._seq_itr;
    return _i;
}

/**
 * Vector iterator add offset.
 * O(log(1)).
 */
template <typename _T>
inline VectorItr<_T> &operator +(VectorItr<_T> &_i, ssize_t _offset)
{
    _i._seq_itr += _offset;
    return _i;
}

/**
 * Vector iterator substract offset.
 * O(log(1))
 */
template <typename _T>
inline VectorItr<_T> &operator -(VectorItr<_T> &_i, ssize_t _offset)
{
    _i._seq_itr -= _offset;
    return _i;
}

/**
 * Vector iterator add offset.
 * O(log(1))
 */
template <typename _T>
inline VectorItr<_T> &operator +=(VectorItr<_T> &_i, ssize_t _offset)
{
    _i._seq_itr += _offset;
    return _i;
}

/**
 * Vector iterator subtract offset.
 * O(log(1))
 */
template <typename _T>
inline VectorItr<_T> &operator -=(VectorItr<_T> &_i, ssize_t _offset)
{
    _i._seq_itr -= _offset;
    return _i;
}

/**
 * Vector iterator dereference.
 * O(log(delta)), where delta is distance to last dereference.
 */
template <typename _T>
inline PURE _T operator *(VectorItr<_T> &_i)
{
    size_t _idx;
    _Frag _frag = _seq_itr_get(&_i._seq_itr, &_idx);
    Value<Word> _a = _vector_frag_lookup(_frag, sizeof(_T), _idx);
    return _bit_cast<Value<_T>>(_a);
}

/**
 * Vector iterator same offset.
 * O(1).
 */
template <typename _T>
inline PURE bool operator ==(const VectorItr<_T> &_i, const VectorItr<_T> &_j)
{
    return (_i._seq_itr == _j._seq_itr);
}

/**
 * Vector iterator different offset.
 * O(1).
 */
template <typename _T>
inline PURE bool operator !=(const VectorItr<_T> &_i, const VectorItr<_T> &_j)
{
    return (_i._seq_itr != _j._seq_itr);
}

}               /* namespace F */

#include "flist.h"
#include "fstring.h"

#endif          /* _FVECTOR_H */
