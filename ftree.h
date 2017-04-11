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

#include "fbase.h"
#include "fvalue.h"

#include "flist_defs.h"
#include "fstring_defs.h"

namespace F
{

/*
 * Tree object.
 */
struct _TreeNil
{
    // Empty  
};
struct _Tree2;
struct _Tree3;
struct _Tree4;
typedef Union<_TreeNil, _Tree2, _Tree3, _Tree4> _Tree;

typedef int (*_Compare)(Value<Word> _a, Value<Word> _b);

inline PURE _Tree _tree_empty(void)
{
    return (_TreeNil){};
}
inline PURE bool _tree_is_empty(_Tree _t)
{
    return (index(_t) == _Tree::index<_TreeNil>());
}

extern PURE _Tree _tree_singleton(Value<Word> _k);
extern PURE const Value<Word> *_tree_search(_Tree _t, Value<Word> _k,
    _Compare _compare);
extern PURE _Tree _tree_insert(_Tree _t, Value<Word> _k, _Compare _compare);
extern PURE _Tree _tree_delete(_Tree _t, Value<Word> _k, _Compare _compare);
extern PURE size_t _tree_size(_Tree _t);
extern PURE Value<Word> _tree_foldl(_Tree _t, Value<Word> _arg,
    Value<Word> (*_func)(void *, Value<Word>, Value<Word>), void *_data);
extern PURE Value<Word> _tree_foldr(_Tree _t, Value<Word> _arg,
    Value<Word> (*_func)(void *, Value<Word>, Value<Word>), void *_data);
extern PURE _Tree _tree_map(_Tree _t,
    Value<Word> (*_func)(void *, Value<Word>), void *_data);
extern PURE List<Value<Word>> _tree_to_list(_Tree _t,
    Value<Word> (*_func)(void *, Value<Word>), void *_data);
extern PURE _Tree _tree_from_list(List<Value<Word>> _xs, _Compare _compare);
extern PURE Result<_Tree, _Tree> _tree_split(_Tree _t, Value<Word> _k,
    _Compare _compare);
extern PURE _Tree _tree_union(_Tree _t, _Tree _u, _Compare _compare);
extern PURE _Tree _tree_intersect(_Tree _t, _Tree _u, _Compare _compare);
extern PURE _Tree _tree_diff(_Tree _t, _Tree _u, _Compare _compare);
extern PURE bool _tree_verify(_Tree _t);
extern PURE int _tree_compare(_Tree _t, _Tree u, void *_data,
    int (*_val_compare)(void *, Value<Word>, Value<Word>));
extern PURE String _tree_show(_Tree _t, String (*_f)(Value<Word>));

struct _TreeItrEntry
{
    uint64_t _offset;
    _Tree _value;
};

struct _TreeItr
{
    uint64_t _ptr:8;
    uint64_t _idx:56;
    Value<Word> _state;
};

extern void _tree_itr_begin(_TreeItr *_itr, _Tree _t);
extern void _tree_itr_end(_TreeItr *_itr, _Tree _t);
extern const Value<Word> &_tree_itr_get(_TreeItr *_itr);

inline PURE _TreeItr begin(_Tree _t)
{
    _TreeItr _itr;
    _tree_itr_begin(&_itr, _t);
    return _itr;
}

inline PURE _TreeItr end(_Tree _t)
{
    _TreeItr _itr;
    _tree_itr_end(&_itr, _t);
    return _itr;
}

inline _TreeItr &operator++ (_TreeItr &_i)
{
    _i._idx++;
    return _i;
}

inline _TreeItr &operator-- (_TreeItr &_i)
{
    _i._idx--;
    return _i;
}

inline _TreeItr &operator+ (_TreeItr &_i, ssize_t _offset)
{
    _i._idx = (ssize_t)_i._idx + _offset;
    return _i;
}

inline _TreeItr &operator- (_TreeItr &_i, ssize_t _offset)
{
    _i._idx = (ssize_t)_i._idx - _offset;
    return _i;
}

inline _TreeItr &operator+= (_TreeItr &_i, ssize_t _offset)
{
    _i._idx = (ssize_t)_i._idx + _offset;
    return _i;
}

inline _TreeItr &operator-= (_TreeItr &_i, ssize_t _offset)
{
    _i._idx = (ssize_t)_i._idx - _offset;
    return _i;
}

inline PURE const Value<Word> &operator* (_TreeItr &_i)
{
    return _tree_itr_get(&_i);
}

inline bool operator< (const _TreeItr &_i, const _TreeItr &_j)
{
    return (_i._idx < _j._idx);
}

inline bool operator== (const _TreeItr &_i, const _TreeItr &_j)
{
    return (_i._idx == _j._idx);
}

inline bool operator!= (const _TreeItr &_i, const _TreeItr &_j)
{
    return (_i._idx != _j._idx);
}

}           /* namespace F */

#include "flist.h"
#include "fstring.h"

#endif      /* _FTREE_H */
