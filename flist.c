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

#include <stdlib.h>

#include "flist.h"
#include "fstring.h"

namespace F
{

typedef _Node node;

/*
 * Last.
 */
extern PURE Any _list_last(node *xs)
{
    while (xs->next != nullptr)
        xs = xs->next;
    return xs->val;
}

/*
 * Length.
 */
extern PURE size_t _list_length(node *xs)
{
    size_t len;
    for (len = 0; xs != nullptr; len++)
        xs = xs->next;
    return len;
}

/*
 * Append.
 */
extern PURE node *_list_append(node *xs, node *ys)
{
    node *zs = ys, *ws = xs;

    while (ws != nullptr)
    {
        node *vs = (node *)gc_malloc(sizeof(node));
        vs->next = zs;
        zs = vs;
        ws = ws->next;
    }

    ws = xs;
    node *ts = zs;
    while (ws != nullptr)
    {
        ts->val = ws->val;
        ws = ws->next;
        ts = ts->next;
    }

    return zs;
}

/*
 * Reverse.
 */
extern PURE node *_list_reverse(node *xs)
{
    node *ys = nullptr;

    while (xs != nullptr)
    {
        node *zs = (node *)gc_malloc(sizeof(node));
        zs->val = xs->val;
        zs->next = ys;
        ys = zs;
        xs = xs->next;
    }
    return ys;
}

/*
 * Zip.
 */
extern PURE node *_list_zip(node *xs, node *ys)
{
    if (xs == nullptr || ys == nullptr)
        return nullptr;
    Tuple<Any, Any> t = tuple(xs->val, ys->val);
    node *zs = (node *)gc_malloc(sizeof(node));
    zs->val = cast<Any>(t);
    xs = xs->next;
    ys = ys->next;
    node *ws = zs;
    while (xs != nullptr && ys != nullptr)
    {
        t = tuple(xs->val, ys->val);
        node *vs = (node *)gc_malloc(sizeof(node));
        vs->val = cast<Any>(t);
        ws->next = vs;
        ws = vs;
        xs = xs->next;
        ys = ys->next;
    }
    ws->next = nullptr;
    return zs;
}

/*
 * Sort.
 */
extern PURE node *_list_sort(node *xs, void *data, _SortCompare cmp)
{
    if (xs == nullptr || xs->next == nullptr)
        return xs;

    size_t len = _list_length(xs);
    Any *vals = (Any *)gc_malloc_atomic(len * sizeof(Any));
    node *ys = xs;
    for (size_t i = 0; i < len; i++)
    {
        vals[i] = ys->val;
        ys = ys->next;
    }
#ifdef LINUX
    qsort_r(vals, len, sizeof(Any), cmp, data);
#else
    qsort_r(vals, len, sizeof(Any), data, cmp);
#endif
    ys = nullptr;
    for (size_t i = 0; i < len; i++)
    {
        node *zs = (node *)gc_malloc(sizeof(node));
        zs->val  = vals[len-i-1];
        zs->next = ys;
        ys = zs;
    }
    gc_free(vals);

    return ys;
}

/*
 * Take.
 */
extern PURE node *_list_take(node *xs, size_t len)
{
    if (len == 0 || xs == nullptr)
        return nullptr;
    node *ys = (node *)gc_malloc(sizeof(node));
    ys->val = xs->val;
    len--;
    xs = xs->next;
    node *zs = ys;
    for (size_t i = 0; xs != nullptr && i < len; i++)
    {
        node *ws = (node *)gc_malloc(sizeof(node));
        ws->val = xs->val;
        zs->next = ws;
        zs = ws;
        xs = xs->next;
    }
    zs->next = nullptr;
    return ys;
}

/*
 * Take while.
 */
extern PURE node *_list_take_while(node *xs, bool (*f)(void *,Any), void *data)
{
    if (xs == nullptr || !f(data, xs->val))
        return nullptr;
    node *ys = (node *)gc_malloc(sizeof(node));
    ys->val = xs->val;
    xs = xs->next;
    node *zs = ys;
    while (xs != nullptr && f(data, xs->val))
    {
        node *ws = (node *)gc_malloc(sizeof(node));
        ws->val = xs->val;
        zs->next = ws;
        zs = ws;
        xs = xs->next;
    }
    zs->next = nullptr;
    return ys;
}

/*
 * Foldl.
 */
extern PURE Any _list_foldl(node *xs, Any (*f)(void *,Any,Any), Any a,
    void *data)
{
    while (xs != nullptr)
    {
        a = f(data, a, xs->val);
        xs = xs->next;
    }
    return a;
}

/*
 * Foldr.
 */
extern PURE Any _list_foldr(node *xs, Any (*f)(void *,Any,Any), Any a,
    void *data)
{
    size_t len = _list_length(xs);
    Any *vals = (Any *)gc_malloc_atomic(len * sizeof(Any));
    for (size_t i = 0; xs != nullptr; i++)
    {
        vals[i] = xs->val;
        xs = xs->next;
    }
    for (size_t i = 0; i < len; i++)
        a = f(data, a, vals[len-i-1]);
    gc_free(vals);
    return a;
}

/*
 * Map.
 */
extern PURE node *_list_map(node *xs, Any (*f)(void *,Any), void *data)
{
    if (xs == nullptr)
        return xs;
    node *ys = (node *)gc_malloc(sizeof(node));
    ys->val = f(data, xs->val);
    node *zs = ys;
    xs = xs->next;
    while (xs != nullptr)
    {
        node *ws = (node *)gc_malloc(sizeof(node));
        ws->val = f(data, xs->val);
        zs->next = ws;
        zs = ws;
        xs = xs->next;
    }
    zs->next = nullptr;
    return ys;
}

/*
 * Filter.
 */
extern PURE node *_list_filter(node *xs, bool (*f)(void *,Any), void *data)
{
    node *ys = nullptr, *zs = nullptr;
    while (xs != nullptr)
    {
        if (f(data, xs->val))
        {
            node *ws = (node *)gc_malloc(sizeof(node));
            ws->val = xs->val;
            if (ys == nullptr)
                ys = zs = ws;
            else
            {
                zs->next = ws;
                zs = ws;
            }
        }
        xs = xs->next;
    }
    if (ys == nullptr)
        return ys;
    zs->next = nullptr;
    return ys;
}

/*
 * Compare.
 */
extern PURE int _list_compare(node *xs, node *ys, int (*f)(Any,Any))
{
    while (true)
    {
        if (xs == nullptr)
        {
            if (ys == nullptr)
                return 0;
            return -1;
        }
        if (ys == nullptr)
            return 1;
        int cmp = f(xs->val, ys->val);
        if (cmp != 0)
            return cmp;
        xs = xs->next;
        ys = ys->next;
    }
}

/*
 * Show.
 */
extern PURE String _list_show(node *xs, String (*f)(Any))
{
    String r = string('[');
    while (xs != nullptr)
    {
        r = append(r, f(xs->val));
        if (xs->next != nullptr)
            r = append(r, ',');
        xs = xs->next;
    }
    r = append(r, ']');
    return r;
}

}

