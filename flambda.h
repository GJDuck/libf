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

#ifndef _FLAMBDA_H
#define _FLAMBDA_H

#include <cstring>

#include "fgc.h"

namespace F
{

template<typename _T>
struct _func_impl {};

template<typename _T, typename... _A>
struct _func_impl<_T(_A...)>
{
    _T (*_exec)(_func_impl<_T(_A...)> *, _A...);
    char _func[];
};

/*
 * Lamba type.
 */
template<typename _T>
struct Func {};

template<typename _T, typename... _A>
struct Func<_T(_A...)>
{
    template<typename _F>
    Func(const _F &_func_0)
    {
        _impl = (_func_impl<_T(_A...)> *)gc_malloc(
            sizeof(_func_impl<_T(_A...)>) + sizeof(_func_0));
        std::memmove(_impl->_func, (void *)&_func_0, sizeof(_func_0));
        _impl->_exec =
            [] (_func_impl<_T(_A...)> *_data, _A..._args) -> _T
        {
            return ((_F *)_data->_func)->operator()(_args...);
        };
    }

    _T operator()(_A... _args) const
    {
        return _impl->_exec(_impl, _args...);
    }

    struct _func_impl<_T(_A...)> *_impl;
};

}           /* namsepace F */

#endif      /* _FLAMBDA_H */
