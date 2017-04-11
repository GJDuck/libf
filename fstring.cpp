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

#define STRING_FRAG_MAX_SIZE    (32 - sizeof(struct StrData))
#define STRING_FRAG_MIN_SIZE    8

#define MAX_ESCAPE_CHAR_BUF     32

#define CHAR32_MAX_SIZE         4

struct StrData
{
    _FragHeader header;
    size_t size;
    char data[];
};

static inline StrData *str_data_from_frag(_Frag frag)
{
    _FragHeader &header = frag;
    return (StrData *)&header;
}

static inline _Frag str_frag_from_data(StrData *str)
{
    _FragHeader *header = &str->header;
    _Frag frag = header;
    return frag;
}

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
extern PURE Value<Word> _string_frag_foldl(size_t idx, _Frag frag,
    Value<Word> arg,
    Value<Word> (*f)(void *, Value<Word>, size_t, char32_t), void *data)
{
    StrData *str = str_data_from_frag(frag);
    ssize_t len = (ssize_t)str->header._len - 1;
    for (size_t i = 0; len >= 0; len--)
    {
        char32_t c = char32_decode(str->data + i);
        arg = f(data, arg, idx + i, c);
        i += char32_size(c);
    }
    return arg;
}

// TODO: _string_frag_foldr

/*
 * String fragment map.
 */
extern PURE _Frag _string_frag_map(size_t idx, _Frag frag,
    char32_t (*f)(void *, size_t, char32_t), void *data)
{
    StrData *str = str_data_from_frag(frag);
    char cs[CHAR32_MAX_SIZE * str->header._len];
    size_t i = 0, j = 0, k = 0;
    for (; i < str->header._len; i++)
    {
        char32_t c = char32_decode(str->data + j);
        j += char32_size(c);
        c = f(data, idx + i, c);
        char32_encode(cs + k, c);
        k += char32_size(c);
    }
    StrData *new_str = (StrData *)gc_malloc_atomic(sizeof(StrData) +
        k * sizeof(char));
    new_str->header._len = str->header._len;
    new_str->size = k;
    memmove(new_str->data, cs, k * sizeof(char));
    return str_frag_from_data(new_str);
}

/*
 * String fragment filter map.
 */
extern PURE Optional<_Frag> _string_frag_filter_map(size_t idx, _Frag frag,
    Optional<char32_t> (*f)(void *, size_t, char32_t), void *data)
{
    StrData *str = str_data_from_frag(frag);
    char cs[CHAR32_MAX_SIZE * str->header._len];
    size_t i = 0, j = 0, k = 0, l = 0;
    for (; i < str->header._len; i++)
    {
        char32_t c = char32_decode(str->data + j);
        j += char32_size(c);
        Optional<char32_t> r = f(data, idx + i, c);
        if (empty(r))
            continue;
        char32_encode(cs + k, r);
        k += char32_size(c);
        l++;
    }
    if (k == 0)
        return Optional<_Frag>();
    StrData *new_str = (StrData *)gc_malloc_atomic(sizeof(StrData) +
        k * sizeof(char));
    new_str->header._len = l;
    new_str->size = k;
    memmove(new_str->data, cs, k * sizeof(char));
    return str_frag_from_data(new_str);
}

/*
 * String init.
 */
