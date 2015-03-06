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

#ifndef _FDEFS_H
#define _FDEFS_H

#include "fbase.h"

namespace F
{

/*
 * Lists.
 */
struct _Node
{
    Any val;
    _Node *next;
};

template <typename _T>
struct List
{
    _Node *_impl;
};

/*
 * Trees.
 */
struct tree_nil_s;
struct tree2_s;
struct tree3_s;
struct tree4_s;
typedef Multi<tree_nil_s, tree2_s, tree3_s, tree4_s> _Tree;

/*
 * Maps.
 */
template <typename _K, typename _V>
struct Map
{
    _Tree _impl;
};

/*
 * Sets.
 */
template <typename _K>
struct Set
{
    _Tree _impl;
};

/*
 * Tuples.
 */
template <typename... _T>
struct Tuple
{
    Any *_impl;
};

/*
 * Sequences.
 */
struct seq_nil_s;
struct seq_single_s;
struct seq_deep_s;
typedef Multi<seq_nil_s, seq_single_s, seq_deep_s> _Seq;

struct _frag_s
{
    uint8_t len;
};
typedef struct _frag_s *_Frag;

/*
 * Strings.
 */
struct String
{
    _Seq _impl;
};

/*
 * Vectors.
 */
template <typename _T>
struct Vector
{
    _Seq _impl;
};

/*
 * Maybe.
 */
template <typename _T>
struct Maybe
{
    const _T *_impl;
};

}           /* namespace F */

#endif      /* _FDEFS_H */
