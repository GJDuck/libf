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
typedef _Tree tree_t;
typedef _Compare Compare;
typedef Any K;
typedef Any A;
typedef Any C;
#define TREE_EMPTY          _TREE_EMPTY

/*
 * Tree node definitions.
 */
struct tree2_s
{
    K k[1];
    tree_t t[2];
};
struct tree3_s
{
    K k[2];
    tree_t t[3];
};
struct tree4_s
{
    K k[3];
    tree_t t[4];
};
typedef struct tree2_s *tree2_t;
typedef struct tree3_s *tree3_t;
typedef struct tree4_s *tree4_t;

/*
 * Tree node types.
 */
enum tree_type_t
{
    TREE_NIL = type_index<tree_nil_s, tree_t>(),
    TREE_2   = type_index<tree2_s, tree_t>(),
    TREE_3   = type_index<tree3_s, tree_t>(),
    TREE_4   = type_index<tree4_s, tree_t>()
};

/*
 * Insert info.
 */
struct tree_insert_info_s
{
    K *v;
    int grow:1;
    int replace:1;
    int changed:1;
};
typedef struct tree_insert_info_s *tree_insert_info_t;

/*
 * Iterators.
 */
#define MAX_ITR_SIZE        48
struct entry_s
{
    uint8_t offset;
    tree_t t;
};
struct itr_s
{
    size_t ptr;
    entry_s stack[MAX_ITR_SIZE];
};
typedef struct itr_s *itr_t;

/*
 * Node construction.
 */
static inline tree_t tree2(tree_t t0, K k0, tree_t t1)
{
    tree2_s node = {{k0}, {t0, t1}};
    tree_t t = set<tree_t>(box<tree2_s>(node));
    return t;
}
static inline tree_t tree3(tree_t t0, K k0, tree_t t1, K k1, tree_t t2)
{
    tree3_s node = {{k0, k1}, {t0, t1, t2}};
    tree_t t = set<tree_t>(box<tree3_s>(node));
    return t;
}
static inline tree_t tree4(tree_t t0, K k0, tree_t t1, K k1, tree_t t2, K k2,
    tree_t t3)
{
    tree4_s node = {{k0, k1, k2}, {t0, t1, t2, t3}};
    tree_t t = set<tree_t>(box<tree4_s>(node));
    return t;
}

/*
 * Prototypes.
 */
static tree_t tree2_insert(tree2_t t, K k, tree_insert_info_t info,
    Compare compare);
static tree_t tree3_insert(tree3_t t, K k, tree_insert_info_t info,
    Compare compare);
static tree_t tree_delete_2(tree_t t, K k, K **v, Compare compare,
    bool *reduced);
static tree_t tree_delete_min_2(tree_t t, K *k, bool *reduced);
static tree_t tree_delete_max_2(tree_t t, K *k, bool *reduced);
static tree_t tree2_fix_t0(tree_t t0, K k0, tree_t t1, bool *reduced);
static tree_t tree2_fix_t1(tree_t t0, K k0, tree_t t1, bool *reduced);
static tree_t tree3_fix_t0(tree_t t0, K k0, tree_t t1, K k1, tree_t t2,
    bool *reduced);
static tree_t tree3_fix_t1(tree_t t0, K k0, tree_t t1, K k1, tree_t t2,
    bool *reduced);
static tree_t tree3_fix_t2(tree_t t0, K k0, tree_t t1, K k1, tree_t t2,
    bool *reduced);
static tree_t tree4_fix_t0(tree_t t0, K k0, tree_t t1, K k1, tree_t t2, K k2,
    tree_t t3, bool *reduced);
static tree_t tree4_fix_t1(tree_t t0, K k0, tree_t t1, K k1, tree_t t2, K k2,
    tree_t t3, bool *reduced);
static tree_t tree4_fix_t2(tree_t t0, K k0, tree_t t1, K k1, tree_t t2, K k2,
    tree_t t3, bool *reduced);
static tree_t tree4_fix_t3(tree_t t0, K k0, tree_t t1, K k1, tree_t t2, K k2,
    tree_t t3, bool *reduced);
static tree_t tree2_concat_3_min(tree2_t t, K k, tree_t u, size_t depth);
static tree_t tree3_concat_3_min(tree3_t t, K k, tree_t u, size_t depth);
static tree_t tree2_concat_3_max(tree2_t t, K k, tree_t u, size_t depth);
static tree_t tree3_concat_3_max(tree3_t t, K k, tree_t u, size_t depth);
static tree_t tree_concat_3(tree_t t, K k, tree_t u, size_t t_depth,
    size_t u_depth, size_t *depth);
static bool tree_split_2(tree_t t, K k, size_t depth, tree_t *lt, tree_t *rt,
    size_t *l_depth, size_t *r_depth, Compare compare);
static tree_t tree_union_2(tree_t t, tree_t u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare);
static tree_t tree_intersect_2(tree_t t, tree_t u, size_t t_depth,
    size_t u_depth, size_t *depth, Compare compare);
static tree_t tree_diff_2(tree_t t, tree_t u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare);
static List<C> tree_to_list_2(tree_t t, C (*f)(void *, K), void *data,
    List<C> xs);
static bool tree_verify_2(tree_t t, size_t depth);
static String tree_show_2(tree_t t, String r, bool last, String (*f)(A));

/*
 * Constructor.
 */
extern PURE tree_t _tree_singleton(K k)
{
    return tree2(TREE_EMPTY, k, TREE_EMPTY);
}

/*
 * Test if empty.
 */
extern bool tree_is_empty(tree_t t)
{
    return (index(t) == TREE_NIL);
}

/*
 * Test if singleton.
 */
extern bool tree_is_singleton(tree_t t)
{
    if (index(t) != TREE_2)
        return false;
    tree2_t t2 = get<tree2_s>(t);
    return (index(t2->t[0]) == TREE_NIL);
}

/*
 * Search.
 */
extern K *_tree_search(tree_t t, K k, Compare compare)
{
    while (true)
    {
        switch (index(t))
        {
            case TREE_NIL:
                return nullptr;
            case TREE_2:
            {
                tree2_t t2 = get<tree2_s>(t);
                int cmp = compare(k, t2->k[0]);
                if (cmp < 0)
                {
                    t = t2->t[0];
                    continue;
                }
                else if (cmp > 0)
                {
                    t = t2->t[1];
                    continue;
                }
                return &t2->k[0];
            }
            case TREE_3:
            {
                tree3_t t3 = get<tree3_s>(t);
                int cmp = compare(k, t3->k[0]);
                if (cmp < 0)
                {
                    t = t3->t[0];
                    continue;
                }
                else if (cmp > 0)
                {
                    cmp = compare(k, t3->k[1]);
                    if (cmp < 0)
                    {
                        t = t3->t[1];
                        continue;
                    }
                    else if (cmp > 0)
                    {
                        t = t3->t[2];
                        continue;
                    }
                    return &t3->k[1];
                }
                return &t3->k[0];
            }
            case TREE_4:
            {
                tree4_t t4 = get<tree4_s>(t);
                int cmp = compare(k, t4->k[1]);
                if (cmp < 0)
                {
                    cmp = compare(k, t4->k[0]);
                    if (cmp < 0)
                    {
                        t = t4->t[0];
                        continue;
                    }
                    else if (cmp > 0)
                    {
                        t = t4->t[1];
                        continue;
                    }
                    return &t4->k[0];
                }
                else if (cmp > 0)
                {
                    cmp = compare(k, t4->k[2]);
                    if (cmp < 0)
                    {
                        t = t4->t[2];
                        continue;
                    }
                    else if (cmp > 0)
                    {
                        t = t4->t[3];
                        continue;
                    }
                    return &t4->k[2];
                }
                return &t4->k[1];
            }
        }
    }
}

/*
 * Search any.
 */
