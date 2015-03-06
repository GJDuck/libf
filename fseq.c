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

typedef _Frag frag_t;
typedef _frag_s frag_s;
typedef _Seq seq_t;
#define SEQ_EMPTY           _SEQ_EMPTY

struct tree2_s;
struct tree3_s;

typedef Multi<frag_s, tree2_s, tree3_s> tree_t;

struct tree2_s
{
    size_t len;
    tree_t t[2];
};
struct tree3_s
{
    size_t len;
    tree_t t[3];
};
typedef struct tree2_s *tree2_t;
typedef struct tree3_s *tree3_t;

enum tree_type_t
{
    TREE_LEAF = type_index<frag_s, tree_t>(),
    TREE_2    = type_index<tree2_s, tree_t>(),
    TREE_3    = type_index<tree3_s, tree_t>(),
};

struct dig1_s
{
    size_t len;
    tree_t t[1];
};
struct dig2_s
{
    size_t len;
    tree_t t[2];
};
struct dig3_s
{
    size_t len;
    tree_t t[3];
};
struct dig4_s
{
    size_t len;
    tree_t t[4];
};
typedef Multi<dig1_s, dig2_s, dig3_s, dig4_s> dig_t;
typedef struct dig1_s *dig1_t;
typedef struct dig2_s *dig2_t;
typedef struct dig3_s *dig3_t;
typedef struct dig4_s *dig4_t;

enum dig_type_t
{
    DIG_1 = type_index<dig1_s, dig_t>(),
    DIG_2 = type_index<dig2_s, dig_t>(),
    DIG_3 = type_index<dig3_s, dig_t>(),
    DIG_4 = type_index<dig4_s, dig_t>(),
};

struct seq_nil_s;
struct seq_single_s
{
    size_t len;
    tree_t t[1];
};
struct seq_deep_s
{
    size_t len;
    dig_t l;
    seq_t m;
    dig_t r;
};
typedef struct seq_nil_s *seq_nil_t;
typedef struct seq_single_s *seq_single_t;
typedef struct seq_deep_s *seq_deep_t;

enum seq_type_t
{
    SEQ_NIL    = type_index<seq_nil_s, seq_t>(),
    SEQ_SINGLE = type_index<seq_single_s, seq_t>(),
    SEQ_DEEP   = type_index<seq_deep_s, seq_t>()
};

// typedef void (*print_t)(Any a, size_t len);

static size_t dig_length(dig_t s);
static size_t tree_length(tree_t s);
static Result<frag_t, size_t> dig_lookup(dig_t s, size_t idx);
static Result<frag_t, size_t> tree_lookup(tree_t s, size_t idx);
static seq_t seq_push_front(seq_t s, tree_t t);
static seq_t seq_pop_front(seq_t s, tree_t *t);
static seq_t seq_push_back(seq_t s, tree_t t);
static seq_t seq_pop_back(seq_t s, tree_t *t);
static seq_t seq_append(seq_t s, tree_t *m, size_t m_len, seq_t t);
static size_t seq_append_middle(dig_t s, tree_t *m, size_t m_len, dig_t t);
static void seq_split(seq_t s, size_t *idx, seq_t *l, seq_t *r, tree_t *t);
static seq_t seq_deep_r(dig_t l, seq_t m);
static seq_t seq_deep_l(seq_t m, dig_t r);
static void dig_split(dig_t s, size_t *idx, dig_t *l, dig_t *r, bool *have_l,
    bool *have_r, tree_t *t);
static Any dig_foldl(dig_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data);
static Any tree_foldl(tree_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data);
static Any dig_foldr(dig_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data);
static Any tree_foldr(tree_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data);
static dig_t dig_map(dig_t d, frag_t (*f)(void *, frag_t), void *data);
static tree_t tree_map(tree_t t, frag_t (*f)(void *, frag_t), void *data);
static bool seq_verify(seq_t s, size_t level);
static bool dig_verify(dig_t s, size_t level);
static bool tree_verify(tree_t s, size_t level);

/*
 * Node constructors.
 */
static seq_t empty(void)
{
    seq_nil_t node = (seq_nil_t)nullptr;
    return set<seq_t>(node);
}
static seq_t single(tree_t t0)
{
    size_t len = tree_length(t0);
    seq_single_s node = {len, {t0}};
    seq_t t = set<seq_t>(box(node));
    return t;
}
static seq_t deep(dig_t l, seq_t m, dig_t r)
{
    size_t len = dig_length(l) + _seq_length(m) + dig_length(r);
    seq_deep_s node = {len, l, m, r};
    seq_t t = set<seq_t>(box(node));
    return t;
}

static dig_t dig1(tree_t t0)
{
    size_t len = tree_length(t0);
    dig1_s node = {len, {t0}};
    dig_t t = set<dig_t>(box(node));
    return t;
}
static dig_t dig2(tree_t t0, tree_t t1)
{
    size_t len = tree_length(t0) + tree_length(t1);
    dig2_s node = {len, {t0, t1}};
    dig_t t = set<dig_t>(box(node));
    return t;
}
static dig_t dig3(tree_t t0, tree_t t1, tree_t t2)
{
    size_t len = tree_length(t0) + tree_length(t1) + tree_length(t2);
    dig3_s node = {len, {t0, t1, t2}};
    dig_t t = set<dig_t>(box(node));
    return t;
}
static dig_t dig4(tree_t t0, tree_t t1, tree_t t2, tree_t t3)
{
    size_t len = tree_length(t0) + tree_length(t1) + tree_length(t2) + 
        tree_length(t3);
    dig4_s node = {len, {t0, t1, t2, t3}};
    dig_t t = set<dig_t>(box(node));
    return t;
}

static tree_t tree2(tree_t t0, tree_t t1)
{
    size_t len = tree_length(t0) + tree_length(t1);
    tree2_s node = {len, {t0, t1}};
    tree_t t = set<tree_t>(box(node));
    return t;
}
static tree_t tree3(tree_t t0, tree_t t1, tree_t t2)
{
    size_t len = tree_length(t0) + tree_length(t1) + tree_length(t2);
    tree3_s node = {len, {t0, t1, t2}};
    tree_t t = set<tree_t>(box(node));
    return t;
}

/*
 * Conversion.
 */
static dig_t tree_to_dig(tree_t t)
{
    switch (index(t))
    {
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            dig2_t d2 = cast<dig2_t>(t2);
            return set<dig_t>(d2);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            dig3_t d3 = cast<dig3_t>(t3);
            return set<dig_t>(d3);
        }
        default:
            error_bad_tree();
    }
}