static PURE _Seq _string_init_2(_Seq s, const char *cstr)
{
    size_t len = strlen(cstr);
    for (size_t i = 0; i < len; )
    {
        size_t frag_size = (len-i > STRING_FRAG_MAX_SIZE? STRING_FRAG_MAX_SIZE:
            len-i);
        StrData *str = (StrData *)gc_malloc_atomic(sizeof(StrData) +
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
        str->header._len = frag_len;
        str->size = j;
        s = _seq_push_back(s, str_frag_from_data(str));
    }
    return s;
}

/*
 * String init.
 */
extern PURE _Seq _string_init(const char *cstr)
{
    return _string_init_2(_seq_empty(), cstr);
}

/*
 * String init.
 */
extern PURE _Seq _string_init_with_char(char32_t c)
{
    size_t clen = char32_size(c);
    StrData *str = (StrData *)gc_malloc_atomic(
        sizeof(StrData) + clen * sizeof(char));
    str->header._len = 1;
    str->size = clen;
    char32_encode(str->data, c);
    _Seq s = _seq_empty();
    s = _seq_push_back(s, str_frag_from_data(str));
    return s;
}

/*
 * String length accumulate.
 */
static Value<Word> string_cstr_len_accumulate(void *data0, Value<Word> arg,
    size_t idx, _Frag frag)
{
    size_t len = _bit_cast<size_t>(arg);
    StrData *str = str_data_from_frag(frag);
    len += str->size;
    return _bit_cast<Value<Word>>(len);
}

/*
 * String accumulate.
 */
static Value<Word> string_cstr_accumulate(void *data0, Value<Word> arg,
    size_t idx, _Frag frag)
{
    size_t j = _bit_cast<size_t>(arg);
    StrData *str = str_data_from_frag(frag);
    char *data = (char *)data0;
    for (size_t i = 0; i < str->size; i++)
        data[j++] = str->data[i];
    return _bit_cast<Value<Word>>(j);
}

/*
 * String to C-String.
 */
extern PURE char *_string_cstring(_Seq s)
{
    const Value<Word> len0 = _seq_foldl(s, _bit_cast<Value<Word>>((size_t)0),
        string_cstr_len_accumulate, nullptr);
    size_t len = _bit_cast<size_t>(len0);
    char *cstr = (char *)gc_malloc_atomic((len + 1) * sizeof(char));
    const Value<Word> len1 = _seq_foldl(s, _bit_cast<Value<Word>>((size_t)0),
        string_cstr_accumulate, (void *)cstr);
    len = _bit_cast<size_t>(len1);
    cstr[len] = '\0';
    return cstr;
}

/*
 * String list accumulate.
 */
static Value<Word> string_list_accumulate(void *data, Value<Word> arg,
    size_t idx, _Frag frag)
{
    List<char32_t> cs = _bit_cast<List<char32_t>>(arg);
    StrData *str = str_data_from_frag(frag);
    for (size_t i = 0, j = 0; i < str->header._len; i++)
    {
        char32_t c = char32_decode(str->data + j);
        cs = list(c, cs);
        j += char32_size(c);
    }
    return _bit_cast<Value<Word>>(cs);
}

/*
 * String to list.
 */
extern PURE List<char32_t> _string_list(const _Seq s)
{
    const Value<Word> r = _seq_foldl(s,
        _bit_cast<Value<Word>>(list<char32_t>()), string_list_accumulate,
        nullptr);
    return _bit_cast<List<char32_t>>(r);
}

/*
 * String lookup.
 */
extern PURE char32_t _string_lookup(_Seq s, size_t idx0)
{
    auto [frag, idx] = _seq_lookup(s, idx0);
    StrData *str = str_data_from_frag(frag);
    idx = cstr_index(str->data, idx);
    return char32_decode(str->data + idx);
}

/*
 * String fragment lookup.
 */
extern PURE char32_t _string_frag_lookup(_Frag frag, size_t idx)
{
    StrData *str = str_data_from_frag(frag);
    idx = cstr_index(str->data, idx);
    return char32_decode(str->data + idx);
}

/*
 * String search.
 */
extern PURE char32_t _string_search(_Seq s, size_t idx)
{
    if (idx >= _seq_length(s))
        return '\0';
    return _string_lookup(s, idx);
}

/*
 * String append char.
 */
extern PURE _Seq _string_append_char(_Seq s, char32_t c)
{
    size_t clen = char32_size(c);
    if (_seq_is_empty(s))
    {
string_append_char_push_back:
        StrData *str = (StrData *)gc_malloc_atomic(sizeof(StrData) +
            clen * sizeof(char));
        str->header._len = 1;
        str->size = clen;
        char32_encode(str->data, c);
        s = _seq_push_back(s, str_frag_from_data(str));
        return s;
    }
    const _Frag frag = _seq_peek_back(s);
    const StrData *str = str_data_from_frag(frag);
    if (str->size + clen > STRING_FRAG_MAX_SIZE)
        goto string_append_char_push_back;

    StrData *new_str = (StrData *)gc_malloc_atomic(sizeof(StrData) +
        (str->size + clen) * sizeof(char));
    new_str->header._len  = str->header._len + 1;
    new_str->size = str->size + clen;
    memmove(new_str->data, str->data, str->size);
    char32_encode(new_str->data + str->size, c);
    return _seq_replace_back(s, str_frag_from_data(new_str));
}

/*
 * String append C-String.
 */
extern PURE _Seq _string_append_cstring(_Seq s, const char *cstr)
{
    size_t len = strlen(cstr);
    if (!_seq_is_empty(s) && len < STRING_FRAG_MIN_SIZE)
    {
        _Frag frag = _seq_peek_back(s);
        StrData *str = str_data_from_frag(frag);
        if (len + str->size <= STRING_FRAG_MAX_SIZE)
        {
            StrData *new_str = (StrData *)gc_malloc_atomic(sizeof(StrData) +
                (str->size + len) * sizeof(char));
            memmove(new_str->data, str->data, str->size);
            memmove(new_str->data + str->size, cstr, len);
            new_str->header._len = str->header._len + cstr_len(cstr);
            new_str->size = str->size + len;
            return _seq_replace_back(s, str_frag_from_data(new_str));
        }
    }
    return _string_init_2(s, cstr);
}

/*
 * String split.
 */
extern PURE Result<_Seq, _Seq> _string_split(const _Seq s, size_t i0)
{
    if (i0 >= _seq_length(s))
        return {s, _seq_empty()};
    const auto [sl0, frag, i, sr0] = _seq_split(s, i0);
    _Seq sl = sl0;
    _Seq sr = sr0;
    StrData *str = str_data_from_frag(frag);
    if (i == 0)
        sr = _seq_push_front(sr, frag);
    else if (i == str->header._len)
        sl = _seq_push_back(sl, frag);
    else
    {
        size_t idx = cstr_index(str->data, i);
        StrData *left = (StrData *)gc_malloc_atomic(sizeof(StrData) +
            idx * sizeof(char));
        StrData *right = (StrData *)gc_malloc_atomic(sizeof(StrData) +
            (str->size - idx) * sizeof(char));
        memmove(left->data, str->data, idx);
        memmove(right->data, str->data + idx, str->size - idx);
        left->size = idx;
        right->size = str->size - idx;
        left->header._len = i;
        right->header._len = str->header._len - i;
        sl = _seq_push_back(sl, str_frag_from_data(left));
        sr = _seq_push_front(sr, str_frag_from_data(right));
    }
    return {sl, sr};
}

/*
 * String split left.
 */
extern PURE _Seq _string_left(const _Seq s, size_t i0)
{
    if (i0 >= _seq_length(s))
        return s;
    const auto [sl0, frag, i] = _seq_left(s, i0);
    _Seq sl = sl0;
    StrData *str = str_data_from_frag(frag);
    if (i == str->header._len)
        sl = _seq_push_back(sl, frag);
    else if (i > 0)
    {
        size_t idx = cstr_index(str->data, i);
        StrData *left = (StrData *)gc_malloc_atomic(sizeof(StrData) +
            idx * sizeof(char));
        memmove(left->data, str->data, idx);
        left->size = idx;
        left->header._len = i;
        sl = _seq_push_back(sl, str_frag_from_data(left));
    }
    return sl;
}

/*
 * String split right.
 */
extern PURE _Seq _string_right(const _Seq s, size_t i0)
{
    if (i0 >= _seq_length(s))
        return _seq_empty();
    const auto [frag, i, sr0] = _seq_right(s, i0);
    _Seq sr = sr0;
    StrData *str = str_data_from_frag(frag);
    if (i == 0)
        sr = _seq_push_front(sr, frag);
    else if (i < str->header._len)
    {
        size_t idx = cstr_index(str->data, i);
        StrData *right = (StrData *)gc_malloc_atomic(sizeof(StrData) +
            (str->size - idx) * sizeof(char));
        memmove(right->data, str->data + idx, str->size - idx);
        right->size = str->size - idx;
        right->header._len = str->header._len - i;
        sr = _seq_push_front(sr, str_frag_from_data(right));
    }
    return sr;
}

/*
 * String between.
 */
extern PURE _Seq _string_between(const _Seq s, size_t i, size_t j)
{
    if (j <= i)
        return _seq_empty();
    _Seq r = _string_right(s, i);
    r = _string_left(r, j-i);
    return r;
}

/*
 * String insert.
 */
extern PURE _Seq _string_insert(const _Seq s, size_t i, const _Seq t)
{
    auto [s1, s2] = _string_split(s, i);
    _Seq a = _seq_append(s1, t);
    a = _seq_append(a, s2);
    return a;
}

/*
 * String delete.
 */
extern PURE _Seq _string_delete(const _Seq s, size_t i, size_t j)
{
    _Seq r = (i == 0? _seq_empty(): _string_left(s, i));
    if (j >= _seq_length(s)-1)
        return r;
    r = _seq_append(r, _string_right(s, j));
    return r;
}

/*
 * String fragment compare.
 */
extern PURE int _string_frag_compare(void *unused, _Frag a, size_t idx1,
    _Frag b, size_t idx2)
{
    const StrData *str1 = str_data_from_frag(a);
    const StrData *str2 = str_data_from_frag(b);

    size_t i = cstr_index(str1->data, idx1);
    size_t j = cstr_index(str2->data, idx2);

    while (idx1 < str1->header._len && idx2 < str2->header._len)
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
 * Character find.
 */
extern PURE Optional<size_t> find(String s, char32_t c, size_t pos)
{
    StringItr i = begin(s), ie = end(s);
    if (pos != 0)
        i += pos;
    while (i != ie)
    {
        char32_t d = *i;
        if (c == d)
            return Optional<size_t>(pos);
        pos++;
        ++i;
    }
    return Optional<size_t>();
}


/*
 * Sub-string find.
 */
extern PURE Optional<size_t> find(String s, String t, size_t pos)
{
    StringItr i = begin(s), ie = end(s);
    StringItr j = begin(t), je = end(t);
    if (pos != 0)
        i += pos;
    if (j == je)
        return Optional<size_t>(pos);
    char32_t d = *j;
    while (i != ie)
    {
        char32_t c = *i;
        if (c != d)
        {
            ++i;
            pos++;
            continue;
        }
        size_t count = 0;
        ++i;
        ++j;
        while (i != ie && j != je && *i == *j)
        {
            count++;
            ++i;
            ++j;
        }
        if (j == je)
            return Optional<size_t>(pos);
        i -= count;
        j -= (count + 1);
        pos++;
    }
    return Optional<size_t>();
}

/*
 * Sub-string find.
 */
extern PURE Optional<size_t> find(String s, const char *t, size_t pos)
{
    StringItr i = begin(s), ie = end(s);
    if (pos != 0)
        i += pos;
    if (t[0] == '\0')
        return Optional<size_t>(pos);
    while (i != ie)
    {
        char32_t c = *i;
        if (c != t[0])
        {
            ++i;
            pos++;
            continue;
        }
        size_t count = 0;
        ++i;
        while (i != ie && t[count+1] != '\0' && *i == t[count+1])
        {
            count++;
            ++i;
        }
        if (t[count+1] == '\0')
            return Optional<size_t>(pos);
        i -= count;
        pos++;
    }
    return Optional<size_t>();
}

/*
 * Sub-string replace.
 */
extern PURE Result<String, Optional<size_t>> replace(String s, String t,
    String r, size_t pos)
{
    auto idx = find(s, t, pos);
    if (empty(idx))
        return {s, idx};
    String sl = left(s, idx);
    String sr = right(s, idx+size(t));
    return {sl + r + sr, idx};
}

/*
 * Sub-string replace.
 */
extern PURE Result<String, Optional<size_t>> replace(String s, const char *t,
    String r, size_t pos)
{
    size_t len = strlen(t);
    auto idx = find(s, t, pos);
    if (empty(idx))
        return {s, idx};
    String sl = left(s, idx);
    String sr = right(s, idx+len);
    return {sl + r + sr, idx};
}

/*
 * Sub-string replace all.
 */
extern PURE String replace_all(String s, String t, String r, size_t pos)
{
    while (true)
    {
        auto [s1, idx] = replace(s, t, r, pos);
        if (empty(idx))
            return s;
        pos = idx + size(r);
        s = s1;
    }
}

/*
 * Sub-string replace all.
 */
extern PURE String replace_all(String s, const char *t, String r, size_t pos)
{
    while (true)
    {
        auto [s1, idx] = replace(s, t, r, pos);
        if (empty(idx))
            return s;
        pos = idx + size(r);
        s = s1;
    }
}

/*
 * String show.
 */
extern PURE String show(String s)
{
    String r = string('\"');
    r = foldl(s, r, [] (String acc, size_t idx, char32_t c) -> String
    {
        switch (c)
        {
            case '\0':
                return acc + "\\0";
            case '\a':
                return acc + "\\a";
            case '\f':
                return acc + "\\f";
            case '\n':
                return acc + "\\n";
            case '\r':
                return acc + "\\r";
            case '\t':
                return acc + "\\t";
            case '\v':
                return acc + "\\v";
            case '\\':
                return acc + "\\\\";
            case '\"':
                return acc + "\\\"";
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
            return acc + buf;
        }
        return acc + c;
    });
    r = r + '\"';
    return r;
}

}

