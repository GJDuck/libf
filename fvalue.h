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

#ifndef _FVALUE_H
#define _FVALUE_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "fgc.h"

namespace F
{

template <typename _T, typename _U>
_T _bit_cast(const _U _u)
{
    union _cast_helper {
        _T _t_val;
        _U _u_val;
        _cast_helper()
        {
            std::memset(this, 0, sizeof(*this));
        }
    } _helper;
    _helper._u_val = _u;
    return _helper._t_val;
}

/*
 * Word object.
 */
typedef uintptr_t Word;

/*
 * A boxed object.
 */
template <typename _T>
struct _Boxed
{
private:
    _T *_ptr;

public:
    _Boxed() : _ptr(nullptr)
    {

    }

    _Boxed(const _T &_x)
    {
        _ptr = (_T *)gc_malloc(sizeof(_T));
        *_ptr = _x;
    }

    _Boxed(_T *_x) : _ptr(_x)
    {

    }

    operator _T&() const
    {
        return *_ptr;
    }
};

/*
 * Value<T> object.
 */
template <typename _T>
struct Value
{
private:
    uint8_t _val[sizeof(Word)];
    
public:
    Value()
    {
        *(Word *)_val = (Word)0;
    }

    Value(const _T &_x)
    {
        if (sizeof(_T) <= sizeof(_val))
        {
            *(Word *)_val = (Word)0;
            *(_T *)_val = _x;
        }
        else
        {
            _T *_ptr = (_T *)gc_malloc(sizeof(_T));
            *_ptr = _x;
            *(_T **)_val = _ptr;
        }
    }

    operator const _T&() const
    {
        if (sizeof(_T) <= sizeof(_val))
            return *(_T *)_val;
        else
            return **(_T **)_val;
    }

    bool operator== (Value<_T> _y) const
    {
        return *(Word *)_val == *(Word *)_y._val;
    }
};

/*
 * Optional<T> object.
 */
template <typename _T>
struct Optional
{
private:
    bool _have;
    Value<_T> _val;

public:
    Optional() : _have(false), _val()
    {

    }

    Optional(const _T &_x) : _have(true), _val(_x)
    {

    }

    Optional(Value<_T> &_x) : _have(true), _val(_x)
    {

    }

    operator const _T&() const
    {
        if (!_have)
            error("Optional<T> value not present");
        return _val;
    }

    operator Value<_T>() const
    {
        if (!_have)
            error("Optional<T> value not present");
        return _val;  
    }

    bool _is_empty()
    {
        return !_have;
    }

    const _T &_get()
    {
        if (!_have)
            error("Optional<T> value not present");
        return _val;  
    }
};

/**
 * Test if an optional is empty.
 */
template <typename _T>
inline PURE bool empty(Optional<_T> _x)
{
    return _x._is_empty();
}

/**
 * If an optional is non-empty, get its value.
 */
template <typename _T>
inline PURE const _T &get(Optional<_T> _x)
{
    return _x._get();
}

#define _UNION_TAG_BITS         4
#define _UNION_TAG_MAX          16
#define _UNION_TAG_MASK         (_UNION_TAG_MAX-1)
#define _UNION_DIRECT_SIZE      (sizeof(Word)/2)

template <unsigned _tag, typename... _Ts>
struct _tag_helper
{
    // Empty
};

template <unsigned _tag, typename _U, typename... _Ts>
struct _tag_helper<_tag, _U, _U, _Ts...>
{
    constexpr unsigned _get_tag() const
    {
        static_assert(_tag <= _UNION_TAG_MAX,
            "Union<...> with too many elements");
        return _tag;
    }
};

template <unsigned _tag, typename _U, typename _T, typename... _Ts>
struct _tag_helper<_tag, _U, _T, _Ts...>
{
    _tag_helper<_tag + 1, _U, _Ts...> _helper;

    constexpr unsigned _get_tag() const
    {
        return _helper._get_tag();
    }
};

template <typename _T>
struct _is_boxed_helper
{
    static const bool _value = false;
};

template <typename _T>
struct _is_boxed_helper<_Boxed<_T> *>
{
    static const bool _value = true;
};

/*
 * Union<T1, ..., Tn> object.
 */
template <typename... _Ts>
struct Union
{
private:
    uint8_t _val[sizeof(Word)];

public:
    Union()
    {
        *(Word *)_val = (Word)0;
    }

    template <typename _U>
    Union(const _U &_x)
    {
        _tag_helper<0, _U, _Ts...> _helper;
        unsigned _tag = _helper._get_tag();
        if (sizeof(_U) <= _UNION_DIRECT_SIZE)
        {
            *(Word *)_val = (Word)0;
            *(_U *)(_val + _UNION_DIRECT_SIZE) = _x;
        }
        else if (_is_boxed_helper<_U>::_value)
            std::memcpy(_val, &_x, sizeof(_val));
        else
        {
            _U *_ptr = (_U *)gc_malloc(sizeof(_U));
            *_ptr = _x;
            *(_U **)_val = _ptr;
        }
        *(Word *)_val = *(Word *)_val | (Word)_tag;
    }