extern bool tree_search_any(tree_t t, K *k)
{
    switch (index(t))
    {
        case TREE_NIL:
            return false;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            if (k != nullptr)
                *k = t2->k[0];
            return true;
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            if (k != nullptr)
                *k = t3->k[0];
            return true;
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            if (k != nullptr)
                *k = t4->k[1];
            return true;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Search min.
 */
extern bool tree_search_min(tree_t t, K *k)
{
    while (true)
    {
        switch (index(t))
        {
            case TREE_NIL:
                return false;
            case TREE_2:
            {
                tree2_t t2 = get<tree2_s>(t);
                if (index(t2->t[0]) == TREE_NIL)
                {
                    if (k != nullptr)
                        *k = t2->k[0];
                    return true;
                }
                else
                {
                    t = t2->t[0];
                    continue;
                }
            }
            case TREE_3:
            {
                tree3_t t3 = get<tree3_s>(t);
                if (index(t3->t[0]) == TREE_NIL)
                {
                    if (k != nullptr)
                        *k = t3->k[0];
                    return true;
                }
                else
                {
                    t = t3->t[0];
                    continue;
                }
            }
            case TREE_4:
            {
                tree4_t t4 = get<tree4_s>(t);
                if (index(t4->t[0]) == TREE_NIL)
                {
                    if (k != nullptr)
                        *k = t4->k[0];
                    return true;
                }
                else
                {
                    t = t4->t[0];
                    continue;
                }
            }
        }
    }
}

/*
 * Search max.
 */
extern bool tree_search_max(tree_t t, K *k)
{
    while (true)
    {
        switch (index(t))
        {
            case TREE_NIL:
                return false;
            case TREE_2:
            {
                tree2_t t2 = get<tree2_s>(t);
                if (index(t2->t[0]) == TREE_NIL)
                {
                    if (k != nullptr)
                        *k = t2->k[0];
                    return true;
                }
                else
                {
                    t = t2->t[1];
                    continue;
                }
            }
            case TREE_3:
            {
                tree3_t t3 = get<tree3_s>(t);
                if (index(t3->t[1]) == TREE_NIL)
                {
                    if (k != nullptr)
                        *k = t3->k[1];
                    return true;
                }
                else
                {
                    t = t3->t[2];
                    continue;
                }
            }
            case TREE_4:
            {
                tree4_t t4 = get<tree4_s>(t);
                if (index(t4->t[2]) == TREE_NIL)
                {
                    if (k != nullptr)
                        *k = t4->k[2];
                    return true;
                }
                else
                {
                    t = t4->t[3];
                    continue;
                }
            }
        }
    }
}

/*
 * Search (<).
 */
extern bool tree_search_lt(tree_t t, K k, K *v, Compare compare)
{
    switch (index(t))
    {
        case TREE_NIL:
            return false;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            int cmp = compare(k, t2->k[0]);
            if (cmp > 0)
            {
                if (!tree_search_lt(t2->t[1], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t2->k[0];
                    return true;
                }
                return true;
            }
            return tree_search_lt(t2->t[0], k, v, compare);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            int cmp = compare(k, t3->k[1]);
            if (cmp > 0)
            {
                if (!tree_search_lt(t3->t[2], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t3->k[1];
                    return true;
                }
                return true;
            }
            cmp = compare(k, t3->k[0]);
            if (cmp > 0)
            {
                if (!tree_search_lt(t3->t[1], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t3->k[0];
                    return true;
                }
                return true;
            }
            return tree_search_lt(t3->t[0], k, v, compare);
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            int cmp = compare(k, t4->k[1]);
            if (cmp > 0)
            {
                cmp = compare(k, t4->k[2]);
                if (cmp > 0)
                {
                    if (!tree_search_lt(t4->t[3], k, v, compare))
                    {
                        if (v != nullptr)
                            *v = t4->k[2];
                        return true;
                    }
                    return true;
                }
                if (!tree_search_lt(t4->t[2], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t4->k[1];
                    return true;
                }
                return true;
            }
            cmp = compare(k, t4->k[0]);
            if (cmp > 0)
            {
                if (!tree_search_lt(t4->t[1], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t4->k[0];
                    return true;
                }
                return true;
            }
            return tree_search_lt(t4->t[0], k, v, compare);
        }
        default:
            error_bad_tree();
    }
}

/*
 * Search (>).
 */
extern bool tree_search_gt(tree_t t, K k, K *v, Compare compare)
{
    switch (index(t))
    {
        case TREE_NIL:
            return false;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            int cmp = compare(k, t2->k[0]);
            if (cmp < 0)
            {
                if (!tree_search_gt(t2->t[0], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t2->k[0];
                    return true;
                }
                return true;
            }
            return tree_search_gt(t2->t[1], k, v, compare);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            int cmp = compare(k, t3->k[0]);
            if (cmp < 0)
            {
                if (!tree_search_gt(t3->t[0], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t3->k[0];
                    return true;
                }
                return true;
            }
            cmp = compare(k, t3->k[1]);
            if (cmp < 0)
            {
                if (!tree_search_gt(t3->t[1], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t3->k[1];
                    return true;
                }
                return true;
            }
            return tree_search_gt(t3->t[2], k, v, compare);
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            int cmp = compare(k, t4->k[1]);
            if (cmp < 0)
            {
                cmp = compare(k, t4->k[0]);
                if (cmp < 0)
                {
                    if (!tree_search_gt(t4->t[0], k, v, compare))
                    {
                        if (v != nullptr)
                            *v = t4->k[0];
                        return true;
                    }
                    return true;
                }
                if (!tree_search_gt(t4->t[1], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t4->k[1];
                    return true;
                }
                return true;
            }
            cmp = compare(k, t4->k[2]);
            if (cmp < 0)
            {
                if (!tree_search_gt(t4->t[2], k, v, compare))
                {
                    if (v != nullptr)
                        *v = t4->k[2];
                    return true;
                }
                return true;
            }
            return tree_search_gt(t4->t[3], k, v, compare);
        }
        default:
            error_bad_tree();
    }
}

/*
 * Insert.
 */
extern PURE Result<tree_t, K *> _tree_insert(tree_t t, K k, int flags,
    Compare compare)
{
    struct tree_insert_info_s info;
    info.v = nullptr;
    info.replace = ((flags & _TREE_INSERT_REPLACE_FLAG) != 0);
    info.grow    = ((flags & _TREE_INSERT_GROW_FLAG) != 0);
    info.changed = 0;
    switch (index(t))
    {
        case TREE_NIL:
            if (info.grow)
                return {tree2(TREE_EMPTY, k, TREE_EMPTY), info.v};
            else
                return {t, info.v};
        case TREE_2:
            t = tree2_insert(get<tree2_s>(t), k, &info, compare);
            break;
        case TREE_3:
            t = tree3_insert(get<tree3_s>(t), k, &info, compare);
            break;
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
            tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
            tree_t nt = tree2(lt, t4->k[1], rt);
            tree_t t1 = tree2_insert(get<tree2_s>(nt), k, &info, compare);
            if (!info.changed)
            {
                gc_free(get<tree2_s>(lt));
                gc_free(get<tree2_s>(rt));
                gc_free(get<tree2_s>(nt));
            }
            else
                t = t1;
            break;
        }
        default:
            error_bad_tree();
    }
    return {t, info.v};
}

static tree_t tree2_insert(tree2_t t, K k, tree_insert_info_t info,
    Compare compare)
{
    int cmp = compare(k, t->k[0]);
    if (index(t->t[0]) == TREE_NIL)
    {
        tree_t nil = TREE_EMPTY;
        if (cmp == 0)
        {
            info->v = &t->k[0];
            if (!info->replace)
                return set<tree_t>(t);
        }
        else 
        {
            if (!info->grow)
                return set<tree_t>(t);
        }
        info->changed = true;
        if (cmp < 0)
            return tree3(nil, k, nil, t->k[0], nil);
        else if (cmp > 0)
            return tree3(nil, t->k[0], nil, k, nil);
        else
            return tree2(nil, k, nil);
    }
    else
    {
        if (cmp < 0)
        {
            switch (index(t->t[0]))
            {
                case TREE_2:
                {
                    tree2_t t2 = get<tree2_s>(t->t[0]);
                    tree_t nt = tree2_insert(t2, k, info, compare);
                    if (info->changed)
                        return tree2(nt, t->k[0], t->t[1]);
                    else
                        return set<tree_t>(t);
                }
                case TREE_3:
                {
                    tree3_t t3 = get<tree3_s>(t->t[0]);
                    tree_t nt = tree3_insert(t3, k, info, compare);
                    if (info->changed)
                        return tree2(nt, t->k[0], t->t[1]);
                    else
                        return set<tree_t>(t);
                }
                case TREE_4:
                {
                    tree4_t t4 = get<tree4_s>(t->t[0]);
                    cmp = compare(k, t4->k[1]);
                    if (cmp == 0)
                    {
                        info->v = &t->k[0];
                        if (!info->replace)
                            return set<tree_t>(t);
                        info->changed = true;
                    }
                    tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
                    tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
                    if (cmp < 0)
                    {
                        tree_t nt =
                            tree2_insert(get<tree2_s>(lt), k, info, compare);
                        if (info->changed)
                            return tree3(nt, t4->k[1], rt, t->k[0], t->t[1]);
                        else
                        {
                            gc_free(get<tree2_s>(lt));
                            gc_free(get<tree2_s>(rt));
                            return set<tree_t>(t);
                        }
                    }
                    else if (cmp > 0)
                    {
                        tree_t nt =
                            tree2_insert(get<tree2_s>(rt), k, info, compare);
                        if (info->changed)
                            return tree3(lt, t4->k[1], nt, t->k[0], t->t[1]);
                        else
                        {
                            gc_free(get<tree2_s>(lt));
                            gc_free(get<tree2_s>(rt));
                            return set<tree_t>(t);
                        }
                    }
                    else
                        return tree3(lt, k, rt, t->k[0], t->t[1]);
                }
                default:
                    error_bad_tree();
            }
        }
        else if (cmp > 0)
        {
            switch (index(t->t[1]))
            {
                case TREE_2:
                {
                    tree2_t t2 = get<tree2_s>(t->t[1]);
                    tree_t nt = tree2_insert(t2, k, info, compare);
                    if (info->changed)
                        return tree2(t->t[0], t->k[0], nt);
                    else
                        return set<tree_t>(t);
                }
                case TREE_3:
                {
                    tree3_t t3 = get<tree3_s>(t->t[1]);
                    tree_t nt = tree3_insert(t3, k, info, compare);
                    if (info->changed)
                        return tree2(t->t[0], t->k[0], nt);
                    else
                        return set<tree_t>(t);
                }
                case TREE_4:
                {
                    tree4_t t4 = get<tree4_s>(t->t[1]);
                    cmp = compare(k, t4->k[1]);
                    if (cmp == 0)
                    {
                        info->v = &t4->k[1];
                        if (!info->replace)
                            return set<tree_t>(t);
                        info->changed = true;
                    }
                    tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
                    tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
                    if (cmp < 0)
                    {
                        tree_t nt =
                            tree2_insert(get<tree2_s>(lt), k, info, compare);
                        if (info->changed)
                            return tree3(t->t[0], t->k[0], nt, t4->k[1], rt);
                        else
                        {
                            gc_free(get<tree2_s>(lt));
                            gc_free(get<tree2_s>(rt));
                            return set<tree_t>(t);
                        }
                    }
                    else if (cmp > 0)
                    {
                        tree_t nt =
                            tree2_insert(get<tree2_s>(rt), k, info, compare);
                        if (info->changed)
                            return tree3(t->t[0], t->k[0], lt, t4->k[1], nt);
                        else
                        {
                            gc_free(get<tree2_s>(lt));
                            gc_free(get<tree2_s>(rt));
                            return set<tree_t>(t);
                        }
                    }
                    else
                        return tree3(t->t[0], t->k[0], lt, k, rt);
                }
                default:
                    error_bad_tree();
            }
        }
        else
        {
            info->v = &t->k[0];
            if (!info->replace)
                return set<tree_t>(t);
            info->changed = true;
            return tree2(t->t[0], k, t->t[1]);
        }
    }
}

static tree_t tree3_insert(tree3_t t, K k, tree_insert_info_t info,
    Compare compare)
{
    int cmp = compare(k, t->k[0]);
    if (index(t->t[0]) == TREE_NIL)
    {
        tree_t nil = TREE_EMPTY;
        if (cmp < 0)
        {
            if (!info->grow)
                return set<tree_t>(t);
            info->changed = true;
            return tree4(nil, k, nil, t->k[0], nil, t->k[1], nil);
        }
        else if (cmp > 0)
        {
            cmp = compare(k, t->k[1]);
            if (cmp == 0)
            {
                info->v = &t->k[1];
                if (!info->replace)
                    return set<tree_t>(t);
            }
            else
            {
                if (!info->grow)
                    return set<tree_t>(t);
            }
            info->changed = true;
            if (cmp < 0)
                return tree4(nil, t->k[0], nil, k, nil, t->k[1], nil);
            else if (cmp > 0)
                return tree4(nil, t->k[0], nil, t->k[1], nil, k, nil);
            else
                return tree3(nil, t->k[0], nil, k, nil);
        }
        else
        {
            info->v = &t->k[0];
            if (!info->replace)
                return set<tree_t>(t);
            info->changed = true;
            return tree3(nil, k, nil, t->k[1], nil);
        }
    }
    else
    {
        if (cmp < 0)
        {
            switch (index(t->t[0]))
            {
                case TREE_2:
                {
                    tree2_t t2 = get<tree2_s>(t->t[0]);
                    tree_t nt = tree2_insert(t2, k, info, compare);
                    if (info->changed)
                        return tree3(nt, t->k[0], t->t[1], t->k[1], t->t[2]);
                    else
                        return set<tree_t>(t);
                }
                case TREE_3:
                {
                    tree3_t t3 = get<tree3_s>(t->t[0]);
                    tree_t nt = tree3_insert(t3, k, info, compare);
                    if (info->changed)
                        return tree3(nt, t->k[0], t->t[1], t->k[1], t->t[2]);
                    else
                        return set<tree_t>(t);
                }
                case TREE_4:
                {
                    tree4_t t4 = get<tree4_s>(t->t[0]);
                    cmp = compare(k, t4->k[1]);
                    if (cmp == 0)
                    {
                        info->v = &t4->k[1];
                        if (!info->replace)
                            return set<tree_t>(t);
                        info->changed = true;
                    }
                    tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
                    tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
                    if (cmp < 0)
                    {
                        tree_t nt =
                            tree2_insert(get<tree2_s>(lt), k, info, compare);
                        if (info->changed)
                            return tree4(nt, t4->k[1], rt, t->k[0], t->t[1],
                                t->k[1], t->t[2]);
                        else
                        {
                            gc_free(get<tree2_s>(lt));
                            gc_free(get<tree2_s>(rt));
                            return set<tree_t>(t);
                        }
                    }
                    else if (cmp > 0)
                    {
                        tree_t nt =
                            tree2_insert(get<tree2_s>(rt), k, info, compare);
                        if (info->changed)
                            return tree4(lt, t4->k[1], nt, t->k[0], t->t[1],
                                t->k[1], t->t[2]);
                        else
                        {
                            gc_free(get<tree2_s>(lt));
                            gc_free(get<tree2_s>(rt));
                            return set<tree_t>(t);
                        }
                    }
                    else
                        return tree4(lt, k, rt, t->k[0], t->t[1], t->k[1],
                            t->t[2]);
                }
                default:
                    error_bad_tree();
            }
        }
        else if (cmp > 0)
        {
            cmp = compare(k, t->k[1]);
            if (cmp < 0)
            {
                switch (index(t->t[1]))
                {
                    case TREE_2:
                    {
                        tree2_t t2 = get<tree2_s>(t->t[1]);
                        tree_t nt = tree2_insert(t2, k, info, compare);
                        if (info->changed)
                            return tree3(t->t[0], t->k[0], nt, t->k[1],
                                t->t[2]);
                        else
                            return set<tree_t>(t);
                    }
                    case TREE_3:
                    {
                        tree3_t t3 = get<tree3_s>(t->t[1]);
                        tree_t nt = tree3_insert(t3, k, info, compare);
                        if (info->changed)
                            return tree3(t->t[0], t->k[0], nt, t->k[1],
                                t->t[2]);
                        else
                            return set<tree_t>(t);
                    }
                    case TREE_4:
                    {
                        tree4_t t4 = get<tree4_s>(t->t[1]);
                        cmp = compare(k, t4->k[1]);
                        if (cmp == 0)
                        {
                            info->v = &t->k[1];
                            if (!info->replace)
                                return set<tree_t>(t);
                            info->changed = true;
                        }
                        tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
                        tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
                        if (cmp < 0)
                        {
                            tree_t nt = tree2_insert(get<tree2_s>(lt), k,
                                info, compare);
                            if (info->changed)
                                return tree4(t->t[0], t->k[0], nt, t4->k[1],
                                    rt, t->k[1], t->t[2]);
                            else
                            {
                                gc_free(get<tree2_s>(lt));
                                gc_free(get<tree2_s>(rt));
                                return set<tree_t>(t);
                            }
                        }
                        else if (cmp > 0)
                        {
                            tree_t nt = tree2_insert(get<tree2_s>(rt), k,
                                info, compare);
                            if (info->changed)
                                return tree4(t->t[0], t->k[0], lt, t4->k[1],
                                    nt, t->k[1], t->t[2]);
                            else
                            {
                                gc_free(get<tree2_s>(lt));
                                gc_free(get<tree2_s>(rt));
                                return set<tree_t>(t);
                            }
                        }
                        else
                            return tree4(t->t[0], t->k[0], lt, k, rt, t->k[1],
                                t->t[2]);
                    }
                    default:
                        error_bad_tree();
                }
            }
            else if (cmp > 0)
            {
                switch (index(t->t[2]))
                {
                    case TREE_2:
                    {
                        tree2_t t2 = get<tree2_s>(t->t[2]);
                        tree_t nt = tree2_insert(t2, k, info, compare);
                        if (info->changed)
                            return tree3(t->t[0], t->k[0], t->t[1], t->k[1],
                                nt);
                        else
                            return set<tree_t>(t);
                    }
                    case TREE_3:
                    {
                        tree3_t t3 = get<tree3_s>(t->t[2]);
                        tree_t nt = tree3_insert(t3, k, info, compare);
                        if (info->changed)
                            return tree3(t->t[0], t->k[0], t->t[1], t->k[1],
                                nt);
                        else
                            return set<tree_t>(t);
                    }
                    case TREE_4:
                    {
                        tree4_t t4 = get<tree4_s>(t->t[2]);
                        cmp = compare(k, t4->k[1]);
                        if (cmp == 0)
                        {
                            info->v = &t->k[1];
                            if (!info->replace)
                                return set<tree_t>(t);
                            info->changed = true;
                        }
                        tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
                        tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
                        if (cmp < 0)
                        {
                            tree_t nt = tree2_insert(get<tree2_s>(lt), k,
                                info, compare);
                            if (info->changed)
                                return tree4(t->t[0], t->k[0], t->t[1],
                                    t->k[1], nt, t4->k[1], rt);
                            else
                            {
                                gc_free(get<tree2_s>(lt));
                                gc_free(get<tree2_s>(rt));
                                return set<tree_t>(t);
                            }
                        }
                        else if (cmp > 0)
                        {
                            tree_t nt = tree2_insert(get<tree2_s>(rt), k,
                                info, compare);
                            if (info->changed)
                                return tree4(t->t[0], t->k[0], t->t[1],
                                    t->k[1], lt, t4->k[1], nt);
                            else
                            {
                                gc_free(get<tree2_s>(lt));
                                gc_free(get<tree2_s>(rt));
                                return set<tree_t>(t);
                            }
                        }
                        else
                            return tree4(t->t[0], t->k[0], t->t[1], t->k[1],
                                lt, k, rt);
                    }
                    default:
                        error_bad_tree();
                }
            }
            else
            {
                info->v = &t->k[1];
                if (!info->replace)
                    return set<tree_t>(t);
                info->changed = true;
                return tree3(t->t[0], t->k[0], t->t[1], k, t->t[2]);
            }
        }
        else
        {
            info->v = &t->k[0];
            if (!info->replace)
                return set<tree_t>(t);
            info->changed = true;
            return tree3(t->t[0], k, t->t[1], t->k[1], t->t[2]);
        }
    }
}

/*
 * Delete.
 */
extern Result<tree_t, K *> _tree_delete(tree_t t, K k, Compare compare)
{
    bool reduced = false;
    K *v = nullptr;
    t = tree_delete_2(t, k, &v, compare, &reduced);
    return {t, v};
}

static tree_t tree_delete_2(tree_t t, K k, K **v, Compare compare,
    bool *reduced)
{
    switch (index(t))
    {
        case TREE_NIL:
            *reduced = false;
            return t;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            int cmp = compare(k, t2->k[0]);
            if (cmp < 0)
            {
                tree_t nt = tree_delete_2(t2->t[0], k, v, compare, reduced);
                return tree2_fix_t0(nt, t2->k[0], t2->t[1], reduced);
            }
            else if (cmp > 0)
            {
                tree_t nt = tree_delete_2(t2->t[1], k, v, compare, reduced);
                return tree2_fix_t1(t2->t[0], t2->k[0], nt, reduced);
            }
            else
            {
                if (v != nullptr)
                    *v = &t2->k[0];
                if (index(t2->t[1]) == TREE_NIL)
                {
                    *reduced = true;
                    return TREE_EMPTY;
                }
                else
                {
                    K ks;
                    tree_t nt = tree_delete_min_2(t2->t[1], &ks, reduced);
                    return tree2_fix_t1(t2->t[0], ks, nt, reduced);
                }
            }
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            int cmp = compare(k, t3->k[0]);
            if (cmp < 0)
            {
                tree_t nt = tree_delete_2(t3->t[0], k, v, compare, reduced);
                return tree3_fix_t0(nt, t3->k[0], t3->t[1], t3->k[1],
                    t3->t[2], reduced);
            }
            else if (cmp > 0)
            {
                cmp = compare(k, t3->k[1]);
                if (cmp < 0)
                {
                    tree_t nt = tree_delete_2(t3->t[1], k, v, compare,
                        reduced);
                    return tree3_fix_t1(t3->t[0], t3->k[0], nt, t3->k[1],
                        t3->t[2], reduced);
                }
                else if (cmp > 0)
                {
                    tree_t nt = tree_delete_2(t3->t[2], k, v, compare,
                        reduced);
                    return tree3_fix_t2(t3->t[0], t3->k[0], t3->t[1],
                        t3->k[1], nt, reduced);
                }
                else
                {
                    if (v != nullptr)
                        *v = &t3->k[1];
                    if (index(t3->t[2]) == TREE_NIL)
                    {
                        tree_t nil = TREE_EMPTY;
                        return tree2(nil, t3->k[0], nil);
                    }
                    else
                    {
                        K ks;
                        tree_t nt = tree_delete_min_2(t3->t[2], &ks, reduced);
                        return tree3_fix_t2(t3->t[0], t3->k[0], t3->t[1],
                            ks, nt, reduced);
                    }
                }
            }
            else
            {
                if (v != nullptr)
                    *v = &t3->k[0];
                if (index(t3->t[1]) == TREE_NIL)
                {
                    tree_t nil = TREE_EMPTY;
                    return tree2(nil, t3->k[1], nil);
                }
                else
                {
                    K ks;
                    tree_t nt = tree_delete_min_2(t3->t[1], &ks, reduced);
                    return tree3_fix_t1(t3->t[0], ks, nt, t3->k[1], t3->t[2],
                        reduced);
                }
            }
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            int cmp = compare(k, t4->k[1]);
            if (cmp < 0)
            {
                cmp = compare(k, t4->k[0]);
                if (cmp < 0)
                {
                    tree_t nt = tree_delete_2(t4->t[0], k, v, compare,
                        reduced);
                    return tree4_fix_t0(nt, t4->k[0], t4->t[1], t4->k[1],
                        t4->t[2], t4->k[2], t4->t[3], reduced);
                }
                else if (cmp > 0)
                {
                    tree_t nt = tree_delete_2(t4->t[1], k, v, compare,
                        reduced);
                    return tree4_fix_t1(t4->t[0], t4->k[0], nt, t4->k[1],
                        t4->t[2], t4->k[2], t4->t[3], reduced);
                }
                else
                {
                    if (v != nullptr)
                        *v = &t4->k[0];
                    if (index(t4->t[1]) == TREE_NIL)
                    {
                        tree_t nil = TREE_EMPTY;
                        return tree3(nil, t4->k[1], nil, t4->k[2], nil);
                    }
                    else
                    {
                        K ks;
                        tree_t nt = tree_delete_min_2(t4->t[1], &ks, reduced);
                        return tree4_fix_t1(t4->t[0], ks, nt, t4->k[1],
                            t4->t[2], t4->k[2], t4->t[3], reduced);
                    }
                }
            }
            else if (cmp > 0)
            {
                cmp = compare(k, t4->k[2]);
                if (cmp < 0)
                {
                    tree_t nt = tree_delete_2(t4->t[2], k, v, compare,
                        reduced);
                    return tree4_fix_t2(t4->t[0], t4->k[0], t4->t[1],
                        t4->k[1], nt, t4->k[2], t4->t[3], reduced);
                }
                else if (cmp > 0)
                {
                    tree_t nt = tree_delete_2(t4->t[3], k, v, compare,
                        reduced);
                    return tree4_fix_t3(t4->t[0], t4->k[0], t4->t[1],
                        t4->k[1], t4->t[2], t4->k[2], nt, reduced);
                }
                else
                {
                    if (v != nullptr)
                        *v = &t4->k[2];
                    if (index(t4->t[3]) == TREE_NIL)
                    {
                        tree_t nil = TREE_EMPTY;
                        return tree3(nil, t4->k[0], nil, t4->k[1], nil);
                    }
                    else
                    {
                        K ks;
                        tree_t nt = tree_delete_min_2(t4->t[3], &ks, reduced);
                        return tree4_fix_t3(t4->t[0], t4->k[0], t4->t[1],
                            t4->k[1], t4->t[2], ks, nt, reduced);
                    }
                }
            }
            else
            {
                if (v != nullptr)
                    *v = &t4->k[1];
                if (index(t4->t[2]) == TREE_NIL)
                {
                    tree_t nil = TREE_EMPTY;
                    return tree3(nil, t4->k[0], nil, t4->k[2], nil);
                }
                else
                {
                    K ks;
                    tree_t nt = tree_delete_min_2(t4->t[2], &ks, reduced);
                    return tree4_fix_t2(t4->t[0], t4->k[0], t4->t[1], ks, nt,
                        t4->k[2], t4->t[3], reduced);
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
extern tree_t tree_delete_min(tree_t t, K *k)
{
    bool reduced = false;
    return tree_delete_min_2(t, k, &reduced);
}

static tree_t tree_delete_min_2(tree_t t, K *k, bool *reduced)
{
    switch (index(t))
    {
        case TREE_NIL:
            *reduced = false;
            return t;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            if (index(t2->t[0]) == TREE_NIL)
            {
                *reduced = true;
                if (k != nullptr)
                    *k = t2->k[0];
                return TREE_EMPTY;
            }
            else
            {
                tree_t nt = tree_delete_min_2(t2->t[0], k, reduced);
                return tree2_fix_t0(nt, t2->k[0], t2->t[1], reduced);
            }
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            if (index(t3->t[0]) == TREE_NIL)
            {
                if (k != nullptr)
                    *k = t3->k[0];
                tree_t nil = TREE_EMPTY;
                return tree2(nil, t3->k[1], nil);
            }
            else
            {
                tree_t nt = tree_delete_min_2(t3->t[0], k, reduced);
                return tree3_fix_t0(nt, t3->k[0], t3->t[1], t3->k[1],
                    t3->t[2], reduced);
            }
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            if (index(t4->t[0]) == TREE_NIL)
            {
                if (k != nullptr)
                    *k = t4->k[0];
                tree_t nil = TREE_EMPTY;
                return tree3(nil, t4->k[1], nil, t4->k[2], nil);
            }
            else
            {
                tree_t nt = tree_delete_min_2(t4->t[0], k, reduced);
                return tree4_fix_t0(nt, t4->k[0], t4->t[1], t4->k[1],
                    t4->t[2], t4->k[2], t4->t[3], reduced);
            }
        }
        default:
            error_bad_tree();
    }
}

/*
 * Delete max.
 */
extern tree_t tree_delete_max(tree_t t, K *k)
{
    bool reduced = false;
    return tree_delete_max_2(t, k, &reduced);
}

static tree_t tree_delete_max_2(tree_t t, K *k, bool *reduced)
{
    switch (index(t))
    {
        case TREE_NIL:
            *reduced = false;
            return t;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            if (index(t2->t[1]) == TREE_NIL)
            {
                *reduced = true;
                if (k != nullptr)
                    *k = t2->k[0];
                return TREE_EMPTY;
            }
            else
            {
                tree_t nt = tree_delete_max_2(t2->t[1], k, reduced);
                return tree2_fix_t1(t2->t[0], t2->k[0], nt, reduced);
            }
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            if (index(t3->t[2]) == TREE_NIL)
            {
                if (k != nullptr)
                    *k = t3->k[1];
                tree_t nil = TREE_EMPTY;
                return tree2(nil, t3->k[0], nil);
            }
            else
            {
                tree_t nt = tree_delete_max_2(t3->t[2], k, reduced);
                return tree3_fix_t2(t3->t[0], t3->k[0], t3->t[1], t3->k[1],
                    nt, reduced);
            }
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            if (index(t4->t[3]) == TREE_NIL)
            {
                if (k != nullptr)
                    *k = t4->k[2];
                tree_t nil = TREE_EMPTY;
                return tree3(nil, t4->k[0], nil, t4->k[1], nil);
            }
            else
            {
                tree_t nt = tree_delete_max_2(t4->t[3], k, reduced);
                return tree4_fix_t3(t4->t[0], t4->k[0], t4->t[1], t4->k[1],
                    t4->t[2], t4->k[2], nt, reduced);
            }
        }
        default:
            error_bad_tree();
    }
}

static tree_t tree2_fix_t0(tree_t t0, K k0, tree_t t1, bool *reduced)
{
    if (*reduced)
    {
        switch (index(t1))
        {
            case TREE_2:
            {
                tree2_t t12 = get<tree2_s>(t1);
                return tree3(t0, k0, t12->t[0], t12->k[0], t12->t[1]);
            }
            case TREE_3:
            {
                *reduced = false;
                tree3_t t13 = get<tree3_s>(t1);
                tree_t nt1 = tree2(t13->t[1], t13->k[1], t13->t[2]);
                tree_t nt0 = tree2(t0, k0, t13->t[0]);
                return tree2(nt0, t13->k[0], nt1);
            }
            case TREE_4:
            {
                *reduced = false;
                tree4_t t14 = get<tree4_s>(t1);
                tree_t nt1 = tree3(t14->t[1], t14->k[1], t14->t[2],
                    t14->k[2], t14->t[3]);
                tree_t nt0 = tree2(t0, k0, t14->t[0]);
                return tree2(nt0, t14->k[0], nt1);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree2(t0, k0, t1);
}

static tree_t tree2_fix_t1(tree_t t0, K k0, tree_t t1, bool *reduced)
{
    if (*reduced)
    {
        switch (index(t0))
        {
            case TREE_2:
            {
                tree2_t t02 = get<tree2_s>(t0);
                return tree3(t02->t[0], t02->k[0], t02->t[1], k0, t1);
            }
            case TREE_3:
            {
                *reduced = false;
                tree3_t t03 = get<tree3_s>(t0);
                tree_t nt0 = tree2(t03->t[0], t03->k[0], t03->t[1]);
                tree_t nt1 = tree2(t03->t[2], k0, t1);
                return tree2(nt0, t03->k[1], nt1);
            }
            case TREE_4:
            {
                *reduced = false;
                tree4_t t04 = get<tree4_s>(t0);
                tree_t nt0 = tree3(t04->t[0], t04->k[0], t04->t[1], t04->k[1],
                    t04->t[2]);
                tree_t nt1 = tree2(t04->t[3], k0, t1);
                return tree2(nt0, t04->k[2], nt1);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree2(t0, k0, t1);
}

static tree_t tree3_fix_t0(tree_t t0, K k0, tree_t t1, K k1, tree_t t2,
    bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t1))
        {
            case TREE_2:
            {
                tree2_t t12 = get<tree2_s>(t1);
                tree_t nt1 = tree3(t0, k0, t12->t[0], t12->k[0], t12->t[1]);
                return tree2(nt1, k1, t2);
            }
            case TREE_3:
            {
                tree3_t t13 = get<tree3_s>(t1);
                tree_t nt1 = tree2(t13->t[1], t13->k[1], t13->t[2]);
                tree_t nt0 = tree2(t0, k0, t13->t[0]);
                return tree3(nt0, t13->k[0], nt1, k1, t2);
            }
            case TREE_4:
            {
                tree4_t t14 = get<tree4_s>(t1);
                tree_t nt1 = tree3(t14->t[1], t14->k[1], t14->t[2],
                    t14->k[2], t14->t[3]);
                tree_t nt0 = tree2(t0, k0, t14->t[0]);
                return tree3(nt0, t14->k[0], nt1, k1, t2);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree3(t0, k0, t1, k1, t2);
}

static tree_t tree3_fix_t1(tree_t t0, K k0, tree_t t1, K k1, tree_t t2,
    bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t0))
        {
            case TREE_2:
            {
                tree2_t t02 = get<tree2_s>(t0);
                tree_t nt0 = tree3(t02->t[0], t02->k[0], t02->t[1], k0, t1);
                return tree2(nt0, k1, t2);
            }
            case TREE_3:
            {
                tree3_t t03 = get<tree3_s>(t0);
                tree_t nt0 = tree2(t03->t[0], t03->k[0], t03->t[1]);
                tree_t nt1 = tree2(t03->t[2], k0, t1);
                return tree3(nt0, t03->k[1], nt1, k1, t2);
            }
            case TREE_4:
            {
                tree4_t t04 = get<tree4_s>(t0);
                tree_t nt0 = tree3(t04->t[0], t04->k[0], t04->t[1],
                    t04->k[1], t04->t[2]);
                tree_t nt1 = tree2(t04->t[3], k0, t1);
                return tree3(nt0, t04->k[2], nt1, k1, t2);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree3(t0, k0, t1, k1, t2);
}

static tree_t tree3_fix_t2(tree_t t0, K k0, tree_t t1, K k1, tree_t t2,
    bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t1))
        {
            case TREE_2:
            {
                tree2_t t12 = get<tree2_s>(t1);
                tree_t nt1 = tree3(t12->t[0], t12->k[0], t12->t[1], k1, t2);
                return tree2(t0, k0, nt1);
            }
            case TREE_3:
            {
                tree3_t t13 = get<tree3_s>(t1);
                tree_t nt1 = tree2(t13->t[0], t13->k[0], t13->t[1]);
                tree_t nt2 = tree2(t13->t[2], k1, t2);
                return tree3(t0, k0, nt1, t13->k[1], nt2);
            }
            case TREE_4:
            {
                tree4_t t14 = get<tree4_s>(t1);
                tree_t nt1 = tree3(t14->t[0], t14->k[0], t14->t[1],
                    t14->k[1], t14->t[2]);
                tree_t nt2 = tree2(t14->t[3], k1, t2);
                return tree3(t0, k0, nt1, t14->k[2], nt2);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree3(t0, k0, t1, k1, t2);
}

static tree_t tree4_fix_t0(tree_t t0, K k0, tree_t t1, K k1, tree_t t2, K k2,
    tree_t t3, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t1))
        {
            case TREE_2:
            {
                tree2_t t12 = get<tree2_s>(t1);
                tree_t nt1 = tree3(t0, k0, t12->t[0], t12->k[0], t12->t[1]);
                return tree3(nt1, k1, t2, k2, t3);
            }
            case TREE_3:
            {
                tree3_t t13 = get<tree3_s>(t1);
                tree_t nt1 = tree2(t13->t[1], t13->k[1], t13->t[2]);
                tree_t nt0 = tree2(t0, k0, t13->t[0]);
                return tree4(nt0, t13->k[0], nt1, k1, t2, k2, t3);
            }
            case TREE_4:
            {
                tree4_t t14 = get<tree4_s>(t1);
                tree_t nt1 = tree3(t14->t[1], t14->k[1], t14->t[2], t14->k[2],
                    t14->t[3]);
                tree_t nt0 = tree2(t0, k0, t14->t[0]);
                return tree4(nt0, t14->k[0], nt1, k1, t2, k2, t3);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree4(t0, k0, t1, k1, t2, k2, t3);
}

static tree_t tree4_fix_t1(tree_t t0, K k0, tree_t t1, K k1, tree_t t2, K k2,
    tree_t t3, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t2))
        {
            case TREE_2:
            {
                tree2_t t22 = get<tree2_s>(t2);
                tree_t nt2 = tree3(t1, k1, t22->t[0], t22->k[0], t22->t[1]);
                return tree3(t0, k0, nt2, k2, t3);
            }
            case TREE_3:
            {
                tree3_t t23 = get<tree3_s>(t2);
                tree_t nt2 = tree2(t23->t[1], t23->k[1], t23->t[2]);
                tree_t nt1 = tree2(t1, k1, t23->t[0]);
                return tree4(t0, k0, nt1, t23->k[0], nt2, k2, t3);
            }
            case TREE_4:
            {
                tree4_t t24 = get<tree4_s>(t2);
                tree_t nt2 = tree3(t24->t[1], t24->k[1], t24->t[2],
                    t24->k[2], t24->t[3]);
                tree_t nt1 = tree2(t1, k1, t24->t[0]);
                return tree4(t0, k0, nt1, t24->k[0], nt2, k2, t3);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree4(t0, k0, t1, k1, t2, k2, t3);
}

static tree_t tree4_fix_t2(tree_t t0, K k0, tree_t t1, K k1, tree_t t2, K k2,
    tree_t t3, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t3))
        {
            case TREE_2:
            {
                tree2_t t32 = get<tree2_s>(t3);
                tree_t nt3 = tree3(t2, k2, t32->t[0], t32->k[0], t32->t[1]);
                return tree3(t0, k0, t1, k1, nt3);
            }
            case TREE_3:
            {
                tree3_t t33 = get<tree3_s>(t3);
                tree_t nt3 = tree2(t33->t[1], t33->k[1], t33->t[2]);
                tree_t nt2 = tree2(t2, k2, t33->t[0]);
                return tree4(t0, k0, t1, k1, nt2, t33->k[0], nt3);
            }
            case TREE_4:
            {
                tree4_t t34 = get<tree4_s>(t3);
                tree_t nt3 = tree3(t34->t[1], t34->k[1], t34->t[2],
                    t34->k[2], t34->t[3]);
                tree_t nt2 = tree2(t2, k2, t34->t[0]);
                return tree4(t0, k0, t1, k1, nt2, t34->k[0], nt3);
            }
            default:
                error_bad_tree();
        }
    }
    else
        return tree4(t0, k0, t1, k1, t2, k2, t3);
}

static tree_t tree4_fix_t3(tree_t t0, K k0, tree_t t1, K k1, tree_t t2, K k2,
    tree_t t3, bool *reduced)
{
    if (*reduced)
    {
        *reduced = false;
        switch (index(t2))
        {
            case TREE_2:
            {
                tree2_t t22 = get<tree2_s>(t2);
                tree_t nt2 = tree3(t22->t[0], t22->k[0], t22->t[1], k2, t3);
                return tree3(t0, k0, t1, k1, nt2);
            }
            case TREE_3:
            {
                tree3_t t23 = get<tree3_s>(t2);
                tree_t nt2 = tree2(t23->t[0], t23->k[0], t23->t[1]);
                tree_t nt3 = tree2(t23->t[2], k2, t3);
                return tree4(t0, k0, t1, k1, nt2, t23->k[1], nt3);
            }
            case TREE_4:
            {
                tree4_t t24 = get<tree4_s>(t2);
                tree_t nt2 = tree3(t24->t[0], t24->k[0], t24->t[1],
                    t24->k[1], t24->t[2]);
                tree_t nt3 = tree2(t24->t[3], k2, t3);
                return tree4(t0, k0, t1, k1, nt2, t24->k[2], nt3);
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
extern size_t _tree_size(tree_t t)
{
    switch (index(t))
    {
        case TREE_NIL:
            return 0;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            return 1 + _tree_size(t2->t[0]) + _tree_size(t2->t[1]);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            return 2 + _tree_size(t3->t[0]) + _tree_size(t3->t[1]) +
                _tree_size(t3->t[2]);
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            return 3 + _tree_size(t4->t[0]) + _tree_size(t4->t[1]) +
                _tree_size(t4->t[2]) + _tree_size(t4->t[3]);
        }
        default:
            error_bad_tree();
    }
}

/*
 * Depth.
 */
extern size_t tree_depth(tree_t t)
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
                tree2_t t2 = get<tree2_s>(t);
                t = t2->t[0];
                continue;
            }
            case TREE_3:
            {
                depth++;
                tree3_t t3 = get<tree3_s>(t);
                t = t3->t[0];
                continue;
            }
            case TREE_4:
            {
                depth++;
                tree4_t t4 = get<tree4_s>(t);
                t = t4->t[0];
                continue;
            }
        }
    }
}

/*
 * Concat.
 */
static tree_t tree2_concat_3_min(tree2_t t, K k, tree_t u, size_t depth)
{
    if (depth == 1)
        return tree3(u, k, t->t[0], t->k[0], t->t[1]);
    switch (index(t->t[0]))
    {
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t->t[0]);
            tree_t nt = tree2_concat_3_min(t2, k, u, depth-1);
            return tree2(nt, t->k[0], t->t[1]);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t->t[0]);
            tree_t nt = tree3_concat_3_min(t3, k, u, depth-1);
            return tree2(nt, t->k[0], t->t[1]);
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t->t[0]);
            tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
            tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
            tree_t nt = tree2_concat_3_min(get<tree2_s>(lt), k, u, depth-1);
            return tree3(nt, t4->k[1], rt, t->k[0], t->t[1]);
        }
        default:
            error_bad_tree();
    }
}

static tree_t tree3_concat_3_min(tree3_t t, K k, tree_t u, size_t depth)
{
    if (depth == 1)
        return tree4(u, k, t->t[0], t->k[0], t->t[1], t->k[1], t->t[2]);
    switch (index(t->t[0]))
    {
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t->t[0]);
            tree_t nt = tree2_concat_3_min(t2, k, u, depth-1);
            return tree3(nt, t->k[0], t->t[1], t->k[1], t->t[2]);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t->t[0]);
            tree_t nt = tree3_concat_3_min(t3, k, u, depth-1);
            return tree3(nt, t->k[0], t->t[1], t->k[1],  t->t[2]);
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t->t[0]);
            tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
            tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
            tree_t nt = tree2_concat_3_min(get<tree2_s>(lt), k, u, depth-1);
            return tree4(nt, t4->k[1], rt, t->k[0], t->t[1], t->k[1], t->t[2]);
        }
        default:
            error_bad_tree();
    }
}

static tree_t tree2_concat_3_max(tree2_t t, K k, tree_t u, size_t depth)
{
    if (depth == 1)
        return tree3(t->t[0], t->k[0], t->t[1], k, u);
    switch (index(t->t[1]))
    {
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t->t[1]);
            tree_t nt = tree2_concat_3_max(t2, k, u, depth-1);
            return tree2(t->t[0], t->k[0], nt);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t->t[1]);
            tree_t nt = tree3_concat_3_max(t3, k, u, depth-1);
            return tree2(t->t[0], t->k[0], nt);
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t->t[1]);
            tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
            tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
            tree_t nt = tree2_concat_3_max(get<tree2_s>(rt), k, u, depth-1);
            return tree3(t->t[0], t->k[0], lt, t4->k[1], nt);
        }
        default:
            error_bad_tree();
    }
}

