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
#include <stdlib.h>

#include "../flib.h"

#define STR0(x)         #x
#define STR(x)          STR0(x)

#define TEST(c)                                                         \
    do {                                                                \
        bool b = (c);                                                   \
        total++;                                                        \
        passed += (b? 1: 0);                                            \
        printf("\"" STR(c) "\"...%s\33[0m\n",                           \
            (b? "\33[32mpassed": "\33[31mFAILED"));                     \
    } while(false)

/****************************************************************************/
// Custom lists:
template <typename T>
struct NODE;
struct EMPTY;
template <typename T>
using LIST = Multi<EMPTY, NODE<T>>;
template <typename T>
struct NODE
{
    T val;
    LIST<T> next;
};
#define LIST_EMPTY      type_index<EMPTY, LIST<int>>()
#define LIST_NODE       type_index<NODE<int>, LIST<int>>()

template <typename T>
LIST<T> reverse2(LIST<T> xs, LIST<T> ys)
{
    // switch-case-idiom
    switch (index(xs))
    {
        case LIST_EMPTY:
            return ys;
        case LIST_NODE:
        {
            EMPTY *z = get<EMPTY>(xs);
            NODE<T> *x = get<NODE<T>>(xs);
            NODE<T> *y = box<NODE<T>>({x->val, ys});
            ys = set<LIST<T>>(y);
            return reverse2(x->next, ys);
        }
        default:
            abort();
    }
}
template <typename T>
LIST<T> reverse(LIST<T> xs)
{
    return reverse2(xs, set<LIST<T>>((EMPTY *)nullptr));
}

template <typename T>
String show2(bool prev, LIST<T> xs)
{
    // if-decl-idiom
    if (NODE<T> *x = get<NODE<T>>(xs))
    {
        String str = string();
        if (prev)
            str = append(str, ',');
        str = append(str, show(x->val));
        str = append(str, show2(true, x->next));
        return str;
    }
    else
        return string();
}
template <typename T>
String show(LIST<T> xs)
{
    String str = string('[');
    str = append(str, show2(false, xs));
    str = append(str, ']');
    return str;
}

/****************************************************************************/

