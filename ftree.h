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

#ifndef _FTREE_H
#define _FTREE_H

#include "fdefs.h"
#include "fbase.h"
#include "flist.h"

namespace F
{

typedef int (*_Compare)(Any _a, Any _b);

static PURE inline _Tree _tree_empty(void)
{
    _Tree _empty = set<_Tree>((tree_nil_s *)nullptr);
    return _empty;
}

#define _TREE_EMPTY                    _tree_empty()

#define _TREE_INSERT_REPLACE_FLAG      1
#define _TREE_INSERT_GROW_FLAG         2

extern PURE _Tree _tree_singleton(Any _k);
extern PURE Any *_tree_search(_Tree _t, Any _k, _Compare _compare);
extern PURE Result<_Tree, Any *> _tree_insert(_Tree _t, Any _k,
    int _flags, _Compare _compare);
extern PURE Result<_Tree, Any *> _tree_delete(_Tree _t, Any _k,
    _Compare _compare);
extern PURE size_t _tree_size(_Tree _t);
extern PURE Any _tree_foldl(_Tree _t, Any _arg,
    Any (*_func)(void *, Any, Any), void *_data);
extern PURE Any _tree_foldr(_Tree _t, Any _arg,
    Any (*_func)(void *, Any, Any), void *_data);
extern PURE _Tree _tree_map(_Tree _t, Any (*_func)(void *, Any), void *_data);
extern PURE List<Any> _tree_to_list(_Tree _t, Any (*_func)(void *, Any),
    void *_data);
extern PURE _Tree _tree_from_list(List<Any> _xs, _Compare _compare);
extern PURE Result<_Tree, _Tree> _tree_split(_Tree _t, Any _k,
    _Compare _compare);
extern PURE _Tree _tree_union(_Tree _t, _Tree _u, _Compare _compare);
extern PURE _Tree _tree_intersect(_Tree _t, _Tree _u, _Compare _compare);
extern PURE _Tree _tree_diff(_Tree _t, _Tree _u, _Compare _compare);
extern PURE bool _tree_verify(_Tree _t);
extern PURE int _tree_compare(_Tree _t, _Tree u, void *_data,
    int (*_val_compare)(void *, Any, Any));
extern PURE String _tree_show(_Tree _t, String (*_f)(Any));

}           /* namespace F */

#endif      /* _FTREE_H */