    template <typename _U>
    Union(Value<_U> _x)
    {
        _tag_helper<0, _U, _Ts...> _helper;
        unsigned _tag = _helper._get_tag();
        if (sizeof(_U) <= _UNION_DIRECT_SIZE)
        {
            *(Word *)_val = (Word)0;
            *(_U *)(_val + _UNION_DIRECT_SIZE) = _U(_x);
        }
        else if (_is_boxed_helper<_U>::_value)
            std::memcpy(_val, &_x, sizeof(_val));
        else if (sizeof(_U) <= sizeof(_val))
        {
            _U *_ptr = (_U *)gc_malloc(sizeof(_U));
            *_ptr = _U(_x);
            *(_U **)_val = _ptr;
        }
        else
            std::memcpy(_val, &_x, sizeof(_val));
        *(Word *)_val = *(Word *)_val | (Word)_tag;
    }

    template <typename _U>
    operator const _U&() const
    {
        _tag_helper<0, _U, _Ts...> _helper;
        unsigned _tag = _helper._get_tag();
        if ((*(Word *)_val & _UNION_TAG_MASK) != _tag)
        {
            _U *_null = nullptr;
            return *_null;
        }
        if (sizeof(_U) <= _UNION_DIRECT_SIZE)
            return *(_U *)(_val + _UNION_DIRECT_SIZE);
        else
        {
            Word _ptr = *(Word *)_val;
            _ptr -= (Word)_tag;
            return *(_U *)_ptr;
        }
    }

    template <typename _U>
    operator Value<_U>() const
    {
        _tag_helper<0, _U, _Ts...> _helper;
        unsigned _tag = _helper._get_tag();
        if ((*(Word *)_val & _UNION_TAG_MASK) != _tag)
        {
            _U *_null = nullptr;
            return *_null;
        }
        if (sizeof(_U) <= sizeof(_val))
            return Value<_U>(*(_U *)(_val + _UNION_DIRECT_SIZE));
        else
        {
            Word _ptr = *(Word *)_val;
            _ptr -= (Word)_tag;
            return _bit_cast<Value<_U>>(_ptr);
        }
    }

    template <typename _U>
    static constexpr unsigned index()
    {
        return (_tag_helper<0, _U, _Ts...>){}._get_tag();
    }

    bool operator== (Union<_Ts...> _y) const
    {
        return *(Word *)_val == *(Word *)_y._val;
    }

    unsigned _get_index()
    {
        return (unsigned)(*(Word *)_val & _UNION_TAG_MASK);
    }
};

/**
 * Get the index of the type of value stored in the union object.
 */
template <typename... _Ts>
inline PURE unsigned index(Union<_Ts...> _x)
{
    return _x._get_index();
}

/*
 * Result<T...> object.
 */
template <typename... _T>
struct Result { };

template <typename _T>
struct Result<_T>
{
    _T _result_0;
};

template <typename _T, typename _U>
struct Result<_T, _U>
{
    _T _result_0;
    _U _result_1;
};

template <typename _T, typename _U, typename _V>
struct Result<_T, _U, _V>
{
    _T _result_0;
    _U _result_1;
    _V _result_2;
};

template <typename _T, typename _U, typename _V, typename _W>
struct Result<_T, _U, _V, _W>
{
    _T _result_0;
    _U _result_1;
    _V _result_2;
    _W _result_3;
};

template <typename _T, typename _U, typename _V, typename _W, typename _X>
struct Result<_T, _U, _V, _W, _X>
{
    _T _result_0;
    _U _result_1;
    _V _result_2;
    _W _result_3;
    _X _result_4;
};

template <typename _T, typename _U, typename _V, typename _W, typename _X, typename _Y>
struct Result<_T, _U, _V, _W, _X, _Y>
{
    _T _result_0;
    _U _result_1;
    _V _result_2;
    _W _result_3;
    _X _result_4;
    _Y _result_5;
};

template <typename _T, typename _U, typename _V, typename _W, typename _X, typename _Y, typename _Z>
struct Result<_T, _U, _V, _W, _X, _Y, _Z>
{
    _T _result_0;
    _U _result_1;
    _V _result_2;
    _W _result_3;
    _X _result_4;
    _Y _result_5;
    _Z _result_6;
};

template <typename _T, typename _U, typename _V, typename _W, typename _X, typename _Y, typename _Z, typename _A>
struct Result<_T, _U, _V, _W, _X, _Y, _Z, _A>
{
    _T _result_0;
    _U _result_1;
    _V _result_2;
    _W _result_3;
    _X _result_4;
    _Y _result_5;
    _Z _result_6;
    _A _result_7;
};

}           /* namespace F */

#endif      /* FVALUE_H */
