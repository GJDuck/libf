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

#include <stdlib.h>
#include <stdio.h>

#include "fseq.h"

namespace F
{

/*
 * Finger trees.
 */

#define error_bad_tree()        error("data-structure invariant violated")

#define SET(ptr, val)                       \
    do {                                    \
        if ((ptr) != nullptr)               \
            *(ptr) = (val);                 \
    } while (false)

typedef _FragHeader FragHeader;
typedef _Frag Frag;
typedef _Seq Seq;

struct Tree2;
struct Tree3;

typedef Union<Frag, Tree2, Tree3> Tree;

struct Tree2
{
    size_t len;
    Tree t[2];
};
struct Tree3
{
    size_t len;
    Tree t[3];
};

enum 
{
    TREE_LEAF = Tree::index<Frag>(),
    TREE_2    = Tree::index<Tree2>(),
    TREE_3    = Tree::index<Tree3>()
};

struct Dig1
{
    size_t len;
    Tree t[1];
};
struct Dig2
{
    size_t len;
    Tree t[2];
};
struct Dig3
{
    size_t len;
    Tree t[3];
};
struct Dig4
{
    size_t len;
    Tree t[4];
};

typedef Union<Dig1, Dig2, Dig3, Dig4> Dig;

enum
{
    DIG_1 = Dig::index<Dig1>(),
    DIG_2 = Dig::index<Dig2>(),
    DIG_3 = Dig::index<Dig3>(),
    DIG_4 = Dig::index<Dig4>()
};

struct _SeqSingle
{
    size_t len;
    Tree t[1];
};
struct _SeqDeep
{
    size_t len;
    Dig l;
    Seq m;
    Dig r;
};
typedef _SeqNil Nil;
typedef _SeqSingle Single;
typedef _SeqDeep Deep;
typedef _Seq Seq;

enum
{
    NIL    = Seq::index<Nil>(),
    SINGLE = Seq::index<Single>(),
    DEEP   = Seq::index<Deep>()
};

static size_t dig_length(Dig s);
static size_t tree_length(Tree s);
static size_t seq_depth(Seq s);
static Result<Frag, size_t> dig_lookup(Dig s, size_t idx);
static Result<Frag, size_t> tree_lookup(Tree s, size_t idx);
static Seq seq_push_front(Seq s, Tree t);
static Seq seq_pop_front(Seq s, Tree *t);
static Seq seq_push_back(Seq s, Tree t);
static Seq seq_pop_back(Seq s, Tree *t);
static Seq seq_append(Seq s, Tree *m, size_t m_len, Seq t);
static size_t seq_append_middle(Dig s, Tree *m, size_t m_len, Dig t);
static void seq_split(Seq s, size_t *idx, Seq *l, Seq *r, Tree *t);
static Seq deep(Dig l, Seq m);
static Seq deep(Seq m, Dig r);
static void dig_split(Dig s, size_t *idx, Dig *l, Dig *r, bool *have_l,
    bool *have_r, Tree *t);
static Value<Word> seq_foldl(Seq s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data);
static Value<Word> dig_foldl(Dig s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data);
static Value<Word> tree_foldl(Tree s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data);
static Value<Word> seq_foldr(Seq s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data);
static Value<Word> dig_foldr(Dig s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data);
static Value<Word> tree_foldr(Tree s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data);
static Seq seq_map(Seq s, size_t *idx, Frag (*f)(void *, size_t, Frag),
    void *data);
static Dig dig_map(Dig d, size_t *idx, Frag (*f)(void *, size_t, Frag),
    void *data);
static Tree tree_map(Tree t, size_t *idx, Frag (*f)(void *, size_t, Frag),
    void *data);
static bool seq_verify(Seq s, size_t level);
static bool dig_verify(Dig s, size_t level);
static bool tree_verify(Tree s, size_t level);
static PURE Value<Word> dig_search_left(Dig s, void *data, Value<Word> state,
    Value<Word> (*next)(void *, Frag, Value<Word>),
    bool (*stop)(Value<Word>));
static PURE Value<Word> tree_search_left(Tree s, void *data, Value<Word> state,
    Value<Word> (*next)(void *, Frag, Value<Word>),
    bool (*stop)(Value<Word>));

/*
 * Node constructors.
 */
static Seq empty(void)
{
    return (Nil){};
}

static Seq single(Tree t0)
{
    size_t len = tree_length(t0);
    Single node = {len, {t0}};
    return node;
}

static Seq deep(Dig l, Seq m, Dig r)
{
    size_t len = dig_length(l) + _seq_length(m) + dig_length(r);
    Deep node = {len, l, m, r};
    return node;
}

static Dig dig1(Tree t0)
{
    size_t len = tree_length(t0);
    Dig1 node = {len, {t0}};
    return node;
}

static Dig dig2(Tree t0, Tree t1)
{
    size_t len = tree_length(t0) + tree_length(t1);
    Dig2 node = {len, {t0, t1}};
    return node;
}

static Dig dig3(Tree t0, Tree t1, Tree t2)
{
    size_t len = tree_length(t0) + tree_length(t1) + tree_length(t2);
    Dig3 node = {len, {t0, t1, t2}};
    return node;
}

static Dig dig4(Tree t0, Tree t1, Tree t2, Tree t3)
{
    size_t len = tree_length(t0) + tree_length(t1) + tree_length(t2) + 
        tree_length(t3);
    Dig4 node = {len, {t0, t1, t2, t3}};
    return node;
}

static Tree tree2(Tree t0, Tree t1)
{
    size_t len = tree_length(t0) + tree_length(t1);
    Tree2 node = {len, {t0, t1}};
    return node;
}

static Tree tree3(Tree t0, Tree t1, Tree t2)
{
    size_t len = tree_length(t0) + tree_length(t1) + tree_length(t2);
    Tree3 node = {len, {t0, t1, t2}};
    return node;
}

/*
 * Conversion.
 */
static Dig tree_to_dig(Tree t)
{
    switch (index(t))
    {
        case TREE_2:
        {
            const Tree2 &t2 = t;
            return dig2(t2.t[0], t2.t[1]);
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            return dig3(t3.t[0], t3.t[1], t3.t[2]);
        }
        default:
            error_bad_tree();
    }
}

static Seq dig_to_seq(Dig d)
{
    switch (index(d))
    {
        case DIG_1:
        {
            const Dig1 &d1 = d;
            return single(d1.t[0]);
        }
        case DIG_2:
        {
            const Dig2 &d2 = d;
            return deep(dig1(d2.t[0]), empty(), dig1(d2.t[1]));
        }
        case DIG_3:
        {
            const Dig3 &d3 = d;
            return deep(dig2(d3.t[0], d3.t[1]), empty(), dig1(d3.t[2]));
        }
        case DIG_4:
        {
            const Dig4 &d4 = d;
            return deep(dig2(d4.t[0], d4.t[1]), empty(),
                        dig2(d4.t[2], d4.t[3]));
        }
        default:
            error_bad_tree();
    }
}

/*
 * Is empty.
 */
extern PURE bool _seq_is_empty(Seq s)
{
    return (index(s) == NIL);
}

/*
 * Length.
 */
extern PURE size_t _seq_length(Seq s)
{
    switch (index(s))
    {
        case NIL:
            return 0;
        case SINGLE:
        {
            const Single &ss = s;
            return ss.len;
        }
        case DEEP:
        {
            const Deep &sd = s;
            return sd.len;
        }
        default:
            error_bad_tree();
    }
}

static size_t dig_length(Dig s)
{
    switch (index(s))
    {
        case DIG_1:
        {
            const Dig1 &d1 = s;
            return d1.len;
        }
        case DIG_2:
        {
            const Dig2 &d2 = s;
            return d2.len;
        }
        case DIG_3:
        {
            const Dig3 &d3 = s;
            return d3.len;
        }
        case DIG_4:
        {
            const Dig4 &d4 = s;
            return d4.len;
        }
        default:
            error_bad_tree();
    }
}

static size_t tree_length(Tree s)
{
    switch (index(s))
    {
        case TREE_LEAF:
        {
            const Frag &sl = s;
            const FragHeader &sh = sl;
            return sh._len;
        }
        case TREE_2:
        {
            const Tree2 &s2 = s;
            return s2.len;
        }
        case TREE_3:
        {
            const Tree3 &s3 = s;
            return s3.len;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Depth.
 */
static size_t seq_depth(Seq s)
{
    size_t level = 0;
    while (true)
    {
        switch (index(s))
        {
            case NIL:
                return 2 * level + 1;
            case SINGLE:
                return 2 * level + 2;
            case DEEP:
            {
                const Deep &sd = s;
                level++;
                s = sd.m;
                continue;
            }
            default:
                error_bad_tree();
        }
    }
}

/*
 * Verify.
 */
extern PURE bool _seq_verify(Seq s)
{
    return seq_verify(s, 0);
}

static bool seq_verify(Seq s, size_t level)
{
    switch (index(s))
    {
        case NIL:
            return true;
        case SINGLE:
        {
            const Single &ss = s;
            return tree_verify(ss.t[0], level);
        }
        case DEEP:
        {
            const Deep &sd = s;
            return dig_verify(sd.l, level) &&
                   seq_verify(sd.m, level+1) &&
                   dig_verify(sd.r, level);
        }
        default:
            return false;
    }
}

static bool dig_verify(Dig s, size_t level)
{
    switch (index(s))
    {
        case DIG_1:
        {
            const Dig1 &s1 = s;
            return tree_verify(s1.t[0], level);
        }
        case DIG_2:
        {
            const Dig2 &s2 = s;
            return tree_verify(s2.t[0], level) &&
                   tree_verify(s2.t[1], level);
        }
        case DIG_3:
        {
            const Dig3 &s3 = s;
            return tree_verify(s3.t[0], level) &&
                   tree_verify(s3.t[1], level) &&
                   tree_verify(s3.t[2], level);
        }
        case DIG_4:
        {
            const Dig4 &s4 = s;
            return tree_verify(s4.t[0], level) &&
                   tree_verify(s4.t[1], level) &&
                   tree_verify(s4.t[2], level) &&
                   tree_verify(s4.t[3], level);
        }
        default:
            return false;
    }
}

static bool tree_verify(Tree s, size_t level)
{
    switch (index(s))
    {
        case TREE_LEAF:
        {
            const Frag &sl = s;
            const FragHeader &sh = sl;
            if (sh._len == 0)
                return false;
            return (level == 0);
        }
        case TREE_2:
        {
            const Tree2 &s2 = s;
            return tree_verify(s2.t[0], level-1) &&
                   tree_verify(s2.t[1], level-1);
        }
        case TREE_3:
        {
            const Tree3 &s3 = s;
            return tree_verify(s3.t[0], level-1) &&
                   tree_verify(s3.t[1], level-1) &&
                   tree_verify(s3.t[2], level-1);
        }
        default:
            return false;
    }
}

/*
 * Lookup.
 */
extern PURE Result<Frag, size_t> _seq_lookup(Seq s, size_t idx)
{
    size_t len;
    while (true)
    {
        switch (index(s))
        {
            case NIL:
                error("seq lookup out-of-range", ERANGE);
            case SINGLE:
            {
                const Single &ss = s;
                if (idx < ss.len)
                    return tree_lookup(ss.t[0], idx);
                error("seq lookup out-of-range", ERANGE);
            }
            case DEEP:
            {
                const Deep &sd = s;
                len = dig_length(sd.l);
                if (idx < len)
                    return dig_lookup(sd.l, idx);
                idx -= len;
                len = _seq_length(sd.m);
                if (idx < len)
                {
                    s = sd.m;
                    continue;
                }
                idx -= len;
                return dig_lookup(sd.r, idx);
            }
            default:
                error_bad_tree();
        }
    }
}

static Result<Frag, size_t> dig_lookup(Dig s, size_t idx)
{
    size_t len;
    switch (index(s))
    {
        case DIG_1:
        {
            const Dig1 &s1 = s;
            return tree_lookup(s1.t[0], idx);
        }
        case DIG_2:
        {
            const Dig2 &s2 = s;
            len = tree_length(s2.t[0]);
            if (idx < len)
                return tree_lookup(s2.t[0], idx);
            idx -= len;
            return tree_lookup(s2.t[1], idx);
        }
        case DIG_3:
        {
            const Dig3 &s3 = s;
            len = tree_length(s3.t[0]);
            if (idx < len)
                return tree_lookup(s3.t[0], idx);
            idx -= len;
            len = tree_length(s3.t[1]);
            if (idx < len)
                return tree_lookup(s3.t[1], idx);
            idx -= len;
            return tree_lookup(s3.t[2], idx);
        }
        case DIG_4:
        {
            const Dig4 &s4 = s;
            len = tree_length(s4.t[0]);
            if (idx < len)
                return tree_lookup(s4.t[0], idx);
            idx -= len;
            len = tree_length(s4.t[1]);
            if (idx < len)
                return tree_lookup(s4.t[1], idx);
            idx -= len;
            len = tree_length(s4.t[2]);
            if (idx < len)
                return tree_lookup(s4.t[2], idx);
            idx -= len;
            return tree_lookup(s4.t[3], idx);
        }
        default:
            error_bad_tree();
    }
}

static Result<Frag, size_t> tree_lookup(Tree s, size_t idx)
{
    size_t len;
    while (true)
    {
        switch (index(s))
        {
            case TREE_LEAF:
            {
                const Frag &sl = s;
                const FragHeader &sh = sl;
                if (idx < sh._len)
                    return {sl, idx};
                error("seq lookup out-of-range", ERANGE);
            }
            case TREE_2:
            {
                const Tree2 &s2 = s;
                len = tree_length(s2.t[0]);
                if (idx < len)
                {
                    s = s2.t[0];
                    continue;
                }
                idx -= len;
                s = s2.t[1];
                continue;
            }
            case TREE_3:
            {
                const Tree3 &s3 = s;
                len = tree_length(s3.t[0]);
                if (idx < len)
                {
                    s = s3.t[0];
                    continue;
                }
                idx -= len;
                len = tree_length(s3.t[1]);
                if (idx < len)
                {
                    s = s3.t[1];
                    continue;
                }
                idx -= len;
                s = s3.t[2];
                continue;
            }
            default:
                error_bad_tree();
        }
    }
}

/*
 * Push front.
 */
extern PURE Seq _seq_push_front(Seq s, Frag f)
{
    return seq_push_front(s, f);
}

static Seq seq_push_front(Seq s, Tree t)
{
    switch (index(s))
    {
        case NIL:
            return single(t);
        case SINGLE:
        {
            const Single &ss = s;
            return deep(dig1(t), empty(), dig1(ss.t[0]));
        }
        case DEEP:
        {
            const Deep &sd = s;
            switch (index(sd.l))
            {
                case DIG_1:
                {
                    const Dig1 &s1 = sd.l;
                    return deep(dig2(t, s1.t[0]), sd.m, sd.r);
                }
                case DIG_2:
                {
                    const Dig2 &s2 = sd.l;
                    return deep(dig3(t, s2.t[0], s2.t[1]), sd.m, sd.r);
                }
                case DIG_3:
                {
                    const Dig3 &s3 = sd.l;
                    return deep(dig4(t, s3.t[0], s3.t[1], s3.t[2]),
                        sd.m, sd.r);
                }
                case DIG_4:
                {
                    const Dig4 &s4 = sd.l;
                    if (index(sd.m) == NIL && index(sd.r) == DIG_1)
                    {
                        const Dig1 &r1 = sd.r;
                        return deep(dig3(t, s4.t[0], s4.t[1]), empty(),
                            dig3(s4.t[2], s4.t[3], r1.t[0]));
                    }
                    Tree nt = tree3(s4.t[1], s4.t[2], s4.t[3]);
                    Seq m = seq_push_front(sd.m, nt);
                    return deep(dig2(t, s4.t[0]), m, sd.r);
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Pop front.
 */
extern PURE Result<Seq, Frag> _seq_pop_front(Seq s)
{
    Tree t;
    s = seq_pop_front(s, &t);
    const Frag &f = t;
    return {s, f};
}

static Seq seq_pop_front(Seq s, Tree *t)
{
    switch (index(s))
    {
        case NIL:
            error("pop-front empty");
        case SINGLE:
        {
            const Single &ss = s;
            *t = ss.t[0];
            return empty();
        }
        case DEEP:
        {
            const Deep &sd = s;
            switch (index(sd.l))
            {
                case DIG_1:
                {
                    const Dig1 &s1 = sd.l;
                    *t = s1.t[0];
                    return deep(sd.m, sd.r);
                }
                case DIG_2:
                {
                    const Dig2 &s2 = sd.l;
                    *t = s2.t[0];
                    return deep(dig1(s2.t[1]), sd.m, sd.r);
                }
                case DIG_3:
                {
                    const Dig3 &s3 = sd.l;
                    *t = s3.t[0];
                    return deep(dig2(s3.t[1], s3.t[2]), sd.m, sd.r);
                }
                case DIG_4:
                {
                    const Dig4 &s4 = sd.l;
                    *t = s4.t[0];
                    return deep(dig3(s4.t[1], s4.t[2], s4.t[3]), sd.m, sd.r);
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Replace front.
 */
extern PURE Seq _seq_replace_front(Seq s, Frag f)
{
    Tree l = f;
    switch (index(s))
    {
        case NIL:
            error("replace-back empty");
        case SINGLE:
            return single(l);
        case DEEP:
        {
            const Deep &sd = s;
            switch (index(sd.l))
            {
                case DIG_1:
                    return deep(dig1(l), sd.m, sd.r);
                case DIG_2:
                {
                    const Dig2 &s2 = sd.l;
                    return deep(dig2(l, s2.t[1]), sd.m, sd.r);
                }
                case DIG_3:
                {
                    const Dig3 &s3 = sd.l;
                    return deep(dig3(l, s3.t[1], s3.t[2]), sd.m, sd.r);
                }
                case DIG_4:
                {
                    const Dig4 &s4 = sd.l;
                    return deep(dig4(l, s4.t[1], s4.t[2], s4.t[3]), sd.m,
                        sd.r);
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Peek front.
 */
extern PURE Frag _seq_peek_front(Seq s)
{
    switch (index(s))
    {
        case NIL:
            error("peek-front empty");
        case SINGLE:
        {
            const Single &ss = s;
            return ss.t[0];
        }
        case DEEP:
        {
            const Deep &sd = s;
            switch (index(sd.l))
            {
                case DIG_1:
                {
                    const Dig1 &r1 = sd.l;
                    return r1.t[0];
                }
                case DIG_2:
                {
                    const Dig2 &r2 = sd.l;
                    return r2.t[0];
                }
                case DIG_3:
                {
                    const Dig3 &r3 = sd.l;
                    return r3.t[0];
                }
                case DIG_4:
                {
                    const Dig4 r4 = sd.l;
                    return r4.t[0];
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Push back.
 */
extern PURE Seq _seq_push_back(Seq s, Frag f)
{
    Tree t = f;
    return seq_push_back(s, t);
}

static Seq seq_push_back(Seq s, Tree t)
{
    switch (index(s))
    {
        case NIL:
            return single(t);
        case SINGLE:
        {
            const Single &ss = s;
            return deep(dig1(ss.t[0]), empty(), dig1(t));
        }
        case DEEP:
        {
            const Deep &sd = s;
            switch (index(sd.r))
            {
                case DIG_1:
                {
                    const Dig1 &s1 = sd.r;
                    return deep(sd.l, sd.m, dig2(s1.t[0], t));
                }
                case DIG_2:
                {
                    const Dig2 &s2 = sd.r;
                    return deep(sd.l, sd.m, dig3(s2.t[0], s2.t[1], t));
                }
                case DIG_3:
                {
                    const Dig3 &s3 = sd.r;
                    return deep(sd.l, sd.m, dig4(s3.t[0], s3.t[1], s3.t[2],
                        t));
                }
                case DIG_4:
                {
                    const Dig4 &s4 = sd.r;
                    if (index(sd.m) == NIL && index(sd.l) == DIG_1)
                    {
                        Dig1 l1 = sd.l;
                        return deep(dig3(l1.t[0], s4.t[0], s4.t[1]),
                            empty(), dig3(s4.t[2], s4.t[3], t));
                    }
                    Tree nt = tree3(s4.t[0], s4.t[1], s4.t[2]);
                    Seq m = seq_push_back(sd.m, nt);
                    return deep(sd.l, m, dig2(s4.t[3], t));
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Pop back.
 */
extern PURE Result<Seq, Frag> _seq_pop_back(Seq s)
{
    Tree t;
    s = seq_pop_back(s, &t);
    const Frag &f = t;
    return {s, f};
}

static Seq seq_pop_back(Seq s, Tree *t)
{
    switch (index(s))
    {
        case NIL:
            error("pop-back empty");
        case SINGLE:
        {
            const Single &ss = s;
            *t = ss.t[0];
            return empty();
        }
        case DEEP:
        {
            const Deep &sd = s;
            switch (index(sd.r))
            {
                case DIG_1:
                {
                    const Dig1 &s1 = sd.r;
                    *t = s1.t[0];
                    return deep(sd.l, sd.m);
                }
                case DIG_2:
                {
                    const Dig2 &s2 = sd.r;
                    *t = s2.t[1];
                    return deep(sd.l, sd.m, dig1(s2.t[0]));
                }
                case DIG_3:
                {
                    const Dig3 &s3 = sd.r;
                    *t = s3.t[2];
                    return deep(sd.l, sd.m, dig2(s3.t[0], s3.t[1]));
                }
                case DIG_4:
                {
                    const Dig4 &s4 = sd.r;
                    *t = s4.t[3];
                    return deep(sd.l, sd.m, dig3(s4.t[0], s4.t[1], s4.t[2]));
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Replace back.
 */
extern PURE Seq _seq_replace_back(Seq s, Frag f)
{
    Tree l = f;
    switch (index(s))
    {
        case NIL:
            error("replace-back empty");
        case SINGLE:
            return single(l);
        case DEEP:
        {
            const Deep &sd = s;
            switch (index(sd.r))
            {
                case DIG_1:
                    return deep(sd.l, sd.m, dig1(l));
                case DIG_2:
                {
                    const Dig2 &s2 = sd.r;
                    return deep(sd.l, sd.m, dig2(s2.t[0], l));
                }
                case DIG_3:
                {
                    const Dig3 &s3 = sd.r;
                    return deep(sd.l, sd.m, dig3(s3.t[0], s3.t[1], l));
                }
                case DIG_4:
                {
                    const Dig4 &s4 = sd.r;
                    return deep(sd.l, sd.m, dig4(s4.t[0], s4.t[1], s4.t[2],
                        l));
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Peek back.
 */
extern PURE Frag _seq_peek_back(Seq s)
{
    switch (index(s))
    {
        case NIL:
            error("peek-back empty");
        case SINGLE:
        {
            const Single &ss = s;
            return ss.t[0];
        }
        case DEEP:
        {
            const Deep &sd = s;
            switch (index(sd.r))
            {
                case DIG_1:
                {
                    const Dig1 &r1 = sd.r;
                    return r1.t[0];
                }
                case DIG_2:
                {
                    const Dig2 &r2 = sd.r;
                    return r2.t[1];
                }
                case DIG_3:
                {
                    const Dig3 &r3 = sd.r;
                    return r3.t[2];
                }
                case DIG_4:
                {
                    const Dig4 &r4 = sd.r;
                    return r4.t[3];
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Append.
 */
extern PURE Seq _seq_append(Seq s, Seq t)
{
    Tree m[4];
    size_t m_len = 0;
    s = seq_append(s, m, m_len, t);
    return s;
}

static Seq seq_append(Seq s, Tree *m, size_t m_len, Seq t)
{
    switch (index(s))
    {
        case NIL: case SINGLE:
            for (size_t i = 0; i < m_len; i++)
                t = seq_push_front(t, m[m_len - i - 1]);
            if (index(s) == SINGLE)
            {
                const Single &ss = s;
                t = seq_push_front(t, ss.t[0]);
            }
            return t;
        case DEEP:
        {
            switch (index(t))
            {
                case NIL: case SINGLE:
                    for (size_t i = 0; i < m_len; i++)
                        s = seq_push_back(s, m[i]);
                    if (index(t) == SINGLE)
                    {
                        Single ts = (t);
                        s = seq_push_back(s, ts.t[0]);
                    }
                    return s;
                case DEEP:
                {
                    const Deep &sd = s;
                    Deep td = (t);
                    m_len = seq_append_middle(sd.r, m, m_len, td.l);
                    Seq u = seq_append(sd.m, m, m_len, td.m);
                    return deep(sd.l, u, td.r);
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

static size_t seq_append_middle(Dig s, Tree *m, size_t m_len, Dig t)
{
    Tree n[12];
    size_t n_len = 0;
    switch (index(s))
    {
        case DIG_1:
        {
            const Dig1 &s1 = s;
            n[n_len++] = s1.t[0];
            break;
        }
        case DIG_2:
        {
            const Dig2 &s2 = s;
            n[n_len++] = s2.t[0];
            n[n_len++] = s2.t[1];
            break;
        }
        case DIG_3:
        {
            const Dig3 &s3 = s;
            n[n_len++] = s3.t[0];
            n[n_len++] = s3.t[1];
            n[n_len++] = s3.t[2];
            break;
        }
        case DIG_4:
        {
            const Dig4 &s4 = s;
            n[n_len++] = s4.t[0];
            n[n_len++] = s4.t[1];
            n[n_len++] = s4.t[2];
            n[n_len++] = s4.t[3];
            break;
        }
        default:
            error_bad_tree();
    }
    for (size_t i = 0; i < m_len; i++)
        n[n_len++] = m[i];
    switch (index(t))
    {
        case DIG_1:
        {
            const Dig1 &t1 = t;
            n[n_len++] = t1.t[0];
            break;
        }
        case DIG_2:
        {
            const Dig2 &t2 = t;
            n[n_len++] = t2.t[0];
            n[n_len++] = t2.t[1];
            break;
        }
        case DIG_3:
        {
            const Dig3 &t3 = t;
            n[n_len++] = t3.t[0];
            n[n_len++] = t3.t[1];
            n[n_len++] = t3.t[2];
            break;
        }
        case DIG_4:
        {
            const Dig4 &t4 = t;
            n[n_len++] = t4.t[0];
            n[n_len++] = t4.t[1];
            n[n_len++] = t4.t[2];
            n[n_len++] = t4.t[3];
            break;
        }
        default:
            error_bad_tree();
    }
    m_len = 0;
    size_t i = 0;
    while (true)
    {
        switch (n_len)
        {
            case 2:
                m[m_len++] = tree2(n[i], n[i+1]);
                return m_len;
            case 3:
                m[m_len++] = tree3(n[i], n[i+1], n[i+2]);
                return m_len;
            case 4:
                m[m_len++] = tree2(n[i], n[i+1]);
                m[m_len++] = tree2(n[i+2], n[i+3]);
                return m_len;
            case 5:
                // Nb: This case is missing from the paper?
                m[m_len++] = tree3(n[i], n[i+1], n[i+2]);
                m[m_len++] = tree2(n[i+3], n[i+4]);
                return m_len;
            default:
                m[m_len++] = tree3(n[i], n[i+1], n[i+2]);
                n_len -= 3;
                i += 3;
                continue;
        }
    }
}

/*
 * Split.
 */
extern PURE Result<Seq, Frag, size_t, Seq> _seq_split(Seq s, size_t idx)
{
    Seq l, r;
    Tree t;
    if (idx >= _seq_length(s))
        error("split out-of-bounds");
    seq_split(s, &idx, &l, &r, &t);
    const Frag &f = t;
    return {l, f, idx, r};
}

/*
 * Split left.
 */
extern PURE Result<Seq, Frag, size_t> _seq_left(Seq s, size_t idx)
{
    Seq l;
    Tree t;
    if (idx >= _seq_length(s))
        error("left out-of-bounds");
    seq_split(s, &idx, &l, nullptr, &t);
    const Frag &f = t;
    return {l, f, idx};
}

/*
 * Split right.
 */
extern PURE Result<Frag, size_t, Seq> _seq_right(Seq s, size_t idx)
{
    Seq r;
    Tree t;
    if (idx >= _seq_length(s))
        error("right out-of-bounds");
    seq_split(s, &idx, nullptr, &r, &t);
    const Frag &f = t;
    return {f, idx, r};
}

static void seq_split(Seq s, size_t *idx, Seq *l, Seq *r, Tree *t)
{
    size_t len;
    switch (index(s))
    {
        case SINGLE:
        {
            const Single &ss = s;
            SET(l, empty());
            SET(r, empty());
            *t = ss.t[0];
            return;
        }
        case DEEP:
        {
            const Deep &sd = s;
            len = dig_length(sd.l);
            if (*idx < len)
            {
                Dig dl, dr;
                bool have_dl, have_dr;
                dig_split(sd.l, idx, (l == nullptr? nullptr: &dl),
                    (r == nullptr? nullptr: &dr), &have_dl, &have_dr, t);
                if (have_dr)
                    SET(r, deep(dr, sd.m, sd.r));
                else
                    SET(r, deep(sd.m, sd.r));
                if (have_dl)
                    SET(l, dig_to_seq(dl));
                else
                    SET(l, empty());
                return;
            }
            *idx -= len;
            len = _seq_length(sd.m);
            if (*idx < len)
            {
                Seq l1, r1;
                Tree t1;
                seq_split(sd.m, idx, (l == nullptr? nullptr: &l1),
                    (r == nullptr? nullptr: &r1), &t1);
                switch (index(t1))
                {
                    case TREE_2:
                    {
                        const Tree2 &t2 = t1;
                        len = tree_length(t2.t[0]);
                        if (*idx < len)
                        {
                            SET(l, deep(sd.l, l1));
                            SET(r, deep(dig1(t2.t[1]), r1, sd.r));
                            *t = t2.t[0];
                        }
                        else
                        {
                            *idx -= len;
                            SET(l, deep(sd.l, l1, dig1(t2.t[0])));
                            SET(r, deep(r1, sd.r));
                            *t = t2.t[1];
                        }
                        return;
                    }
                    case TREE_3:
                    {
                        const Tree3 &t3 = t1;
                        len = tree_length(t3.t[0]);
                        if (*idx < len)
                        {
                            SET(l, deep(sd.l, l1));
                            SET(r, deep(dig2(t3.t[1], t3.t[2]), r1, sd.r));
                            *t = t3.t[0];
                            return;
                        }
                        *idx -= len;
                        len = tree_length(t3.t[1]);
                        if (*idx < len)
                        {
                            SET(l, deep(sd.l, l1, dig1(t3.t[0])));
                            SET(r, deep(dig1(t3.t[2]), r1, sd.r));
                            *t = t3.t[1];
                        }
                        else
                        {
                            *idx -= len;
                            SET(l, deep(sd.l, l1, dig2(t3.t[0], t3.t[1])));
                            SET(r, deep(r1, sd.r));
                            *t = t3.t[2];
                        }
                        return;
                    }
                    default:
                        error_bad_tree();
                }
            }
            *idx -= len;
            {
                Dig dl, dr;
                bool have_dl, have_dr;
                dig_split(sd.r, idx, (l == nullptr? nullptr: &dl),
                    (r == nullptr? nullptr: &dr), &have_dl, &have_dr, t);
                if (have_dl)
                    SET(l, deep(sd.l, sd.m, dl));
                else
                    SET(l, deep(sd.l, sd.m));
                if (have_dr)
                    SET(r, dig_to_seq(dr));
                else
                    SET(r, empty());
                return;
            }
        }
        default:
            error_bad_tree();
    }
}

static Seq deep(Dig l, Seq m)
{
    if (index(m) == NIL)
        return dig_to_seq(l);
    Tree t;
    m = seq_pop_back(m, &t);
    return deep(l, m, tree_to_dig(t));
}

static Seq deep(Seq m, Dig r)
{
    if (index(m) == NIL)
        return dig_to_seq(r);
    Tree t;
    m = seq_pop_front(m, &t);
    return deep(tree_to_dig(t), m, r);
}

static void dig_split(Dig s, size_t *idx, Dig *l, Dig *r, bool *have_l,
    bool *have_r, Tree *t)
{
    size_t len;
    switch (index(s))
    {
        case DIG_1:
        {
            const Dig1 &d1 = s;
            *t = d1.t[0];
            *have_l = false;
            *have_r = false;
            return;
        }
        case DIG_2:
        {
            const Dig2 &d2 = s;
            len = tree_length(d2.t[0]);
            if (*idx < len)
            {
                *t = d2.t[0];
                SET(r, dig1(d2.t[1]));
                *have_l = false;
                *have_r = true;
            }
            else
            {
                *idx -= len;
                *t = d2.t[1];
                SET(l, dig1(d2.t[0]));
                *have_l = true;
                *have_r = false;
            }
            return;
        }
        case DIG_3:
        {
            const Dig3 &d3 = s;
            len = tree_length(d3.t[0]);
            if (*idx < len)
            {
                *t = d3.t[0];
                SET(r, dig2(d3.t[1], d3.t[2]));
                *have_l = false;
                *have_r = true;
                return;
            }
            *idx -= len;
            len = tree_length(d3.t[1]);
            if (*idx < len)
            {
                *t = d3.t[1];
                SET(l, dig1(d3.t[0]));
                SET(r, dig1(d3.t[2]));
                *have_l = true;
                *have_r = true;
            }
            else
            {
                *idx -= len;
                *t = d3.t[2];
                SET(l, dig2(d3.t[0], d3.t[1]));
                *have_l = true;
                *have_r = false;
            }
            return;
        }
        case DIG_4:
        {
            const Dig4 &d4 = s;
            len = tree_length(d4.t[0]);
            if (*idx < len)
            {
                *t = d4.t[0];
                SET(r, dig3(d4.t[1], d4.t[2], d4.t[3]));
                *have_l = false;
                *have_r = true;
                return;
            }
            *idx -= len;
            len = tree_length(d4.t[1]);
            if (*idx < len)
            {
                *t = d4.t[1];
                SET(l, dig1(d4.t[0]));
                SET(r, dig2(d4.t[2], d4.t[3]));
                *have_l = true;
                *have_r = true;
                return;
            }
            *idx -= len;
            len = tree_length(d4.t[2]);
            if (*idx < len)
            {
                *t = d4.t[2];
                SET(l, dig2(d4.t[0], d4.t[1]));
                SET(r, dig1(d4.t[3]));
                *have_l = true;
                *have_r = true;
            }
            else
            {
                *idx -= len;
                *t = d4.t[3];
                SET(l, dig3(d4.t[0], d4.t[1], d4.t[2]));
                *have_l = true;
                *have_r = false;
            }
            return;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Fold left.
 */
extern PURE Value<Word> _seq_foldl(Seq s, Value<Word> arg,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data)
{
    size_t idx = 0;
    return seq_foldl(s, arg, &idx, f, data);
}

static Value<Word> seq_foldl(Seq s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data)
{
    switch (index(s))
    {
        case NIL:
            return arg;
        case SINGLE:
        {
            const Single &ss = s;
            arg = tree_foldl(ss.t[0], arg, idx, f, data);
            return arg;
        }
        case DEEP:
        {
            const Deep &sd = s;
            arg = dig_foldl(sd.l, arg, idx, f, data);
            arg = seq_foldl(sd.m, arg, idx, f, data);
            arg = dig_foldl(sd.r, arg, idx, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

static Value<Word> dig_foldl(Dig s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data)
{
    switch (index(s))
    {
        case DIG_1:
        {
            const Dig1 &s1 = s;
            arg = tree_foldl(s1.t[0], arg, idx, f, data);
            return arg;
        }
        case DIG_2:
        {
            const Dig2 &s2 = s;
            arg = tree_foldl(s2.t[0], arg, idx, f, data);
            arg = tree_foldl(s2.t[1], arg, idx, f, data);
            return arg;
        }
        case DIG_3:
        {
            const Dig3 &s3 = s;
            arg = tree_foldl(s3.t[0], arg, idx, f, data);
            arg = tree_foldl(s3.t[1], arg, idx, f, data);
            arg = tree_foldl(s3.t[2], arg, idx, f, data);
            return arg;
        }
        case DIG_4:
        {
            const Dig4 &s4 = s;
            arg = tree_foldl(s4.t[0], arg, idx, f, data);
            arg = tree_foldl(s4.t[1], arg, idx, f, data);
            arg = tree_foldl(s4.t[2], arg, idx, f, data);
            arg = tree_foldl(s4.t[3], arg, idx, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

static Value<Word> tree_foldl(Tree s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data)
{
    switch (index(s))
    {
        case TREE_LEAF:
        {
            const Frag &sl = s;
            arg = f(data, arg, *idx, sl);
            const FragHeader &slh = sl;
            *idx += slh._len;
            return arg;
        }
        case TREE_2:
        {
            const Tree2 &s2 = s;
            arg = tree_foldl(s2.t[0], arg, idx, f, data);
            arg = tree_foldl(s2.t[1], arg, idx, f, data);
            return arg;
        }
        case TREE_3:
        {
            const Tree3 &s3 = s;
            arg = tree_foldl(s3.t[0], arg, idx, f, data);
            arg = tree_foldl(s3.t[1], arg, idx, f, data);
            arg = tree_foldl(s3.t[2], arg, idx, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Fold right.
 */
extern PURE Value<Word> _seq_foldr(Seq s, Value<Word> arg,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data)
{
    size_t idx = _seq_length(s);
    return seq_foldr(s, arg, &idx, f, data);
}

static Value<Word> seq_foldr(Seq s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data)
{
    switch (index(s))
    {
        case NIL:
            return arg;
        case SINGLE:
        {
            const Single &ss = s;
            arg = tree_foldr(ss.t[0], arg, idx, f, data);
            return arg;
        }
        case DEEP:
        {
            const Deep &sd = s;
            arg = dig_foldr(sd.r, arg, idx, f, data);
            arg = seq_foldr(sd.m, arg, idx, f, data);
            arg = dig_foldr(sd.l, arg, idx, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

static Value<Word> dig_foldr(Dig s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data)
{
    switch (index(s))
    {
        case DIG_1:
        {
            const Dig1 &s1 = s;
            arg = tree_foldr(s1.t[0], arg, idx, f, data);
            return arg;
        }
        case DIG_2:
        {
            const Dig2 &s2 = s;
            arg = tree_foldr(s2.t[1], arg, idx, f, data);
            arg = tree_foldr(s2.t[0], arg, idx, f, data);
            return arg;
        }
        case DIG_3:
        {
            const Dig3 &s3 = s;
            arg = tree_foldr(s3.t[2], arg, idx, f, data);
            arg = tree_foldr(s3.t[1], arg, idx, f, data);
            arg = tree_foldr(s3.t[0], arg, idx, f, data);
            return arg;
        }
        case DIG_4:
        {
            const Dig4 &s4 = s;
            arg = tree_foldr(s4.t[3], arg, idx, f, data);
            arg = tree_foldr(s4.t[2], arg, idx, f, data);
            arg = tree_foldr(s4.t[1], arg, idx, f, data);
            arg = tree_foldr(s4.t[0], arg, idx, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

static Value<Word> tree_foldr(Tree s, Value<Word> arg, size_t *idx,
    Value<Word> (*f)(void *, Value<Word>, size_t, Frag), void *data)
{
    switch (index(s))
    {
        case TREE_LEAF:
        {
            const Frag &sl = s;
            const FragHeader &slh = sl;
            *idx -= slh._len;
            arg = f(data, arg, *idx, sl);
            return arg;
        }
        case TREE_2:
        {
            const Tree2 &s2 = s;
            arg = tree_foldr(s2.t[1], arg, idx, f, data);
            arg = tree_foldr(s2.t[0], arg, idx, f, data);
            return arg;
        }
        case TREE_3:
        {
            const Tree3 &s3 = s;
            arg = tree_foldr(s3.t[2], arg, idx, f, data);
            arg = tree_foldr(s3.t[1], arg, idx, f, data);
            arg = tree_foldr(s3.t[0], arg, idx, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Map.
 */
extern PURE Seq _seq_map(Seq s, Frag (*f)(void *, size_t, Frag), void *data)
{
    size_t idx = 0;
    return seq_map(s, &idx, f, data);
}

static Seq seq_map(Seq s, size_t *idx, Frag (*f)(void *, size_t, Frag),
    void *data)
{
    switch (index(s))
    {
        case NIL:
            return s;
        case SINGLE:
        {
            const Single &ss = s;
            Tree t = tree_map(ss.t[0], idx, f, data);
            return single(t);
        }
        case DEEP:
        {
            const Deep &sd = s;
            Dig l = dig_map(sd.l, idx, f, data);
            Seq m = seq_map(sd.m, idx, f, data);
            Dig r = dig_map(sd.r, idx, f, data);
            return deep(l, m, r);
        }
        default:
            error_bad_tree();
    }
}

static Dig dig_map(Dig d, size_t *idx, Frag (*f)(void *, size_t, Frag),
    void *data)
{
    switch (index(d))
    {
        case DIG_1:
        {
            const Dig1 &d1 = d;
            Tree t0 = tree_map(d1.t[0], idx, f, data);
            return dig1(t0);
        }
        case DIG_2:
        {
            const Dig2 &d2 = d;
            Tree t0 = tree_map(d2.t[0], idx, f, data);
            Tree t1 = tree_map(d2.t[1], idx, f, data);
            return dig2(t0, t1);
        }
        case DIG_3:
        {
            const Dig3 &d3 = d;
            Tree t0 = tree_map(d3.t[0], idx, f, data);
            Tree t1 = tree_map(d3.t[1], idx, f, data);
            Tree t2 = tree_map(d3.t[2], idx, f, data);
            return dig3(t0, t1, t2);
        }
        case DIG_4:
        {
            const Dig4 &d4 = d;
            Tree t0 = tree_map(d4.t[0], idx, f, data);
            Tree t1 = tree_map(d4.t[1], idx, f, data);
            Tree t2 = tree_map(d4.t[2], idx, f, data);
            Tree t3 = tree_map(d4.t[3], idx, f, data);
            return dig4(t0, t1, t2, t3);
        }
        default:
            error_bad_tree();
    }
}

static Tree tree_map(Tree t, size_t *idx, Frag (*f)(void *, size_t, Frag),
    void *data)
{
    switch (index(t))
    {
        case TREE_LEAF:
        {
            const Frag &tl = t;
            Frag tl1 = f(data, *idx, tl);
            const FragHeader &tlh = tl;
            *idx += tlh._len;
            return tl1;
        }
        case TREE_2:
        {
            const Tree2 &t2 = (t);
            Tree t0 = tree_map(t2.t[0], idx, f, data);
            Tree t1 = tree_map(t2.t[1], idx, f, data);
            return tree2(t0, t1);
        }
        case TREE_3:
        {
            const Tree3 &t3 = (t);
            Tree t0 = tree_map(t3.t[0], idx, f, data);
            Tree t1 = tree_map(t3.t[1], idx, f, data);
            Tree t2 = tree_map(t3.t[2], idx, f, data);
            return tree3(t0, t1, t2);
        }
        default:
            error_bad_tree();
    }
}

/*
 * Search.
 */
extern PURE Value<Word> _seq_search_left(Seq s, void *data, Value<Word> state,
    Value<Word> (*next)(void *, Frag, Value<Word>), bool (*stop)(Value<Word>))
{
    switch (index(s))
    {
        case NIL:
            return state;
        case SINGLE:
        {
            const Single &ss = s;
            return tree_search_left(ss.t[0], data, state, next, stop);
        }
        case DEEP:
        {
            const Deep &sd = s;
            state = dig_search_left(sd.l, data, state, next, stop);
            if (stop(state))
                return state;
            state = _seq_search_left(sd.m, data, state, next, stop);
            if (stop(state))
                return state;
            state = dig_search_left(sd.r, data, state, next, stop);
            return state;
        }
        default:
            error_bad_tree();
    }
}

static PURE Value<Word> dig_search_left(Dig s, void *data, Value<Word> state,
    Value<Word> (*next)(void *, Frag, Value<Word>), bool (*stop)(Value<Word>))
{
    switch (index(s))
    {
        case DIG_1:
        {
            const Dig1 &s1 = s;
            return tree_search_left(s1.t[0], data, state, next, stop);
        }
        case DIG_2:
        {
            const Dig2 &s2 = s;
            state = tree_search_left(s2.t[0], data, state, next, stop);
            if (stop(state))
                return state;
            state = tree_search_left(s2.t[1], data, state, next, stop);
            return state;
        }
        case DIG_3:
        {
            const Dig3 &s3 = s;
            state = tree_search_left(s3.t[0], data, state, next, stop);
            if (stop(state))
                return state;
            state = tree_search_left(s3.t[1], data, state, next, stop);
            if (stop(state))
                return state;
            state = tree_search_left(s3.t[2], data, state, next, stop);
            return state;
        }
        case DIG_4:
        {
            const Dig4 &s4 = s;
            state = tree_search_left(s4.t[0], data, state, next, stop);
            if (stop(state))
                return state;
            state = tree_search_left(s4.t[1], data, state, next, stop);
            if (stop(state))
                return state;
            state = tree_search_left(s4.t[2], data, state, next, stop);
            if (stop(state))
                return state;
            state = tree_search_left(s4.t[3], data, state, next, stop);
            return state;
        }
        default:
            error_bad_tree();
    }
}

static PURE Value<Word> tree_search_left(Tree s, void *data, Value<Word> state,
    Value<Word> (*next)(void *, Frag, Value<Word>), bool (*stop)(Value<Word>))
{
    switch (index(s))
    {
        case TREE_LEAF:
        {
            const Frag &sl = s;
            return next(data, sl, state);
        }
        case TREE_2:
        {
            const Tree2 &s2 = s;
            state = tree_search_left(s2.t[0], data, state, next, stop);
            if (stop(state))
                return state;
            state = tree_search_left(s2.t[1], data, state, next, stop);
            return state;
        }
        case TREE_3:
        {
            const Tree3 &s3 = s;
            state = tree_search_left(s3.t[0], data, state, next, stop);
            if (stop(state))
                return state;
            state = tree_search_left(s3.t[1], data, state, next, stop);
            if (stop(state))
                return state;
            state = tree_search_left(s3.t[2], data, state, next, stop);
            return state;
        }
        default:
            error_bad_tree();
    }
}

#define ENTRY_NIL       0
#define ENTRY_TREE      1
#define ENTRY_DIG       2
#define ENTRY_SEQ       3
typedef struct _SeqItr *Itr;
typedef struct _SeqItrEntry Entry;

static inline PURE Entry entry_seq(Seq s, size_t offset)
{
    Entry e;
    e._type = ENTRY_SEQ;
    e._offset = offset;
    e._value = _bit_cast<Value<Word>>(s);
    return e;
}

static inline PURE Entry entry_dig(Dig d, size_t offset)
{
    Entry e;
    e._type = ENTRY_DIG;
    e._offset = offset;
    e._value = _bit_cast<Value<Word>>(d);
    return e;
}

static inline PURE Entry entry_tree(Tree t, size_t offset)
{
    Entry e;
    e._type = ENTRY_TREE;
    e._offset = offset;
    e._value = _bit_cast<Value<Word>>(t);
    return e;
}

static void seq_itr_move(_SeqItr *itr, size_t idx)
{
    if (itr->_ptr == 0)
    {
        Seq s = _bit_cast<Seq>(itr->_state);
        size_t depth = seq_depth(s);
        if (depth > UINT8_MAX)
            error_bad_tree();
        Entry *stack = (Entry *)gc_malloc(sizeof(Entry) * depth);
        stack[0] = entry_seq(s, 0);
        itr->_state = _bit_cast<Value<Word>>(stack);
        itr->_ptr = 1;
        return;
    }

    Entry *stack = _bit_cast<Entry *>(itr->_state);
    while (true)
    {
        if (itr->_ptr == 1)
            break;
        Entry *entry = stack + itr->_ptr - 1;
        size_t lo = entry->_offset, hi;
        if (idx < lo)
        {
            itr->_ptr--;
            continue;
        }
        switch (entry->_type)
        {
            case ENTRY_SEQ:
            {
                Seq s = _bit_cast<Seq>(entry->_value);
                hi = lo + _seq_length(s);
                break;
            }
            case ENTRY_DIG:
            {
                Dig d = _bit_cast<Dig>(entry->_value);
                hi = lo + dig_length(d);
                break;
            }
            case ENTRY_TREE:
            {
                Tree t = _bit_cast<Tree>(entry->_value);
                hi = lo + tree_length(t);
                break;
            }
            default:
                error_bad_tree();
        }
        if (idx < hi)
            break;
        itr->_ptr--;
    }
}

static Optional<Frag> seq_itr_get(_SeqItr *itr)
{
    size_t idx = itr->_idx;
    Entry *stack = _bit_cast<Entry *>(itr->_state);
    while (true)
    {
        Entry *entry = stack + itr->_ptr - 1;
        size_t lo = entry->_offset, hi;
        switch (entry->_type)
        {
            case ENTRY_SEQ:
            {
                Seq s = _bit_cast<Seq>(entry->_value);
                switch (index(s))
                {
                    case NIL:
                        return Optional<Frag>();
                    case SINGLE:
                    {
                        const Single &ss = s;
                        stack[itr->_ptr++] = entry_tree(ss.t[0], lo);
                        continue;
                    }
                    case DEEP:
                    {
                        const Deep &sd = s;
                        hi = lo + dig_length(sd.l);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_dig(sd.l, lo);
                            continue;
                        }
                        lo = hi;
                        hi = lo + _seq_length(sd.m);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_seq(sd.m, lo);
                            continue;
                        }
                        lo = hi;
                        stack[itr->_ptr++] = entry_dig(sd.r, lo);
                        continue;
                    }
                    default:
                        error_bad_tree();
                }
            }
            case ENTRY_DIG:
            {
                Dig d = _bit_cast<Dig>(entry->_value);
                switch (index(d))
                {
                    case DIG_1:
                    {
                        const Dig1 &d1 = d;
                        stack[itr->_ptr++] = entry_tree(d1.t[0], lo);
                        continue;
                    }
                    case DIG_2:
                    {
                        const Dig2 &d2 = d;
                        hi = lo + tree_length(d2.t[0]);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_tree(d2.t[0], lo);
                            continue;
                        }
                        lo = hi;
                        stack[itr->_ptr++] = entry_tree(d2.t[1], lo);
                        continue;
                    }
                    case DIG_3:
                    {
                        const Dig3 &d3 = d;
                        hi = lo + tree_length(d3.t[0]);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_tree(d3.t[0], lo);
                            continue;
                        }
                        lo = hi;
                        hi = lo + tree_length(d3.t[1]);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_tree(d3.t[1], lo);
                            continue;
                        }
                        lo = hi;
                        stack[itr->_ptr++] = entry_tree(d3.t[2], lo);
                        continue;
                    }
                    case DIG_4:
                    {
                        const Dig4 &d4 = d;
                        hi = lo + tree_length(d4.t[0]);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_tree(d4.t[0], lo);
                            continue;
                        }
                        lo = hi;
                        hi = lo + tree_length(d4.t[1]);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_tree(d4.t[1], lo);
                            continue;
                        }
                        lo = hi;
                        hi = lo + tree_length(d4.t[2]);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_tree(d4.t[2], lo);
                            continue;
                        }
                        lo = hi;
                        stack[itr->_ptr++] = entry_tree(d4.t[3], lo);
                        continue;
                    }
                    default:
                        error_bad_tree();
                }
            }
            case ENTRY_TREE:
            {
                Tree t = _bit_cast<Tree>(entry->_value);
                switch (index(t))
                {
                    case TREE_LEAF:
                    {
                        const Frag &f0 = t;
                        Optional<Frag> f = f0;
                        return f;
                    }
                    case TREE_2:
                    {
                        const Tree2 &t2 = t;
                        hi = lo + tree_length(t2.t[0]);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_tree(t2.t[0], lo);
                            continue;
                        }
                        lo = hi;
                        stack[itr->_ptr++] = entry_tree(t2.t[1], lo);
                        continue;
                    }
                    case TREE_3:
                    {
                        const Tree3 &t3 = t;
                        hi = lo + tree_length(t3.t[0]);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_tree(t3.t[0], lo);
                            continue;
                        }
                        lo = hi;
                        hi = lo + tree_length(t3.t[1]);
                        if (idx < hi)
                        {
                            stack[itr->_ptr++] = entry_tree(t3.t[1], lo);
                            continue;
                        }
                        lo = hi;
                        stack[itr->_ptr++] = entry_tree(t3.t[2], lo);
                        continue;
                    }
                    default:
                        error_bad_tree();
                }
            }
            default:
                error_bad_tree();
        }
    }
}

/*
 * Compare.
 */
#define MIN(a, b)       ((a) < (b)? (a): (b))
extern PURE int _seq_compare(Seq s, Seq t, void *data,
    int (*val_compare)(void *, Frag, size_t, Frag, size_t))
{
    if (s == t)
        return 0;

    _SeqItr itr_s = begin(s), itr_t = begin(t);
    _SeqItr itr_se = end(s),  itr_te = end(t);
    Frag ls;
    Frag lt;
    size_t lens = 0, lent = 0, is = 0, it = 0;
    while (true)
    {
        if (lens - is == 0)
        {
            if (itr_s == itr_se)
                break;
            ls = *itr_s;
            const Frag &fs = ls;
            const FragHeader &lh = fs;
            lens = lh._len;
            itr_s += lens;
            is = 0;
        }
        if (lent - it == 0)
        {
            if (itr_t == itr_te)
                return -1;
            lt = *itr_t;
            const Frag &ft = lt;
            const FragHeader &lh = ft;
            lent = lh._len;
            itr_t += lent;
            it = 0;
        }
        int cmp = val_compare(data, ls, is, lt, it);
        if (cmp != 0)
            return cmp;
        size_t len = MIN(lens - is, lent - it);
        is += len;
        it += len;
    }
    if (lent - it == 0)
    {
        if (itr_t == itr_te)
            return 0;
        else
            return 1;
    }
    return 1;
}

extern void _seq_itr_begin(_SeqItr *itr, Seq s)
{
    itr->_idx    = 0;
    itr->_ptr    = 0;
    itr->_state  = _bit_cast<Value<Word>>(s);
}

extern void _seq_itr_end(_SeqItr *itr, Seq s)
{
    itr->_idx    = _seq_length(s);
    itr->_ptr    = 0;
    itr->_state  = _bit_cast<Value<Word>>(s);
}

extern Frag _seq_itr_get(_SeqItr *itr, size_t *idx_ptr)
{
    seq_itr_move(itr, itr->_idx);
    Optional<Frag> f = seq_itr_get(itr);
    if (empty(f))
        error("iterator out-of-bounds access");
    if (idx_ptr != nullptr)
    {
        Entry *stack = _bit_cast<Entry *>(itr->_state);
        *idx_ptr = itr->_idx - stack[itr->_ptr-1]._offset;
    }
    return f;
}

}

