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

#ifndef _FSEQ_H
#define _FSEQ_H

#include "fbase.h"
#include "fvalue.h"

namespace F
{

/*
 * Seq object.
 */
struct _SeqNil
{
    // Empty
};
struct _SeqSingle;
struct _SeqDeep;
typedef Union<_SeqNil, _SeqSingle, _SeqDeep> _Seq;

struct _FragHeader
{
    size_t _len;
};

typedef _Boxed<_FragHeader> _Frag;

inline PURE _Seq _seq_empty(void)
{
    return (_SeqNil){};
}

extern PURE bool _seq_is_empty(_Seq _s);
extern PURE size_t _seq_length(_Seq _s);
extern PURE Result<_Frag, size_t> _seq_lookup(_Seq _s, size_t _idx);
extern PURE Value<Word> _seq_search_left(_Seq _s, void *_data,
    Value<Word> _state, Value<Word> (*_next)(void *, _Frag, Value<Word>),
    bool (*_stop)(Value<Word>));
extern PURE _Seq _seq_push_front(_Seq _s, _Frag _frag);
extern PURE _Seq _seq_replace_front(_Seq _s, _Frag _frag);
extern PURE Result<_Seq, _Frag> _seq_pop_front(_Seq _s);
extern PURE _Frag _seq_peek_front(_Seq s);
extern PURE _Seq _seq_push_back(_Seq _s, _Frag _frag);
extern PURE _Seq _seq_replace_back(_Seq _s, _Frag _frag);
extern PURE Result<_Seq, _Frag> _seq_pop_back(_Seq _s);
extern PURE _Frag _seq_peek_back(_Seq _s);
extern PURE _Seq _seq_append(_Seq _s, _Seq _t);
extern PURE Result<_Seq, _Frag, size_t, _Seq> _seq_split(_Seq _s,
    size_t _idx);
extern PURE Result<_Seq, _Frag, size_t> _seq_left(_Seq _s, size_t _idx);
extern PURE Result<_Frag, size_t, _Seq> _seq_right(_Seq _s, size_t _idx);
extern PURE int _seq_compare(_Seq _s, _Seq t, void *_data,
    int (_compare)(void *, _Frag, size_t, _Frag, size_t));
extern PURE Value<Word> _seq_foldl(_Seq _s, Value<Word> _arg,
    Value<Word> (*_f)(void *, Value<Word>, size_t, _Frag), void *_data);
extern PURE Value<Word> _seq_foldr(_Seq _s, Value<Word> _arg,
    Value<Word> (*_f)(void *, Value<Word>, size_t, _Frag), void *_data);
extern PURE _Seq _seq_map(_Seq _s, _Frag (*_f)(void *, size_t, _Frag),
    void *_data);
extern PURE bool _seq_verify(_Seq _s);
extern _Frag _seq_frag_alloc(size_t _size);

struct _SeqItrEntry
{
    uint64_t _type:3;
    uint64_t _offset:48;
    Value<Word> _value;
};

struct _SeqItr;

extern void _seq_itr_begin(_SeqItr *_itr, _Seq _s);
extern void _seq_itr_end(_SeqItr *_itr, _Seq _s);
extern void _seq_itr_move(_SeqItr *_itr, ssize_t);
extern _Frag _seq_itr_get(_SeqItr *_itr, size_t *idx_ptr);
extern int _seq_itr_compare(const _SeqItr *_i, const _SeqItr *_j);
extern void _seq_itr_copy(_SeqItr *_dstitr, const _SeqItr *_srcitr);

struct _SeqItr
{
    uint64_t _ptr:8;
    uint64_t _idx:56;
    Value<Word> _state;

    _SeqItr()
    {
        _seq_itr_begin(this, _seq_empty());
    }

    _SeqItr(const _SeqItr &_itr)
    {
        _seq_itr_copy(this, &_itr);
    }

    _SeqItr(_SeqItr &&_itr) : _ptr(_itr._ptr), _idx(_itr._idx),
        _state(_itr._state)
    {

    }

    _SeqItr &operator=(const _SeqItr &_itr)
    {
        _seq_itr_copy(this, &_itr);
        return *this;
    }
};

inline PURE _SeqItr begin(_Seq _s)
{
    _SeqItr _itr;
    _seq_itr_begin(&_itr, _s);
    return _itr;
}

inline PURE _SeqItr end(_Seq _s)
{
    _SeqItr _itr;
    _seq_itr_end(&_itr, _s);
    return _itr;
}

inline _SeqItr &operator++ (_SeqItr &_i)
{
    _i._idx++;
    return _i;
}

inline _SeqItr &operator-- (_SeqItr &_i)
{
    _i._idx--;
    return _i;
}

inline _SeqItr &operator+ (_SeqItr &_i, ssize_t _offset)
{
    _i._idx = (ssize_t)_i._idx + _offset;
    return _i;
}

inline _SeqItr &operator+= (_SeqItr &_i, ssize_t _offset)
{
    _i._idx = (ssize_t)_i._idx + _offset;
    return _i;
}

inline _SeqItr &operator-= (_SeqItr &_i, ssize_t _offset)
{
    _i._idx = (ssize_t)_i._idx - _offset;
    return _i;
}

inline _Frag operator *(_SeqItr &_i)
{
    return _seq_itr_get(&_i, nullptr);
}

inline bool operator <(const _SeqItr &_i, const _SeqItr &_j)
{
    return (_i._idx < _j._idx);
}

inline bool operator ==(const _SeqItr &_i, const _SeqItr &_j)
{
    return (_i._idx == _j._idx);
}

inline bool operator !=(const _SeqItr &_i, const _SeqItr &_j)
{
    return (_i._idx != _j._idx);
}

}               /* namespace F */

#endif          /* _FSEQ_H */
