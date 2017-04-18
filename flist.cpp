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

#include <stdlib.h>

#include "flist.h"
#include "fstring.h"

/*
 * NOTE: Uses destructive update to avoid recursion.  Every `const_cast'
 *       corresponds to a destructive update.
 */

namespace F
{

/*
 * Covert to C-array.
 */
extern PURE void *_list_data(List<Word> xs, size_t size,
    void (*copy)(void *, Value<Word>))
{
    size_t len = _list_length(xs);
	if (len == 0)
		return nullptr;
    void *ptr0 = (size < sizeof(void *)?
        gc_malloc_atomic(len * size): gc_malloc(len * size));
    uint8_t *ptr = (uint8_t *)ptr0;
	while (!empty(xs))
    {
        const Node<Word> &node = xs;
        copy(ptr, node.elem);
        ptr += size;
        xs = tail(xs);
    }
    return ptr0;
}

/*
 * Last.
 */
extern PURE List<Word> _list_last(List<Word> xs)
{
    if (empty(xs))
        error("last []");
    while (!empty(tail(xs)))
        xs = tail(xs);
    return xs;
}

/*
 * Length.
 */
extern PURE size_t _list_length(List<Word> xs)
{
    size_t len;
    for (len = 0; !empty(xs); len++)
        xs = tail(xs);
    return len;
}

/*
 * Append.
 */
extern PURE List<Word> _list_append(List<Word> xs, List<Word> ys)
{
    List<Word> zs = ys, ws = xs;

    while (!empty(ws))
    {
        List<Word> vs = list<Word>((Word)0, zs);
        zs = vs;
        ws = tail(ws);
    }

    ws = xs;
    List<Word> ts = zs;
    while (!empty(ws))
    {
		const Node<Word> &node = ts;
		const_cast<Node<Word> &>(node).elem = head(ws);
        ws = tail(ws);
        ts = tail(ts);
    }

    return zs;
}

/*
 * Reverse.
 */
extern PURE List<Word> _list_reverse(List<Word> xs)
{
    List<Word> ys = list<Word>();

    while (!empty(xs))
    {
        List<Word> zs = list<Word>(head(xs), ys);
        ys = zs;
        xs = tail(xs);
    }

    return ys;
}

/*
 * Zip.
 */
extern PURE List<Word> _list_zip(List<Word> xs, List<Word> ys)
{
    if (empty(xs) || empty(ys))
        return list<Word>();

    const Node<Word> &nodex = xs;
    const Node<Word> &nodey = ys;
    Tuple<Value<Word>, Value<Word>> t = tuple(nodex.elem, nodey.elem);
    List<Tuple<Value<Word>, Value<Word>>> zs =
        list(t, list<Tuple<Value<Word>, Value<Word>>>());
    xs = tail(xs);
    ys = tail(ys);
    List<Tuple<Value<Word>, Value<Word>>> ws = zs;

    while (!empty(xs) && !empty(ys))
    {
        const Node<Word> &nodex = xs;
        const Node<Word> &nodey = ys;
        Tuple<Value<Word>, Value<Word>> t = tuple(nodex.elem, nodey.elem);
		List<Tuple<Value<Word>, Value<Word>>> vs =
			list(t, list<Tuple<Value<Word>, Value<Word>>>());
        const Node<Tuple<Value<Word>, Value<Word>>> &node = zs;
		const_cast<Node<Tuple<Value<Word>, Value<Word>>> &>(node).next = vs;
        ws = vs;
        xs = tail(xs);
        ys = tail(ys);
    }

    return _bit_cast<List<Word>>(zs);
}

/*
 * Sort.
 */
extern PURE List<Word> _list_sort(List<Word> xs, void *data, _SortCompare cmp)
{
    if (empty(xs) || empty(tail(xs)))
        return xs;

    size_t len = _list_length(xs);
    Value<Word> *vals = (Value<Word> *)gc_malloc_atomic(
        len * sizeof(Value<Word>));
    List<Word> ys = xs;
    for (size_t i = 0; i < len; i++)
    {
        vals[i] = head(ys);
        ys = tail(ys);
    }
#ifdef LINUX
    qsort_r(vals, len, sizeof(Value<Word>), cmp, data);
#else
    qsort_r(vals, len, sizeof(Value<Word>), data, cmp);
#endif
    ys = list<Word>();

    for (size_t i = 0; i < len; i++)
        ys = list<Word>(vals[len-i-1], ys);
    gc_free(vals);

    return ys;
}

/*
 * Take.
 */
extern PURE List<Word> _list_take(List<Word> xs, size_t len)
{
    if (len == 0 || empty(xs))
        return list<Word>();

    List<Word> ys = list<Word>(head(xs), list<Word>());
    len--;
    xs = tail(xs);
    List<Word> zs = ys;

    for (size_t i = 0; !empty(xs) && i < len; i++)
    {
        List<Word> ws = list<Word>(head(xs), list<Word>());
        const Node<Word> &node = zs;
        const_cast<Node<Word> &>(node).next = ws;
        zs = ws;
        xs = tail(xs);
    }

    return ys;
}

/*
 * Take while.
 */
extern PURE List<Word> _list_take_while(List<Word> xs,
    bool (*f)(void *, Value<Word>), void *data)
{
    if (empty(xs) || !f(data, head(xs)))
        return list<Word>();

    List<Word> ys = list<Word>(head(xs), list<Word>());
    xs = tail(xs);
    List<Word> zs = ys;

    while (!empty(xs) && f(data, head(xs)))
    {
        List<Word> ws = list<Word>(head(xs), list<Word>());
        const Node<Word> &node = zs;
        const_cast<Node<Word> &>(node).next = ws;
        zs = ws;
        xs = tail(xs);
    }

    return ys;
}

/*
 * Foldl.
 */
extern PURE Value<Word> _list_foldl(List<Word> xs,
    Value<Word> (*f)(void *,Value<Word>,Value<Word>), Value<Word> a,
    void *data)
{
    while (!empty(xs))
    {
        a = f(data, a, head(xs));
        xs = tail(xs);
    }

    return a;
}

/*
 * Foldr.
 */
extern PURE Value<Word> _list_foldr(List<Word> xs,
    Value<Word> (*f)(void *,Value<Word>,Value<Word>), Value<Word> a,
    void *data)
{
	// Avoid stack allocation & possible overflow:
    size_t len = _list_length(xs);
    Value<Word> *vals = (Value<Word> *)gc_malloc_atomic(
        len * sizeof(Value<Word>));

    for (size_t i = 0; !empty(xs); i++)
    {
        vals[i] = head(xs);
        xs = tail(xs);
    }

    for (size_t i = 0; i < len; i++)
        a = f(data, a, vals[len-i-1]);

    gc_free(vals);
    return a;
}

/*
 * Map.
 */
extern PURE List<Word> _list_map(List<Word> xs,
    Value<Word> (*f)(void *, Value<Word>), void *data)
{
    if (empty(xs))
        return xs;

    List<Word> ys = list<Word>(f(data, head(xs)), list<Word>());
    List<Word> zs = ys;
    xs = tail(xs);
    
    while (!empty(xs))
    {
        List<Word> ws = list<Word>(f(data, head(xs)), list<Word>());
		const Node<Word> &node = zs;
        const_cast<Node<Word> &>(node).next = ws;
        zs = ws;
        xs = tail(xs);
    }

    return ys;
}

/*
 * Filter.
 */
extern PURE List<Word> _list_filter(List<Word> xs,
    bool (*f)(void *, Value<Word>), void *data)
{
    List<Word> ys = list<Word>(), zs = list<Word>();
    
    while (!empty(xs))
    {
        if (f(data, head(xs)))
        {
            List<Word> ws = list<Word>(head(xs), list<Word>());
            if (empty(ys))
                ys = zs = ws;
            else
            {
                const Node<Word> &node = zs;
                const_cast<Node<Word> &>(node).next = ws;
                zs = ws;
            }
        }
        xs = tail(xs);
    }

    return ys;
}

/*
 * Compare.
 */
extern PURE int _list_compare(List<Word> xs, List<Word> ys,
    int (*f)(Value<Word>,Value<Word>))
{
    while (true)
    {
        if (empty(xs))
        {
            if (empty(ys))
                return 0;
            return -1;
        }
        if (empty(ys))
            return 1;
        int cmp = f(head(xs), head(ys));
        if (cmp != 0)
            return cmp;
        xs = tail(xs);
        ys = tail(ys);
    }
}

/*
 * Show.
 */
extern PURE String _list_show(List<Word> xs, String (*f)(Value<Word>))
{
    String r = string('[');
    while (!empty(xs))
    {
        r = append(r, f(head(xs)));
        if (!empty(tail(xs)))
            r = append(r, ',');
        xs = tail(xs);
    }
    r = append(r, ']');
    return r;
}

}

