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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fbase.h"
#include "fgc.h"
#include "flist.h"
#include "fstring.h"
#include "ftree.h"


namespace F
{

/*
 * 234 Trees.
 */

#define error_bad_tree()    error("data-structure invariant violated")

/*
 * Interface renaming.
 */
typedef _Tree Tree;
typedef _TreeNil TreeNil;
typedef _Tree2 Tree2;
typedef _Tree3 Tree3;
typedef _Tree4 Tree4;
typedef _Compare Compare;
typedef Value<Word> K;
typedef Value<Word> A;
typedef Value<Word> C;
#define TREE_EMPTY _tree_empty()

/*
 * Tree node definitions.
 */
struct _Tree2
{
    size_t size;
    K k[1];
    Tree t[2];
};
struct _Tree3
{
    size_t size;
    K k[2];
    Tree t[3];
};
struct _Tree4
{
    size_t size;
    K k[3];
    Tree t[4];
};

/*
 * Tree node types.
 */
enum
{
    TREE_NIL = Tree::index<TreeNil>(),
    TREE_2   = Tree::index<Tree2>(),
    TREE_3   = Tree::index<Tree3>(),
    TREE_4   = Tree::index<Tree4>()
};

/*
 * Node constructors.
 */
static inline Tree tree2(Tree t0, K k0, Tree t1)
{
    size_t size = 1 + _tree_size(t0) + _tree_size(t1);
    Tree2 node = {size, {k0}, {t0, t1}};
    return node;
}
static inline Tree tree3(Tree t0, K k0, Tree t1, K k1, Tree t2)
{
    size_t size = 2 + _tree_size(t0) + _tree_size(t1) + _tree_size(t2);
    Tree3 node = {size, {k0, k1}, {t0, t1, t2}};
    return node;
}
static inline Tree tree4(Tree t0, K k0, Tree t1, K k1, Tree t2, K k2,
    Tree t3)
{
    size_t size = 3 + _tree_size(t0) + _tree_size(t1) + _tree_size(t2) +
        _tree_size(t3);
    Tree4 node = {size, {k0, k1, k2}, {t0, t1, t2, t3}};
    return node;
}

/*
 * Prototypes.
 */
static Tree tree2_insert(const Tree2 &t, K k, Compare compare);
static Tree tree3_insert(const Tree3 &t, K k, Compare compare);
static Tree tree_delete_2(Tree t, K k, Compare compare, bool *reduced);
static Tree tree_delete_min_2(Tree t, K *k, bool *reduced);
static Tree tree_delete_max_2(Tree t, K *k, bool *reduced);
static Tree tree2_fix_t0(Tree t0, K k0, Tree t1, bool *reduced);
static Tree tree2_fix_t1(Tree t0, K k0, Tree t1, bool *reduced);
static Tree tree3_fix_t0(Tree t0, K k0, Tree t1, K k1, Tree t2, bool *reduced);
static Tree tree3_fix_t1(Tree t0, K k0, Tree t1, K k1, Tree t2, bool *reduced);
static Tree tree3_fix_t2(Tree t0, K k0, Tree t1, K k1, Tree t2, bool *reduced);
static Tree tree4_fix_t0(Tree t0, K k0, Tree t1, K k1, Tree t2, K k2,
    Tree t3, bool *reduced);
static Tree tree4_fix_t1(Tree t0, K k0, Tree t1, K k1, Tree t2, K k2,
    Tree t3, bool *reduced);
static Tree tree4_fix_t2(Tree t0, K k0, Tree t1, K k1, Tree t2, K k2,
    Tree t3, bool *reduced);
static Tree tree4_fix_t3(Tree t0, K k0, Tree t1, K k1, Tree t2, K k2,
    Tree t3, bool *reduced);
static Tree tree2_concat_3_min(const Tree2 &t, K k, Tree u, size_t depth);
static Tree tree3_concat_3_min(const Tree3 &t, K k, Tree u, size_t depth);
static Tree tree2_concat_3_max(const Tree2 &t, K k, Tree u, size_t depth);
static Tree tree3_concat_3_max(const Tree3 &t, K k, Tree u, size_t depth);
static Tree tree_concat_3(Tree t, K k, Tree u, size_t t_depth, size_t u_depth,
    size_t *depth);
static bool tree_split_2(Tree t, K k, size_t depth, Tree *lt, Tree *rt,
    size_t *l_depth, size_t *r_depth, Compare compare);
static Tree tree_union_2(Tree t, Tree u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare);
static Tree tree_intersect_2(Tree t, Tree u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare);
static Tree tree_diff_2(Tree t, Tree u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare);
static List<C> tree_to_list_2(Tree t, C (*f)(void *, K), void *data,
    List<C> xs);
static bool tree_verify_2(Tree t, size_t depth);
static String tree_show_2(Tree t, String r, bool last, String (*f)(A));

/*
 * Constructor.
 */
extern PURE Tree _tree_singleton(K k)
{
    return tree2(TREE_EMPTY, k, TREE_EMPTY);
}

/*
 * Search.
 */
extern const K *_tree_search(Tree t, K k, Compare compare)
{
    while (true)
    {
        switch (index(t))
        {
            case TREE_NIL:
                return nullptr;
            case TREE_2:
            {
                const Tree2 &t2 = t;
                int cmp = compare(k, t2.k[0]);
                if (cmp < 0)
                {
                    t = t2.t[0];
                    continue;
                }
                else if (cmp > 0)
                {
                    t = t2.t[1];
                    continue;
                }
                return &t2.k[0];
            }
            case TREE_3:
            {
                const Tree3 &t3 = t;
                int cmp = compare(k, t3.k[0]);
                if (cmp < 0)
                {
                    t = t3.t[0];
                    continue;
                }
                else if (cmp > 0)
                {
                    cmp = compare(k, t3.k[1]);
                    if (cmp < 0)
                    {
                        t = t3.t[1];
                        continue;
                    }
                    else if (cmp > 0)
                    {
                        t = t3.t[2];
                        continue;
                    }
                    return &t3.k[1];
                }
                return &t3.k[0];
            }
            case TREE_4:
            {
                const Tree4 &t4 = t;
                int cmp = compare(k, t4.k[1]);
                if (cmp < 0)
                {
                    cmp = compare(k, t4.k[0]);
                    if (cmp < 0)
                    {
                        t = t4.t[0];
                        continue;
                    }
                    else if (cmp > 0)
                    {
                        t = t4.t[1];
                        continue;
                    }
                    return &t4.k[0];
                }
                else if (cmp > 0)
                {
                    cmp = compare(k, t4.k[2]);
                    if (cmp < 0)
                    {
                        t = t4.t[2];
                        continue;
                    }
                    else if (cmp > 0)
                    {
                        t = t4.t[3];
                        continue;
                    }
                    return &t4.k[2];
                }
                return &t4.k[1];
            }
        }
    }
}

/*
 * Insert.
 */
extern PURE Tree _tree_insert(Tree t, K k, Compare compare)
{
    switch (index(t))
    {
        case TREE_NIL:
            return tree2(TREE_EMPTY, k, TREE_EMPTY);
        case TREE_2:
            return tree2_insert(t, k, compare);
        case TREE_3:
            return tree3_insert(t, k, compare);
        case TREE_4:
        {
            const Tree4 &t4 = t;
            Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
            Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
            Tree nt = tree2(lt, t4.k[1], rt);
            return tree2_insert(nt, k, compare);
        }
        default:
            error_bad_tree();
    }
}

static Tree tree2_insert(const Tree2 &t, K k, Compare compare)
{
    int cmp = compare(k, t.k[0]);
    if (index(t.t[0]) == TREE_NIL)
    {
        Tree nil = TREE_EMPTY;
        if (cmp < 0)
            return tree3(nil, k, nil, t.k[0], nil);
        else if (cmp > 0)
            return tree3(nil, t.k[0], nil, k, nil);
        else
            return tree2(nil, k, nil);
    }
    if (cmp < 0)
    {
        switch (index(t.t[0]))
        {
            case TREE_2:
            {
                const Tree2 &t2 = t.t[0];
                Tree nt = tree2_insert(t2, k, compare);
                return tree2(nt, t.k[0], t.t[1]);
            }
            case TREE_3:
            {
                const Tree3 &t3 = t.t[0];
                Tree nt = tree3_insert(t3, k, compare);
                return tree2(nt, t.k[0], t.t[1]);
            }
            case TREE_4:
            {
                const Tree4 &t4 = t.t[0];
                cmp = compare(k, t4.k[1]);
                Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
                Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
                if (cmp < 0)
                {
                    Tree nt = tree2_insert(lt, k, compare);
                    return tree3(nt, t4.k[1], rt, t.k[0], t.t[1]);
                }
                else if (cmp > 0)
                {
                    Tree nt = tree2_insert(rt, k, compare);
                    return tree3(lt, t4.k[1], nt, t.k[0], t.t[1]);
                }
                else
                    return tree3(lt, k, rt, t.k[0], t.t[1]);
            }
            default:
                error_bad_tree();
        }
    }
    else if (cmp > 0)
    {
        switch (index(t.t[1]))
        {
            case TREE_2:
            {
                const Tree2 &t2 = t.t[1];
                Tree nt = tree2_insert(t2, k, compare);
                return tree2(t.t[0], t.k[0], nt);
            }
            case TREE_3:
            {
                const Tree3 &t3 = t.t[1];
                Tree nt = tree3_insert(t3, k, compare);
                return tree2(t.t[0], t.k[0], nt);
            }
            case TREE_4:
            {
                const Tree4 &t4 = t.t[1];
                cmp = compare(k, t4.k[1]);
                Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
                Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
                if (cmp < 0)
                {
                    Tree nt = tree2_insert(lt, k, compare);
                    return tree3(t.t[0], t.k[0], nt, t4.k[1], rt);
                }
                else if (cmp > 0)
                {
                    Tree nt = tree2_insert(rt, k, compare);
                    return tree3(t.t[0], t.k[0], lt, t4.k[1], nt);
                }
                else
                    return tree3(t.t[0], t.k[0], lt, k, rt);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree2(t.t[0], k, t.t[1]);
}

static Tree tree3_insert(const Tree3 &t, K k, Compare compare)
{
    int cmp = compare(k, t.k[0]);
    if (index(t.t[0]) == TREE_NIL)
    {
        Tree nil = TREE_EMPTY;
        if (cmp < 0)
            return tree4(nil, k, nil, t.k[0], nil, t.k[1], nil);
        else if (cmp > 0)
        {
            cmp = compare(k, t.k[1]);
            if (cmp < 0)
                return tree4(nil, t.k[0], nil, k, nil, t.k[1], nil);
            else if (cmp > 0)
                return tree4(nil, t.k[0], nil, t.k[1], nil, k, nil);
            else
                return tree3(nil, t.k[0], nil, k, nil);
        }
        else
            return tree3(nil, k, nil, t.k[1], nil);
    }

    if (cmp < 0)
    {
        switch (index(t.t[0]))
        {
            case TREE_2:
            {
                const Tree2 &t2 = t.t[0];
                Tree nt = tree2_insert(t2, k, compare);
                return tree3(nt, t.k[0], t.t[1], t.k[1], t.t[2]);
            }
            case TREE_3:
            {
                const Tree3 &t3 = t.t[0];
                Tree nt = tree3_insert(t3, k, compare);
                return tree3(nt, t.k[0], t.t[1], t.k[1], t.t[2]);
            }
            case TREE_4:
            {
                const Tree4 &t4 = t.t[0];
                cmp = compare(k, t4.k[1]);
                Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
                Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
                if (cmp < 0)
                {
                    Tree nt = tree2_insert(lt, k, compare);
                    return tree4(nt, t4.k[1], rt, t.k[0], t.t[1], t.k[1],
                        t.t[2]);
                }
                else if (cmp > 0)
                {
                    Tree nt = tree2_insert(rt, k, compare);
                    return tree4(lt, t4.k[1], nt, t.k[0], t.t[1], t.k[1],
                        t.t[2]);
                }
                else
                    return tree4(lt, k, rt, t.k[0], t.t[1], t.k[1], t.t[2]);
            }
            default:
                error_bad_tree();
        }
    }
    else if (cmp > 0)
    {
        cmp = compare(k, t.k[1]);
        if (cmp < 0)
        {
            switch (index(t.t[1]))
            {
                case TREE_2:
                {
                    const Tree2 &t2 = t.t[1];
                    Tree nt = tree2_insert(t2, k, compare);
                    return tree3(t.t[0], t.k[0], nt, t.k[1], t.t[2]);
                }
                case TREE_3:
                {
                    const Tree3 &t3 = t.t[1];
                    Tree nt = tree3_insert(t3, k, compare);
                    return tree3(t.t[0], t.k[0], nt, t.k[1], t.t[2]);
                }
                case TREE_4:
                {
                    const Tree4 &t4 = t.t[1];
                    cmp = compare(k, t4.k[1]);
                    Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
                    Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
                    if (cmp < 0)
                    {
                        Tree nt = tree2_insert(lt, k, compare);
                        return tree4(t.t[0], t.k[0], nt, t4.k[1], rt, t.k[1],
                            t.t[2]);
                    }
                    else if (cmp > 0)
                    {
                        Tree nt = tree2_insert(rt, k, compare);
                        return tree4(t.t[0], t.k[0], lt, t4.k[1], nt, t.k[1],
                            t.t[2]);
                    }
                    else
                        return tree4(t.t[0], t.k[0], lt, k, rt, t.k[1],
                            t.t[2]);
                }
                default:
                    error_bad_tree();
            }
        }
        else if (cmp > 0)
        {
            switch (index(t.t[2]))
            {
                case TREE_2:
                {
                    const Tree2 &t2 = t.t[2];
                    Tree nt = tree2_insert(t2, k, compare);
                    return tree3(t.t[0], t.k[0], t.t[1], t.k[1], nt);
                }
                case TREE_3:
                {
                    const Tree3 &t3 = t.t[2];
                    Tree nt = tree3_insert(t3, k, compare);
                    return tree3(t.t[0], t.k[0], t.t[1], t.k[1], nt);
                }
                case TREE_4:
                {
                    const Tree4 &t4 = t.t[2];
                    cmp = compare(k, t4.k[1]);
                    Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
                    Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
                    if (cmp < 0)
                    {
                        Tree nt = tree2_insert(lt, k, compare);
                        return tree4(t.t[0], t.k[0], t.t[1], t.k[1], nt,
                            t4.k[1], rt);
                    }
                    else if (cmp > 0)
                    {
                        Tree nt = tree2_insert(rt, k, compare);
                        return tree4(t.t[0], t.k[0], t.t[1], t.k[1], lt,
                            t4.k[1], nt);
                    }
                    else
                        return tree4(t.t[0], t.k[0], t.t[1], t.k[1], lt, k,
                            rt);
                }
                default:
                    error_bad_tree();
            }
        }
        else
            return tree3(t.t[0], t.k[0], t.t[1], k, t.t[2]);
    }
    else
        return tree3(t.t[0], k, t.t[1], t.k[1], t.t[2]);
}

/*
 * Delete.
 */
extern Tree _tree_delete(Tree t, K k, Compare compare)
{
    bool reduced = false;
    t = tree_delete_2(t, k, compare, &reduced);
    return t;
}

static Tree tree_delete_2(Tree t, K k, Compare compare, bool *reduced)
{
    switch (index(t))
    {
        case TREE_NIL:
            *reduced = false;
            return t;
        case TREE_2:
        {
            const Tree2 &t2 = t;
            int cmp = compare(k, t2.k[0]);
            if (cmp < 0)
            {
                Tree nt = tree_delete_2(t2.t[0], k, compare, reduced);
                return tree2_fix_t0(nt, t2.k[0], t2.t[1], reduced);
            }
            else if (cmp > 0)
            {
                Tree nt = tree_delete_2(t2.t[1], k, compare, reduced);
                return tree2_fix_t1(t2.t[0], t2.k[0], nt, reduced);
            }
            else
            {
                if (index(t2.t[1]) == TREE_NIL)
                {
                    *reduced = true;
                    return TREE_EMPTY;
                }
                else
                {
                    K ks;
                    Tree nt = tree_delete_min_2(t2.t[1], &ks, reduced);
                    return tree2_fix_t1(t2.t[0], ks, nt, reduced);
                }
            }
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            int cmp = compare(k, t3.k[0]);
            if (cmp < 0)
            {
                Tree nt = tree_delete_2(t3.t[0], k, compare, reduced);
                return tree3_fix_t0(nt, t3.k[0], t3.t[1], t3.k[1], t3.t[2],
                    reduced);
            }
            else if (cmp > 0)
            {
                cmp = compare(k, t3.k[1]);
                if (cmp < 0)
                {
                    Tree nt = tree_delete_2(t3.t[1], k, compare, reduced);
                    return tree3_fix_t1(t3.t[0], t3.k[0], nt, t3.k[1], t3.t[2],
                        reduced);
                }
                else if (cmp > 0)
                {
                    Tree nt = tree_delete_2(t3.t[2], k, compare, reduced);
                    return tree3_fix_t2(t3.t[0], t3.k[0], t3.t[1], t3.k[1], nt,
                        reduced);
                }
                else
                {
                    if (index(t3.t[2]) == TREE_NIL)
                    {
                        Tree nil = TREE_EMPTY;
                        return tree2(nil, t3.k[0], nil);
                    }
                    else
                    {
                        K ks;
                        Tree nt = tree_delete_min_2(t3.t[2], &ks, reduced);
                        return tree3_fix_t2(t3.t[0], t3.k[0], t3.t[1], ks, nt,
                            reduced);
                    }
                }
            }
            else
            {
                if (index(t3.t[1]) == TREE_NIL)
                {
                    Tree nil = TREE_EMPTY;
                    return tree2(nil, t3.k[1], nil);
                }
                else
                {
                    K ks;
                    Tree nt = tree_delete_min_2(t3.t[1], &ks, reduced);
                    return tree3_fix_t1(t3.t[0], ks, nt, t3.k[1], t3.t[2],
                        reduced);
                }
            }
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            int cmp = compare(k, t4.k[1]);
            if (cmp < 0)
            {
                cmp = compare(k, t4.k[0]);
                if (cmp < 0)
                {
                    Tree nt = tree_delete_2(t4.t[0], k, compare, reduced);
                    return tree4_fix_t0(nt, t4.k[0], t4.t[1], t4.k[1], t4.t[2],
                        t4.k[2], t4.t[3], reduced);
                }
                else if (cmp > 0)
                {
                    Tree nt = tree_delete_2(t4.t[1], k, compare, reduced);
                    return tree4_fix_t1(t4.t[0], t4.k[0], nt, t4.k[1], t4.t[2],
                        t4.k[2], t4.t[3], reduced);
                }
                else
                {
                    if (index(t4.t[1]) == TREE_NIL)
                    {
                        Tree nil = TREE_EMPTY;
                        return tree3(nil, t4.k[1], nil, t4.k[2], nil);
                    }
                    else
                    {
                        K ks;
                        Tree nt = tree_delete_min_2(t4.t[1], &ks, reduced);
                        return tree4_fix_t1(t4.t[0], ks, nt, t4.k[1], t4.t[2],
                            t4.k[2], t4.t[3], reduced);
                    }
                }
            }
            else if (cmp > 0)
            {
                cmp = compare(k, t4.k[2]);
                if (cmp < 0)
                {
                    Tree nt = tree_delete_2(t4.t[2], k, compare, reduced);
                    return tree4_fix_t2(t4.t[0], t4.k[0], t4.t[1], t4.k[1], nt,
                        t4.k[2], t4.t[3], reduced);
                }
                else if (cmp > 0)
                {
                    Tree nt = tree_delete_2(t4.t[3], k, compare, reduced);
                    return tree4_fix_t3(t4.t[0], t4.k[0], t4.t[1], t4.k[1],
                        t4.t[2], t4.k[2], nt, reduced);
                }
                else
                {
                    if (index(t4.t[3]) == TREE_NIL)
                    {
                        Tree nil = TREE_EMPTY;
                        return tree3(nil, t4.k[0], nil, t4.k[1], nil);
                    }
                    else
                    {
                        K ks;
                        Tree nt = tree_delete_min_2(t4.t[3], &ks, reduced);
                        return tree4_fix_t3(t4.t[0], t4.k[0], t4.t[1], t4.k[1],
                            t4.t[2], ks, nt, reduced);
                    }
                }
            }
            else
            {
                if (index(t4.t[2]) == TREE_NIL)
                {
                    Tree nil = TREE_EMPTY;
                    return tree3(nil, t4.k[0], nil, t4.k[2], nil);
                }
                else
                {
                    K ks;
                    Tree nt = tree_delete_min_2(t4.t[2], &ks, reduced);
                    return tree4_fix_t2(t4.t[0], t4.k[0], t4.t[1], ks, nt,
                        t4.k[2], t4.t[3], reduced);
                }
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Delete min.
 */
static Tree tree_delete_min_2(Tree t, K *k, bool *reduced)
{
    switch (index(t))
    {
        case TREE_NIL:
            *reduced = false;
            return t;
        case TREE_2:
        {
            const Tree2 &t2 = t;
            if (index(t2.t[0]) == TREE_NIL)
            {
                *reduced = true;
                if (k != nullptr)
                    *k = t2.k[0];
                return TREE_EMPTY;
            }
            else
            {
                Tree nt = tree_delete_min_2(t2.t[0], k, reduced);
                return tree2_fix_t0(nt, t2.k[0], t2.t[1], reduced);
            }
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            if (index(t3.t[0]) == TREE_NIL)
            {
                if (k != nullptr)
                    *k = t3.k[0];
                Tree nil = TREE_EMPTY;
                return tree2(nil, t3.k[1], nil);
            }
            else
            {
                Tree nt = tree_delete_min_2(t3.t[0], k, reduced);
                return tree3_fix_t0(nt, t3.k[0], t3.t[1], t3.k[1], t3.t[2],
                    reduced);
            }
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            if (index(t4.t[0]) == TREE_NIL)
            {
                if (k != nullptr)
                    *k = t4.k[0];
                Tree nil = TREE_EMPTY;
                return tree3(nil, t4.k[1], nil, t4.k[2], nil);
            }
            else
            {
                Tree nt = tree_delete_min_2(t4.t[0], k, reduced);
                return tree4_fix_t0(nt, t4.k[0], t4.t[1], t4.k[1], t4.t[2],
                    t4.k[2], t4.t[3], reduced);
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Delete max.
 */
static Tree tree_delete_max_2(Tree t, K *k, bool *reduced)
{
    switch (index(t))
    {
        case TREE_NIL:
            *reduced = false;
            return t;
        case TREE_2:
        {
            const Tree2 &t2 = t;
            if (index(t2.t[1]) == TREE_NIL)
            {
                *reduced = true;
                if (k != nullptr)
                    *k = t2.k[0];
                return TREE_EMPTY;
            }
            else
            {
                Tree nt = tree_delete_max_2(t2.t[1], k, reduced);
                return tree2_fix_t1(t2.t[0], t2.k[0], nt, reduced);
            }
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            if (index(t3.t[2]) == TREE_NIL)
            {
                if (k != nullptr)
                    *k = t3.k[1];
                Tree nil = TREE_EMPTY;
                return tree2(nil, t3.k[0], nil);
            }
            else
            {
                Tree nt = tree_delete_max_2(t3.t[2], k, reduced);
                return tree3_fix_t2(t3.t[0], t3.k[0], t3.t[1], t3.k[1], nt,
                    reduced);
            }
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            if (index(t4.t[3]) == TREE_NIL)
            {
                if (k != nullptr)
                    *k = t4.k[2];
                Tree nil = TREE_EMPTY;
                return tree3(nil, t4.k[0], nil, t4.k[1], nil);
            }
            else
            {
                Tree nt = tree_delete_max_2(t4.t[3], k, reduced);
                return tree4_fix_t3(t4.t[0], t4.k[0], t4.t[1], t4.k[1],
                    t4.t[2], t4.k[2], nt, reduced);
            }
        }
        default:
            error_bad_tree();
    }
}

static Tree tree2_fix_t0(Tree t0, K k0, Tree t1, bool *reduced)
{
    if (*reduced)
    {
        switch (index(t1))
        {
            case TREE_2:
            {
                const Tree2 &t12 = t1;
                return tree3(t0, k0, t12.t[0], t12.k[0], t12.t[1]);
            }
            case TREE_3:
            {
                *reduced = false;
                const Tree3 &t13 = t1;
                Tree nt1 = tree2(t13.t[1], t13.k[1], t13.t[2]);
                Tree nt0 = tree2(t0, k0, t13.t[0]);
                return tree2(nt0, t13.k[0], nt1);
            }
            case TREE_4:
            {
                *reduced = false;
                const Tree4 &t14 = t1;
                Tree nt1 = tree3(t14.t[1], t14.k[1], t14.t[2],
                    t14.k[2], t14.t[3]);
                Tree nt0 = tree2(t0, k0, t14.t[0]);
                return tree2(nt0, t14.k[0], nt1);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree2(t0, k0, t1);
}

static Tree tree2_fix_t1(Tree t0, K k0, Tree t1, bool *reduced)
{
    if (*reduced)
    {
        switch (index(t0))
        {
            case TREE_2:
            {
                const Tree2 &t02 = t0;
                return tree3(t02.t[0], t02.k[0], t02.t[1], k0, t1);
            }
            case TREE_3:
            {
                *reduced = false;
                const Tree3 &t03 = t0;
                Tree nt0 = tree2(t03.t[0], t03.k[0], t03.t[1]);
                Tree nt1 = tree2(t03.t[2], k0, t1);
                return tree2(nt0, t03.k[1], nt1);
            }
            case TREE_4:
            {
                *reduced = false;
                const Tree4 &t04 = t0;
                Tree nt0 = tree3(t04.t[0], t04.k[0], t04.t[1], t04.k[1],
                    t04.t[2]);
                Tree nt1 = tree2(t04.t[3], k0, t1);
                return tree2(nt0, t04.k[2], nt1);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree2(t0, k0, t1);
}

static Tree tree3_fix_t0(Tree t0, K k0, Tree t1, K k1, Tree t2, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t1))
        {
            case TREE_2:
            {
                const Tree2 &t12 = t1;
                Tree nt1 = tree3(t0, k0, t12.t[0], t12.k[0], t12.t[1]);
                return tree2(nt1, k1, t2);
            }
            case TREE_3:
            {
                const Tree3 &t13 = t1;
                Tree nt1 = tree2(t13.t[1], t13.k[1], t13.t[2]);
                Tree nt0 = tree2(t0, k0, t13.t[0]);
                return tree3(nt0, t13.k[0], nt1, k1, t2);
            }
            case TREE_4:
            {
                const Tree4 &t14 = t1;
                Tree nt1 = tree3(t14.t[1], t14.k[1], t14.t[2],
                    t14.k[2], t14.t[3]);
                Tree nt0 = tree2(t0, k0, t14.t[0]);
                return tree3(nt0, t14.k[0], nt1, k1, t2);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree3(t0, k0, t1, k1, t2);
}

static Tree tree3_fix_t1(Tree t0, K k0, Tree t1, K k1, Tree t2, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t0))
        {
            case TREE_2:
            {
                const Tree2 &t02 = t0;
                Tree nt0 = tree3(t02.t[0], t02.k[0], t02.t[1], k0, t1);
                return tree2(nt0, k1, t2);
            }
            case TREE_3:
            {
                const Tree3 &t03 = t0;
                Tree nt0 = tree2(t03.t[0], t03.k[0], t03.t[1]);
                Tree nt1 = tree2(t03.t[2], k0, t1);
                return tree3(nt0, t03.k[1], nt1, k1, t2);
            }
            case TREE_4:
            {
                const Tree4 &t04 = t0;
                Tree nt0 = tree3(t04.t[0], t04.k[0], t04.t[1],
                    t04.k[1], t04.t[2]);
                Tree nt1 = tree2(t04.t[3], k0, t1);
                return tree3(nt0, t04.k[2], nt1, k1, t2);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree3(t0, k0, t1, k1, t2);
}

static Tree tree3_fix_t2(Tree t0, K k0, Tree t1, K k1, Tree t2, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t1))
        {
            case TREE_2:
            {
                const Tree2 &t12 = t1;
                Tree nt1 = tree3(t12.t[0], t12.k[0], t12.t[1], k1, t2);
                return tree2(t0, k0, nt1);
            }
            case TREE_3:
            {
                const Tree3 &t13 = t1;
                Tree nt1 = tree2(t13.t[0], t13.k[0], t13.t[1]);
                Tree nt2 = tree2(t13.t[2], k1, t2);
                return tree3(t0, k0, nt1, t13.k[1], nt2);
            }
            case TREE_4:
            {
                const Tree4 &t14 = t1;
                Tree nt1 = tree3(t14.t[0], t14.k[0], t14.t[1],
                    t14.k[1], t14.t[2]);
                Tree nt2 = tree2(t14.t[3], k1, t2);
                return tree3(t0, k0, nt1, t14.k[2], nt2);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree3(t0, k0, t1, k1, t2);
}

static Tree tree4_fix_t0(Tree t0, K k0, Tree t1, K k1, Tree t2, K k2,
    Tree t3, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t1))
        {
            case TREE_2:
            {
                const Tree2 &t12 = t1;
                Tree nt1 = tree3(t0, k0, t12.t[0], t12.k[0], t12.t[1]);
                return tree3(nt1, k1, t2, k2, t3);
            }
            case TREE_3:
            {
                const Tree3 &t13 = t1;
                Tree nt1 = tree2(t13.t[1], t13.k[1], t13.t[2]);
                Tree nt0 = tree2(t0, k0, t13.t[0]);
                return tree4(nt0, t13.k[0], nt1, k1, t2, k2, t3);
            }
            case TREE_4:
            {
                const Tree4 &t14 = t1;
                Tree nt1 = tree3(t14.t[1], t14.k[1], t14.t[2], t14.k[2],
                    t14.t[3]);
                Tree nt0 = tree2(t0, k0, t14.t[0]);
                return tree4(nt0, t14.k[0], nt1, k1, t2, k2, t3);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree4(t0, k0, t1, k1, t2, k2, t3);
}

static Tree tree4_fix_t1(Tree t0, K k0, Tree t1, K k1, Tree t2, K k2,
    Tree t3, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t2))
        {
            case TREE_2:
            {
                const Tree2 &t22 = t2;
                Tree nt2 = tree3(t1, k1, t22.t[0], t22.k[0], t22.t[1]);
                return tree3(t0, k0, nt2, k2, t3);
            }
            case TREE_3:
            {
                const Tree3 &t23 = t2;
                Tree nt2 = tree2(t23.t[1], t23.k[1], t23.t[2]);
                Tree nt1 = tree2(t1, k1, t23.t[0]);
                return tree4(t0, k0, nt1, t23.k[0], nt2, k2, t3);
            }
            case TREE_4:
            {
                const Tree4 &t24 = t2;
                Tree nt2 = tree3(t24.t[1], t24.k[1], t24.t[2],
                    t24.k[2], t24.t[3]);
                Tree nt1 = tree2(t1, k1, t24.t[0]);
                return tree4(t0, k0, nt1, t24.k[0], nt2, k2, t3);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree4(t0, k0, t1, k1, t2, k2, t3);
}

static Tree tree4_fix_t2(Tree t0, K k0, Tree t1, K k1, Tree t2, K k2,
    Tree t3, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t3))
        {
            case TREE_2:
            {
                const Tree2 &t32 = t3;
                Tree nt3 = tree3(t2, k2, t32.t[0], t32.k[0], t32.t[1]);
                return tree3(t0, k0, t1, k1, nt3);
            }
            case TREE_3:
            {
                const Tree3 &t33 = t3;
                Tree nt3 = tree2(t33.t[1], t33.k[1], t33.t[2]);
                Tree nt2 = tree2(t2, k2, t33.t[0]);
                return tree4(t0, k0, t1, k1, nt2, t33.k[0], nt3);
            }
            case TREE_4:
            {
                const Tree4 &t34 = t3;
                Tree nt3 = tree3(t34.t[1], t34.k[1], t34.t[2],
                    t34.k[2], t34.t[3]);
                Tree nt2 = tree2(t2, k2, t34.t[0]);
                return tree4(t0, k0, t1, k1, nt2, t34.k[0], nt3);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree4(t0, k0, t1, k1, t2, k2, t3);
}

static Tree tree4_fix_t3(Tree t0, K k0, Tree t1, K k1, Tree t2, K k2,
    Tree t3, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t2))
        {
            case TREE_2:
            {
                const Tree2 &t22 = t2;
                Tree nt2 = tree3(t22.t[0], t22.k[0], t22.t[1], k2, t3);
                return tree3(t0, k0, t1, k1, nt2);
            }
            case TREE_3:
            {
                const Tree3 &t23 = t2;
                Tree nt2 = tree2(t23.t[0], t23.k[0], t23.t[1]);
                Tree nt3 = tree2(t23.t[2], k2, t3);
                return tree4(t0, k0, t1, k1, nt2, t23.k[1], nt3);
            }
            case TREE_4:
            {
                const Tree4 &t24 = t2;
                Tree nt2 = tree3(t24.t[0], t24.k[0], t24.t[1],
                    t24.k[1], t24.t[2]);
                Tree nt3 = tree2(t24.t[3], k2, t3);
                return tree4(t0, k0, t1, k1, nt2, t24.k[2], nt3);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree4(t0, k0, t1, k1, t2, k2, t3);
}

/*
 * Size.
 */
extern size_t _tree_size(Tree t)
{
    switch (index(t))
    {
        case TREE_NIL:
            return 0;
        case TREE_2:
        {
            const Tree2 &t2 = t;
            return t2.size;
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            return t3.size;
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            return t4.size;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Depth.
 */
extern size_t tree_depth(Tree t)
{
    size_t depth = 0;
    while (true)
    {
        switch (index(t))
        {
            case TREE_NIL:
                return depth;
            case TREE_2:
            {
                depth++;
                const Tree2 &t2 = t;
                t = t2.t[0];
                continue;
            }
            case TREE_3:
            {
                depth++;
                const Tree3 &t3 = t;
                t = t3.t[0];
                continue;
            }
            case TREE_4:
            {
                depth++;
                const Tree4 &t4 = t;
                t = t4.t[0];
                continue;
            }
        }
    }
}

/*
 * Concat.
 */
static Tree tree2_concat_3_min(const Tree2 &t, K k, Tree u, size_t depth)
{
    if (depth == 1)
        return tree3(u, k, t.t[0], t.k[0], t.t[1]);
    switch (index(t.t[0]))
    {
        case TREE_2:
        {
            const Tree2 &t2 = t.t[0];
            Tree nt = tree2_concat_3_min(t2, k, u, depth-1);
            return tree2(nt, t.k[0], t.t[1]);
        }
        case TREE_3:
        {
            const Tree3 &t3 = t.t[0];
            Tree nt = tree3_concat_3_min(t3, k, u, depth-1);
            return tree2(nt, t.k[0], t.t[1]);
        }
        case TREE_4:
        {
            const Tree4 &t4 = t.t[0];
            Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
            Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
            Tree nt = tree2_concat_3_min(lt, k, u, depth-1);
            return tree3(nt, t4.k[1], rt, t.k[0], t.t[1]);
        }
        default:
            error_bad_tree();
    }
}

static Tree tree3_concat_3_min(const Tree3 &t, K k, Tree u, size_t depth)
{
    if (depth == 1)
        return tree4(u, k, t.t[0], t.k[0], t.t[1], t.k[1], t.t[2]);
    switch (index(t.t[0]))
    {
        case TREE_2:
        {
            const Tree2 &t2 = t.t[0];
            Tree nt = tree2_concat_3_min(t2, k, u, depth-1);
            return tree3(nt, t.k[0], t.t[1], t.k[1], t.t[2]);
        }
        case TREE_3:
        {
            const Tree3 &t3 = t.t[0];
            Tree nt = tree3_concat_3_min(t3, k, u, depth-1);
            return tree3(nt, t.k[0], t.t[1], t.k[1],  t.t[2]);
        }
        case TREE_4:
        {
            const Tree4 &t4 = t.t[0];
            Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
            Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
            Tree nt = tree2_concat_3_min(lt, k, u, depth-1);
            return tree4(nt, t4.k[1], rt, t.k[0], t.t[1], t.k[1], t.t[2]);
        }
        default:
            error_bad_tree();
    }
}

static Tree tree2_concat_3_max(const Tree2 &t, K k, Tree u, size_t depth)
{
    if (depth == 1)
        return tree3(t.t[0], t.k[0], t.t[1], k, u);
    switch (index(t.t[1]))
    {
        case TREE_2:
        {
            const Tree2 &t2 = t.t[1];
            Tree nt = tree2_concat_3_max(t2, k, u, depth-1);
            return tree2(t.t[0], t.k[0], nt);
        }
        case TREE_3:
        {
            const Tree3 &t3 = t.t[1];
            Tree nt = tree3_concat_3_max(t3, k, u, depth-1);
            return tree2(t.t[0], t.k[0], nt);
        }
        case TREE_4:
        {
            const Tree4 &t4 = t.t[1];
            Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
            Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
            Tree nt = tree2_concat_3_max(rt, k, u, depth-1);
            return tree3(t.t[0], t.k[0], lt, t4.k[1], nt);
        }
        default:
            error_bad_tree();
    }
}

static Tree tree3_concat_3_max(const Tree3 &t, K k, Tree u, size_t depth)
{
    if (depth == 1)
        return tree4(t.t[0], t.k[0], t.t[1], t.k[1], t.t[2], k, u);
    switch (index(t.t[2]))
    {
        case TREE_2:
        {
            const Tree2 &t2 = t.t[2];
            Tree nt = tree2_concat_3_max(t2, k, u, depth-1);
            return tree3(t.t[0], t.k[0], t.t[1], t.k[1], nt);
        }
        case TREE_3:
        {
            const Tree3 &t3 = t.t[2];
            Tree nt = tree3_concat_3_max(t3, k, u, depth-1);
            return tree3(t.t[0], t.k[0], t.t[1], t.k[1],  nt);
        }
        case TREE_4:
        {
            const Tree4 &t4 = t.t[2];
            Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
            Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
            Tree nt = tree2_concat_3_max(rt, k, u, depth-1);
            return tree4(t.t[0], t.k[0], t.t[1], t.k[1], lt, t4.k[1], nt);
        }
        default:
            error_bad_tree();
    }
}

static Tree tree_concat_3(Tree t, K k, Tree u, size_t t_depth, size_t u_depth,
    size_t *depth)
{
    if (t_depth == u_depth)
    {
        *depth = t_depth + 1;
        return tree2(t, k, u);
    }
    if (t_depth < u_depth)
    {
        *depth = u_depth;
        switch (index(u))
        {
            case TREE_2:
                return tree2_concat_3_min(u, k, t, u_depth - t_depth);
            case TREE_3:
                return tree3_concat_3_min(u, k, t, u_depth - t_depth);
            case TREE_4:
            {
                *depth = u_depth + 1;
                const Tree4 &u4 = u;
                Tree lu = tree2(u4.t[0], u4.k[0], u4.t[1]);
                Tree ru = tree2(u4.t[2], u4.k[2], u4.t[3]);
                Tree nu = tree2(lu, u4.k[1], ru);
                return tree2_concat_3_min(nu, k, t, u_depth - t_depth + 1);
            }
            default:
                error_bad_tree();
        }
    }
    else
    {
        *depth = t_depth;
        switch (index(t))
        {
            case TREE_2:
                return tree2_concat_3_max(t, k, u, t_depth - u_depth);
            case TREE_3:
                return tree3_concat_3_max(t, k, u, t_depth - u_depth);
            case TREE_4:
            {
                *depth = t_depth + 1;
                const Tree4 &t4 = t;
                Tree lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
                Tree rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
                Tree nt = tree2(lt, t4.k[1], rt);
                return tree2_concat_3_max(nt, k, u, t_depth - u_depth + 1);
            }
            default:
                error_bad_tree();
        }
    }
}

static Tree tree_concat(Tree t, Tree u, size_t t_depth, size_t u_depth,
    size_t *depth)
{
    bool reduced;
    K k;
    if (t_depth == 0)
    {
        *depth = u_depth;
        return u;
    }
    if (u_depth == 0)
    {
        *depth = t_depth;
        return t;
    }
    if (t_depth < u_depth)
    {
        t = tree_delete_max_2(t, &k, &reduced);
        return tree_concat_3(t, k, u, (reduced? t_depth-1: t_depth), u_depth,
            depth);
    }
    else
    {
        u = tree_delete_min_2(u, &k, &reduced);
        return tree_concat_3(t, k, u, t_depth, (reduced? u_depth-1: u_depth),
            depth);
    }
}

/*
 * Split.
 */
extern PURE Result<Tree, Tree> _tree_split(Tree t, K k, Compare compare)
{
    size_t depth = tree_depth(t), l_depth, r_depth;
    Tree lt = TREE_EMPTY, rt = TREE_EMPTY;
    tree_split_2(t, k, depth, &lt, &rt, &l_depth, &r_depth, compare);
    return {lt, rt};
}

static bool tree_split_2(Tree t, K k, size_t depth, Tree *lt, Tree *rt,
    size_t *l_depth, size_t *r_depth, Compare compare)
{
    if (depth == 0)
    {
        *lt = TREE_EMPTY;
        *rt = TREE_EMPTY;
        *l_depth = 0;
        *r_depth = 0;
        return false;
    }

    switch (index(t))
    {
        case TREE_2:
        {
            const Tree2 &t2 = t;
            int cmp = compare(k, t2.k[0]);
            bool r = true;
            if (cmp < 0)
            {
                r = tree_split_2(t2.t[0], k, depth-1, lt, rt, l_depth,
                    r_depth, compare);
                *rt = tree_concat_3(*rt, t2.k[0], t2.t[1], *r_depth, depth-1,
                    r_depth);
            }
            else if (cmp > 0)
            {
                r = tree_split_2(t2.t[1], k, depth-1, lt, rt, l_depth,
                    r_depth, compare);
                *lt = tree_concat_3(t2.t[0], t2.k[0], *lt, depth-1, *l_depth,
                    l_depth);
            }
            else
            {
                *lt = t2.t[0];
                *rt = t2.t[1];
                *l_depth = depth-1;
                *r_depth = depth-1;
            }
            return r;
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            int cmp = compare(k, t3.k[0]);
            bool r = true;
            if (cmp < 0)
            {
                r = tree_split_2(t3.t[0], k, depth-1, lt, rt, l_depth,
                    r_depth, compare);
                Tree nt = tree2(t3.t[1], t3.k[1], t3.t[2]);
                *rt = tree_concat_3(*rt, t3.k[0], nt, *r_depth, depth,
                    r_depth);
            }
            else if (cmp > 0)
            {
                cmp = compare(k, t3.k[1]);
                if (cmp < 0)
                {
                    r = tree_split_2(t3.t[1], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    *lt = tree_concat_3(t3.t[0], t3.k[0], *lt, depth-1,
                        *l_depth, l_depth);
                    *rt = tree_concat_3(*rt, t3.k[1], t3.t[2], *r_depth,
                        depth-1, r_depth);
                }
                else if (cmp > 0)
                {
                    r = tree_split_2(t3.t[2], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    Tree nt = tree2(t3.t[0], t3.k[0], t3.t[1]);
                    *lt = tree_concat_3(nt, t3.k[1], *lt, depth, *l_depth,
                        l_depth);
                }
                else
                {
                    *lt = tree2(t3.t[0], t3.k[0], t3.t[1]);
                    *rt = t3.t[2];
                    *l_depth = depth;
                    *r_depth = depth-1;
                }
            }
            else
            {
                *lt = t3.t[0];
                *rt = tree2(t3.t[1], t3.k[1], t3.t[2]);
                *l_depth = depth-1;
                *r_depth = depth;
            }
            return r;
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            int cmp = compare(k, t4.k[1]);
            bool r = true;
            if (cmp < 0)
            {
                cmp = compare(k, t4.k[0]);
                if (cmp < 0)
                {
                    r = tree_split_2(t4.t[0], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    Tree nt = tree3(t4.t[1], t4.k[1], t4.t[2], t4.k[2],
                        t4.t[3]);
                    *rt = tree_concat_3(*rt, t4.k[0], nt, *r_depth, depth,
                        r_depth);
                }
                else if (cmp > 0)
                {
                    r = tree_split_2(t4.t[1], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    Tree nt = tree2(t4.t[2], t4.k[2], t4.t[3]);
                    *lt = tree_concat_3(t4.t[0], t4.k[0], *lt, depth-1,
                        *l_depth, l_depth);
                    *rt = tree_concat_3(*rt, t4.k[1], nt, *r_depth, depth,
                        r_depth);
                }
                else
                {
                    *lt = t4.t[0];
                    *rt = tree3(t4.t[1], t4.k[1], t4.t[2], t4.k[2],
                        t4.t[3]);
                    *l_depth = depth-1;
                    *r_depth = depth;
                }
            }
            else if (cmp > 0)
            {
                cmp = compare(k, t4.k[2]);
                if (cmp < 0)
                {
                    r = tree_split_2(t4.t[2], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    Tree nt = tree2(t4.t[0], t4.k[0], t4.t[1]);
                    *lt = tree_concat_3(nt, t4.k[1], *lt, depth, *l_depth,
                        l_depth);
                    *rt = tree_concat_3(*rt, t4.k[2], t4.t[3], *r_depth,
                        depth-1, r_depth);
                }
                else if (cmp > 0)
                {
                    r = tree_split_2(t4.t[3], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    Tree nt = tree3(t4.t[0], t4.k[0], t4.t[1], t4.k[1],
                        t4.t[2]);
                    *lt = tree_concat_3(nt, t4.k[2], *lt, depth, *l_depth,
                        l_depth);
                }
                else
                {
                    *lt = tree3(t4.t[0], t4.k[0], t4.t[1], t4.k[1],
                        t4.t[2]);
                    *rt = t4.t[3];
                    *l_depth = depth;
                    *r_depth = depth-1;
                }
            }
            else
            {
                *lt = tree2(t4.t[0], t4.k[0], t4.t[1]);
                *rt = tree2(t4.t[2], t4.k[2], t4.t[3]);
                *l_depth = depth;
                *r_depth = depth;
            }
            return r;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Union.
 */
extern PURE Tree _tree_union(Tree t, Tree u, Compare compare)
{
    size_t t_depth = tree_depth(t);
    size_t u_depth = tree_depth(u);
    return tree_union_2(t, u, t_depth, u_depth, &t_depth, compare);
}

static Tree tree_union_2(Tree t, Tree u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare)
{
    switch (index(u))
    {
        case TREE_NIL:
            *depth = t_depth;
            return t;
        case TREE_2:
        {
            const Tree2 &u2 = u;
            Tree lt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, r_depth;
            tree_split_2(t, u2.k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            lt = tree_union_2(lt, u2.t[0], l_depth, u_depth-1, &l_depth,
                compare);
            rt = tree_union_2(rt, u2.t[1], r_depth, u_depth-1, &r_depth,
                compare);
            t = tree_concat_3(lt, u2.k[0], rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_3:
        {
            const Tree3 &u3 = u;
            Tree lt = TREE_EMPTY, mt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, m_depth, r_depth;
            tree_split_2(t, u3.k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            tree_split_2(rt, u3.k[1], r_depth, &mt, &rt, &m_depth, &r_depth,
                compare);
            lt = tree_union_2(lt, u3.t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_union_2(mt, u3.t[1], m_depth, u_depth-1, &m_depth,
                compare);
            rt = tree_union_2(rt, u3.t[2], r_depth, u_depth-1, &r_depth,
                compare);
            lt = tree_concat_3(lt, u3.k[0], mt, l_depth, m_depth, &l_depth);
            t  = tree_concat_3(lt, u3.k[1], rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_4:
        {
            const Tree4 &u4 = u;
            Tree lt = TREE_EMPTY, mt = TREE_EMPTY, nt = TREE_EMPTY,
                rt = TREE_EMPTY;
            size_t l_depth, m_depth, n_depth, r_depth;
            tree_split_2(t, u4.k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            tree_split_2(rt, u4.k[1], r_depth, &mt, &rt, &m_depth, &r_depth,
                compare);
            tree_split_2(rt, u4.k[2], r_depth, &nt, &rt, &n_depth, &r_depth,
                compare);
            lt = tree_union_2(lt, u4.t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_union_2(mt, u4.t[1], m_depth, u_depth-1, &m_depth,
                compare);
            nt = tree_union_2(nt, u4.t[2], n_depth, u_depth-1, &n_depth,
                compare);
            rt = tree_union_2(rt, u4.t[3], r_depth, u_depth-1, &r_depth,
                compare);
            lt = tree_concat_3(lt, u4.k[0], mt, l_depth, m_depth, &l_depth);
            lt = tree_concat_3(lt, u4.k[1], nt, l_depth, n_depth, &l_depth);
            t  = tree_concat_3(lt, u4.k[2], rt, l_depth, r_depth, depth);
            return t;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Intersection.
 */
extern PURE Tree _tree_intersect(Tree t, Tree u, Compare compare)
{
    size_t t_depth = tree_depth(t);
    size_t u_depth = tree_depth(u);
    return tree_intersect_2(t, u, t_depth, u_depth, &t_depth, compare);
}

static Tree tree_intersect_2(Tree t, Tree u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare)
{
    switch (index(u))
    {
        case TREE_NIL:
            *depth = 0;
            return TREE_EMPTY;
        case TREE_2:
        {
            const Tree2 &u2 = u;
            Tree lt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, r_depth;
            bool in0 = tree_split_2(t, u2.k[0], t_depth, &lt, &rt, &l_depth,
                &r_depth, compare);
            lt = tree_intersect_2(lt, u2.t[0], l_depth, u_depth-1, &l_depth,
                compare);
            rt = tree_intersect_2(rt, u2.t[1], r_depth, u_depth-1, &r_depth,
                compare);
            if (in0)
                t = tree_concat_3(lt, u2.k[0], rt, l_depth, r_depth, depth);
            else
                t = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_3:
        {
            const Tree3 &u3 = u;
            Tree lt = TREE_EMPTY, mt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, m_depth, r_depth;
            bool in0 = tree_split_2(t, u3.k[0], t_depth, &lt, &rt, &l_depth,
                &r_depth, compare);
            bool in1 = tree_split_2(rt, u3.k[1], r_depth, &mt, &rt, &m_depth,
                &r_depth, compare);
            lt = tree_intersect_2(lt, u3.t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_intersect_2(mt, u3.t[1], m_depth, u_depth-1, &m_depth,
                compare);
            rt = tree_intersect_2(rt, u3.t[2], r_depth, u_depth-1, &r_depth,
                compare);
            if (in0)
                lt = tree_concat_3(lt, u3.k[0], mt, l_depth, m_depth,
                    &l_depth);
            else
                lt = tree_concat(lt, mt, l_depth, m_depth, &l_depth);
            if (in1)
                t = tree_concat_3(lt, u3.k[1], rt, l_depth, r_depth, depth);
            else
                t = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_4:
        {
            const Tree4 &u4 = u;
            Tree lt = TREE_EMPTY, mt = TREE_EMPTY, nt = TREE_EMPTY,
                rt = TREE_EMPTY;
            size_t l_depth, m_depth, n_depth, r_depth;
            bool in0 = tree_split_2(t, u4.k[0], t_depth, &lt, &rt, &l_depth,
                &r_depth, compare);
            bool in1 = tree_split_2(rt, u4.k[1], r_depth, &mt, &rt, &m_depth,
                &r_depth, compare);
            bool in2 = tree_split_2(rt, u4.k[2], r_depth, &nt, &rt, &n_depth,
                &r_depth, compare);
            lt = tree_intersect_2(lt, u4.t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_intersect_2(mt, u4.t[1], m_depth, u_depth-1, &m_depth,
                compare);
            nt = tree_intersect_2(nt, u4.t[2], n_depth, u_depth-1, &n_depth,
                compare);
            rt = tree_intersect_2(rt, u4.t[3], r_depth, u_depth-1, &r_depth,
                compare);
            if (in0)
                lt = tree_concat_3(lt, u4.k[0], mt, l_depth, m_depth,
                    &l_depth);
            else
                lt = tree_concat(lt, mt, l_depth, m_depth, &l_depth);
            if (in1)
                lt = tree_concat_3(lt, u4.k[1], nt, l_depth, n_depth,
                    &l_depth);
            else
                lt = tree_concat(lt, nt, l_depth, n_depth, &l_depth);
            if (in2)
                t = tree_concat_3(lt, u4.k[2], rt, l_depth, r_depth, depth);
            else
                t = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Difference.
 */
extern PURE Tree _tree_diff(Tree t, Tree u, Compare compare)
{
    size_t t_depth = tree_depth(t);
    size_t u_depth = tree_depth(u);
    return tree_diff_2(t, u, t_depth, u_depth, &t_depth, compare);
}

static Tree tree_diff_2(Tree t, Tree u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare)
{
    switch (index(u))
    {
        case TREE_NIL:
            *depth = t_depth;
            return t;
        case TREE_2:
        {
            const Tree2 &u2 = u;
            Tree lt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, r_depth;
            tree_split_2(t, u2.k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            lt = tree_diff_2(lt, u2.t[0], l_depth, u_depth-1, &l_depth,
                compare);
            rt = tree_diff_2(rt, u2.t[1], r_depth, u_depth-1, &r_depth,
                compare);
            t = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_3:
        {
            const Tree3 &u3 = u;
            Tree lt = TREE_EMPTY, mt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, m_depth, r_depth;
            tree_split_2(t, u3.k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            tree_split_2(rt, u3.k[1], r_depth, &mt, &rt, &m_depth, &r_depth,
                compare);
            lt = tree_diff_2(lt, u3.t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_diff_2(mt, u3.t[1], m_depth, u_depth-1, &m_depth,
                compare);
            rt = tree_diff_2(rt, u3.t[2], r_depth, u_depth-1, &r_depth,
                compare);
            lt = tree_concat(lt, mt, l_depth, m_depth, &l_depth);
            t  = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_4:
        {
            const Tree4 &u4 = u;
            Tree lt = TREE_EMPTY, mt = TREE_EMPTY, nt = TREE_EMPTY,
                rt = TREE_EMPTY;
            size_t l_depth, m_depth, n_depth, r_depth;
            tree_split_2(t, u4.k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            tree_split_2(rt, u4.k[1], r_depth, &mt, &rt, &m_depth, &r_depth,
                compare);
            tree_split_2(rt, u4.k[2], r_depth, &nt, &rt, &n_depth, &r_depth,
                compare);
            lt = tree_diff_2(lt, u4.t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_diff_2(mt, u4.t[1], m_depth, u_depth-1, &m_depth,
                compare);
            nt = tree_diff_2(nt, u4.t[2], n_depth, u_depth-1, &n_depth,
                compare);
            rt = tree_diff_2(rt, u4.t[3], r_depth, u_depth-1, &r_depth,
                compare);
            lt = tree_concat(lt, mt, l_depth, m_depth, &l_depth);
            lt = tree_concat(lt, nt, l_depth, n_depth, &l_depth);
            t  = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Fold left.
 */
extern PURE C _tree_foldl(Tree t, C arg, C (*f)(void *, C, K), void *data)
{
    switch (index(t))
    {
        case TREE_NIL:
            return arg;
        case TREE_2:
        {
            const Tree2 &t2 = t;
            arg = _tree_foldl(t2.t[0], arg, f, data);
            arg = f(data, arg, t2.k[0]);
            arg = _tree_foldl(t2.t[1], arg, f, data);
            return arg;
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            arg = _tree_foldl(t3.t[0], arg, f, data);
            arg = f(data, arg, t3.k[0]);
            arg = _tree_foldl(t3.t[1], arg, f, data);
            arg = f(data, arg, t3.k[1]);
            arg = _tree_foldl(t3.t[2], arg, f, data);
            return arg;
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            arg = _tree_foldl(t4.t[0], arg, f, data);
            arg = f(data, arg, t4.k[0]);
            arg = _tree_foldl(t4.t[1], arg, f, data);
            arg = f(data, arg, t4.k[1]);
            arg = _tree_foldl(t4.t[2], arg, f, data);
            arg = f(data, arg, t4.k[2]);
            arg = _tree_foldl(t4.t[3], arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Fold right.
 */
extern PURE C _tree_foldr(Tree t, C arg, C (*f)(void *, C, K), void *data)
{
    switch (index(t))
    {
        case TREE_NIL:
            return arg;
        case TREE_2:
        {
            const Tree2 &t2 = t;
            arg = _tree_foldr(t2.t[1], arg, f, data);
            arg = f(data, arg, t2.k[0]);
            arg = _tree_foldr(t2.t[0], arg, f, data);
            return arg;
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            arg = _tree_foldr(t3.t[2], arg, f, data);
            arg = f(data, arg, t3.k[1]);
            arg = _tree_foldr(t3.t[1], arg, f, data);
            arg = f(data, arg, t3.k[0]);
            arg = _tree_foldr(t3.t[0], arg, f, data);
            return arg;
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            arg = _tree_foldr(t4.t[3], arg, f, data);
            arg = f(data, arg, t4.k[2]);
            arg = _tree_foldr(t4.t[2], arg, f, data);
            arg = f(data, arg, t4.k[1]);
            arg = _tree_foldr(t4.t[1], arg, f, data);
            arg = f(data, arg, t4.k[0]);
            arg = _tree_foldr(t4.t[0], arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Map.
 */
extern PURE Tree _tree_map(Tree t, K (*f)(void *, K), void *data)
{
    switch (index(t))
    {
        case TREE_NIL:
            return TREE_EMPTY;
        case TREE_2:
        {
            const Tree2 &t2 = t;
            Tree t0 = _tree_map(t2.t[0], f, data);
            K k0 = f(data, t2.k[0]);
            Tree t1 = _tree_map(t2.t[1], f, data);
            return tree2(t0, k0, t1);
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            Tree t0 = _tree_map(t3.t[0], f, data);
            K k0 = f(data, t3.k[0]);
            Tree t1 = _tree_map(t3.t[1], f, data);
            K k1 = f(data, t3.k[1]);
            Tree t2 = _tree_map(t3.t[2], f, data);
            return tree3(t0, k0, t1, k1, t2);
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            Tree t0 = _tree_map(t4.t[0], f, data);
            K k0 = f(data, t4.k[0]);
            Tree t1 = _tree_map(t4.t[1], f, data);
            K k1 = f(data, t4.k[1]);
            Tree t2 = _tree_map(t4.t[2], f, data);
            K k2 = f(data, t4.k[2]);
            Tree t3 = _tree_map(t4.t[3], f, data);
            return tree4(t0, k0, t1, k1, t2, k2, t3);
        }
        default:
            error_bad_tree();
    }
}

/*
 * From list.
 */
extern PURE Tree _tree_from_list(const List<Word> xs, Compare compare)
{
    Tree t = TREE_EMPTY;
    List<Word> ys = xs;
    while (!empty(ys))
    {
        t = _tree_insert(t, head(ys), compare);
        ys = tail(ys);
    }
    return t;
}

/*
 * To list.
 */
extern PURE List<C> _tree_to_list(Tree t, C (*f)(void *, K), void *data)
{
    return tree_to_list_2(t, f, data, list<C>());
}

static List<C> tree_to_list_2(Tree t, C (*f)(void *, K), void *data,
    List<C> xs)
{
    switch (index(t))
    {
        case TREE_NIL:
            return xs;
        case TREE_2:
        {
            const Tree2 &t2 = t;
            xs = tree_to_list_2(t2.t[1], f, data, xs);
            xs = list<C>(f(data, t2.k[0]), xs);
            xs = tree_to_list_2(t2.t[0], f, data, xs);
            return xs;
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            xs = tree_to_list_2(t3.t[2], f, data, xs);
            xs = list<C>(f(data, t3.k[1]), xs);
            xs = tree_to_list_2(t3.t[1], f, data, xs);
            xs = list<C>(f(data, t3.k[0]), xs);
            xs = tree_to_list_2(t3.t[0], f, data, xs);
            return xs;
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            xs = tree_to_list_2(t4.t[3], f, data, xs);
            xs = list<C>(f(data, t4.k[2]), xs);
            xs = tree_to_list_2(t4.t[2], f, data, xs);
            xs = list<C>(f(data, t4.k[1]), xs);
            xs = tree_to_list_2(t4.t[1], f, data, xs);
            xs = list<C>(f(data, t4.k[0]), xs);
            xs = tree_to_list_2(t4.t[0], f, data, xs);
            return xs;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Verify.
 */
extern PURE bool _tree_verify(Tree t)
{
    size_t depth = tree_depth(t);
    return tree_verify_2(t, depth);
}

static bool tree_verify_2(Tree t, size_t depth)
{
    switch (index(t))
    {
        case TREE_NIL:
            return (depth == 0);
        case TREE_2:
        {
            const Tree2 &t2 = t;
            size_t size = 1 + _tree_size(t2.t[0]) + _tree_size(t2.t[1]);
            return (t2.size == size) &&
                   tree_verify_2(t2.t[0], depth-1) &&
                   tree_verify_2(t2.t[1], depth-1);
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            size_t size = 2 + _tree_size(t3.t[0]) + _tree_size(t3.t[1]) +
                _tree_size(t3.t[2]);
            return (t3.size == size) &&
                   tree_verify_2(t3.t[0], depth-1) &&
                   tree_verify_2(t3.t[1], depth-1) &&
                   tree_verify_2(t3.t[2], depth-1);
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            size_t size = 3 + _tree_size(t4.t[0]) + _tree_size(t4.t[1]) +
                _tree_size(t4.t[2]) + _tree_size(t4.t[3]);
            return (t4.size == size) &&
                   tree_verify_2(t4.t[0], depth-1) &&
                   tree_verify_2(t4.t[1], depth-1) &&
                   tree_verify_2(t4.t[2], depth-1) &&
                   tree_verify_2(t4.t[3], depth-1);
        }
        default:
            return false;
    }
}

/*
 * Compare.
 */
extern PURE int _tree_compare(Tree t, Tree u, void *data,
    int (*val_compare)(void *, Value<Word>, Value<Word>))
{
    if (t == u)
        return 0;

    _TreeItr itr_t = begin(t), itr_u = begin(u);
    _TreeItr itr_te = end(t),  itr_ue = end(u);

    while (true)
    {
        if (itr_t == itr_te)
        {
            if (itr_u == itr_ue)
                return 0;
            else
                return -1;
        }
        if (itr_u == itr_ue)
            return 1;
        Value<Word> kt = *itr_t;
        Value<Word> ku = *itr_u;
        int cmp = val_compare(data, kt, ku);
        if (cmp != 0)
            return cmp;
        ++itr_t;
        ++itr_u;
    }
}

/*
 * Show.
 */
extern PURE String _tree_show(Tree t, String (*f)(A))
{
    String r = string('{');
    r = tree_show_2(t, r, true, f);
    r = append(r, '}');
    return r;
}

#if 1
static String tree_show_2(Tree t, String r, bool last, String (*f)(A))
{
    switch (index(t))
    {
        case TREE_NIL:
            return r;
        case TREE_2:
        {
            const Tree2 &t2 = t;
            r = tree_show_2(t2.t[0], r, false, f);
            r = append(r, f(t2.k[0]));
            if (!last || index(t2.t[1]) != TREE_NIL)
                r = append(r, ',');
            r = tree_show_2(t2.t[1], r, last, f);
            return r;
        }
        case TREE_3:
        {
            const Tree3 &t3 = t;
            r = tree_show_2(t3.t[0], r, false, f);
            r = append(r, f(t3.k[0]));
            r = append(r, ',');
            r = tree_show_2(t3.t[1], r, false, f);
            r = append(r, f(t3.k[1]));
            if (!last || index(t3.t[2]) != TREE_NIL)
                r = append(r, ',');
            r = tree_show_2(t3.t[2], r, last, f);
            return r;
        }
        case TREE_4:
        {
            const Tree4 &t4 = t;
            r = tree_show_2(t4.t[0], r, false, f);
            r = append(r, f(t4.k[0]));
            r = append(r, ',');
            r = tree_show_2(t4.t[1], r, false, f);
            r = append(r, f(t4.k[1]));
            r = append(r, ',');
            r = tree_show_2(t4.t[2], r, false, f);
            r = append(r, f(t4.k[2]));
            if (!last || index(t4.t[3]) != TREE_NIL)
                r = append(r, ',');
            r = tree_show_2(t4.t[3], r, last, f);
            return r;
        }
        default:
            error_bad_tree();
    }
}
#else
/*
 * Alternative that shows the tree structure (useful for debugging).
 */
static String tree_show_2(Tree t, String r, bool last, String (*f)(A))
{
    switch (index(t))
    {
        case TREE_NIL:
            r = append(r, "emp");
            return r;
        case TREE_2:
        {
            const Tree2 &t2 = get<tree2_s>(t);
            r = append(r, "t2(");
            r = tree_show_2(t2->t[0], r, false, f);
            r = append(r, ',');
            r = append(r, f(t2->k[0]));
            r = append(r, ',');
            r = tree_show_2(t2->t[1], r, last, f);
            r = append(r, ')');
            return r;
        }
        case TREE_3:
        {
            const Tree3 &t3 = get<tree3_s>(t);
            r = append(r, "t3(");
            r = tree_show_2(t3->t[0], r, false, f);
            r = append(r, ',');
            r = append(r, f(t3->k[0]));
            r = append(r, ',');
            r = tree_show_2(t3->t[1], r, false, f);
            r = append(r, ',');
            r = append(r, f(t3->k[1]));
            r = append(r, ',');
            r = tree_show_2(t3->t[2], r, last, f);
            r = append(r, ')');
            return r;
        }
        case TREE_4:
        {
            const Tree4 &t4 = get<tree4_s>(t);
            r = append(r, "t4(");
            r = tree_show_2(t4->t[0], r, false, f);
            r = append(r, ',');
            r = append(r, f(t4->k[0]));
            r = append(r, ',');
            r = tree_show_2(t4->t[1], r, false, f);
            r = append(r, ',');
            r = append(r, f(t4->k[1]));
            r = append(r, ',');
            r = tree_show_2(t4->t[2], r, false, f);
            r = append(r, ',');
            r = append(r, f(t4->k[2]));
            r = append(r, ',');
            r = tree_show_2(t4->t[3], r, last, f);
            r = append(r, ')');
            return r;
        }
        default:
            error_bad_tree();
    }
}
#endif

static _TreeItrEntry tree_itr_entry(_Tree t, size_t offset)
{
    _TreeItrEntry entry = {offset, t};
    return entry;
}

static void tree_itr_move(_TreeItr *itr, size_t idx)
{
    if (itr->_ptr == 0)
    {
        _Tree t = _bit_cast<_Tree>(itr->_state);
        size_t depth = tree_depth(t);
        if (depth > UINT8_MAX)
            error_bad_tree();
        _TreeItrEntry *stack = (_TreeItrEntry *)gc_malloc(
            sizeof(_TreeItrEntry) * depth);
        stack[0] = tree_itr_entry(t, 0);
        itr->_state = _bit_cast<Value<Word>>(stack);
        itr->_ptr = 1;
        return;
    }

    _TreeItrEntry *stack = _bit_cast<_TreeItrEntry *>(itr->_state);
    while (true)
    {
        if (itr->_ptr == 1)
            break;
        _TreeItrEntry *entry = stack + itr->_ptr - 1;
        size_t lo = entry->_offset;
        if (idx < lo)
        {
            itr->_ptr--;
            continue;
        }
        size_t hi = lo + _tree_size(entry->_value);
        if (idx < hi)
            break;
        itr->_ptr--;
    }
}

static const Value<Word> &tree_itr_get(_TreeItr *itr)
{
    size_t idx = itr->_idx;
    _TreeItrEntry *stack = _bit_cast<_TreeItrEntry *>(itr->_state);
    while (true)
    {
        _TreeItrEntry *entry = stack + itr->_ptr - 1;
        size_t lo = entry->_offset;
        switch (index(entry->_value))
        {
            case TREE_2:
            {
                const Tree2 &t2 = entry->_value;
                size_t hi = lo + _tree_size(t2.t[0]);
                if (idx < hi)
                {
                    stack[itr->_ptr++] = tree_itr_entry(t2.t[0], lo);
                    continue;
                }
                if (idx == hi)
                    return t2.k[0];
                lo = hi + 1;
                stack[itr->_ptr++] = tree_itr_entry(t2.t[1], lo);
                continue;
            }
            case TREE_3:
            {
                const Tree3 &t3 = entry->_value;
                size_t hi = lo + _tree_size(t3.t[0]);
                if (idx < hi)
                {
                    stack[itr->_ptr++] = tree_itr_entry(t3.t[0], lo);
                    continue;
                }
                if (idx == hi)
                    return t3.k[0];
                lo = hi + 1;
                hi = lo + _tree_size(t3.t[1]);
                if (idx < hi)
                {
                    stack[itr->_ptr++] = tree_itr_entry(t3.t[1], lo);
                    continue;
                }
                if (idx == hi)
                    return t3.k[1];
                lo = hi + 1;
                stack[itr->_ptr++] = tree_itr_entry(t3.t[2], lo);
                continue;
            }
            case TREE_4:
            {
                const Tree4 &t4 = entry->_value;
                size_t hi = lo + _tree_size(t4.t[0]);
                if (idx < hi)
                {
                    stack[itr->_ptr++] = tree_itr_entry(t4.t[0], lo);
                    continue;
                }
                if (idx == hi)
                    return t4.k[0];
                lo = hi + 1;
                hi = lo + _tree_size(t4.t[1]);
                if (idx < hi)
                {
                    stack[itr->_ptr++] = tree_itr_entry(t4.t[1], lo);
                    continue;
                }
                if (idx == hi)
                    return t4.k[1];
                lo = hi + 1;
                hi = lo + _tree_size(t4.t[2]);
                if (idx < hi)
                {
                    stack[itr->_ptr++] = tree_itr_entry(t4.t[2], lo);
                    continue;
                }
                if (idx == hi)
                    return t4.k[2];
                lo = hi + 1;
                stack[itr->_ptr++] = tree_itr_entry(t4.t[3], lo);
                continue;
            }
            default:
                error_bad_tree();
        }
    }
}

extern void _tree_itr_begin(_TreeItr *itr, _Tree t)
{
    itr->_idx = 0;
    itr->_ptr = 0;
    itr->_state = _bit_cast<Value<Word>>(t);
}

extern void _tree_itr_end(_TreeItr *itr, _Tree t)
{
    itr->_idx = _tree_size(t);
    itr->_ptr = 0;
    itr->_state = _bit_cast<Value<Word>>(t);
}

extern const Value<Word> &_tree_itr_get(_TreeItr *itr)
{
    tree_itr_move(itr, itr->_idx);
    return tree_itr_get(itr);
}

}