static tree_t tree3_concat_3_max(tree3_t t, K k, tree_t u, size_t depth)
{
    if (depth == 1)
        return tree4(t->t[0], t->k[0], t->t[1], t->k[1], t->t[2], k, u);
    switch (index(t->t[2]))
    {
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t->t[2]);
            tree_t nt = tree2_concat_3_max(t2, k, u, depth-1);
            return tree3(t->t[0], t->k[0], t->t[1], t->k[1], nt);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t->t[2]);
            tree_t nt = tree3_concat_3_max(t3, k, u, depth-1);
            return tree3(t->t[0], t->k[0], t->t[1], t->k[1],  nt);
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t->t[2]);
            tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
            tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
            tree_t nt = tree2_concat_3_max(get<tree2_s>(rt), k, u, depth-1);
            return tree4(t->t[0], t->k[0], t->t[1], t->k[1], lt, t4->k[1], nt);
        }
        default:
            error_bad_tree();
    }
}

static tree_t tree_concat_3(tree_t t, K k, tree_t u, size_t t_depth,
    size_t u_depth, size_t *depth)
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
                return tree2_concat_3_min(get<tree2_s>(u), k, t, u_depth - t_depth);
            case TREE_3:
                return tree3_concat_3_min(get<tree3_s>(u), k, t, u_depth - t_depth);
            case TREE_4:
            {
                *depth = u_depth + 1;
                tree4_t u4 = get<tree4_s>(u);
                tree_t lu = tree2(u4->t[0], u4->k[0], u4->t[1]);
                tree_t ru = tree2(u4->t[2], u4->k[2], u4->t[3]);
                tree_t nu = tree2(lu, u4->k[1], ru);
                return tree2_concat_3_min(get<tree2_s>(nu), k, t,
                    u_depth - t_depth + 1);
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
                return tree2_concat_3_max(get<tree2_s>(t), k, u, t_depth - u_depth);
            case TREE_3:
                return tree3_concat_3_max(get<tree3_s>(t), k, u, t_depth - u_depth);
            case TREE_4:
            {
                *depth = t_depth + 1;
                tree4_t t4 = get<tree4_s>(t);
                tree_t lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
                tree_t rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
                tree_t nt = tree2(lt, t4->k[1], rt);
                return tree2_concat_3_max(get<tree2_s>(nt), k, u,
                    t_depth - u_depth + 1);
            }
            default:
                error_bad_tree();
        }
    }
}

