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

#ifndef _FBASE_H
#define _FBASE_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "fgc.h"

/*
 * Assertions.
 */
#define check(b, msg)       assert((b))

/*
 * PURE keyword.
 */
#define PURE                __attribute__((__pure__)) 

/*
 * Error macro.
 */
#define error(s, ...)                                                   \
    do {                                                                \
        fprintf(stderr, "error: %s\n", (s));                            \
        abort();                                                        \
    } while (false)

/*
 * System dependencies.
 */
#ifdef _MINGW32_
#define WINDOWS     1
#endif

#ifdef _APPLE_
#define MACOSX      1
#endif

#if !defined _MINGW32_ && !defined _APPLE_
#define LINUX       1
#endif

namespace F
{

extern PURE void *gc_malloc_pure(size_t size);
extern PURE void *gc_malloc_atomic_pure(size_t size);
extern PURE void *_box_clone_pure(const void *ptr, size_t size);

/*
 * Box.
 */
template <typename _T, size_t _size = sizeof(_T)>
inline PURE _T *box(const _T &_x)
{
    // Nb: using sizeof(_T) directly sometimes gives the wrong size
    //     (clang/gcc); I have no idea why.  The _size as a template
    //     parameter works however.
    _T *_y = (_T *)_box_clone_pure((const void *)&_x, _size);
    return _y;
}

/*
 * Ubox.
 */
template <typename _T>
_T unbox(_T *_x)
{
    return *_x;
}

/*
 * Casting.
 */
template <typename _T, typename _U>
union _cast_helper
{
    _T _t_val;
    _U _u_val;
    void *_p_val;

    // This is required to get cast<func<...>> to work
    _cast_helper(void) { }
};

template <typename _T, typename _U>
_T cast(const _U _u)
{
    static_assert(sizeof(_T) <= sizeof(void *),
        "arg type too big for cast()");
    static_assert(sizeof(_U) <= sizeof(void *),
        "return type too big for cast()");
    union _cast_helper<_T, _U> _helper;
    _helper._p_val = nullptr;
    _helper._u_val = _u;
    return _helper._t_val;
}

/*
 * Generic.
 */
struct Any
{
    uintptr_t _val;

    bool operator ==(const Any _a)
    {
        return _val == _a._val;
    }
};

/*
 * Null.
 */
template <typename _T>
inline PURE _T null(void)
{
    const Any _x = {0};
    return cast<_T>(_x);
}

/****************************************************************************/

#define _TAGMAX    16
#define _TAGBITS   (_TAGMAX-1)

static inline PURE Any _multiptr_settag(const Any _x, size_t _tag)
{
    return cast<Any>(gc_clobbertag(cast<void *>(_x), _tag));
}
static inline PURE Any _multiptr_untag(const Any _x, size_t _tag)
{
    return cast<Any>(gc_deltag(cast<void *>(_x), _tag));
}
static inline PURE size_t _multiptr_gettag(const Any _x)
{
    return gc_gettag(cast<void *>(_x));
}

template <size_t _tag, typename... _T>
struct _multiptr_get_helper_s { };

template <size_t _tag, typename _U, typename... _T>
struct _multiptr_get_helper_s<_tag, _U, _U, _T...>
{
    Any _get_helper_result(Any _impl)
    {
        // Note: this test should be compiled away in typical use idioms.
        if (_multiptr_gettag(_impl) != _tag)
            return cast<Any>(nullptr);
        else
            return _multiptr_untag(_impl, _tag);
    }
};

template <size_t _tag, typename _U, typename _H, typename... _T>
struct _multiptr_get_helper_s<_tag, _U, _H, _T...>
{
    Any _get_helper_result(Any _impl)
    {
        static_assert(_tag < _TAGMAX-1,
            "too many members in Multi<...>");
        struct _multiptr_get_helper_s<_tag + 1, _U, _T...> _next;
        return _next._get_helper_result(_impl);
    }
};

template <size_t _tag, typename... _T>
struct _multiptr_set_helper_s { };

template <size_t _tag, typename _U, typename... _T>
struct _multiptr_set_helper_s<_tag, _U, _U, _T...>
{
    Any _set_helper_result(Any _impl)
    {
        return _multiptr_settag(_impl, _tag);
    }
};

template <size_t _tag, typename _U, typename _H, typename... _T>
struct _multiptr_set_helper_s<_tag, _U, _H, _T...>
{
    Any _set_helper_result(Any _impl)
    {
        static_assert(_tag < _TAGMAX-1,
            "too many members in Multi<...>");
        struct _multiptr_set_helper_s<_tag + 1, _U, _T...> _next;
        return _next._set_helper_result(_impl);
    }
};

template <typename... _T>
struct Multi
{
    Any _impl;

    template <typename _U>
    void _set_helper(_U *_u_ptr)
    {
//        check((((uintptr_t)_u_ptr) & _TAGBITS) == 0,
//            "mis-aligned pointer used to initialize Multi");
        _multiptr_set_helper_s<0, _U, _T...> _helper;
        _impl = _helper._set_helper_result(cast<Any>(_u_ptr));
    }

    Multi(void) : _impl(null<Any>()) { }

