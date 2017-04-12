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

#include <string.h>

#include "fvector.h"

namespace F
{

// The maximum size of a fragment.  Note that this is a soft limit.
#define VECTOR_FRAG_MAX_SIZE        16

typedef Value<Word> C;

struct VecData
{
    _FragHeader header;
    uint8_t data[];
};

static inline VecData *vec_data_from_frag(_Frag frag)
{
    _FragHeader &header = frag;
    return (VecData *)&header;
}

static inline _Frag vec_frag_from_data(VecData *vec)
{
    _FragHeader *header = &vec->header;
    _Frag frag = header;
    return frag;
}

static inline PURE size_t vec_get_frag_size(size_t size, size_t len)
{
    return sizeof(_FragHeader) + size * len;
}

static inline PURE size_t vec_get_best_frag_len(size_t size, size_t len)
{
    if (vec_get_frag_size(size, len) <= VECTOR_FRAG_MAX_SIZE)
        return len;
    return (VECTOR_FRAG_MAX_SIZE - sizeof(_FragHeader)) / size;
}

static inline VecData *vec_malloc(size_t size, size_t len)
{
    size_t total_size = vec_get_frag_size(size, len);
    if (size >= sizeof(void *))
        return (VecData *)gc_malloc(total_size);
    else
        return (VecData *)gc_malloc_atomic(total_size);
}

static inline PURE void *vec_get_elem_ptr(VecData *vec, size_t size,
    size_t idx)
{
    return &vec->data[size * idx];
}

static inline PURE Value<Word> vec_get_value(VecData *vec, size_t size,
    size_t idx)
{
    void *elem_ptr = vec_get_elem_ptr(vec, size, idx);
    Value<Word> elem;
    std::memcpy(&elem, elem_ptr, size);
    return elem;
}

static inline void vec_set_value(VecData *vec, size_t size, size_t idx,
    Value<Word> elem)
{
    void *elem_ptr = vec_get_elem_ptr(vec, size, idx);
    std::memcpy(elem_ptr, &elem, size);
}

static inline void vec_copy(VecData *vec_dst, VecData *vec_src, size_t idx_dst,
    size_t idx_src, size_t len, size_t size)
{
    void *dst_ptr = vec_get_elem_ptr(vec_dst, size, idx_dst);
    void *src_ptr = vec_get_elem_ptr(vec_src, size, idx_src);
    std::memmove(dst_ptr, src_ptr, len * size);
}

/*
 * Vector construct from string.
 */
extern PURE Vector<char32_t> vector(String str)
{
    Vector<char32_t> v = vector<char32_t>();
    Vector<char32_t> (*func_ptr)(Vector<char32_t>, size_t, char32_t) =
        [](Vector<char32_t> v, size_t idx, char32_t c) -> Vector<char32_t>
    {
        return push_back(v, c);
    };
    return foldl(str, v, func_ptr);
}

/*
 * Vector init.
 */
static PURE _Seq vector_init_2(_Seq s, const void *a, size_t size,
    size_t len)
{
    for (size_t i = 0; i < len; )
    {
        size_t frag_len = vec_get_best_frag_len(size, len-i);
        VecData *vec = vec_malloc(size, frag_len);
        vec->header._len = frag_len;
        void *elems = vec_get_elem_ptr(vec, size, 0);
        std::memcpy(elems, (char *)a + (i * size), size * frag_len);
        i += frag_len;
        s = _seq_push_back(s, vec_frag_from_data(vec));
    }
    return s;
}

/*
 * Vector init.
 */
extern PURE _Seq _vector_init(const void *a, size_t size, size_t len)
{
    return vector_init_2(_seq_empty(), a, size, len);
}

/*
 * Push back.
 */
extern PURE _Seq _vector_push_back(_Seq s, size_t size, Value<Word> elem)
{
    if (_seq_is_empty(s))
    {
vector_push_back_vec:
        VecData *vec = vec_malloc(size, 1);
        vec->header._len = 1;
        vec_set_value(vec, size, 0, elem);
        s = _seq_push_back(s, vec_frag_from_data(vec));
        return s;
    }
    _Frag frag = _seq_peek_back(s);
    VecData *vec = vec_data_from_frag(frag);
    if (vec_get_frag_size(size, vec->header._len + 1) > VECTOR_FRAG_MAX_SIZE)
        goto vector_push_back_vec;
    
    VecData *new_vec = vec_malloc(size, vec->header._len+1);
    new_vec->header._len = vec->header._len + 1;
    vec_copy(new_vec, vec, 0, 0, vec->header._len, size);
    vec_set_value(new_vec, size, vec->header._len, elem);
    return _seq_replace_back(s, vec_frag_from_data(new_vec));
}

/*
 * Pop back.
 */
extern PURE _Seq _vector_pop_back(_Seq s, size_t size)
{
    _Frag frag = _seq_peek_back(s);
    VecData *vec = vec_data_from_frag(frag);
    if (vec->header._len == 1)
    {
        auto [s1, _] = _seq_pop_back(s);
        return s1;
    }
    VecData *new_vec = vec_malloc(size, vec->header._len-1);
    new_vec->header._len = vec->header._len - 1;
    vec_copy(new_vec, vec, 0, 0, vec->header._len - 1, size);
    return _seq_replace_back(s, vec_frag_from_data(new_vec));
}

/*
 * Push front.
 */
extern PURE _Seq _vector_push_front(_Seq s, size_t size, Value<Word> elem)
{
    if (_seq_is_empty(s))
    {
vector_push_front_vec:
        VecData *vec = vec_malloc(size, 1);
        vec->header._len = 1;
        vec_set_value(vec, size, 0, elem);
        s = _seq_push_front(s, vec_frag_from_data(vec));
        return s;
    }
    _Frag frag = _seq_peek_front(s);
    VecData *vec = vec_data_from_frag(frag);
    if (vec_get_frag_size(size, vec->header._len + 1) > VECTOR_FRAG_MAX_SIZE)
        goto vector_push_front_vec;
    
    VecData *new_vec = vec_malloc(size, vec->header._len+1);
    new_vec->header._len = vec->header._len + 1;
    vec_copy(new_vec, vec, 1, 0, vec->header._len, size);
    vec_set_value(new_vec, size, 0, elem);
    return _seq_replace_front(s, vec_frag_from_data(new_vec));
}

/*
 * Pop front.
 */
extern PURE _Seq _vector_pop_front(_Seq s, size_t size)
{
    _Frag frag = _seq_peek_front(s);
    VecData *vec = vec_data_from_frag(frag);
    if (vec->header._len == 1)
    {
        auto [s1, _] = _seq_pop_front(s);
        return s1;
    }
    VecData *new_vec = vec_malloc(size, vec->header._len-1);
    new_vec->header._len = vec->header._len - 1;
    vec_copy(new_vec, vec, 0, 1, vec->header._len - 1, size);
    return _seq_replace_front(s, vec_frag_from_data(new_vec));
}

/*
 * Lookup.
 */
extern PURE void * _vector_lookup(_Seq s, size_t size, size_t idx0)
{
    auto [frag, idx] = _seq_lookup(s, idx0);
    VecData *vec = vec_data_from_frag(frag);
	void *elem_ptr = vec_get_elem_ptr(vec, size, idx);
	return elem_ptr;
}

/*
 * Fragment lookup.
 */
extern PURE Value<Word> _vector_frag_lookup(_Frag frag, size_t size,
    size_t idx)
{
    VecData *vec = vec_data_from_frag(frag);
    return vec_get_value(vec, size, idx);
}

/*
 * Split.
 */
extern PURE Result<_Seq, _Seq> _vector_split(_Seq s, size_t size, size_t i0)
{
    if (i0 >= _seq_length(s))
        return {s, _seq_empty()};
    const auto [sl0, frag, i, sr0] = _seq_split(s, i0);
    _Seq sl = sl0, sr = sr0;
    VecData *vec = vec_data_from_frag(frag);
    if (i == 0)
        sr = _seq_push_front(sr, frag);
    else if (i == vec->header._len)
        sl = _seq_push_front(sl, frag);
    else
    {
        VecData *left = vec_malloc(size, i);
        VecData *right = vec_malloc(size, vec->header._len - i);
        vec_copy(left, vec, 0, 0, i, size);
        vec_copy(right, vec, 0, i, vec->header._len - i, size);
        left->header._len = i;
        right->header._len = vec->header._len - i;
        sl = _seq_push_back(sl, vec_frag_from_data(left));
        sr = _seq_push_front(sr, vec_frag_from_data(right));
    }
    return {sl, sr};
}

/*
 * Split left.
 */
extern PURE _Seq _vector_left(_Seq s, size_t size, size_t i0)
{
    if (i0 >= _seq_length(s))
        return s;
    const auto [sl0, frag, i] = _seq_left(s, i0);
    _Seq sl = sl0;
    VecData *vec = vec_data_from_frag(frag);
    if (i == vec->header._len)
        sl = _seq_push_back(sl, frag);
    else if (i > 0)
    {
        VecData *left = vec_malloc(size, i);
        vec_copy(left, vec, 0, 0, i, size);
        left->header._len = i;
        sl = _seq_push_back(sl, vec_frag_from_data(left));
    }
    return sl;
}

/*
 * Split right.
 */
extern PURE _Seq _vector_right(_Seq s, size_t size, size_t i0)
{
    if (i0 >= _seq_length(s))
        return _seq_empty();
    const auto [frag, i, sr0] = _seq_right(s, i0);
    _Seq sr = sr0;
    VecData *vec = vec_data_from_frag(frag);
    if (i == 0)
        sr = _seq_push_front(sr, frag);
    else if (i < vec->header._len)
    {
        VecData *right = vec_malloc(size, vec->header._len - i);
        vec_copy(right, vec, 0, i, vec->header._len - i, size);
        right->header._len = vec->header._len - i;
        sr = _seq_push_front(sr, vec_frag_from_data(right));
    }
    return sr;
}

/*
 * Vector between.
 */
extern PURE _Seq _vector_between(_Seq s, size_t size, size_t i, size_t j)
{
    if (j <= i)
        return _seq_empty();
    _Seq r = _vector_right(s, size, i);
    r = _vector_left(r, size, j-i);
    return r;
}

/*
 * Vector insert.
 */
extern PURE _Seq _vector_insert(_Seq s, size_t size, size_t i, _Seq t)
{
    auto [s1, s2] = _vector_split(s, size, i);
    _Seq a = _seq_append(s1, t);
    a = _seq_append(a, s2);
    return a;
}

/*
 * Vector delete.
 */
extern PURE _Seq _vector_delete(_Seq s, size_t size, size_t i, size_t j)
{
    _Seq r = (i == 0? _seq_empty(): _vector_left(s, size, i));
    if (j >= _seq_length(s)-1)
        return r;
    r = _seq_append(r, _vector_right(s, size, j));
    return r;
}

/*
 * Fold left.
 */
extern PURE C _vector_frag_foldl(_Frag frag, size_t size, C arg,
    size_t idx, C (*f)(void *, C, size_t, Value<Word>), void *data)
{
    VecData *vec = vec_data_from_frag(frag);
    for (size_t i = 0; i < vec->header._len; i++)
    {
        Value<Word> elem = vec_get_value(vec, size, i);
        arg = f(data, arg, idx + i, elem);
    }
    return arg;
}

/*
 * Fold right.
 */
extern PURE C _vector_frag_foldr(_Frag frag, size_t size, C arg,
    size_t idx, C (*f)(void *, C, size_t, Value<Word>), void *data)
{
    VecData *vec = vec_data_from_frag(frag);
    for (ssize_t i = vec->header._len - 1; i >= 0; i--)
    {
        Value<Word> elem = vec_get_value(vec, size, i);
        arg = f(data, arg, idx + i, elem);
    }
    return arg;
}

/*
 * Map.
 */
extern PURE _Frag _vector_frag_map(_Frag frag, size_t size_0, size_t size_1,
    size_t idx, Value<Word> (*f)(void *, size_t, Value<Word>), void *data)
{
    VecData *vec = vec_data_from_frag(frag);
    VecData *new_vec = vec_malloc(size_1, vec->header._len);
    new_vec->header._len = vec->header._len;
    for (size_t i = 0; i < vec->header._len; i++)
    {
        Value<Word> elem = vec_get_value(vec, size_0, i);
        elem = f(data, idx + i, elem);
        vec_set_value(new_vec, size_1, i, elem);
    }
    return vec_frag_from_data(new_vec);
}

/*
 * Filter map.
 */
extern PURE Optional<_Frag> _vector_frag_filter_map(_Frag frag, size_t size_0,
    size_t size_1, size_t idx,
    Optional<Word> (*f)(void *, size_t, Value<Word>), void *data)
{
    VecData *vec = vec_data_from_frag(frag);
    Value<Word> xs[vec->header._len];
    size_t i = 0, j = 0;
    for (; i < vec->header._len; i++)
    {
        Value<Word> elem = vec_get_value(vec, size_0, i);
        Optional<Word> r = f(data, idx + i, elem);
        if (empty(r))
            continue;
        xs[j++] = r;
    }
    if (j == 0)
    {
        Optional<_Frag> nothing;
        return nothing;
    }
    VecData *new_vec = vec_malloc(size_1, j);
    new_vec->header._len = j;
    for (i = 0; i < j; i++)
        vec_set_value(new_vec, size_1, i, xs[i]);
    return vec_frag_from_data(new_vec);
}

/*
 * Compare.
 */
extern PURE int _vector_frag_compare(void *data, _Frag a, size_t idx1,
    _Frag b, size_t idx2)
{
    VecData *vec1 = vec_data_from_frag(a);
    VecData *vec2 = vec_data_from_frag(b);
    auto info = (Result<int (*)(Value<Word>, Value<Word>), size_t> *)(data);

    while (idx1 < vec1->header._len && idx2 < vec2->header._len)
    {
        Value<Word> x = vec_get_value(vec1, info->_result_1, idx1);
        Value<Word> y = vec_get_value(vec2, info->_result_1, idx2);
        int cmp = info->_result_0(x, y);
        if (cmp != 0)
            return cmp;
        idx1++;
        idx2++;
    }
    return 0;
}

}