static seq_t dig_to_seq(dig_t d)
{
    switch (index(d))
    {
        case DIG_1:
        {
            dig1_t d1 = get<dig1_s>(d);
            seq_single_t ss = cast<seq_single_t>(d1);
            return set<seq_t>(ss);
        }
        case DIG_2:
        {
            dig2_t d2 = get<dig2_s>(d);
            return deep(dig1(d2->t[0]), empty(), dig1(d2->t[1]));
        }
        case DIG_3:
        {
            dig3_t d3 = get<dig3_s>(d);
            return deep(dig2(d3->t[0], d3->t[1]), empty(), dig1(d3->t[2]));
        }
        case DIG_4:
        {
            dig4_t d4 = get<dig4_s>(d);
            return deep(dig2(d4->t[0], d4->t[1]), empty(),
                        dig2(d4->t[2], d4->t[2]));
        }
        default:
            error_bad_tree();
    }
}

/*
 * Is empty.
 */
extern PURE bool _seq_is_empty(seq_t s)
{
    return (index(s) == SEQ_NIL);
}

/*
 * Length.
 */
extern PURE size_t _seq_length(seq_t s)
{
    switch (index(s))
    {
        case SEQ_NIL:
            return 0;
        case SEQ_SINGLE:
            return (get<seq_single_s>(s))->len;
        case SEQ_DEEP:
            return (get<seq_deep_s>(s))->len;
        default:
            error_bad_tree();
    }
}

static size_t dig_length(dig_t s)
{
    switch (index(s))
    {
        case DIG_1:
            return (get<dig1_s>(s))->len;
        case DIG_2:
            return (get<dig2_s>(s))->len;
        case DIG_3:
            return (get<dig3_s>(s))->len;
        case DIG_4:
            return (get<dig4_s>(s))->len;
        default:
            error_bad_tree();
    }
}

static size_t tree_length(tree_t s)
{
    switch (index(s))
    {
        case TREE_LEAF:
            return (get<frag_s>(s))->len;
        case TREE_2:
            return (get<tree2_s>(s))->len;
        case TREE_3:
            return (get<tree3_s>(s))->len;
        default:
            error_bad_tree();
    }
}

/*
 * Verify.
 */
extern PURE bool _seq_verify(seq_t s)
{
    return seq_verify(s, 0);
}

static bool seq_verify(seq_t s, size_t level)
{
    switch (index(s))
    {
        case SEQ_NIL:
            return true;
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            return tree_verify(ss->t[0], level);
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            return dig_verify(sd->l, level) &&
                   seq_verify(sd->m, level+1) &&
                   dig_verify(sd->r, level);
        }
        default:
            return false;
    }
}

static bool dig_verify(dig_t s, size_t level)
{
    switch (index(s))
    {
        case DIG_1:
        {
            dig1_t s1 = get<dig1_s>(s);
            return tree_verify(s1->t[0], level);
        }
        case DIG_2:
        {
            dig2_t s2 = get<dig2_s>(s);
            return tree_verify(s2->t[0], level) &&
                   tree_verify(s2->t[1], level);
        }
        case DIG_3:
        {
            dig3_t s3 = get<dig3_s>(s);
            return tree_verify(s3->t[0], level) &&
                   tree_verify(s3->t[1], level) &&
                   tree_verify(s3->t[2], level);
        }
        case DIG_4:
        {
            dig4_t s4 = get<dig4_s>(s);
            return tree_verify(s4->t[0], level) &&
                   tree_verify(s4->t[1], level) &&
                   tree_verify(s4->t[2], level) &&
                   tree_verify(s4->t[3], level);
        }
        default:
            return false;
    }
}

static bool tree_verify(tree_t s, size_t level)
{
    switch (index(s))
    {
        case TREE_LEAF:
        {
            frag_t sl = get<frag_s>(s);
            if (sl->len == 0)
                return false;
            return (level == 0);
        }
        case TREE_2:
        {
            tree2_t s2 = get<tree2_s>(s);
            return tree_verify(s2->t[0], level-1) &&
                   tree_verify(s2->t[1], level-1);
        }
        case TREE_3:
        {
            tree3_t s3 = get<tree3_s>(s);
            return tree_verify(s3->t[0], level-1) &&
                   tree_verify(s3->t[1], level-1) &&
                   tree_verify(s3->t[2], level-1);
        }
        default:
            return false;
    }
}

/*
 * Lookup.
 */
extern PURE Result<frag_t, size_t> _seq_lookup(seq_t s, size_t idx)
{
    size_t len;
    while (true)
    {
        switch (index(s))
        {
            case SEQ_NIL:
                error("seq lookup out-of-range", ERANGE);
            case SEQ_SINGLE:
            {
                seq_single_t ss = get<seq_single_s>(s);
                if (idx < ss->len)
                    return tree_lookup(ss->t[0], idx);
                error("seq lookup out-of-range", ERANGE);
            }
            case SEQ_DEEP:
            {
                seq_deep_t sd = get<seq_deep_s>(s);
                len = dig_length(sd->l);
                if (idx < len)
                    return dig_lookup(sd->l, idx);
                idx -= len;
                len = _seq_length(sd->m);
                if (idx < len)
                {
                    s = sd->m;
                    continue;
                }
                idx -= len;
                return dig_lookup(sd->r, idx);
            }
            default:
                error_bad_tree();
        }
    }
}

static Result<frag_t, size_t> dig_lookup(dig_t s, size_t idx)
{
    size_t len;
    switch (index(s))
    {
        case DIG_1:
        {
            dig1_t s1 = get<dig1_s>(s);
            return tree_lookup(s1->t[0], idx);
        }
        case DIG_2:
        {
            dig2_t s2 = get<dig2_s>(s);
            len = tree_length(s2->t[0]);
            if (idx < len)
                return tree_lookup(s2->t[0], idx);
            idx -= len;
            return tree_lookup(s2->t[1], idx);
        }
        case DIG_3:
        {
            dig3_t s3 = get<dig3_s>(s);
            len = tree_length(s3->t[0]);
            if (idx < len)
                return tree_lookup(s3->t[0], idx);
            idx -= len;
            len = tree_length(s3->t[1]);
            if (idx < len)
                return tree_lookup(s3->t[1], idx);
            idx -= len;
            return tree_lookup(s3->t[2], idx);
        }
        case DIG_4:
        {
            dig4_t s4 = get<dig4_s>(s);
            len = tree_length(s4->t[0]);
            if (idx < len)
                return tree_lookup(s4->t[0], idx);
            idx -= len;
            len = tree_length(s4->t[1]);
            if (idx < len)
                return tree_lookup(s4->t[1], idx);
            idx -= len;
            len = tree_length(s4->t[2]);
            if (idx < len)
                return tree_lookup(s4->t[2], idx);
            idx -= len;
            return tree_lookup(s4->t[3], idx);
        }
        default:
            error_bad_tree();
    }
}

