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

#include <string.h>

#include "fvector.h"

namespace F
{

#define VECTOR_FRAG_MAX_SIZE        16

typedef _frag_s frag_s;
typedef _Frag frag_t;
typedef _Seq seq_t;
typedef Any C;

struct vec_s
{
    uint8_t len;
};
typedef struct vec_s *vec_t;

static inline PURE size_t vec_size(size_t size, size_t len)
{
    return size + size * len;
}

static inline vec_t vec_malloc(size_t size, size_t elem_size)
{
    if (elem_size >= sizeof(void *))
        return (vec_t)gc_malloc(size);
    else
        return (vec_t)gc_malloc_atomic(size);
}

static inline PURE void *vec_elem_ptr(vec_t vec, size_t size, size_t idx)
{
    return ((char *)vec) + size + size * idx;
}

static inline PURE Any vec_get(vec_t vec, size_t size, size_t idx)
{
    void *elem_ptr  = vec_elem_ptr(vec, size, idx);
    Any elem = null<Any>();
    memmove(&elem, elem_ptr, size);
    return elem;
}

static inline void vec_set(vec_t vec, size_t size, size_t idx, Any elem)
{
    void *elem_ptr  = vec_elem_ptr(vec, size, idx);
    memmove(elem_ptr, &elem, size);
}

static inline void vec_copy(vec_t vec_dst, vec_t vec_src, size_t idx_dst,
    size_t idx_src, size_t len, size_t size)
{
    void *dst_ptr = vec_elem_ptr(vec_dst, size, idx_dst);
    void *src_ptr = vec_elem_ptr(vec_src, size, idx_src);
    memmove(dst_ptr, src_ptr, len * size);
}

/*
 * Vector init.
 */
static PURE seq_t _vector_init_2(seq_t s, const void *a, size_t size,
    size_t len)
{
    for (size_t i = 0; i < len; )
    {
        size_t frag_len = (vec_size(size, len-i) <= VECTOR_FRAG_MAX_SIZE?
            len-i: 1);
        size_t frag_size = vec_size(size, frag_len);
        vec_t vec = vec_malloc(frag_size, size);
        vec->len = frag_len;
        void *elems = vec_elem_ptr(vec, size, 0);
        memcpy(elems, (char *)a + (i * size), size * frag_len);
        i += frag_len;
        s = _seq_push_back(s, cast<frag_t>(vec));
    }
    return s;
}

/*
 * Vector init.
 */
extern PURE seq_t _vector_init(const void *a, size_t size, size_t len)
{
    return _vector_init_2(_SEQ_EMPTY, a, size, len);
}

/*
 * Push back.
 */
extern PURE seq_t _vector_push_back(seq_t s, size_t size, Any elem)
{
    if (_seq_is_empty(s))
    {
vector_push_back_vec:
        vec_t vec = vec_malloc(vec_size(size, 1), size);
        vec->len = 1;
        vec_set(vec, size, 0, elem);
        s = _seq_push_back(s, cast<frag_t>(vec));
        return s;
    }
    frag_t frag = _seq_peek_back(s);
    vec_t vec = cast<vec_t>(frag);
    if (vec_size(size, vec->len) + size > VECTOR_FRAG_MAX_SIZE)
        goto vector_push_back_vec;
    
    vec_t new_vec = vec_malloc(vec_size(size, vec->len+1), size);
    new_vec->len = vec->len + 1;
    vec_copy(new_vec, vec, 0, 0, vec->len, size);
    vec_set(new_vec, size, vec->len, elem);
    return _seq_replace_back(s, cast<frag_t>(new_vec));
}

/*
 * Pop back.
 */
extern PURE seq_t _vector_pop_back(seq_t s, size_t size)
{
    frag_t frag = _seq_peek_back(s);
    vec_t vec = cast<vec_t>(frag);
    if (vec->len == 1)
    {
        auto r = _seq_pop_back(s);
        return r.fst;
    }
    vec_t new_vec = vec_malloc(vec_size(size, vec->len-1), size);
    new_vec->len = vec->len - 1;
    vec_copy(new_vec, vec, 0, 0, vec->len - 1, size);
    return _seq_replace_back(s, cast<frag_t>(new_vec));
}

/*
 * Push front.
 */
extern PURE seq_t _vector_push_front(seq_t s, size_t size, Any elem)
{
    if (_seq_is_empty(s))
    {
vector_push_front_vec:
        vec_t vec = vec_malloc(vec_size(size, 1), size);
        vec->len = 1;
        vec_set(vec, size, 0, elem);
        s = _seq_push_front(s, cast<frag_t>(vec));
        return s;
    }
    frag_t frag = _seq_peek_front(s);
    vec_t vec = cast<vec_t>(frag);
    if (vec_size(size, vec->len) + size > VECTOR_FRAG_MAX_SIZE)
        goto vector_push_front_vec;
    
    vec_t new_vec = vec_malloc(vec_size(size, vec->len+1), size);
    new_vec->len = vec->len + 1;
    vec_copy(new_vec, vec, 1, 0, vec->len, size);
    vec_set(new_vec, size, 0, elem);
    return _seq_replace_front(s, cast<frag_t>(new_vec));
}

/*
 * Pop front.
 */
extern PURE seq_t _vector_pop_front(seq_t s, size_t size)
{
    frag_t frag = _seq_peek_front(s);
    vec_t vec = cast<vec_t>(frag);
    if (vec->len == 1)
    {
        auto r = _seq_pop_front(s);
        return r.fst;
    }
    vec_t new_vec = vec_malloc(vec_size(size, vec->len-1), size);
    new_vec->len = vec->len - 1;
    vec_copy(new_vec, vec, 0, 1, vec->len - 1, size);
    return _seq_replace_front(s, cast<frag_t>(new_vec));
}

/*
 * Lookup.
 */
extern PURE Any _vector_lookup(seq_t s, size_t size, size_t idx)
{
    auto r = _seq_lookup(s, idx);
    idx = r.snd;
    vec_t vec = cast<vec_t>(r.fst);
    return vec_get(vec, size, idx);
}

/*
 * Split.
 */
extern PURE Result<seq_t, seq_t> _vector_split(seq_t s, size_t size, size_t i)
{
    if (i >= _seq_length(s))
        return {s, _SEQ_EMPTY};
    const auto r = _seq_split(s, i);
    i = r.third;
    seq_t sl = r.fst;
    seq_t sr = r.fourth;
    vec_t vec = cast<vec_t>(r.snd);
    if (i == 0)
        sr = _seq_push_front(sr, r.snd);
    else if (i == vec->len)
        sl = _seq_push_front(sl, r.snd);
    else
    {
        vec_t left = vec_malloc(vec_size(size, i), size);
        vec_t right = vec_malloc(vec_size(size, vec->len - i), size);
        vec_copy(left, vec, 0, 0, i, size);
        vec_copy(right, vec, 0, i, vec->len - i, size);
        left->len = i;
        right->len = vec->len - i;
        sl = _seq_push_back(sl, cast<frag_t>(left));
        sr = _seq_push_front(sr, cast<frag_t>(right));
    }
    return {sl, sr};
}

/*
 * Fold left.
 */
extern PURE C _vector_frag_foldl(frag_t frag, size_t size, C arg,
    C (*f)(void *, C, Any), void *data)
{
    vec_t vec = cast<vec_t>(frag);
    for (size_t i = 0; i < vec->len; i++)
    {
        Any elem = vec_get(vec, size, i);
        arg = f(data, arg, elem);
    }
    return arg;
}

/*
 * Fold right.
 */
extern PURE C _vector_frag_foldr(frag_t frag, size_t size, C arg,
    C (*f)(void *, C, Any), void *data)
{
    vec_t vec = cast<vec_t>(frag);
    for (ssize_t i = vec->len - 1; i >= 0; i--)
    {
        Any elem = vec_get(vec, size, i);
        arg = f(data, arg, elem);
    }
    return arg;
}

/*
 * Map.
 */
extern PURE frag_t _vector_frag_map(frag_t frag, size_t size_0, size_t size_1,
    Any (*f)(void *, Any), void *data)
{
    vec_t vec = cast<vec_t>(frag);
    vec_t new_vec = vec_malloc(vec_size(size_0, vec->len), size_1);
    new_vec->len = vec->len;
    for (size_t i = 0; i < vec->len; i++)
    {
        Any elem = vec_get(vec, size_0, i);
        elem = f(data, elem);
        vec_set(new_vec, size_1, i, elem);
    }
    return cast<frag_t>(new_vec);
}

/*
 * Filter map.
 */
extern PURE frag_t _vector_frag_filter_map(frag_t frag, size_t size_0,
    size_t size_1, Result<bool, Any> (*f)(void *, Any), void *data)
{
    vec_t vec = cast<vec_t>(frag);
    Any xs[vec->len];
    size_t i = 0, j = 0;
    for (; i < vec->len; i++)
    {
        Any elem = vec_get(vec, size_0, i);
        Result<bool, Any> r = f(data, elem);
        if (!r.fst)
            continue;
        xs[j++] = r.snd;
    }
    if (j == 0)
        return nullptr;
    vec_t new_vec = vec_malloc(vec_size(size_0, j), size_1);
    new_vec->len = j;
    for (i = 0; i < j; i++)
        vec_set(new_vec, size_1, i, xs[i]);
    return cast<frag_t>(new_vec);
}

/*
 * Compare.
 */
extern PURE int _vector_frag_compare(void *data, frag_t a, size_t idx1,
    frag_t b, size_t idx2)
{
    vec_t vec1 = cast<vec_t>(a);
    vec_t vec2 = cast<vec_t>(b);
    auto info = cast<Result<int (*)(Any, Any), size_t> *>(data);

    while (idx1 < vec1->len && idx2 < vec2->len)
    {
        Any x = vec_get(vec1, info->snd, idx1);
        Any y = vec_get(vec2, info->snd, idx2);
        int cmp = info->fst(x, y);
        if (cmp != 0)
            return cmp;
        idx1++;
        idx2++;
    }
    return 0;
}

}
