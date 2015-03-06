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

#ifndef _FMAYBE_H
#define _FMAYBE_H

#include "fdefs.h"
#include "fbase.h"

namespace F
{

enum MaybeKind
{
    NOTHING = 0,
    JUST    = 1
};

/*
 * Constructor (Nothing).
 * O(1).
 */
template <typename _T>
inline PURE Maybe<_T> nothing(void)
{
    Maybe<_T> _m = {nullptr};
    return _m;
}

/*
 * Constructor (Just).
 * O(1).
 */
template <typename _T>
inline PURE Maybe<_T> just(_T _x)
{
    Maybe<_T> _m = {box(_x)};
    return _m;
}

/*
 * Constructor.
 * O(1).
 */
template <typename _T>
inline PURE Maybe<_T> maybe(const _T *_x)
{
    Maybe<_T> _m = {_x};
    return _m;
}

/*
 * Kind.
 * O(1).
 */
template <typename _T>
inline PURE MaybeKind kind(Maybe<_T> _m)
{
    return (MaybeKind)(_m._impl != nullptr);
}

/*
 * Is nothing test.
 * O(1).
 */
template <typename _T>
inline PURE bool is_nothing(Maybe<_T> _m)
{
    return (kind(_m) == NOTHING);
}

/*
 * Is just test.
 * O(1).
 */
template <typename _T>
inline PURE bool is_just(Maybe<_T> _m)
{
    return (kind(_m) == JUST);
}

/*
 * Get just value.
 * O(1).
 */
template <typename _T>
inline PURE _T get(Maybe<_T> _m)
{
    check(kind(_m) == JUST, "get nothing");
    return *_m._impl;
}

}               /* namespace F */

#endif          /* _FMAYBE_H */