static Result<frag_t, size_t> tree_lookup(tree_t s, size_t idx)
{
    size_t len;
    while (true)
    {
        switch (index(s))
        {
            case TREE_LEAF:
            {
                frag_t sl = get<frag_s>(s);
                if (idx < sl->len)
                    return {sl, idx};
                error("seq lookup out-of-range", ERANGE);
            }
            case TREE_2:
            {
                tree2_t s2 = get<tree2_s>(s);
                len = tree_length(s2->t[0]);
                if (idx < len)
                {
                    s = s2->t[0];
                    continue;
                }
                idx -= len;
                s = s2->t[1];
                continue;
            }
            case TREE_3:
            {
                tree3_t s3 = get<tree3_s>(s);
                len = tree_length(s3->t[0]);
                if (idx < len)
                {
                    s = s3->t[0];
                    continue;
                }
                idx -= len;
                len = tree_length(s3->t[1]);
                if (idx < len)
                {
                    s = s3->t[1];
                    continue;
                }
                idx -= len;
                s = s3->t[2];
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
extern PURE seq_t _seq_push_front(seq_t s, frag_t f)
{
    return seq_push_front(s, set<tree_t>(f));
}

static seq_t seq_push_front(seq_t s, tree_t t)
{
    switch (index(s))
    {
        case SEQ_NIL:
            return single(t);
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            return deep(dig1(t), empty(), dig1(ss->t[0]));
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            switch (index(sd->l))
            {
                case DIG_1:
                {
                    dig1_t s1 = get<dig1_s>(sd->l);
                    return deep(dig2(t, s1->t[0]), sd->m, sd->r);
                }
                case DIG_2:
                {
                    dig2_t s2 = get<dig2_s>(sd->l);
                    return deep(dig3(t, s2->t[0], s2->t[1]), sd->m, sd->r);
                }
                case DIG_3:
                {
                    dig3_t s3 = get<dig3_s>(sd->l);
                    return deep(dig4(t, s3->t[0], s3->t[1], s3->t[2]),
                        sd->m, sd->r);
                }
                case DIG_4:
                {
                    dig4_t s4 = get<dig4_s>(sd->l);
                    if (index(sd->m) == SEQ_NIL && index(sd->r) == DIG_1)
                    {
                        dig1_t r1 = get<dig1_s>(sd->r);
                        return deep(dig3(t, s4->t[0], s4->t[1]), empty(),
                            dig3(s4->t[2], s4->t[3], r1->t[0]));
                    }
                    tree_t nt = tree3(s4->t[1], s4->t[2], s4->t[3]);
                    seq_t m = seq_push_front(sd->m, nt);
                    return deep(dig2(t, s4->t[0]), m, sd->r);
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
extern PURE Result<seq_t, frag_t> _seq_pop_front(seq_t s)
{
    tree_t t;
    s = seq_pop_front(s, &t);
    frag_t tl = get<frag_s>(t);
    return {s, tl};
}

static seq_t seq_pop_front(seq_t s, tree_t *t)
{
    switch (index(s))
    {
        case SEQ_NIL:
            error("pop-front empty");
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            *t = ss->t[0];
            return empty();
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            switch (index(sd->l))
            {
                case DIG_1:
                {
                    dig1_t s1 = get<dig1_s>(sd->l);
                    *t = s1->t[0];
                    return seq_deep_l(sd->m, sd->r);
                }
                case DIG_2:
                {
                    dig2_t s2 = get<dig2_s>(sd->l);
                    *t = s2->t[0];
                    return deep(dig1(s2->t[1]), sd->m, sd->r);
                }
                case DIG_3:
                {
                    dig3_t s3 = get<dig3_s>(sd->l);
                    *t = s3->t[0];
                    return deep(dig2(s3->t[1], s3->t[2]), sd->m, sd->r);
                }
                case DIG_4:
                {
                    dig4_t s4 = get<dig4_s>(sd->l);
                    *t = s4->t[0];
                    return deep(dig3(s4->t[1], s4->t[2], s4->t[3]), sd->m,
                        sd->r);
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
extern PURE seq_t _seq_replace_front(seq_t s, frag_t f)
{
    tree_t l = set<tree_t>(f);
    switch (index(s))
    {
        case SEQ_NIL:
            error("replace-back empty");
        case SEQ_SINGLE:
            return single(l);
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            switch (index(sd->l))
            {
                case DIG_1:
                    return deep(dig1(l), sd->m, sd->r);
                case DIG_2:
                {
                    dig2_t s2 = get<dig2_s>(sd->l);
                    return deep(dig2(l, s2->t[1]), sd->m, sd->r);
                }
                case DIG_3:
                {
                    dig3_t s3 = get<dig3_s>(sd->l);
                    return deep(dig3(l, s3->t[1], s3->t[2]), sd->m, sd->r);
                }
                case DIG_4:
                {
                    dig4_t s4 = get<dig4_s>(sd->l);
                    return deep(dig4(l, s4->t[1], s4->t[2], s4->t[3]), sd->m,
                        sd->r);
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
extern PURE frag_t _seq_peek_front(seq_t s)
{
    switch (index(s))
    {
        case SEQ_NIL:
            error("peek-front empty");
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            frag_t sl = get<frag_s>(ss->t[0]);
            return sl;
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            switch (index(sd->l))
            {
                case DIG_1:
                {
                    dig1_t r1 = get<dig1_s>(sd->l);
                    frag_t sl = get<frag_s>(r1->t[0]);
                    return sl;
                }
                case DIG_2:
                {
                    dig2_t r2 = get<dig2_s>(sd->l);
                    frag_t sl = get<frag_s>(r2->t[0]);
                    return sl;
                }
                case DIG_3:
                {
                    dig3_t r3 = get<dig3_s>(sd->l);
                    frag_t sl = get<frag_s>(r3->t[0]);
                    return sl;
                }
                case DIG_4:
                {
                    dig4_t r4 = get<dig4_s>(sd->l);
                    frag_t sl = get<frag_s>(r4->t[0]);
                    return sl;
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
extern PURE seq_t _seq_push_back(seq_t s, frag_t f)
{
    tree_t t = set<tree_t>(f);
    return seq_push_back(s, t);
}

static seq_t seq_push_back(seq_t s, tree_t t)
{
    switch (index(s))
    {
        case SEQ_NIL:
            return single(t);
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            return deep(dig1(ss->t[0]), empty(), dig1(t));
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            switch (index(sd->r))
            {
                case DIG_1:
                {
                    dig1_t s1 = get<dig1_s>(sd->r);
                    return deep(sd->l, sd->m, dig2(s1->t[0], t));
                }
                case DIG_2:
                {
                    dig2_t s2 = get<dig2_s>(sd->r);
                    return deep(sd->l, sd->m, dig3(s2->t[0], s2->t[1], t));
                }
                case DIG_3:
                {
                    dig3_t s3 = get<dig3_s>(sd->r);
                    return deep(sd->l, sd->m, dig4(s3->t[0], s3->t[1],
                        s3->t[2], t));
                }
                case DIG_4:
                {
                    dig4_t s4 = get<dig4_s>(sd->r);
                    if (index(sd->m) == SEQ_NIL && index(sd->l) == DIG_1)
                    {
                        dig1_t l1 = get<dig1_s>(sd->l);
                        return deep(dig3(l1->t[0], s4->t[0], s4->t[1]),
                            empty(), dig3(s4->t[2], s4->t[3], t));
                    }
                    tree_t nt = tree3(s4->t[0], s4->t[1], s4->t[2]);
                    seq_t m = seq_push_back(sd->m, nt);
                    return deep(sd->l, m, dig2(s4->t[3], t));
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
extern PURE Result<seq_t, frag_t> _seq_pop_back(seq_t s)
{
    tree_t t;
    s = seq_pop_back(s, &t);
    frag_t tl = get<frag_s>(t);
    return {s, tl};
}

static seq_t seq_pop_back(seq_t s, tree_t *t)
{
    switch (index(s))
    {
        case SEQ_NIL:
            error("pop-back empty");
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            *t = ss->t[0];
            return empty();
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            switch (index(sd->r))
            {
                case DIG_1:
                {
                    dig1_t s1 = get<dig1_s>(sd->r);
                    *t = s1->t[0];
                    return seq_deep_r(sd->l, sd->m);
                }
                case DIG_2:
                {
                    dig2_t s2 = get<dig2_s>(sd->r);
                    *t = s2->t[1];
                    return deep(sd->l, sd->m, dig1(s2->t[0]));
                }
                case DIG_3:
                {
                    dig3_t s3 = get<dig3_s>(sd->r);
                    *t = s3->t[2];
                    return deep(sd->l, sd->m, dig2(s3->t[0], s3->t[1]));
                }
                case DIG_4:
                {
                    dig4_t s4 = get<dig4_s>(sd->r);
                    *t = s4->t[3];
                    return deep(sd->l, sd->m, dig3(s4->t[0], s4->t[1],
                        s4->t[2]));
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
extern PURE seq_t _seq_replace_back(seq_t s, frag_t f)
{
    tree_t l = set<tree_t>(f);
    switch (index(s))
    {
        case SEQ_NIL:
            error("replace-back empty");
        case SEQ_SINGLE:
            return single(l);
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            switch (index(sd->r))
            {
                case DIG_1:
                    return deep(sd->l, sd->m, dig1(l));
                case DIG_2:
                {
                    dig2_t s2 = get<dig2_s>(sd->r);
                    return deep(sd->l, sd->m, dig2(s2->t[0], l));
                }
                case DIG_3:
                {
                    dig3_t s3 = get<dig3_s>(sd->r);
                    return deep(sd->l, sd->m, dig3(s3->t[0], s3->t[1], l));
                }
                case DIG_4:
                {
                    dig4_t s4 = get<dig4_s>(sd->r);
                    return deep(sd->l, sd->m, dig4(s4->t[0], s4->t[1],
                        s4->t[2], l));
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
extern PURE frag_t _seq_peek_back(seq_t s)
{
    switch (index(s))
    {
        case SEQ_NIL:
            error("peek-back empty");
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            frag_t sl = get<frag_s>(ss->t[0]);
            return sl;
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            switch (index(sd->r))
            {
                case DIG_1:
                {
                    dig1_t r1 = get<dig1_s>(sd->r);
                    frag_t sl = get<frag_s>(r1->t[0]);
                    return sl;
                }
                case DIG_2:
                {
                    dig2_t r2 = get<dig2_s>(sd->r);
                    frag_t sl = get<frag_s>(r2->t[1]);
                    return sl;
                }
                case DIG_3:
                {
                    dig3_t r3 = get<dig3_s>(sd->r);
                    frag_t sl = get<frag_s>(r3->t[2]);
                    return sl;
                }
                case DIG_4:
                {
                    dig4_t r4 = get<dig4_s>(sd->r);
                    frag_t sl = get<frag_s>(r4->t[3]);
                    return sl;
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
extern PURE seq_t _seq_append(seq_t s, seq_t t)
{
    tree_t m[4];
    size_t m_len = 0;
    s = seq_append(s, m, m_len, t);
    return s;
}

static seq_t seq_append(seq_t s, tree_t *m, size_t m_len, seq_t t)
{
    switch (index(s))
    {
        case SEQ_NIL: case SEQ_SINGLE:
            for (size_t i = 0; i < m_len; i++)
                t = seq_push_front(t, m[m_len - i - 1]);
            if (index(s) == SEQ_SINGLE)
            {
                seq_single_t ss = get<seq_single_s>(s);
                t = seq_push_front(t, ss->t[0]);
            }
            return t;
        case SEQ_DEEP:
        {
            switch (index(t))
            {
                case SEQ_NIL: case SEQ_SINGLE:
                    for (size_t i = 0; i < m_len; i++)
                        s = seq_push_back(s, m[i]);
                    if (index(t) == SEQ_SINGLE)
                    {
                        seq_single_t ts = get<seq_single_s>(t);
                        s = seq_push_back(s, ts->t[0]);
                    }
                    return s;
                case SEQ_DEEP:
                {
                    seq_deep_t sd = get<seq_deep_s>(s);
                    seq_deep_t td = get<seq_deep_s>(t);
                    m_len = seq_append_middle(sd->r, m, m_len, td->l);
                    seq_t u = seq_append(sd->m, m, m_len, td->m);
                    return deep(sd->l, u, td->r);
                }
                default:
                    error_bad_tree();
            }
        }
        default:
            error_bad_tree();
    }
}

static size_t seq_append_middle(dig_t s, tree_t *m, size_t m_len, dig_t t)
{
    tree_t n[12];
    size_t n_len = 0;
    switch (index(s))
    {
        case DIG_1:
            n[n_len++] = (get<dig1_s>(s))->t[0];
            break;
        case DIG_2:
            n[n_len++] = (get<dig2_s>(s))->t[0];
            n[n_len++] = (get<dig2_s>(s))->t[1];
            break;
        case DIG_3:
            n[n_len++] = (get<dig3_s>(s))->t[0];
            n[n_len++] = (get<dig3_s>(s))->t[1];
            n[n_len++] = (get<dig3_s>(s))->t[2];
            break;
        case DIG_4:
            n[n_len++] = (get<dig4_s>(s))->t[0];
            n[n_len++] = (get<dig4_s>(s))->t[1];
            n[n_len++] = (get<dig4_s>(s))->t[2];
            n[n_len++] = (get<dig4_s>(s))->t[3];
            break;
        default:
            error_bad_tree();
    }
    for (size_t i = 0; i < m_len; i++)
        n[n_len++] = m[i];
    switch (index(t))
    {
        case DIG_1:
            n[n_len++] = (get<dig1_s>(t))->t[0];
            break;
        case DIG_2:
            n[n_len++] = (get<dig2_s>(t))->t[0];
            n[n_len++] = (get<dig2_s>(t))->t[1];
            break;
        case DIG_3:
            n[n_len++] = (get<dig3_s>(t))->t[0];
            n[n_len++] = (get<dig3_s>(t))->t[1];
            n[n_len++] = (get<dig3_s>(t))->t[2];
            break;
        case DIG_4:
            n[n_len++] = (get<dig4_s>(t))->t[0];
            n[n_len++] = (get<dig4_s>(t))->t[1];
            n[n_len++] = (get<dig4_s>(t))->t[2];
            n[n_len++] = (get<dig4_s>(t))->t[3];
            break;
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
extern PURE Result<seq_t, frag_t, size_t, seq_t> _seq_split(seq_t s,
    size_t idx)
{
    seq_t l, r;
    tree_t t;
    if (idx >= _seq_length(s))
        error("split out-of-bounds");
    seq_split(s, &idx, &l, &r, &t);
    frag_t tl = get<frag_s>(t);
    return {l, tl, idx, r};
}

/*
 * Split left.
 */
extern PURE Result<seq_t, frag_t, size_t> _seq_left(seq_t s, size_t idx)
{
    seq_t l;
    tree_t t;
    if (idx >= _seq_length(s))
        error("left out-of-bounds");
    seq_split(s, &idx, &l, nullptr, &t);
    frag_t tl = get<frag_s>(t);
    return {l, tl, idx};
}

/*
 * Split right.
 */
extern PURE Result<frag_t, size_t, seq_t> _seq_right(seq_t s, size_t idx)
{
    seq_t r;
    tree_t t;
    if (idx >= _seq_length(s))
        error("right out-of-bounds");
    seq_split(s, &idx, nullptr, &r, &t);
    frag_t tl = get<frag_s>(t);
    return {tl, idx, r};
}

static void seq_split(seq_t s, size_t *idx, seq_t *l, seq_t *r, tree_t *t)
{
    size_t len;
    switch (index(s))
    {
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            SET(l, empty());
            SET(r, empty());
            *t = ss->t[0];
            return;
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            len = dig_length(sd->l);
            if (*idx < len)
            {
                dig_t dl, dr;
                bool have_dl, have_dr;
                dig_split(sd->l, idx, (l == nullptr? nullptr: &dl),
                    (r == nullptr? nullptr: &dr), &have_dl, &have_dr, t);
                if (have_dr)
                    SET(r, deep(dr, sd->m, sd->r));
                else
                    SET(r, seq_deep_l(sd->m, sd->r));
                if (have_dl)
                    SET(l, dig_to_seq(dl));
                else
                    SET(l, empty());
                return;
            }
            *idx -= len;
            len = _seq_length(sd->m);
            if (*idx < len)
            {
                seq_t l1, r1;
                tree_t t1;
                seq_split(sd->m, idx, (l == nullptr? nullptr: &l1),
                    (r == nullptr? nullptr: &r1), &t1);
                switch (index(t1))
                {
                    case TREE_2:
                    {
                        tree2_t t2 = get<tree2_s>(t1);
                        len = tree_length(t2->t[0]);
                        if (*idx < len)
                        {
                            SET(l, seq_deep_r(sd->l, l1));
                            SET(r, deep(dig1(t2->t[1]), r1, sd->r));
                            *t = t2->t[0];
                        }
                        else
                        {
                            *idx -= len;
                            SET(l, deep(sd->l, l1, dig1(t2->t[0])));
                            SET(r, seq_deep_l(r1, sd->r));
                            *t = t2->t[1];
                        }
                        return;
                    }
                    case TREE_3:
                    {
                        tree3_t t3 = get<tree3_s>(t1);
                        len = tree_length(t3->t[0]);
                        if (*idx < len)
                        {
                            SET(l, seq_deep_r(sd->l, l1));
                            SET(r, deep(dig2(t3->t[1], t3->t[2]), r1, sd->r));
                            *t = t3->t[0];
                            return;
                        }
                        *idx -= len;
                        len = tree_length(t3->t[1]);
                        if (*idx < len)
                        {
                            SET(l, deep(sd->l, l1, dig1(t3->t[0])));
                            SET(r, deep(dig1(t3->t[2]), r1, sd->r));
                            *t = t3->t[1];
                        }
                        else
                        {
                            *idx -= len;
                            SET(l, deep(sd->l, l1, dig2(t3->t[0], t3->t[1])));
                            SET(r, seq_deep_l(r1, sd->r));
                            *t = t3->t[2];
                        }
                        return;
                    }
                    default:
                        error_bad_tree();
                }
            }
            *idx -= len;
            {
                dig_t dl, dr;
                bool have_dl, have_dr;
                dig_split(sd->r, idx, (l == nullptr? nullptr: &dl),
                    (r == nullptr? nullptr: &dr), &have_dl, &have_dr, t);
                if (have_dl)
                    SET(l, deep(sd->l, sd->m, dl));
                else
                    SET(l, seq_deep_r(sd->l, sd->m));
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

static seq_t seq_deep_r(dig_t l, seq_t m)
{
    if (index(m) == SEQ_NIL)
        return dig_to_seq(l);
    tree_t t;
    m = seq_pop_back(m, &t);
    return deep(l, m, tree_to_dig(t));
}

static seq_t seq_deep_l(seq_t m, dig_t r)
{
    if (index(m) == SEQ_NIL)
        return dig_to_seq(r);
    tree_t t;
    m = seq_pop_front(m, &t);
    return deep(tree_to_dig(t), m, r);
}

static void dig_split(dig_t s, size_t *idx, dig_t *l, dig_t *r, bool *have_l,
    bool *have_r, tree_t *t)
{
    size_t len;
    switch (index(s))
    {
        case DIG_1:
        {
            dig1_t d1 = get<dig1_s>(s);
            *t = d1->t[0];
            *have_l = false;
            *have_r = false;
            return;
        }
        case DIG_2:
        {
            dig2_t d2 = get<dig2_s>(s);
            len = tree_length(d2->t[0]);
            if (*idx < len)
            {
                *t = d2->t[0];
                SET(r, dig1(d2->t[1]));
                *have_l = false;
                *have_r = true;
            }
            else
            {
                *idx -= len;
                *t = d2->t[1];
                SET(l, dig1(d2->t[0]));
                *have_l = true;
                *have_r = false;
            }
            return;
        }
        case DIG_3:
        {
            dig3_t d3 = get<dig3_s>(s);
            len = tree_length(d3->t[0]);
            if (*idx < len)
            {
                *t = d3->t[0];
                SET(r, dig2(d3->t[1], d3->t[2]));
                *have_l = false;
                *have_r = true;
                return;
            }
            *idx -= len;
            len = tree_length(d3->t[1]);
            if (*idx < len)
            {
                *t = d3->t[1];
                SET(l, dig1(d3->t[0]));
                SET(r, dig1(d3->t[2]));
                *have_l = true;
                *have_r = true;
            }
            else
            {
                *idx -= len;
                *t = d3->t[2];
                SET(l, dig2(d3->t[0], d3->t[1]));
                *have_l = true;
                *have_r = false;
            }
            return;
        }
        case DIG_4:
        {
            dig4_t d4 = get<dig4_s>(s);
            len = tree_length(d4->t[0]);
            if (*idx < len)
            {
                *t = d4->t[0];
                SET(r, dig3(d4->t[1], d4->t[2], d4->t[3]));
                *have_l = false;
                *have_r = true;
                return;
            }
            *idx -= len;
            len = tree_length(d4->t[1]);
            if (*idx < len)
            {
                *t = d4->t[1];
                SET(l, dig1(d4->t[0]));
                SET(r, dig2(d4->t[2], d4->t[3]));
                *have_l = true;
                *have_r = true;
                return;
            }
            *idx -= len;
            len = tree_length(d4->t[2]);
            if (*idx < len)
            {
                *t = d4->t[2];
                SET(l, dig2(d4->t[0], d4->t[1]));
                SET(r, dig1(d4->t[3]));
                *have_l = true;
                *have_r = true;
            }
            else
            {
                *idx -= len;
                *t = d4->t[3];
                SET(l, dig3(d4->t[0], d4->t[1], d4->t[2]));
                *have_l = true;
                *have_r = false;
            }
            return;
        }
        default:
            error_bad_tree();
    }
}

#if 0
/*
 * Find.
 */
extern PURE ssize_t _seq_find(seq_t s, ssize_t (*f)(void *, frag_t),
    void *data)
{
    switch (index(s))
    {
        case SEQ_NIL:
            return -1;
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            return tree_find(ss->t[0], f, data);
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            ssize_t r = dig_find(sd->l, f, data);
            if (r >= 0)
                return r;
            r = _seq_find(sd->m, f, data);
            if (r >= 0)
                return r + dig_length(sd->l);
            r = dig_find(sd->r, f, data);
            if (r >= 0)
                return r + dig_length(sd->l) + _seq_length(sd->m);
            return -1;
        }
        default:
            error_bad_tree();
    }
}

static PURE ssize_t dig_find(dig_t s, ssize_t (*f)(void *, frag_t),
    void *data)
{
    switch (index(d))
    {
        case DIG_1:
        {
            dig1_t s1 = get<dig1_s>(s);
            ssize_t r = tree_find(s1->t[0], f, data);
            return r;
        }
        case DIG_2:
        {
            dig2_t s2 = get<dig2_s>(s);
            ssize_t r = tree_find(s2->t[0], f, data);
            if (r >= 0)
                return r;
            r = tree_find(s2->t[1], f, data);
            if (r >= 0)
                return r + tree_length(s2->t[0]);
            return -1;
        }
        case DIG_3:
        {
            dig3_t s3 = get<dig3_s>(s);
            ssize_t r = tree_find(s3->t[0], f, data);
            if (r >= 0)
                return r;
            r = tree_find(s3->t[1], f, data);
            if (r >= 0)
                return r + tree_length(s3->t[0]);
            r = tree_find(s3->t[2], f, data);
            if (r >= 0)
                return r + tree_length(s3->t[0]) + tree_length(s3->t[1]);
            return -1;
        }
        case DIG_4:
        {
            dig4_t s4 = get<dig4_s>(s);
            ssize_t r = tree_find(s4->t[0], f, data);
            if (r >= 0)
                return r;
            r = tree_find(s4->t[1], f, data);
            if (r >= 0)
                return r + tree_length(s4->t[0]);
            r = tree_find(s4->t[2], f, data);
            if (r >= 0)
                return r + tree_length(s4->t[0]) + tree_length(s4->t[1]);
            r = tree_find(s4->t[3], f, data);
            if (r >= 0)
                return r + tree_length(s4->t[0]) + tree_length(s4->t[1]) +
                    tree_length(s4->t[2]);
            return -1;
        }
        default:
            error_bad_tree();
    }
}

static PURE ssize_t tree_find(tree_t s, ssize_t (*f)(void *, frag_t),
    void *data)
{
    switch (index(s))
    {
        case TREE_LEAF:
        {
            frag_t sl = get<frag_s>(s);
            arg = f(data, arg, sl);
            return arg;
        }
        case TREE_2:
        {
            tree2_t s2 = get<tree2_s>(s);
            arg = tree_foldl(s2->t[0], arg, f, data);
            arg = tree_foldl(s2->t[1], arg, f, data);
            return arg;
        }
        case TREE_3:
        {
            tree3_t s3 = get<tree3_s>(s);
            arg = tree_foldl(s3->t[0], arg, f, data);
            arg = tree_foldl(s3->t[1], arg, f, data);
            arg = tree_foldl(s3->t[2], arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}
#endif

/*
 * Fold left.
 */
extern PURE Any _seq_foldl(seq_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data)
{
    switch (index(s))
    {
        case SEQ_NIL:
            return arg;
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            arg = tree_foldl(ss->t[0], arg, f, data);
            return arg;
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            arg = dig_foldl(sd->l, arg, f, data);
            arg = _seq_foldl(sd->m, arg, f, data);
            arg = dig_foldl(sd->r, arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

static Any dig_foldl(dig_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data)
{
    switch (index(s))
    {
        case DIG_1:
        {
            dig1_t s1 = get<dig1_s>(s);
            arg = tree_foldl(s1->t[0], arg, f, data);
            return arg;
        }
        case DIG_2:
        {
            dig2_t s2 = get<dig2_s>(s);
            arg = tree_foldl(s2->t[0], arg, f, data);
            arg = tree_foldl(s2->t[1], arg, f, data);
            return arg;
        }
        case DIG_3:
        {
            dig3_t s3 = get<dig3_s>(s);
            arg = tree_foldl(s3->t[0], arg, f, data);
            arg = tree_foldl(s3->t[1], arg, f, data);
            arg = tree_foldl(s3->t[2], arg, f, data);
            return arg;
        }
        case DIG_4:
        {
            dig4_t s4 = get<dig4_s>(s);
            arg = tree_foldl(s4->t[0], arg, f, data);
            arg = tree_foldl(s4->t[1], arg, f, data);
            arg = tree_foldl(s4->t[2], arg, f, data);
            arg = tree_foldl(s4->t[3], arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

static Any tree_foldl(tree_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data)
{
    switch (index(s))
    {
        case TREE_LEAF:
        {
            frag_t sl = get<frag_s>(s);
            arg = f(data, arg, sl);
            return arg;
        }
        case TREE_2:
        {
            tree2_t s2 = get<tree2_s>(s);
            arg = tree_foldl(s2->t[0], arg, f, data);
            arg = tree_foldl(s2->t[1], arg, f, data);
            return arg;
        }
        case TREE_3:
        {
            tree3_t s3 = get<tree3_s>(s);
            arg = tree_foldl(s3->t[0], arg, f, data);
            arg = tree_foldl(s3->t[1], arg, f, data);
            arg = tree_foldl(s3->t[2], arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Fold right.
 */
extern PURE Any _seq_foldr(seq_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data)
{
    switch (index(s))
    {
        case SEQ_NIL:
            return arg;
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            arg = tree_foldr(ss->t[0], arg, f, data);
            return arg;
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            arg = dig_foldr(sd->r, arg, f, data);
            arg = _seq_foldr(sd->m, arg, f, data);
            arg = dig_foldr(sd->l, arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

static Any dig_foldr(dig_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data)
{
    switch (index(s))
    {
        case DIG_1:
        {
            dig1_t s1 = get<dig1_s>(s);
            arg = tree_foldr(s1->t[0], arg, f, data);
            return arg;
        }
        case DIG_2:
        {
            dig2_t s2 = get<dig2_s>(s);
            arg = tree_foldr(s2->t[1], arg, f, data);
            arg = tree_foldr(s2->t[0], arg, f, data);
            return arg;
        }
        case DIG_3:
        {
            dig3_t s3 = get<dig3_s>(s);
            arg = tree_foldr(s3->t[2], arg, f, data);
            arg = tree_foldr(s3->t[1], arg, f, data);
            arg = tree_foldr(s3->t[0], arg, f, data);
            return arg;
        }
        case DIG_4:
        {
            dig4_t s4 = get<dig4_s>(s);
            arg = tree_foldr(s4->t[3], arg, f, data);
            arg = tree_foldr(s4->t[2], arg, f, data);
            arg = tree_foldr(s4->t[1], arg, f, data);
            arg = tree_foldr(s4->t[0], arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

static Any tree_foldr(tree_t s, Any arg, Any (*f)(void *, Any, frag_t),
    void *data)
{
    switch (index(s))
    {
        case TREE_LEAF:
        {
            frag_t sl = get<frag_s>(s);
            arg = f(data, arg, sl);
            return arg;
        }
        case TREE_2:
        {
            tree2_t s2 = get<tree2_s>(s);
            arg = tree_foldr(s2->t[1], arg, f, data);
            arg = tree_foldr(s2->t[0], arg, f, data);
            return arg;
        }
        case TREE_3:
        {
            tree3_t s3 = get<tree3_s>(s);
            arg = tree_foldr(s3->t[2], arg, f, data);
            arg = tree_foldr(s3->t[1], arg, f, data);
            arg = tree_foldr(s3->t[0], arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Map.
 */
extern PURE seq_t _seq_map(seq_t s, frag_t (*f)(void *, frag_t), void *data)
{
    switch (index(s))
    {
        case SEQ_NIL:
            return s;
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            tree_t t = tree_map(ss->t[0], f, data);
            return single(t);
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            dig_t l = dig_map(sd->l, f, data);
            seq_t m = _seq_map(sd->m, f, data);
            dig_t r = dig_map(sd->r, f, data);
            return deep(l, m, r);
        }
        default:
            error_bad_tree();
    }
}

static dig_t dig_map(dig_t d, frag_t (*f)(void *, frag_t), void *data)
{
    switch (index(d))
    {
        case DIG_1:
        {
            dig1_t d1 = get<dig1_s>(d);
            tree_t t0 = tree_map(d1->t[0], f, data);
            return dig1(t0);
        }
        case DIG_2:
        {
            dig2_t d2 = get<dig2_s>(d);
            tree_t t0 = tree_map(d2->t[0], f, data);
            tree_t t1 = tree_map(d2->t[1], f, data);
            return dig2(t0, t1);
        }
        case DIG_3:
        {
            dig3_t d3 = get<dig3_s>(d);
            tree_t t0 = tree_map(d3->t[0], f, data);
            tree_t t1 = tree_map(d3->t[1], f, data);
            tree_t t2 = tree_map(d3->t[2], f, data);
            return dig3(t0, t1, t2);
        }
        case DIG_4:
        {
            dig4_t d4 = get<dig4_s>(d);
            tree_t t0 = tree_map(d4->t[0], f, data);
            tree_t t1 = tree_map(d4->t[1], f, data);
            tree_t t2 = tree_map(d4->t[2], f, data);
            tree_t t3 = tree_map(d4->t[3], f, data);
            return dig4(t0, t1, t2, t3);
        }
        default:
            error_bad_tree();
    }
}

static tree_t tree_map(tree_t t, frag_t (*f)(void *, frag_t), void *data)
{
    switch (index(t))
    {
        case TREE_LEAF:
        {
            frag_t tl = get<frag_s>(t);
            tl = f(data, tl);
            return set<tree_t>(tl);
        }
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            tree_t t0 = tree_map(t2->t[0], f, data);
            tree_t t1 = tree_map(t2->t[1], f, data);
            return tree2(t0, t1);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            tree_t t0 = tree_map(t3->t[0], f, data);
            tree_t t1 = tree_map(t3->t[1], f, data);
            tree_t t2 = tree_map(t3->t[2], f, data);
            return tree3(t0, t1, t2);
        }
        default:
            error_bad_tree();
    }
}

#define MAX_ITR_SIZE    64

#define ENTRY_TREE      0
#define ENTRY_DIG       1
#define ENTRY_SEQ       2
struct entry_s
{
    uint8_t type;
    uint8_t offset;
    union
    {
        seq_t s;
        dig_t d;
        tree_t t;
    };

    entry_s(void) { }
};
struct itr_s
{
    size_t ptr;
    entry_s stack[MAX_ITR_SIZE];
};
typedef struct itr_s *itr_t;

static inline PURE entry_s entry_seq(seq_t s, uint8_t offset)
{
    entry_s e;
    e.type = ENTRY_SEQ;
    e.offset = offset;
    e.s = s;
    return e;
}

static inline PURE entry_s entry_dig(dig_t d, uint8_t offset)
{
    entry_s e;
    e.type = ENTRY_DIG;
    e.offset = offset;
    e.d = d;
    return e;
}

static inline PURE entry_s entry_tree(tree_t t, uint8_t offset)
{
    entry_s e;
    e.type = ENTRY_TREE;
    e.offset = offset;
    e.t = t;
    return e;
}

static void seq_itr_push(itr_t itr, seq_t s);
static void dig_itr_push(itr_t itr, dig_t d);
static void tree_itr_push(itr_t itr, tree_t t);
static frag_t seq_itr_pop(itr_t itr);

static void seq_itr_push(itr_t itr, seq_t s)
{
    switch (index(s))
    {
        case SEQ_NIL:
            return;
        case SEQ_SINGLE:
        {
            seq_single_t ss = get<seq_single_s>(s);
            tree_itr_push(itr, ss->t[0]);
            return;
        }
        case SEQ_DEEP:
        {
            seq_deep_t sd = get<seq_deep_s>(s);
            itr->stack[itr->ptr++] = entry_seq(s, 1);
            dig_itr_push(itr, sd->l);
            return;
        }
        default:
            error_bad_tree();
    }
}

static void dig_itr_push(itr_t itr, dig_t d)
{
    switch (index(d))
    {
        case DIG_1:
        {
            dig1_t d1 = get<dig1_s>(d);
            tree_itr_push(itr, d1->t[0]);
            return;
        }
        case DIG_2:
        {
            dig2_t d2 = get<dig2_s>(d);
            itr->stack[itr->ptr++] = entry_dig(d, 1);
            tree_itr_push(itr, d2->t[0]);
            return;
        }
        case DIG_3:
        {
            dig3_t d3 = get<dig3_s>(d);
            itr->stack[itr->ptr++] = entry_dig(d, 1);
            tree_itr_push(itr, d3->t[0]);
            return;
        }
        case DIG_4:
        {
            dig4_t d4 = get<dig4_s>(d);
            itr->stack[itr->ptr++] = entry_dig(d, 1);
            tree_itr_push(itr, d4->t[0]);
            return;
        }
        default:
            error_bad_tree();
    }
}

static void tree_itr_push(itr_t itr, tree_t t)
{
    while (true)
    {
        switch (index(t))
        {
            case TREE_LEAF:
            {
                itr->stack[itr->ptr++] = entry_tree(t, 0);
                return;
            }
            case TREE_2:
            {
                tree2_t t2 = get<tree2_s>(t);
                itr->stack[itr->ptr++] = entry_tree(t, 1);
                t = t2->t[0];
                continue;
            }
            case TREE_3:
            {
                tree3_t t3 = get<tree3_s>(t);
                itr->stack[itr->ptr++] = entry_tree(t, 1);
                t = t3->t[0];
                continue;
            }
            default:
                error_bad_tree();
        }
    }
}

static frag_t seq_itr_pop(itr_t itr)
{
    if (itr->ptr == 0)
        return nullptr;
    itr->ptr--;
    frag_t tl = get<frag_s>(itr->stack[itr->ptr].t);
    if (itr->ptr > 0)
    {
        size_t ptr = itr->ptr-1;
        size_t offset = itr->stack[ptr].offset;
        itr->stack[ptr].offset++;
        switch (itr->stack[ptr].type)
        {
            case ENTRY_SEQ:
            {
                seq_deep_t sd = get<seq_deep_s>(itr->stack[ptr].s);
                switch (offset)
                {
                    case 1:
                        if (index(sd->m) != SEQ_NIL)
                        {
                            seq_itr_push(itr, sd->m);
                            break;
                        }
                        // Fallthrough:
                    case 2:
                        itr->ptr--;
                        dig_itr_push(itr, sd->r);
                        break;
                    default:
                        error_bad_tree();
                }
                break;
            }
            case ENTRY_DIG:
            {
                dig_t d = itr->stack[ptr].d;
                switch (index(d))
                {
                    case DIG_2:
                    {
                        dig2_t d2 = get<dig2_s>(d);
                        itr->ptr--;
                        tree_itr_push(itr, d2->t[1]);
                        break;
                    }            
                    case DIG_3:
                    {
                        dig3_t d3 = get<dig3_s>(d);
                        if (offset == 2)
                            itr->ptr--;
                        tree_itr_push(itr, d3->t[offset]);
                        break;
                    }
                    case DIG_4:
                    {
                        dig4_t d4 = get<dig4_s>(d);
                        if (offset == 3)
                            itr->ptr--;
                        tree_itr_push(itr, d4->t[offset]);
                        break;
                    }
                    default:
                        error_bad_tree();
                }
                break;
            }
            case ENTRY_TREE:
            {
                tree_t t = itr->stack[ptr].t;
                switch (index(t))
                {
                    case TREE_2:
                    {
                        tree2_t t2 = get<tree2_s>(t);
                        itr->ptr--;
                        tree_itr_push(itr, t2->t[1]);
                        break;
                    }
                    case TREE_3:
                    {
                        tree3_t t3 = get<tree3_s>(t);
                        if (offset == 2)
                            itr->ptr--;
                        tree_itr_push(itr, t3->t[offset]);
                        break;
                    }
                    default:
                        error_bad_tree();
                }
            }
        }
    }

    return tl;
}

/*
 * Compare.
 */
#define MIN(a, b)       ((a) < (b)? (a): (b))
extern PURE int _seq_compare(seq_t s, seq_t t, void *data,
    int (*val_compare)(void *, frag_t, size_t, frag_t, size_t))
{
    if (s == t)
        return 0;

    itr_s itrs_0, itrt_0;
    itr_t itrs = &itrs_0, itrt = &itrt_0;
    itrs->ptr = itrt->ptr = 0;
    seq_itr_push(itrs, s);
    seq_itr_push(itrt, t);

    frag_t ls = nullptr;
    frag_t lt = nullptr;
    size_t lens = 0, lent = 0, is = 0, it = 0;
    while (true)
    {
        if (lens - is == 0)
        {
            ls = seq_itr_pop(itrs);
            if (ls == nullptr)
                break;   
            lens = ls->len;
            is = 0;
        }
        if (lent - it == 0)
        {
            lt = seq_itr_pop(itrt);
            if (lt == nullptr)
                return -1;
            lent = lt->len;
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
        lt = seq_itr_pop(itrt);
        if (lt == nullptr)
            return 0;
        else
            return 1;
    }
    return 1;
}

}