int main(void)
{
    size_t total = 0, passed = 0;

    // Lists:
{
    auto xs = list<int>();
    for (int i = 30; i >= 0; i--)
        xs = cons(i, xs);

    printf("\n\33[33mxs = %s\33[0m\n", cstr(show(xs)));
    TEST(!is_empty(xs));
    TEST(length(xs) == 31);
    TEST(head(xs) == 0);
    TEST(length(tail(xs)) == 30);
    TEST(head(tail(xs)) == 1);
    TEST(last(xs) == 30);
    TEST(length(take(xs, 2)) == 2);
    TEST(last(take_while(xs, [] (int x) { return x <= 2; })) == 2);
    TEST(length(append(xs, xs)) == 62);
    TEST(head(reverse(xs)) == last(xs));
    TEST(snd(head(zip(xs, xs))) == head(xs));
    TEST(compare(sort(xs), xs) == 0);
    TEST(compare(sort(xs, [] (int x, int y) { return y-x; }),
        reverse(xs)) == 0);
    TEST(foldl(xs, 0, [] (int x, int y) { return x+y; }) == 465);
    TEST(foldl(xs, 0, [] (int x, int y) { return y; }) == 30);
    TEST(foldr(xs, 0, [] (int x, int y) { return y; }) == 0);
    TEST(last(map<int>(xs, [] (int x) { return x+1; })) == 31);
    TEST(length(filter(xs, [] (int x) { return x != 1 && x != 2; })) == 29);
    TEST(compare(xs, xs) == 0);
    TEST(compare(xs, tail(xs)) < 0);
    TEST(compare(tail(xs), xs) > 0);
}

    // Strings:
{
    auto str = string("Hello World!\n");
    str = append(str, "ABCDEFGHIJKLMNOP");
    str = append(str, string("QRSTUVWXYZ"));
    str = append(str, "1234567890\n");
    str = append(str, 'a');
    str = append(str, 'b');
    str = append(str, 'c');
    str = append(str, 'd');
    str = append(str, 'e');
    str = append(str, "fghijklmnop");
    str = append(str, string("qrstuvwx"));
    str = append(str, 'y');
    str = append(str, "z");

    printf("\n\33[33mstr = %s\33[0m\n", cstr(show(str)));
    TEST(length(append(str, str)) == 2 * length(str));
    TEST(lookup(append(str, 'X'), 76) == 'X');
    TEST(lookup(append(str, "ABC123"), 76+3) == '1');
    TEST(length(str) == 76);
    TEST(lookup(str, 3) == 'l');
    TEST(lookup(show(str), length(show(str))-1) == '\"');
    TEST(verify(split(str, 27).fst));
    TEST(verify(split(str, 27).snd));
    TEST(compare(append(split(str, 27).fst, split(str, 27).snd), str) == 0);
    TEST(verify(left(str, 41)));
    TEST(verify(right(str, 41)));
    TEST(compare(append(left(str, 41), right(str, 41)), str) == 0);
    TEST(verify(left(str, 65)));
    TEST(length(left(str, 65)) == 65);
    TEST(verify(right(str, 65)));
    TEST(length(right(str, 65)) == 76-65);
    TEST(compare(between(str, 22, 11), string()) == 0);
    TEST(verify(between(str, 13, 13+26)));
    TEST(compare(between(str, 13, 13+26),
        string("ABCDEFGHIJKLMNOPQRSTUVWXYZ")) == 0);
    TEST(find(str, '!') == 11);
    TEST(find(str, '@') < 0);
    TEST(find(str, 33, [] (int a, char32_t b) { return (a == b); }) == 11);
    TEST(find(str, 33, [] (int a, char32_t b) { return (a > b); }) == 5);
    TEST(foldl(str, (char32_t)0,
        [] (char32_t x, char32_t y) { return (x > y? x: y); }) == 'z');
    TEST(verify(map(str, [] (char32_t c) { return 'X'; })));
    TEST(lookup(map(str, [] (char32_t c) { return 'X'; }), 33) == 'X');
    TEST(verify(filter(str,
        [] (char32_t c) { return (bool)isdigit((char)c); })));
    TEST(compare(filter(str,
        [] (char32_t c) { return (bool)isdigit((char)c); }),
            string("1234567890")) == 0);
    TEST(verify(filter_map(str,
        [] (char32_t c) -> Result<bool, char32_t>
            { return {(bool)isdigit((char)(c+1)), c+1}; })));
    TEST(compare(filter_map(str,
        [] (char32_t c) -> Result<bool, char32_t>
            { return {(bool)isdigit((char)(c+1)), c+1}; }),
            string("234567891")) == 0);
}

    // Vectors:
{
    auto xs = vector<int>();
    auto ys = vector(string("Hello World!"));
    auto zs = vector(cons(1.1f, cons(2.4f, cons(3.3f, list<float>()))));
    const int data[] = {7, 5, 4, 3};
    auto ws = vector(data, sizeof(data) / sizeof(data[0]));

    for (int i = 0; i < 300; i++)
        xs = push_back(xs, i);

    printf("\n\33[33mxs = %s\33[0m\n", cstr(show(xs)));
    printf("\33[33mys = %s\33[0m\n", cstr(show(ys)));
    printf("\33[33mzs = %s\33[0m\n", cstr(show(zs)));
    TEST(verify(xs));
    TEST(verify(ys));
    TEST(verify(zs));
    TEST(verify(ws));
    TEST(length(xs) == 300);
    TEST(length(ys) == length(string("Hello World!")));
    TEST(length(zs) == 3);
    TEST(length(ws) == 4);
    TEST(verify(push_front(xs, 333)));
    TEST(lookup(push_front(xs, 333), 0) == 333);
    TEST(verify(push_back(xs, 333)));
    TEST(lookup(push_back(xs, 333), 300) == 333);
    TEST(verify(pop_front(xs)));
    TEST(length(pop_front(xs)) == 299);
    TEST(verify(pop_back(xs)));
    TEST(length(pop_back(xs)) == 299);
    TEST(length(append(xs, xs)) == 600);
    TEST(length(append(ws, ws)) == 8);
    TEST(length(append(xs, ws)) == length(append(ws, xs)));
    TEST(verify(split(xs, 123).fst));
    TEST(verify(split(xs, 123).snd));
    TEST(compare(append(split(xs, 123).fst, split(xs, 123).snd), xs) == 0);
    TEST(foldl(xs, 0, [] (int x, int y) { return x+y; }) == 150*299);
    TEST(foldl(ws, 0, [] (int x, int y) { return x+y; }) == 19);
    TEST(foldr(xs, 0, [] (int x, int y) { return x+y; }) == 150*299);
    TEST(lookup(map<float>(xs, [] (int x) { return (float)x; }), 123) ==
        123.0f);
    TEST(verify(filter(xs, [] (int x) { return !(x & 1); })));
    TEST(lookup(filter(xs, [] (int x) { return !(x & 1); }), 33) == 66);
    TEST(compare(xs, xs) == 0);
    TEST(compare(xs, push_front(xs, 100)) < 0);
    TEST(compare(xs, push_front(xs, -100)) > 0);
    TEST(compare(map<int>(zs, [] (float x) { return (int)x-1; }),
        split(xs, 3).fst) == 0);
}

    // Tuples:
{
    auto t =
        tuple<int, float, char, bool, Tuple<int, int>>(7, 3.125f, 'c', true,
            tuple(1, 2));

    printf("\n\33[33mt = %s\33[0m\n", cstr(show(t)));
    TEST(fst(t) == 7);
    TEST(snd(t) == 3.125f);
    TEST(third(t) == 'c');
    TEST(fourth(t) == true);
    TEST(compare(t, tuple(7, 10.0f, 'x', false, tuple(1, 2))) < 0);
    TEST(compare(t, tuple(7, 3.125f, 'c', true, tuple(1, 2))) == 0);
    TEST(compare(t, tuple(7, 3.125f, 'c', true, tuple(1, 0))) > 0);
}

    // Maps:
{
    auto m = map<int, int>();
    for (int i = 0; i < 200; i++)
        m = insert(m, i, 2*i);
    printf("\n\33[33mt = %s\33[0m\n", cstr(show(m)));

    TEST(verify(m));
    TEST(lookup(m, 25) == 50);
    TEST(get(search(m, 25)) == 50);
    TEST(verify(insert(m, 55, 55)));
    TEST(lookup(insert(m, 55, 55), 55) == 55);
    TEST(verify(replace_search(m, 101, 777).fst));
    TEST(get(replace_search(m, 101, 777).snd) == 202);
    TEST(lookup(replace_search(m, 101, 777).fst, 101) == 777);
    TEST(verify(replace(m, 99, 88)));
    TEST(lookup(replace(m, 99, 88), 99) == 88);
    TEST(verify(expand(m, -44, 776)));
    TEST(size(expand(m, -44, 776)) == 201);
    TEST(lookup(expand(m, -44, 776), -44) == 776);
    TEST(verify(insert_search(m, 133, 12).fst));
    TEST(get(insert_search(m, 133, 12).snd) == 266);
    TEST(lookup(insert_search(m, 133, 12).fst, 133) == 12);
    TEST(verify(insert(m, 134, -12)));
    TEST(lookup(insert(m, 134, -12), 134) == -12);
    TEST(verify(insert(m, 1134, -12)));
    TEST(lookup(insert(m, 1134, -12), 1134) == -12);
    TEST(member(m, 86));
    TEST(!member(m, 203));
    TEST(get(search(m, 20)) == 40);
    TEST(is_nothing(search(m, -20)));
    TEST(lookup(m, 21) == 42);
    TEST(verify(remove(m, 51)));
    TEST(verify(remove(m, -51)));
    TEST(verify(remove(m, 0)));
    TEST(verify(remove(m, 200)));
    TEST(verify(remove(m, 133)));
    TEST(!member(remove(m, 51), 51));
    TEST(member(remove(m, 51), 52));
    TEST(is_nothing(remove_search(m, -20).snd));
    TEST(!member(remove_search(m, 20).fst, 20));
    TEST(member(remove_search(m, 21).fst, 20));
    TEST(size(m) == 200);
    TEST(length(keys(m)) == 200);
    TEST(last(keys(m)) == 199);
    TEST(length(values(m)) == 200);
    TEST(last(values(m)) == 398);
    TEST(verify(split(m, 33).fst));
    TEST(verify(split(m, 33).snd));
    TEST(compare(split(m, 33).fst, split(m, 33).snd) < 0);
    TEST(compare(split(m, 33).snd, split(m, 33).fst) > 0);
    TEST(verify(split(m, 100).fst));
    TEST(verify(split(m, 100).snd));
    TEST(verify(split(m, 199).fst));
    TEST(verify(split(m, 199).snd));
    TEST(compare(split(m, 199).fst, remove(m, 199)) == 0);
    TEST(compare(split(m, 199).snd, map<int, int>()) == 0);
    TEST(verify(merge(split(m, 123).fst, split(m, 123).snd)));
    TEST(compare(merge(split(m, 123).fst, split(m, 123).snd), remove(m, 123))
        == 0);
    TEST(compare(merge(split(m, 123).snd, split(m, 123).fst), remove(m, 123))
        == 0);
    TEST(compare(sort(list(m)), list(m)) == 0);
    TEST(foldl(m, 0, [] (int a, int k, int v) { return a + k; }) == 199*100);
    TEST(foldr(m, 0, [] (int a, int k, int v) { return a + v; }) == 2*199*100);
    TEST(lookup(map<int>(m, [] (int k, int v) { return k; }), 43) == 43);

}
    // Sets:
{
    auto s = set<int>();
    for (int i = 0; i < 100; i++)
        s = insert(s, 2*i);
    printf("\n\33[33ms = %s\33[0m\n", cstr(show(s)));

    TEST(member(s, 64));
    TEST(!member(s, 63));
    TEST(member(insert(s, 999), 999));
    TEST(!member(remove(s, 44), 44));
    TEST(member(merge(s, insert(s, 33)), 33));
    TEST(!member(merge(s, insert(s, 33)), 31));
    TEST(compare(merge(s, s), s) == 0);
    TEST(!member(intersect(s, insert(s, 67)), 67));
    TEST(member(intersect(s, insert(s, 67)), 80));
    TEST(compare(intersect(s, s), s) == 0);
    TEST(compare(intersect(insert(s, 33), insert(s, 11)), s) == 0);
    TEST(member(diff(s, remove(s, 22)), 22));
    TEST(!member(diff(s, remove(s, 22)), 44));
    TEST(compare(diff(s, s), set<int>()) == 0);
    TEST(compare(list(s), sort(list(s))) == 0);
    TEST(size(s) == 100);
    TEST(foldl(s, 0, [] (int a, int x) { return a + x; }) == 99*50*2);
    TEST(foldr(s, 0, [] (int a, int x) { return a + x; }) == 99*50*2);
}

    // Custom lists.
{
    LIST<int> xs = set<LIST<int>>((EMPTY *)nullptr);
    for (int i = 0; i < 10; i++)
        xs = set<LIST<int>>(box<NODE<int>>({i, xs}));
    printf("\n\33[33mxs = %s\33[0m\n", cstr(show(xs)));

    TEST(index(xs) == 1);
    TEST(compare(show(xs), string("[9,8,7,6,5,4,3,2,1,0]")) == 0);
    TEST(compare(show(reverse(xs)), string("[0,1,2,3,4,5,6,7,8,9]")) == 0);
}


    putchar('\n');
    printf("total=%zu; passed=%zu (%.2f%%)\n", total, passed,
        ((double)passed / (double)total) * 100.0);

    return 0;
}

