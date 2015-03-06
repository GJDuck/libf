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

#include <ctype.h>
#include <stdio.h>

#include "flist.h"
#include "fseq.h"
#include "fstring.h"

namespace F
{

#define STRING_FRAG_MAX_SIZE    (16 - sizeof(struct str_s))
#define STRING_FRAG_MIN_SIZE    8

#define MAX_ESCAPE_CHAR_BUF     32

#define CHAR32_MAX_SIZE         4

typedef _frag_s frag_s;
typedef _Frag frag_t;
typedef _Seq seq_t;

struct str_s
{
    uint8_t len;
    uint8_t size;
    char data[];
};
typedef struct str_s *str_t;

/*
 * Char32 decode.
 */
static PURE char32_t char32_decode(const char *cstr)
{
    char32_t c = 0;
    if ((*cstr & 0x80) == 0)
    {
        c = (char32_t)*cstr;
        return c;
    }
    if ((*cstr & 0xE0) == 0xC0)
    {
        c = ((((char32_t)*cstr) & 0x1F) << 6);
char32_decode_2_bytes:
        cstr++;
        if ((*cstr & 0xC0) != 0x80)
            goto char32_decode_bad_seq_error;
        c |= (((char32_t)*cstr) & 0x3F);
        return c;
    }
    if ((*cstr & 0xF0) == 0xE0)
    {
        c = ((((char32_t)*cstr) & 0x0F) << 12);
char32_decode_3_bytes:
        cstr++;
        if ((*cstr & 0xC0) != 0x80)
            goto char32_decode_bad_seq_error;
        c |= ((((char32_t)*cstr) & 0x3F) << 6);
        goto char32_decode_2_bytes;
    }
    if ((*cstr & 0xF8) == 0xF0)
    {
        c = ((((char32_t)*cstr) & 0x07) << 18);
        cstr++;
        if ((*cstr & 0xC0) != 0x80)
            goto char32_decode_bad_seq_error;
        c |= ((((char32_t)*cstr) & 0x3F) << 12);
        goto char32_decode_3_bytes;
    }

char32_decode_bad_seq_error:
    error("bad utf-8 character encoding", EILSEQ);
}

/*
 * Char32 encode.
 */
static void char32_encode(char *cstr, char32_t c)
{
    if (c <= 0x7F)
    {
        cstr[0] = c;
        return;
    }
    if (c <= 0x7FF)
    {
        cstr[1] = (c & 0x3F) | 0x80;
        c >>= 6;
        cstr[0] = c | 0xC0;
        return;
    }
    if (c <= 0xFFFF)
    {
        cstr[2] = (c & 0x3F) | 0x80;
        c >>= 6;
        cstr[1] = (c & 0x3F) | 0x80;
        c >>= 6;
        cstr[0] = c | 0xE0;
        return;
    }
    if (c <= 0x1FFFFF)
    {
        cstr[3] = (c & 0x3F) | 0x80;
        c >>= 6;
        cstr[2] = (c & 0x3F) | 0x80;
        c >>= 6;
        cstr[1] = (c & 0x3F) | 0x80;
        c >>= 6;
        cstr[0] = c | 0xF0;
        return;
    }
    error("bad character", EILSEQ);
}

/*
 * Char32 decode length.
 */
static PURE size_t char32_decode_len(const char *cstr)
{
    char c = *cstr;
    if ((c & 0x80) == 0)
        return 1;
    if ((c & 0xE0) == 0xC0)
        return 2;
    if ((c & 0xF0) == 0xE0)
        return 3;
    if ((c & 0xF8) == 0xF0)
        return 4;
    error("bad utf-8 character encoding", EILSEQ);
}

/*
 * Char32 size.
 */
static PURE size_t char32_size(char32_t c)
{
    if (c <= 0x7F)
        return 1;
    if (c <= 0x7FF)
        return 2;
    if (c <= 0xFFFF)
        return 3;
    if (c <= 0x1FFFFF)
        return 4;
    error("bad character", EILSEQ);
}

/*
 * C-String length.
 */
static PURE size_t cstr_len(const char *cstr)
{
    size_t len = 0, i = 0;
    while (true)
    {
        if (cstr[i] == '\0')
            return len;
        i += char32_decode_len(cstr + i);
        len++;
    }
}

/*
 * C-String index.
 */
static PURE size_t cstr_index(const char *cstr, size_t idx)
{
    size_t j = 0;
    for (size_t i = 0; i < idx; i++)
    {
        size_t clen = char32_decode_len(cstr + j);
        j += clen;
    }
    return j;
}

/*
 * String fragment fold left.
 */
extern PURE Any _string_frag_foldl(frag_t frag, Any arg,
    Any (*f)(void *, Any, char32_t), void *data)
{
    str_t str = cast<str_t>(frag);
    ssize_t len = (ssize_t)str->len - 1;
    for (size_t i = 0; len >= 0; len--)
    {
        char32_t c = char32_decode(str->data + i);
        arg = f(data, arg, c);
        i += char32_size(c);
    }
    return arg;
}

// TODO: _string_frag_foldr

/*
 * String fragment map.
 */
extern PURE frag_t _string_frag_map(frag_t frag,
    char32_t (*f)(void *, char32_t), void *data)
{
    str_t str = cast<str_t>(frag);
    char cs[CHAR32_MAX_SIZE * str->len];
    size_t i = 0, j = 0, k = 0;
    for (; i < str->len; i++)
    {
        char32_t c = char32_decode(str->data + j);
        j += char32_size(c);
        c = f(data, c);
        char32_encode(cs + k, c);
        k += char32_size(c);
    }
    str_t new_str = (str_t)gc_malloc_atomic(sizeof(str_s) + k * sizeof(char));
    new_str->len = str->len;
    new_str->size = k;
    memmove(new_str->data, cs, k * sizeof(char));
    return cast<frag_t>(new_str);
}

/*
 * String fragment filter map.
 */
extern PURE frag_t _string_frag_filter_map(frag_t frag,
    Result<bool, char32_t> (*f)(void *, char32_t), void *data)
{
    str_t str = cast<str_t>(frag);
    char cs[CHAR32_MAX_SIZE * str->len];
    size_t i = 0, j = 0, k = 0, l = 0;
    for (; i < str->len; i++)
    {
        char32_t c = char32_decode(str->data + j);
        j += char32_size(c);
        Result<bool, char32_t> r = f(data, c);
        if (!r.fst)
            continue;
        char32_encode(cs + k, r.snd);
        k += char32_size(c);
        l++;
    }
    if (k == 0)
        return nullptr;
    str_t new_str = (str_t)gc_malloc_atomic(sizeof(str_s) + k * sizeof(char));
    new_str->len = l;
    new_str->size = k;
    memmove(new_str->data, cs, k * sizeof(char));
    return cast<frag_t>(new_str);
}

/*
 * String init.
 */
static PURE seq_t _string_init_2(seq_t s, const char *cstr)
{
    size_t len = strlen(cstr);
    for (size_t i = 0; i < len; )
    {
        size_t frag_size = (len-i > STRING_FRAG_MAX_SIZE? STRING_FRAG_MAX_SIZE:
            len-i);
        str_t str = (str_t)gc_malloc_atomic(sizeof(str_s) +
            frag_size * sizeof(char));
        size_t j = 0, frag_len = 0;
        while (true)
        {
            size_t clen = char32_decode_len(cstr + i + j);
            if (j + clen > frag_size)
                break;
            for (size_t k = 0; k < clen; k++)
                str->data[j + k] = cstr[i + j + k];
            j += clen;
            frag_len++;
        }
        i += j;
        str->len  = frag_len;
        str->size = j;
        s = _seq_push_back(s, cast<frag_t>(str));
    }
    return s;
}

/*
 * String init.
 */
extern PURE seq_t _string_init(const char *cstr)
{
    return _string_init_2(_SEQ_EMPTY, cstr);
}

/*
 * String init.
 */
extern PURE seq_t _string_init_with_char(char32_t c)
{
    size_t clen = char32_size(c);
    str_t str = (str_t)gc_malloc_atomic(sizeof(str_s) + clen * sizeof(char));
    str->len = 1;
    str->size = clen;
    char32_encode(str->data, c);
    seq_t s = _SEQ_EMPTY;
    s = _seq_push_back(s, cast<frag_t>(str));
    return s;
}

/*
 * String length accumulate.
 */
static Any string_cstr_len_accumulate(void *data0, Any arg, frag_t frag)
{
    size_t len = cast<size_t>(arg);
    str_t str = cast<str_t>(frag);
    len += str->size;
    return cast<Any>(len);
}

/*
 * String accumulate.
 */
static Any string_cstr_accumulate(void *data0, Any arg, frag_t frag)
{
    size_t j = cast<size_t>(arg);
    str_t str = cast<str_t>(frag);
    char *data = (char *)data0;
    for (size_t i = 0; i < str->size; i++)
        data[j++] = str->data[i];
    return cast<Any>(j);
}

/*
 * String to C-String.
 */
extern PURE char *_string_cstring(seq_t s)
{
    const Any len0 = _seq_foldl(s, cast<Any>((size_t)0),
        string_cstr_len_accumulate, nullptr);
    size_t len = cast<size_t>(len0);
    char *cstr = (char *)gc_malloc_atomic((len + 1) * sizeof(char));
    const Any len1 = _seq_foldl(s, cast<Any>((size_t)0), string_cstr_accumulate,
        (void *)cstr);
    len = cast<size_t>(len1);
    cstr[len] = '\0';
    return cstr;
}

/*
 * String list accumulate.
 */
static Any string_list_accumulate(void *data, Any arg, frag_t frag)
{
    List<char32_t> cs = cast<List<char32_t>>(arg);
    str_t str = cast<str_t>(frag);
    for (size_t i = 0, j = 0; i < str->len; i++)
    {
        char32_t c = char32_decode(str->data + j);
        cs = cons(c, cs);
        j += char32_size(c);
    }
    return cast<Any>(cs);
}

/*
 * String to list.
 */
extern PURE List<char32_t> _string_list(const seq_t s)
{
    const Any r = _seq_foldl(s, cast<Any>(list<char32_t>()),
        string_list_accumulate, nullptr);
    return cast<List<char32_t>>(r);
}

/*
 * String lookup.
 */
extern PURE char32_t _string_lookup(seq_t s, size_t idx)
{
    auto r = _seq_lookup(s, idx);
    idx = r.snd;
    str_t str = cast<str_t>(r.fst);
    idx = cstr_index(str->data, idx);
    return char32_decode(str->data + idx);
}

/*
 * String append char.
 */
extern PURE seq_t _string_append_char(seq_t s, char32_t c)
{
    size_t clen = char32_size(c);
    if (_seq_is_empty(s))
    {
string_append_char_push_back:
        str_t str = (str_t)gc_malloc_atomic(sizeof(str_s) +
            clen * sizeof(char));
        str->len = 1;
        str->size = clen;
        char32_encode(str->data, c);
        s = _seq_push_back(s, cast<frag_t>(str));
        return s;
    }
    const frag_t frag = _seq_peek_back(s);
    const str_t str = cast<str_t>(frag);
    if (str->size + clen > STRING_FRAG_MAX_SIZE)
        goto string_append_char_push_back;

    str_t new_str = (str_t)gc_malloc_atomic(sizeof(str_s) +
        (str->size + clen) * sizeof(char));
    new_str->len  = str->len + 1;
    new_str->size = str->size + clen;
    memmove(new_str->data, str->data, str->size);
    char32_encode(new_str->data + str->size, c);
    return _seq_replace_back(s, cast<frag_t>(new_str));
}

/*
 * String append C-String.
 */
extern PURE seq_t _string_append_cstring(seq_t s, const char *cstr)
{
    size_t len = strlen(cstr);
    if (!_seq_is_empty(s) && len < STRING_FRAG_MIN_SIZE)
    {
        frag_t frag = _seq_peek_back(s);
        str_t str = cast<str_t>(frag);
        if (len + str->size <= STRING_FRAG_MAX_SIZE)
        {
            str_t new_str = (str_t)gc_malloc_atomic(sizeof(str_s) +
                (str->size + len) * sizeof(char));
            memmove(new_str->data, str->data, str->size);
            memmove(new_str->data + str->size, cstr, len);
            new_str->len = str->len + cstr_len(cstr);
            new_str->size = str->size + len;
            return _seq_replace_back(s, cast<frag_t>(new_str));
        }
    }
    return _string_init_2(s, cstr);
}

/*
 * String split.
 */
extern PURE Result<seq_t, seq_t> _string_split(const seq_t s, size_t i)
{
    if (i >= _seq_length(s))
        return {s, _SEQ_EMPTY};
    const auto r = _seq_split(s, i);
    i = r.third;
    seq_t sl = r.fst;
    seq_t sr = r.fourth;
    str_t str = cast<str_t>(r.snd);
    if (i == 0)
        sr = _seq_push_front(sr, r.snd);
    else if (i == str->len)
        sl = _seq_push_back(sl, r.snd);
    else
    {
        size_t idx = cstr_index(str->data, i);
        str_t left = (str_t)gc_malloc_atomic(sizeof(str_s) +
            idx * sizeof(char));
        str_t right = (str_t)gc_malloc_atomic(sizeof(str_s) +
            (str->size - idx) * sizeof(char));
        memmove(left->data, str->data, idx);
        memmove(right->data, str->data + idx, str->size - idx);
        left->size = idx;
        right->size = str->size - idx;
        left->len = i;
        right->len = str->len - i;
        sl = _seq_push_back(sl, cast<frag_t>(left));
        sr = _seq_push_front(sr, cast<frag_t>(right));
    }
    return {sl, sr};
}

/*
 * String split left.
 */
extern PURE seq_t _string_left(const seq_t s, size_t i)
{
    if (i >= _seq_length(s))
        return s;
    const auto r = _seq_left(s, i);
    i = r.third;
    seq_t sl = r.fst;
    str_t str = cast<str_t>(r.snd);
    if (i == str->len)
        sl = _seq_push_back(sl, r.snd);
    else if (i > 0)
    {
        size_t idx = cstr_index(str->data, i);
        str_t left = (str_t)gc_malloc_atomic(sizeof(str_t) +
            idx * sizeof(char));
        memmove(left->data, str->data, idx);
        left->size = idx;
        left->len = i;
        sl = _seq_push_back(sl, cast<frag_t>(left));
    }
    return sl;
}

/*
 * String split right.
 */
extern PURE seq_t _string_right(const seq_t s, size_t i)
{
    if (i >= _seq_length(s))
        return _SEQ_EMPTY;
    const auto r = _seq_right(s, i);
    i = r.snd;
    seq_t sr = r.third;
    str_t str = cast<str_t>(r.fst);
    if (i == 0)
        sr = _seq_push_front(sr, r.fst);
    else if (i < str->len)
    {
        size_t idx = cstr_index(str->data, i);
        str_t right = (str_t)gc_malloc_atomic(sizeof(str_s) +
            (str->size - idx) * sizeof(char));
        memmove(right->data, str->data + idx, str->size - idx);
        right->size = str->size - idx;
        right->len = str->len - i;
        sr = _seq_push_front(sr, cast<frag_t>(right));
    }
    return sr;
}

/*
 * String between.
 */
extern PURE seq_t _string_between(const seq_t s, size_t i, size_t j)
{
    if (j < i)
        return _SEQ_EMPTY;
    seq_t r = _string_right(s, i);
    r = _string_left(r, j-i);
    return r;
}

/*
 * String insert.
 */
extern PURE seq_t _string_insert(const seq_t s, size_t i, const seq_t t)
{
    const Result<seq_t, seq_t> r = _string_split(s, i);
    seq_t a = _seq_append(r.fst, t);
    a = _seq_append(a, r.snd);
    return a;
}

/*
 * String fragment compare.
 */
extern PURE int _string_frag_compare(void *unused, frag_t a, size_t idx1,
    frag_t b, size_t idx2)
{
    const str_t str1 = cast<str_t>(a);
    const str_t str2 = cast<str_t>(b);

    size_t i = cstr_index(str1->data, idx1);
    size_t j = cstr_index(str2->data, idx2);

    while (idx1 < str1->len && idx2 < str2->len)
    {
        char32_t c = char32_decode(str1->data + i);
        char32_t d = char32_decode(str2->data + j);
        if (c < d)
            return 1;
        if (c > d)
            return -1;
        i += char32_size(c);
        j += char32_size(c);  
        idx1++;
        idx2++;
    }
    return 0;
}

/*
 * String show.
 */
extern PURE String show(String s)
{
    String r = string('\"');
    r = foldl(s, r, [] (String acc, char32_t c) -> String
    {
        switch (c)
        {
            case '\0':
                return append(acc, "\\0");
            case '\a':
                return append(acc, "\\a");
            case '\f':
                return append(acc, "\\f");
            case '\n':
                return append(acc, "\\n");
            case '\r':
                return append(acc, "\\r");
            case '\t':
                return append(acc, "\\t");
            case '\v':
                return append(acc, "\\v");
            case '\\':
                return append(acc, "\\\\");
            case '\"':
                return append(acc, "\\\"");
            default:
                break;
        }
        if (c < 0x7F && !isprint(c))
        {
            char buf[MAX_ESCAPE_CHAR_BUF];
            int r = snprintf(buf, sizeof(buf)-1, "\\x%.2x",
                ((unsigned)c) & 0xFF);
            if (r <= 0 || r >= sizeof(buf)-1)
                error("snprintf failed");
            return append(acc, buf);
        }
        return append(acc, c);
    });
    r = append(r, '\"');
    return r;
}

}