    bool operator ==(const Multi _p)
    {
        return _impl == _p._impl;
    }
};

/*
 * Set ptr to multi.
 */
template <typename _M, typename _U>
inline PURE _M set(_U *_u_ptr)
{
    _M _m;
    _m._set_helper(_u_ptr);
    return _m;
}

/*
 * Get ptr from multi.
 */
template <typename _U, typename... _T>
inline PURE _U *get(Multi<_T ...> _m)
{
    _multiptr_get_helper_s<0, _U, _T...> _helper;
    _U *_u_ptr = cast<_U *>(_helper._get_helper_result(_m._impl));
    return _u_ptr;
}

/*
 * Index.
 */
template <typename... _T>
inline PURE size_t index(Multi<_T...> _u)
{
    return _multiptr_gettag(_u._impl);
}

template <size_t _tag, typename... _T>
struct _type_idx_helper_s { };

template <size_t _tag, typename _U, typename... _T>
struct _type_idx_helper_s<_tag, _U, _U, _T...>
{
    enum
    {
        _type_index = _tag
    };
};

template <size_t _tag, typename _U, typename _H, typename... _T>
struct _type_idx_helper_s<_tag, _U, _H, _T...>
{
    enum
    {
        _type_index =
            _type_idx_helper_s<_tag + 1, _U, _T...>::_type_index
    };
};

template <typename... _T>
struct _type_index_s { };

template <typename _U, typename... _T>
struct _type_index_s<_U, Multi<_T...>>
{
    enum
    {
        _type_index = _type_idx_helper_s<0, _U, _T...>::_type_index
    };
};

/*
 * Type index.
 */
template <typename _T, typename _M>
constexpr int type_index(void)
{
    return (int)_type_index_s<_T, _M>::_type_index;
}

/****************************************************************************/

/*
 * Result<...>
 */
template <typename... _T>
struct Result { };

template <typename _T>
struct Result<_T>
{
    _T fst;
};

template <typename _T, typename _U>
struct Result<_T, _U>
{
    _T fst;
    _U snd;
};

template <typename _T, typename _U, typename _V>
struct Result<_T, _U, _V>
{
    _T fst;
    _U snd;
    _V third;
};

template <typename _T, typename _U, typename _V, typename _W>
struct Result<_T, _U, _V, _W>
{
    _T fst;
    _U snd;
    _V third;
    _W fourth;
};

template <typename _T, typename _U, typename _V, typename _W, typename _X>
struct Result<_T, _U, _V, _W, _X>
{
    _T fst;
    _U snd;
    _V third;
    _W fourth;
    _X fifth;
};

template <typename _T, typename _U, typename _V, typename _W, typename _X,
    typename _Y>
struct Result<_T, _U, _V, _W, _X, _Y>
{
    _T fst;
    _U snd;
    _V third;
    _W fourth;
    _X fifth;
    _Y sixth;
};

template <typename _T, typename _U, typename _V, typename _W, typename _X,
    typename _Y, typename _Z>
struct Result<_T, _U, _V, _W, _X, _Y, _Z>
{
    _T fst;
    _U snd;
    _V third;
    _W fourth;
    _X fifth;
    _Y sixth;
    _Z seventh;
};

template <typename _Z>
static inline _Z operator<<= (_Z &_x, Result<_Z> _r)
{
    _x = _r.fst;
    return _x;
}

template <typename _Y, typename _Z>
static inline Result<_Y> operator<<= (_Z &_x, Result<_Y, _Z> _r)
{
    _x = _r.snd;
    return {_r.fst};
}

template <typename _X, typename _Y, typename _Z>
static inline Result<_X, _Y> operator<<= (_Z &_x,
    Result<_X, _Y, _Z> _r)
{
    _x = _r.third;
    return {_r.fst, _r.snd};
}

template <typename _W, typename _X, typename _Y, typename _Z>
static inline Result<_W, _X, _Y> operator<<= (_Z &_x,
    Result<_W, _X, _Y, _Z> _r)
{
    _x = _r.fourth;
    return {_r.fst, _r.snd, _r.third};
}

template <typename _V, typename _W, typename _X, typename _Y, typename _Z>
static inline Result<_V, _W, _X, _Y> operator<<= (_Z &_x,
    Result<_V, _W, _X, _Y, _Z> _r)
{
    _x = _r.fifth;
    return {_r.fst, _r.snd, _r.third, _r.fourth};
}

template <typename _U, typename _V, typename _W, typename _X, typename _Y,
    typename _Z>
static inline Result<_U, _V, _W, _X, _Y> operator<<= (_Z &_x,
    Result<_U, _V, _W, _X, _Y, _Z> _r)
{
    _x = _r.sixth;
    return {_r.fst, _r.snd, _r.third, _r.fourth, _r.fifth};
}

template <typename _T, typename _U, typename _V, typename _W, typename _X,
    typename _Y, typename _Z>
static inline Result<_T, _U, _V, _W, _X, _Y> operator<<= (_Z &_x,
    Result<_T, _U, _V, _W, _X, _Y, _Z> _r)
{
    _x = _r.seventh;
    return {_r.fst, _r.snd, _r.third, _r.fourth, _r.fifth, _r.sixth};
}

}           /* namespace F */

#endif      /* _FBASE_H */