static tree_t tree_concat(tree_t t, tree_t u, size_t t_depth, size_t u_depth,
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
extern PURE Result<tree_t, tree_t> _tree_split(tree_t t, K k,
    Compare compare)
{
    size_t depth = tree_depth(t), l_depth, r_depth;
    tree_t lt = TREE_EMPTY, rt = TREE_EMPTY;
    tree_split_2(t, k, depth, &lt, &rt, &l_depth, &r_depth, compare);
    return {lt, rt};
}

static bool tree_split_2(tree_t t, K k, size_t depth, tree_t *lt, tree_t *rt,
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
            tree2_t t2 = get<tree2_s>(t);
            int cmp = compare(k, t2->k[0]);
            bool r = true;
            if (cmp < 0)
            {
                r = tree_split_2(t2->t[0], k, depth-1, lt, rt, l_depth,
                    r_depth, compare);
                *rt = tree_concat_3(*rt, t2->k[0], t2->t[1], *r_depth, depth-1,
                    r_depth);
            }
            else if (cmp > 0)
            {
                r = tree_split_2(t2->t[1], k, depth-1, lt, rt, l_depth,
                    r_depth, compare);
                *lt = tree_concat_3(t2->t[0], t2->k[0], *lt, depth-1, *l_depth,
                    l_depth);
            }
            else
            {
                *lt = t2->t[0];
                *rt = t2->t[1];
                *l_depth = depth-1;
                *r_depth = depth-1;
            }
            return r;
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            int cmp = compare(k, t3->k[0]);
            bool r = true;
            if (cmp < 0)
            {
                r = tree_split_2(t3->t[0], k, depth-1, lt, rt, l_depth,
                    r_depth, compare);
                tree_t nt = tree2(t3->t[1], t3->k[1], t3->t[2]);
                *rt = tree_concat_3(*rt, t3->k[0], nt, *r_depth, depth,
                    r_depth);
            }
            else if (cmp > 0)
            {
                cmp = compare(k, t3->k[1]);
                if (cmp < 0)
                {
                    r = tree_split_2(t3->t[1], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    *lt = tree_concat_3(t3->t[0], t3->k[0], *lt, depth-1,
                        *l_depth, l_depth);
                    *rt = tree_concat_3(*rt, t3->k[1], t3->t[2], *r_depth,
                        depth-1, r_depth);
                }
                else if (cmp > 0)
                {
                    r = tree_split_2(t3->t[2], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    tree_t nt = tree2(t3->t[0], t3->k[0], t3->t[1]);
                    *lt = tree_concat_3(nt, t3->k[1], *lt, depth, *l_depth,
                        l_depth);
                }
                else
                {
                    *lt = tree2(t3->t[0], t3->k[0], t3->t[1]);
                    *rt = t3->t[2];
                    *l_depth = depth;
                    *r_depth = depth-1;
                }
            }
            else
            {
                *lt = t3->t[0];
                *rt = tree2(t3->t[1], t3->k[1], t3->t[2]);
                *l_depth = depth-1;
                *r_depth = depth;
            }
            return r;
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            int cmp = compare(k, t4->k[1]);
            bool r = true;
            if (cmp < 0)
            {
                cmp = compare(k, t4->k[0]);
                if (cmp < 0)
                {
                    r = tree_split_2(t4->t[0], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    tree_t nt = tree3(t4->t[1], t4->k[1], t4->t[2], t4->k[2],
                        t4->t[3]);
                    *rt = tree_concat_3(*rt, t4->k[0], nt, *r_depth, depth,
                        r_depth);
                }
                else if (cmp > 0)
                {
                    r = tree_split_2(t4->t[1], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    tree_t nt = tree2(t4->t[2], t4->k[2], t4->t[3]);
                    *lt = tree_concat_3(t4->t[0], t4->k[0], *lt, depth-1,
                        *l_depth, l_depth);
                    *rt = tree_concat_3(*rt, t4->k[1], nt, *r_depth, depth,
                        r_depth);
                }
                else
                {
                    *lt = t4->t[0];
                    *rt = tree3(t4->t[1], t4->k[1], t4->t[2], t4->k[2],
                        t4->t[3]);
                    *l_depth = depth-1;
                    *r_depth = depth;
                }
            }
            else if (cmp > 0)
            {
                cmp = compare(k, t4->k[2]);
                if (cmp < 0)
                {
                    r = tree_split_2(t4->t[2], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    tree_t nt = tree2(t4->t[0], t4->k[0], t4->t[1]);
                    *lt = tree_concat_3(nt, t4->k[1], *lt, depth, *l_depth,
                        l_depth);
                    *rt = tree_concat_3(*rt, t4->k[2], t4->t[3], *r_depth,
                        depth-1, r_depth);
                }
                else if (cmp > 0)
                {
                    r = tree_split_2(t4->t[3], k, depth-1, lt, rt, l_depth,
                        r_depth, compare);
                    tree_t nt = tree3(t4->t[0], t4->k[0], t4->t[1], t4->k[1],
                        t4->t[2]);
                    *lt = tree_concat_3(nt, t4->k[2], *lt, depth, *l_depth,
                        l_depth);
                }
                else
                {
                    *lt = tree3(t4->t[0], t4->k[0], t4->t[1], t4->k[1],
                        t4->t[2]);
                    *rt = t4->t[3];
                    *l_depth = depth;
                    *r_depth = depth-1;
                }
            }
            else
            {
                *lt = tree2(t4->t[0], t4->k[0], t4->t[1]);
                *rt = tree2(t4->t[2], t4->k[2], t4->t[3]);
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
extern PURE tree_t _tree_union(tree_t t, tree_t u, Compare compare)
{
    size_t t_depth = tree_depth(t);
    size_t u_depth = tree_depth(u);
    return tree_union_2(t, u, t_depth, u_depth, &t_depth, compare);
}

static tree_t tree_union_2(tree_t t, tree_t u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare)
{
    switch (index(u))
    {
        case TREE_NIL:
            *depth = t_depth;
            return t;
        case TREE_2:
        {
            tree2_t u2 = get<tree2_s>(u);
            tree_t lt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, r_depth;
            tree_split_2(t, u2->k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            lt = tree_union_2(lt, u2->t[0], l_depth, u_depth-1, &l_depth,
                compare);
            rt = tree_union_2(rt, u2->t[1], r_depth, u_depth-1, &r_depth,
                compare);
            t = tree_concat_3(lt, u2->k[0], rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_3:
        {
            tree3_t u3 = get<tree3_s>(u);
            tree_t lt = TREE_EMPTY, mt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, m_depth, r_depth;
            tree_split_2(t, u3->k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            tree_split_2(rt, u3->k[1], r_depth, &mt, &rt, &m_depth, &r_depth,
                compare);
            lt = tree_union_2(lt, u3->t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_union_2(mt, u3->t[1], m_depth, u_depth-1, &m_depth,
                compare);
            rt = tree_union_2(rt, u3->t[2], r_depth, u_depth-1, &r_depth,
                compare);
            lt = tree_concat_3(lt, u3->k[0], mt, l_depth, m_depth, &l_depth);
            t  = tree_concat_3(lt, u3->k[1], rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_4:
        {
            tree4_t u4 = get<tree4_s>(u);
            tree_t lt = TREE_EMPTY, mt = TREE_EMPTY, nt = TREE_EMPTY,
                rt = TREE_EMPTY;
            size_t l_depth, m_depth, n_depth, r_depth;
            tree_split_2(t, u4->k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            tree_split_2(rt, u4->k[1], r_depth, &mt, &rt, &m_depth, &r_depth,
                compare);
            tree_split_2(rt, u4->k[2], r_depth, &nt, &rt, &n_depth, &r_depth,
                compare);
            lt = tree_union_2(lt, u4->t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_union_2(mt, u4->t[1], m_depth, u_depth-1, &m_depth,
                compare);
            nt = tree_union_2(nt, u4->t[2], n_depth, u_depth-1, &n_depth,
                compare);
            rt = tree_union_2(rt, u4->t[3], r_depth, u_depth-1, &r_depth,
                compare);
            lt = tree_concat_3(lt, u4->k[0], mt, l_depth, m_depth, &l_depth);
            lt = tree_concat_3(lt, u4->k[1], nt, l_depth, n_depth, &l_depth);
            t  = tree_concat_3(lt, u4->k[2], rt, l_depth, r_depth, depth);
            return t;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Intersection.
 */
extern PURE tree_t _tree_intersect(tree_t t, tree_t u, Compare compare)
{
    size_t t_depth = tree_depth(t);
    size_t u_depth = tree_depth(u);
    return tree_intersect_2(t, u, t_depth, u_depth, &t_depth, compare);
}

static tree_t tree_intersect_2(tree_t t, tree_t u, size_t t_depth,
    size_t u_depth, size_t *depth, Compare compare)
{
    switch (index(u))
    {
        case TREE_NIL:
            *depth = 0;
            return TREE_EMPTY;
        case TREE_2:
        {
            tree2_t u2 = get<tree2_s>(u);
            tree_t lt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, r_depth;
            bool in0 = tree_split_2(t, u2->k[0], t_depth, &lt, &rt, &l_depth,
                &r_depth, compare);
            lt = tree_intersect_2(lt, u2->t[0], l_depth, u_depth-1, &l_depth,
                compare);
            rt = tree_intersect_2(rt, u2->t[1], r_depth, u_depth-1, &r_depth,
                compare);
            if (in0)
                t = tree_concat_3(lt, u2->k[0], rt, l_depth, r_depth, depth);
            else
                t = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_3:
        {
            tree3_t u3 = get<tree3_s>(u);
            tree_t lt = TREE_EMPTY, mt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, m_depth, r_depth;
            bool in0 = tree_split_2(t, u3->k[0], t_depth, &lt, &rt, &l_depth,
                &r_depth, compare);
            bool in1 = tree_split_2(rt, u3->k[1], r_depth, &mt, &rt, &m_depth,
                &r_depth, compare);
            lt = tree_intersect_2(lt, u3->t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_intersect_2(mt, u3->t[1], m_depth, u_depth-1, &m_depth,
                compare);
            rt = tree_intersect_2(rt, u3->t[2], r_depth, u_depth-1, &r_depth,
                compare);
            if (in0)
                lt = tree_concat_3(lt, u3->k[0], mt, l_depth, m_depth,
                    &l_depth);
            else
                lt = tree_concat(lt, mt, l_depth, m_depth, &l_depth);
            if (in1)
                t = tree_concat_3(lt, u3->k[1], rt, l_depth, r_depth, depth);
            else
                t = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_4:
        {
            tree4_t u4 = get<tree4_s>(u);
            tree_t lt = TREE_EMPTY, mt = TREE_EMPTY, nt = TREE_EMPTY,
                rt = TREE_EMPTY;
            size_t l_depth, m_depth, n_depth, r_depth;
            bool in0 = tree_split_2(t, u4->k[0], t_depth, &lt, &rt, &l_depth,
                &r_depth, compare);
            bool in1 = tree_split_2(rt, u4->k[1], r_depth, &mt, &rt, &m_depth,
                &r_depth, compare);
            bool in2 = tree_split_2(rt, u4->k[2], r_depth, &nt, &rt, &n_depth,
                &r_depth, compare);
            lt = tree_intersect_2(lt, u4->t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_intersect_2(mt, u4->t[1], m_depth, u_depth-1, &m_depth,
                compare);
            nt = tree_intersect_2(nt, u4->t[2], n_depth, u_depth-1, &n_depth,
                compare);
            rt = tree_intersect_2(rt, u4->t[3], r_depth, u_depth-1, &r_depth,
                compare);
            if (in0)
                lt = tree_concat_3(lt, u4->k[0], mt, l_depth, m_depth,
                    &l_depth);
            else
                lt = tree_concat(lt, mt, l_depth, m_depth, &l_depth);
            if (in1)
                lt = tree_concat_3(lt, u4->k[1], nt, l_depth, n_depth,
                    &l_depth);
            else
                lt = tree_concat(lt, nt, l_depth, n_depth, &l_depth);
            if (in2)
                t = tree_concat_3(lt, u4->k[2], rt, l_depth, r_depth, depth);
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
extern PURE tree_t _tree_diff(tree_t t, tree_t u, Compare compare)
{
    size_t t_depth = tree_depth(t);
    size_t u_depth = tree_depth(u);
    return tree_diff_2(t, u, t_depth, u_depth, &t_depth, compare);
}

static tree_t tree_diff_2(tree_t t, tree_t u, size_t t_depth, size_t u_depth,
    size_t *depth, Compare compare)
{
    switch (index(u))
    {
        case TREE_NIL:
            *depth = t_depth;
            return t;
        case TREE_2:
        {
            tree2_t u2 = get<tree2_s>(u);
            tree_t lt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, r_depth;
            tree_split_2(t, u2->k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            lt = tree_diff_2(lt, u2->t[0], l_depth, u_depth-1, &l_depth,
                compare);
            rt = tree_diff_2(rt, u2->t[1], r_depth, u_depth-1, &r_depth,
                compare);
            t = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_3:
        {
            tree3_t u3 = get<tree3_s>(u);
            tree_t lt = TREE_EMPTY, mt = TREE_EMPTY, rt = TREE_EMPTY;
            size_t l_depth, m_depth, r_depth;
            tree_split_2(t, u3->k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            tree_split_2(rt, u3->k[1], r_depth, &mt, &rt, &m_depth, &r_depth,
                compare);
            lt = tree_diff_2(lt, u3->t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_diff_2(mt, u3->t[1], m_depth, u_depth-1, &m_depth,
                compare);
            rt = tree_diff_2(rt, u3->t[2], r_depth, u_depth-1, &r_depth,
                compare);
            lt = tree_concat(lt, mt, l_depth, m_depth, &l_depth);
            t  = tree_concat(lt, rt, l_depth, r_depth, depth);
            return t;
        }
        case TREE_4:
        {
            tree4_t u4 = get<tree4_s>(u);
            tree_t lt = TREE_EMPTY, mt = TREE_EMPTY, nt = TREE_EMPTY,
                rt = TREE_EMPTY;
            size_t l_depth, m_depth, n_depth, r_depth;
            tree_split_2(t, u4->k[0], t_depth, &lt, &rt, &l_depth, &r_depth,
                compare);
            tree_split_2(rt, u4->k[1], r_depth, &mt, &rt, &m_depth, &r_depth,
                compare);
            tree_split_2(rt, u4->k[2], r_depth, &nt, &rt, &n_depth, &r_depth,
                compare);
            lt = tree_diff_2(lt, u4->t[0], l_depth, u_depth-1, &l_depth,
                compare);
            mt = tree_diff_2(mt, u4->t[1], m_depth, u_depth-1, &m_depth,
                compare);
            nt = tree_diff_2(nt, u4->t[2], n_depth, u_depth-1, &n_depth,
                compare);
            rt = tree_diff_2(rt, u4->t[3], r_depth, u_depth-1, &r_depth,
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
extern PURE C _tree_foldl(tree_t t, C arg, C (*f)(void *, C, K), void *data)
{
    switch (index(t))
    {
        case TREE_NIL:
            return arg;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            arg = _tree_foldl(t2->t[0], arg, f, data);
            arg = f(data, arg, t2->k[0]);
            arg = _tree_foldl(t2->t[1], arg, f, data);
            return arg;
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            arg = _tree_foldl(t3->t[0], arg, f, data);
            arg = f(data, arg, t3->k[0]);
            arg = _tree_foldl(t3->t[1], arg, f, data);
            arg = f(data, arg, t3->k[1]);
            arg = _tree_foldl(t3->t[2], arg, f, data);
            return arg;
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            arg = _tree_foldl(t4->t[0], arg, f, data);
            arg = f(data, arg, t4->k[0]);
            arg = _tree_foldl(t4->t[1], arg, f, data);
            arg = f(data, arg, t4->k[1]);
            arg = _tree_foldl(t4->t[2], arg, f, data);
            arg = f(data, arg, t4->k[2]);
            arg = _tree_foldl(t4->t[3], arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Fold right.
 */
extern PURE C _tree_foldr(tree_t t, C arg, C (*f)(void *, C, K), void *data)
{
    switch (index(t))
    {
        case TREE_NIL:
            return arg;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            arg = _tree_foldr(t2->t[1], arg, f, data);
            arg = f(data, arg, t2->k[0]);
            arg = _tree_foldr(t2->t[0], arg, f, data);
            return arg;
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            arg = _tree_foldr(t3->t[2], arg, f, data);
            arg = f(data, arg, t3->k[1]);
            arg = _tree_foldr(t3->t[1], arg, f, data);
            arg = f(data, arg, t3->k[0]);
            arg = _tree_foldr(t3->t[0], arg, f, data);
            return arg;
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            arg = _tree_foldr(t4->t[3], arg, f, data);
            arg = f(data, arg, t4->k[2]);
            arg = _tree_foldr(t4->t[2], arg, f, data);
            arg = f(data, arg, t4->k[1]);
            arg = _tree_foldr(t4->t[1], arg, f, data);
            arg = f(data, arg, t4->k[0]);
            arg = _tree_foldr(t4->t[0], arg, f, data);
            return arg;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Map.
 */
extern PURE tree_t _tree_map(tree_t t, K (*f)(void *, K), void *data)
{
    switch (index(t))
    {
        case TREE_NIL:
            return TREE_EMPTY;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            tree_t t0 = _tree_map(t2->t[0], f, data);
            K k0 = f(data, t2->k[0]);
            tree_t t1 = _tree_map(t2->t[1], f, data);
            return tree2(t0, k0, t1);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            tree_t t0 = _tree_map(t3->t[0], f, data);
            K k0 = f(data, t3->k[0]);
            tree_t t1 = _tree_map(t3->t[1], f, data);
            K k1 = f(data, t3->k[1]);
            tree_t t2 = _tree_map(t3->t[2], f, data);
            return tree3(t0, k0, t1, k1, t2);
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            tree_t t0 = _tree_map(t4->t[0], f, data);
            K k0 = f(data, t4->k[0]);
            tree_t t1 = _tree_map(t4->t[1], f, data);
            K k1 = f(data, t4->k[1]);
            tree_t t2 = _tree_map(t4->t[2], f, data);
            K k2 = f(data, t4->k[2]);
            tree_t t3 = _tree_map(t4->t[3], f, data);
            return tree4(t0, k0, t1, k1, t2, k2, t3);
        }
        default:
            error_bad_tree();
    }
}

/*
 * From list.
 */
extern PURE tree_t _tree_from_list(const List<K> xs, Compare compare)
{
    tree_t t = TREE_EMPTY;
    List<K> ys = xs;
    while (!is_empty(ys))
    {
        t = _tree_insert(t, head(ys), _TREE_INSERT_GROW_FLAG, compare).fst;
        ys = tail(ys);
    }
    return t;
}

/*
 * To list.
 */
extern PURE List<C> _tree_to_list(tree_t t, C (*f)(void *, K), void *data)
{
    return tree_to_list_2(t, f, data, list<C>());
}

static List<C> tree_to_list_2(tree_t t, C (*f)(void *, K), void *data,
    List<C> xs)
{
    switch (index(t))
    {
        case TREE_NIL:
            return xs;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            xs = tree_to_list_2(t2->t[1], f, data, xs);
            xs = cons(f(data, t2->k[0]), xs);
            xs = tree_to_list_2(t2->t[0], f, data, xs);
            return xs;
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            xs = tree_to_list_2(t3->t[2], f, data, xs);
            xs = cons(f(data, t3->k[1]), xs);
            xs = tree_to_list_2(t3->t[1], f, data, xs);
            xs = cons(f(data, t3->k[0]), xs);
            xs = tree_to_list_2(t3->t[0], f, data, xs);
            return xs;
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            xs = tree_to_list_2(t4->t[3], f, data, xs);
            xs = cons(f(data, t4->k[2]), xs);
            xs = tree_to_list_2(t4->t[2], f, data, xs);
            xs = cons(f(data, t4->k[1]), xs);
            xs = tree_to_list_2(t4->t[1], f, data, xs);
            xs = cons(f(data, t4->k[0]), xs);
            xs = tree_to_list_2(t4->t[0], f, data, xs);
            return xs;
        }
        default:
            error_bad_tree();
    }
}

/*
 * Verify.
 */
extern PURE bool _tree_verify(tree_t t)
{
    size_t depth = tree_depth(t);
    return tree_verify_2(t, depth);
}

static bool tree_verify_2(tree_t t, size_t depth)
{
    switch (index(t))
    {
        case TREE_NIL:
            return (depth == 0);
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            return tree_verify_2(t2->t[0], depth-1) &&
                   tree_verify_2(t2->t[1], depth-1);
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            return tree_verify_2(t3->t[0], depth-1) &&
                   tree_verify_2(t3->t[1], depth-1) &&
                   tree_verify_2(t3->t[2], depth-1);
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            return tree_verify_2(t4->t[0], depth-1) &&
                   tree_verify_2(t4->t[1], depth-1) &&
                   tree_verify_2(t4->t[2], depth-1) &&
                   tree_verify_2(t4->t[3], depth-1);
        }
        default:
            return false;
    }
}

/*
 * Iterator init.
 */
static itr_t tree_itr_init(itr_t i, tree_t t)
{
    ssize_t idx = 0;
    while (true)
    {
        switch (index(t))
        {
            case TREE_NIL:
                i->ptr = idx-1;
                return i;
            case TREE_2:
            {
                tree2_t t2 = get<tree2_s>(t);
                i->stack[idx].t = t;
                i->stack[idx].offset = 0;
                t = t2->t[0];
                break;
            }
            case TREE_3:
            {
                tree3_t t3 = get<tree3_s>(t);
                i->stack[idx].t = t;
                i->stack[idx].offset = 0;
                t = t3->t[0];
                break;
            }
            case TREE_4:
            {
                tree4_t t4 = get<tree4_s>(t);
                i->stack[idx].t = t;
                i->stack[idx].offset = 0;
                t = t4->t[0];
                break;
            }
        }
        idx++;
    }
}

/*
 * Iterator get.
 */
static bool tree_itr_get(itr_t i, Any *k)
{
    ssize_t idx = i->ptr;
    if (idx < 0)
        return false;
    tree_t t = i->stack[idx].t;
    unsigned offset = i->stack[idx].offset;
    switch (index(t))
    {
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            *k = t2->k[0];
            return true;
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            *k = t3->k[offset];
            return true;
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            *k = t4->k[offset];
            return true;
        }
        default:
            error_bad_tree();
    }
    return false;
}

/*
 * Iterator next.
 */
extern void tree_itr_next(itr_t i)
{
    ssize_t idx = i->ptr;
    if (idx < 0)
        return;
    tree_t t = i->stack[idx].t;
    unsigned offset = i->stack[idx].offset;
    switch (index(t))
    {
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            t = t2->t[1];
            break;
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            if (offset > 0)
                t = t3->t[2];
            else
            {
                i->stack[idx].offset++;
                t = t3->t[1];
                idx++;
            }
            break;
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            if (offset == 2)
                t = t4->t[3];
            else
            {
                i->stack[idx].offset++;
                t = t4->t[offset+1];
                idx++;
            }
            break;
        }
        default:
            error_bad_tree();
    }

    while (true)
    {
        switch (index(t))
        {
            case TREE_NIL:
                i->ptr = idx-1;
                return;
            case TREE_2:
            {
                tree2_t t2 = get<tree2_s>(t);
                i->stack[idx].t = t;
                i->stack[idx].offset = 0;
                t = t2->t[0];
                break;
            }
            case TREE_3:
            {
                tree3_t t3 = get<tree3_s>(t);
                i->stack[idx].t = t;
                i->stack[idx].offset = 0;
                t = t3->t[0];
                break;
            }
            case TREE_4:
            {
                tree4_t t4 = get<tree4_s>(t);
                i->stack[idx].t = t;
                i->stack[idx].offset = 0;
                t = t4->t[0];
                break;
            }
        }
        idx++;
    }
}

/*
 * Compare.
 */
extern PURE int _tree_compare(tree_t t, tree_t u, void *data,
    int (*val_compare)(void *, Any, Any))
{
    if (t == u)
        return 0;

    itr_s itrt_0, itru_0;
    itr_t itrt = &itrt_0, itru = &itru_0;
    tree_itr_init(itrt, t);
    tree_itr_init(itru, u);

    while (true)
    {
        Any kt, ku;
        if (!tree_itr_get(itrt, &kt))
        {
            if (!tree_itr_get(itru, &ku))
                return 0;
            return 1;
        }
        if (!tree_itr_get(itru, &ku))
            return -1;
        int cmp = val_compare(data, kt, ku);
        if (cmp != 0)
            return cmp;
        tree_itr_next(itrt);
        tree_itr_next(itru);
    }
}

/*
 * Show.
 */
extern PURE String _tree_show(tree_t t, String (*f)(A))
{
    String r = string('{');
    r = tree_show_2(t, r, true, f);
    r = append(r, '}');
    return r;
}

#if 1
static String tree_show_2(tree_t t, String r, bool last, String (*f)(A))
{
    switch (index(t))
    {
        case TREE_NIL:
            return r;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
            r = tree_show_2(t2->t[0], r, false, f);
            r = append(r, f(t2->k[0]));
            if (!last || index(t2->t[1]) != TREE_NIL)
                r = append(r, ',');
            r = tree_show_2(t2->t[1], r, last, f);
            return r;
        }
        case TREE_3:
        {
            tree3_t t3 = get<tree3_s>(t);
            r = tree_show_2(t3->t[0], r, false, f);
            r = append(r, f(t3->k[0]));
            r = append(r, ',');
            r = tree_show_2(t3->t[1], r, false, f);
            r = append(r, f(t3->k[1]));
            if (!last || index(t3->t[2]) != TREE_NIL)
                r = append(r, ',');
            r = tree_show_2(t3->t[2], r, last, f);
            return r;
        }
        case TREE_4:
        {
            tree4_t t4 = get<tree4_s>(t);
            r = tree_show_2(t4->t[0], r, false, f);
            r = append(r, f(t4->k[0]));
            r = append(r, ',');
            r = tree_show_2(t4->t[1], r, false, f);
            r = append(r, f(t4->k[1]));
            r = append(r, ',');
            r = tree_show_2(t4->t[2], r, false, f);
            r = append(r, f(t4->k[2]));
            if (!last || index(t4->t[3]) != TREE_NIL)
                r = append(r, ',');
            r = tree_show_2(t4->t[3], r, last, f);
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
static String tree_show_2(tree_t t, String r, bool last, String (*f)(A))
{
    switch (index(t))
    {
        case TREE_NIL:
            r = append(r, "emp");
            return r;
        case TREE_2:
        {
            tree2_t t2 = get<tree2_s>(t);
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
            tree3_t t3 = get<tree3_s>(t);
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
            tree4_t t4 = get<tree4_s>(t);
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

}
