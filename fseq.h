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

#ifndef _FSEQ_H
#define _FSEQ_H

#include "fdefs.h"
#include "fbase.h"

namespace F
{

#define _FRAG_MAX                  16

static PURE inline _Seq _seq_empty(void)
{
    const _Seq _empty = set<_Seq>((seq_nil_s *)nullptr);
    return _empty;
}

#define _SEQ_EMPTY                 _seq_empty()

extern PURE bool _seq_is_empty(_Seq _s);
extern PURE size_t _seq_length(_Seq _s);
extern PURE Result<_Frag, size_t> _seq_lookup(_Seq _s, size_t _idx);
extern PURE _Seq _seq_push_front(_Seq _s, _Frag _frag);
extern PURE _Seq _seq_replace_front(_Seq _s, _Frag _frag);
extern PURE Result<_Seq, _Frag> _seq_pop_front(_Seq _s);
extern PURE _Frag _seq_peek_front(_Seq s);
extern PURE _Seq _seq_push_back(_Seq _s, _Frag _frag);
extern PURE _Seq _seq_replace_back(_Seq _s, _Frag _frag);
extern PURE Result<_Seq, _Frag> _seq_pop_back(_Seq _s);
extern PURE _Frag _seq_peek_back(_Seq _s);
extern PURE _Seq _seq_append(_Seq _s, _Seq _t);
extern PURE Result<_Seq, _Frag, size_t, _Seq> _seq_split(_Seq _s, size_t _idx);
extern PURE Result<_Seq, _Frag, size_t> _seq_left(_Seq _s, size_t _idx);
extern PURE Result<_Frag, size_t, _Seq> _seq_right(_Seq _s, size_t _idx);
extern PURE int _seq_compare(_Seq _s, _Seq t, void *_data,
    int (_compare)(void *, _Frag, size_t, _Frag, size_t));
extern PURE Any _seq_foldl(_Seq _s, Any _arg, Any (*_f)(void *, Any, _Frag),
    void *_data);
extern PURE Any _seq_foldr(_Seq _s, Any _arg, Any (*_f)(void *, Any, _Frag),
    void *_data);
extern PURE _Seq _seq_map(_Seq _s, _Frag (*_f)(void *, _Frag), void *data);
extern PURE bool _seq_verify(_Seq _s);

}               /* namespace F */

#endif          /* _FSEQ_H */
