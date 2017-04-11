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

#include "../flist.h"
#include "../fmap.h"
#include "../fmaybe.h"
#include "../fset.h"
#include "../fstring.h"
#include "../fvector.h"

using namespace F;

#define STR0(x)         #x
#define STR(x)          STR0(x)

static size_t total = 0, passed = 0;

#define TEST(c)                                                         \
    do {                                                                \
        printf("\"" STR(c) "\"...");                                    \
        fflush(stdout);                                                 \
        bool b = (c);                                                   \
        total++;                                                        \
        passed += (b? 1: 0);                                            \
        printf("%s\33[0m\n", (b? "\33[32mpassed": "\33[31mFAILED"));    \
        if (!b) exit(EXIT_FAILURE);                                     \
    } while(false)

#define fst     _result_0
#define snd     _result_1

/****************************************************************************/
// Custom lists:
template <typename T>
struct CONS;

struct EMPTY { };

template <typename T>
using LIST = Union<EMPTY, CONS<T>>;

template <typename T>
struct CONS
{
    T val;
    LIST<T> next;
};

#define LIST_EMPTY      LIST<void>::index<EMPTY>()
#define LIST_CONS       LIST<void>::index<CONS<void>>()

template <typename T>
PURE LIST<T> reverse2(LIST<T> xs, LIST<T> ys)
{
    // switch-case-idiom
    switch (index(xs))
    {
        case LIST_EMPTY:
            return ys;
        case LIST_CONS:
        {
            const CONS<T> &x = xs;
            const CONS<T> y = {x.val, ys};
            return reverse2(x.next, LIST<T>(y));
        }
        default:
            abort();
    }
}
template <typename T>
PURE LIST<T> reverse(LIST<T> xs)
{
    return reverse2(xs, LIST<T>((EMPTY){}));
}

template <typename T>
PURE String show(LIST<T> xs)
{
    String str = string('[');
    bool prev = false;
    while (true)
    {
        bool stop = false;
        switch (index(xs))
        {
            case LIST_EMPTY:
                stop = true;
                break;
            case LIST_CONS:
            {
                const CONS<T> &x = xs;
                xs = x.next;
                if (prev)
                    str += ',';
                str += show(x.val);
                break;
            }
            default:
                abort();
        }
        if (stop)
            break;
        prev = true;
    }
    str += ']';
    return str;
}

template <typename T>
void TEST_FIND_ALL(T xs)
{
    for (auto x: xs)
        TEST(!empty(find(xs, x)));
}

/****************************************************************************/

int main(void)
{
    // Lists:
{
    auto xs = list<int>();
    for (int i = 30; i >= 0; i--)
        xs = list(i, xs);

    printf("\n\33[33mxs = %s\33[0m\n", c_str(show(xs)));
    TEST(index(list<int>()) == 0);
    TEST(size(list<int>()) == 0);
    TEST(empty(list<int>()));
    TEST(index(xs) == 1);
    TEST(!empty(xs));
    TEST(size(xs) == 31);
    TEST(head(xs) == 0);
    TEST(size(tail(xs)) == 30);
    TEST(head(tail(xs)) == 1);
    TEST(last(xs) == 30);
    TEST(size(take(xs, 2)) == 2);
    TEST(head(take(xs, 2)) == 0);
    TEST(last(take(xs, 2)) == 1);
    TEST(size(take_while(xs, [] (int x) { return x <= 2; })) == 3);
    TEST(last(take_while(xs, [] (int x) { return x <= 2; })) == 2);
    TEST(size(append(xs, xs)) == 62);
    TEST(last(append(xs, xs)) == last(xs));
    TEST(head(reverse(xs)) == last(xs));
    TEST(second(head(zip(xs, xs))) == head(xs));
    TEST(first(last(zip(xs, xs))) == last(xs));
    TEST(compare(sort(xs), xs) == 0);
    TEST(compare(sort(xs, [] (int x, int y) { return y-x; }),
        reverse(xs)) == 0);
    TEST(foldl(xs, true, [] (bool a, int x) { return (a && (x <= 30)); }));
    TEST(foldl(xs, 0, [] (int x, int y) { return x+y; }) == 465);
    TEST(foldl(xs, 0, [] (int x, int y) { return y; }) == 30);
    TEST(foldr(xs, 0, [] (int x, int y) { return y; }) == 0);
    TEST(({int sum = 0; for (int x: xs) sum += x; sum;}) == 465);
    TEST(last(map<int>(xs, [] (int x) { return x+1; })) == 31);
    TEST(size(filter(xs, [] (int x) { return x != 1 && x != 2; })) == 29);
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

    printf("\n\33[33mstr = %s\33[0m\n", c_str(show(str)));
    TEST(size(string()) == 0);
    TEST(compare(string(), string("")) == 0);
    TEST(compare(string('X'), string("X")) == 0);
    TEST(size(str) == 76);
    TEST(size(append(str, str)) == 2 * size(str));
    TEST(lookup(append(str, 'X'), 76) == 'X');
    TEST(lookup(append(str, "ABC123"), 76+3) == '1');
    TEST(size((str + str)) == 2 * size(str));
    TEST(lookup((str + 'X'), 76) == 'X');
    TEST(lookup((str + "ABC123"), 76+3) == '1');
    TEST(lookup(str, 3) == 'l');
    TEST(lookup(show(str), size(show(str))-1) == '\"');
    TEST(verify(split(str, 27).fst));
    TEST(verify(split(str, 27).snd));
    TEST(compare(append(split(str, 27).fst, split(str, 27).snd), str) == 0);
    TEST(verify(left(str, 41)));
    TEST(verify(right(str, 41)));
    TEST(compare(append(left(str, 41), right(str, 41)), str) == 0);
    TEST(verify(left(str, 65)));
    TEST(size(left(str, 65)) == 65);
    TEST(verify(right(str, 65)));
    TEST(size(right(str, 65)) == 76-65);
    TEST(verify(between(str, 13, 26)));
    TEST(compare(between(str, 13, 26),
        string("ABCDEFGHIJKLMNOPQRSTUVWXYZ")) == 0);
    TEST(compare(between(str, 10, 0), string()) == 0);
    TEST(compare(between(str, 11, 14), left(right(str, 11), 14)) == 0);
    TEST(compare(left(append(str, str), size(str)),
        right(append(str, str), size(str))) == 0);
    TEST(find(str, '!') == 11);
    TEST(empty(find(str, '@')));
    TEST(find(str, "World") == 6);
    TEST(find(str, "ABCD") == 13);
    TEST(find(str, "BCDE") == 14);
    TEST(find(str, "ABCD", 10) == 13);
    TEST(empty(find(str, "World", 7)));
    TEST(find(str, string("World")) == 6);
    TEST(find(str, string("ABCD")) == 13);
    TEST(find(str, string("BCDE")) == 14);
    TEST(find(str, string("ABCD"), 10) == 13);
    TEST(empty(find(str, string("ABCDEFGHIJKLMNOPQRSTUVWY3______"))));
    TEST(find(str, str) == 0);
    TEST(empty(find(str, str, 1)));
    TEST(find(insert(str, 22, str), str) == 22);
    TEST(find(insert(str, 22, str), c_str(str)) == 22);
    TEST(empty(find(insert(str, 22, replace_all(str, "z", string('Z'))), str)));
    TEST(find(replace(str, "World", string("CAT")).fst, "CAT", 3) == 6);
    TEST(find(replace_all(str, "World", string("CAT")), "CAT", 3) == 6);
    TEST(find(replace(str, string("World"), string("CAT")).fst, "CAT", 3) == 6);
    TEST(find(replace_all(str, string("World"), string("CAT")), "CAT", 3) == 6);
    TEST(size(replace_all(str, string("l"), string("333"))) == size(str) + 4*2);
    TEST(empty(find(str, string("World"), 7)));
    TEST(size(erase(str, 0, size(str))) == 0);
    TEST(size(erase(str, 10, 10)) == size(str)-10);
    TEST(size(show(str)) > size(str));
    TEST(compare(insert(erase(str, 6, 5), 6, string("World")), str) == 0);
    TEST(size(list(str)) == size(str));
    TEST(foldl(str, (size_t)0, [] (size_t a, size_t idx, char32_t _) { return (a + idx + 1); }) == 2926);
    TEST(foldl(str, (char32_t)0,
        [] (char32_t x, size_t _, char32_t y) { return (x > y? x: y); }) == 'z');
    TEST(verify(map(str, [] (size_t _, char32_t c) { return 'X'; })));
    TEST(lookup(map(str, [] (size_t _, char32_t c) { return 'X'; }), 33) == 'X');
    TEST(verify(filter(str,
        [] (size_t _, char32_t c) { return (bool)isdigit((char)c); })));
    TEST(compare(filter(str,
        [] (size_t _, char32_t c) { return (bool)isdigit((char)c); }),
            string("1234567890")) == 0);
    TEST(verify(filter_map(str,
        [] (size_t _, char32_t c) -> Optional<char32_t>
            { return (isdigit((char)(c+1))? Optional<char32_t>(c+1):
                Optional<char32_t>()); })));
    TEST(compare(filter_map(str,
        [] (size_t _, char32_t c) -> Optional<char32_t>
            { return (isdigit((char)(c+1))? Optional<char32_t>(c+1):
                Optional<char32_t>()); }),
            string("234567891")) == 0);
    TEST(compare(({String tmp; for (char32_t c: filter_map(string("A man, a plan, a canal, Panama"), [] (size_t _, char32_t c) -> Optional<char32_t> { if (!isalpha(c)) return Optional<char32_t>(); return Optional<char32_t>(tolower(c));})) tmp = append(tmp, c); tmp;}), string("amanaplanacanalpanama")) == 0);

    Set<char32_t> seen;
    for (auto c: str)
    {
        if (find(seen, c))
            continue;
        TEST(!empty(find(str, c)));
        seen = insert(seen, c);
    }

}

    // Vectors:
{
    auto xs = vector<int>();
    auto ys = vector(string("Hello World!"));
    auto zs = vector(list(1.1f, list(2.4f, list(3.3f, list<float>()))));
    const int data[] = {7, 5, 4, 3};
    auto ws = vector(data, sizeof(data) / sizeof(data[0]));

    for (int i = 0; i < 300; i++)
        xs = push_back(xs, i);

    printf("\n\33[33mxs = %s\33[0m\n", c_str(show(xs)));
    printf("\33[33mys = %s\33[0m\n", c_str(show(ys)));
    printf("\33[33mzs = %s\33[0m\n", c_str(show(zs)));
    printf("\33[33mws = %s\33[0m\n", c_str(show(ws)));
    TEST(size(vector<int>()) == 0);
    TEST(size(vector(list(2, list<int>()))) == 1);
    TEST(empty(vector<char>()));
    TEST(!empty(xs));
    TEST(verify(xs));
    TEST(verify(ys));
    TEST(verify(zs));
    TEST(verify(ws));
    TEST(size(xs) == 300);
    TEST(size(ys) == size(string("Hello World!")));
    TEST(size(zs) == 3);
    TEST(size(ws) == 4);
    TEST(at(xs, 10) == 10);
    TEST(at(xs, 100) == 100);
    TEST(verify(push_front(xs, 333)));
    TEST(at(push_front(xs, 333), 0) == 333);
    TEST(front(xs) == 0);
    TEST(verify(push_back(xs, 333)));
    TEST(at(push_back(xs, 333), 300) == 333);
    TEST(back(xs) == 299);
    TEST(verify(pop_front(xs)));
    TEST(size(pop_front(xs)) == 299);
    TEST(verify(pop_back(xs)));
    TEST(size(pop_back(xs)) == 299);
    TEST(size(append(xs, xs)) == 600);
    TEST(size(append(ws, ws)) == 8);
    TEST(size(append(xs, ws)) == size(append(ws, xs)));
    TEST(compare(between(xs, 10, 0), vector<int>()) == 0);
    TEST(compare(between(xs, 11, 14), left(right(xs, 11), 14)) == 0);
    TEST(compare(left(append(xs, xs), size(xs)), right(append(xs, xs), size(xs))) == 0);
    TEST(compare(between(insert(xs, 10, xs), 10, size(xs)), xs) == 0);
    TEST(size(insert(xs, 122, ws)) == size(xs) + size(ws));
    TEST(compare(between(insert(xs, 10, ws), 10, size(ws)), ws) == 0);
    TEST(compare(erase(xs, 0, size(xs)), vector<int>()) == 0);
    TEST(size(erase(xs, 0, 100)) == size(xs) - 100);
    TEST(verify(split(xs, 123).fst));
    TEST(verify(split(xs, 123).snd));
    TEST(compare(append(split(xs, 123).fst, split(xs, 123).snd), xs) == 0);
    TEST(foldl(xs, (size_t)0, [] (size_t a, size_t idx, int _) { return (a + idx + 1); }) == 45150);
    TEST(foldr(xs, (size_t)0, [] (size_t a, size_t idx, int _) { return (a + idx + 1); }) == 45150);
    TEST(foldl(xs, 0, [] (int x, size_t _, int y) { return x+y; }) == 150*299);
    TEST(foldl(ws, 0, [] (int x, size_t _, int y) { return x+y; }) == 19);
    TEST(foldr(xs, 0, [] (int x, size_t _, int y) { return x+y; }) == 150*299);
    TEST(({int sum = 0; for (auto x: xs) sum += x; sum;}) == 150*299);
    TEST(({int sum = 0; for (auto x: ws) sum += x; sum;}) == 19);
    TEST(at(map<float>(xs, [] (size_t _, int x) { return (float)x; }), 123) ==
        123.0f);
    TEST(verify(filter(xs, [] (size_t _, int x) { return !(x & 1); })));
    TEST(at(filter(xs, [] (size_t _, int x) { return !(x & 1); }), 33) == 66);
    TEST(compare(xs, xs) == 0);
    TEST(compare(xs, push_front(xs, 100)) < 0);
    TEST(compare(xs, push_front(xs, -100)) > 0);
    TEST(compare(map<int>(zs, [] (size_t _, float x) { return (int)x-1; }),
        split(xs, 3).fst) == 0);
    TEST(verify(show(xs)));

    for (int i = 0; i < 300; i++)
    {
        printf("(i = %d) ", i);
        TEST(at(xs, i) == i);
        printf("(i = %d) ", i);
        TEST(verify(erase(xs, i, i / 10 + 1)));
    }
}

    // Tuples:
{
    auto t =
        tuple<int, float, char, bool, Tuple<int, int>>(7, 3.125f, 'c', true,
            tuple(1, 2));

    printf("\n\33[33mt = %s\33[0m\n", c_str(show(t)));
    TEST(first(t) == 7);
    TEST(second(t) == 3.125f);
    TEST(third(t) == 'c');
    TEST(fourth(t) == true);
    TEST(size(t) == 5);
    TEST(size(fifth(t)) == 2);
    TEST(compare(t, tuple(7, 10.0f, 'x', false, tuple(1, 2))) < 0);
    TEST(compare(t, tuple(7, 3.125f, 'c', true, tuple(1, 2))) == 0);
    TEST(compare(t, tuple(7, 3.125f, 'c', true, tuple(1, 0))) > 0);
}

    // Maps:
{
    auto m = map<int, int>();
    for (int i = 0; i < 200; i++)
        m = insert(m, tuple(i, 2*i));
    printf("\n\33[33mt = %s\33[0m\n", c_str(show(m)));

    TEST(verify(map<float, float>()));
    TEST(verify(m));
    TEST(!empty(m));
    TEST(second(get(find(m, 25))) == 50);
    TEST(verify(insert(m, tuple(55, 55))));
    TEST(second(get(find(insert(m, tuple(55, 55)), 55))) == 55);
    TEST(verify(insert(m, tuple(134, -12))));
    TEST(second(get(find(insert(m, tuple(134, -12)), 134))) == -12);
    TEST(verify(insert(m, tuple(1134, -12))));
    TEST(second(get(find(insert(m, tuple(1134, -12)), 1134))) == -12);
    TEST(!empty(find(m, 86)));
    TEST(empty(find(m, 203)));
    TEST(second(get(find(m, 20))) == 40);
    TEST(empty(find(m, -20)));
    TEST(second(get(find(m, 21))) == 42);
    TEST(verify(erase(m, 51)));
    TEST(verify(erase(m, -51)));
    TEST(verify(erase(m, 0)));
    TEST(verify(erase(m, 200)));
    TEST(verify(erase(m, 133)));
    TEST(empty(find(erase(m, 51), 51)));
    TEST(!empty(find(erase(m, 51), 52)));
    TEST(size(m) == 200);
    TEST(size(keys(m)) == 200);
    TEST(last(keys(m)) == 199);
    TEST(size(values(m)) == 200);
    TEST(last(values(m)) == 398);
    TEST(verify(split(m, 33).fst));
    TEST(verify(split(m, 33).snd));
    TEST(compare(split(m, 33).fst, split(m, 33).snd) < 0);
    TEST(compare(split(m, 33).snd, split(m, 33).fst) > 0);
    TEST(verify(split(m, 100).fst));
    TEST(verify(split(m, 100).snd));
    TEST(verify(split(m, 199).fst));
    TEST(verify(split(m, 199).snd));
    TEST(compare(split(m, 199).fst, erase(m, 199)) == 0);
    TEST(compare(split(m, 199).snd, map<int, int>()) == 0);
    TEST(verify(merge(split(m, 123).fst, split(m, 123).snd)));
    TEST(compare(merge(split(m, 123).fst, split(m, 123).snd), erase(m, 123))
        == 0);
    TEST(compare(merge(split(m, 123).snd, split(m, 123).fst), erase(m, 123))
        == 0);
    TEST(compare(sort(list(m)), list(m)) == 0);
    TEST(foldl(m, 0, [] (int a, Tuple<int, int> t) { return a + first(t); }) == 199*100);
    TEST(foldr(m, 0, [] (int a, Tuple<int, int> t) { return a + second(t); }) == 2*199*100);
    TEST(({int sum = 0; for (auto t: m) sum += second(t); sum;}) == 2*199*100);
    TEST(second(get(find(map<int>(m, [] (Tuple<int, int> t) { return first(t); }), 43))) == 43);
    TEST(verify(show(m)));

    for (auto t: m)
    {
        printf("(t = %s) ", c_str(show(t)));
        TEST(!empty(find(m, first(t))));
        printf("(t = %s) ", c_str(show(t)));
        TEST(verify(insert(m, tuple(2*first(t), second(t)-30))));
        printf("(t = %s) ", c_str(show(t)));
        TEST(verify(erase(m, first(t))));
    }

}
    // Sets:
{
    auto s = set<int>();
    for (int i = 0; i < 100; i++)
        s = insert(s, 2*i);
    printf("\n\33[33ms = %s\33[0m\n", c_str(show(s)));

    TEST(empty(set<double>()));
    TEST(find(s, 64));
    TEST(!find(s, 63));
    TEST(find(insert(s, 999), 999));
    TEST(!find(erase(s, 44), 44));
    TEST(find(merge(s, insert(s, 33)), 33));
    TEST(!find(merge(s, insert(s, 33)), 31));
    TEST(compare(merge(s, s), s) == 0);
    TEST(!find(intersect(s, insert(s, 67)), 67));
    TEST(find(intersect(s, insert(s, 67)), 80));
    TEST(compare(intersect(s, s), s) == 0);
    TEST(compare(intersect(insert(s, 33), insert(s, 11)), s) == 0);
    TEST(find(diff(s, erase(s, 22)), 22));
    TEST(!find(diff(s, erase(s, 22)), 44));
    TEST(compare(diff(s, s), set<int>()) == 0);
    TEST(compare(list(s), sort(list(s))) == 0);
    TEST(size(s) == 100);
    TEST(foldl(s, 0, [] (int a, int x) { return a + x; }) == 99*50*2);
    TEST(foldr(s, 0, [] (int a, int x) { return a + x; }) == 99*50*2);
    TEST(({int sum = 0; for (auto a: s) sum += a; sum;}) == 99*50*2);
    TEST(verify(show(s)));

    for (auto x: s)
    {
        printf("(x = %d) ", x);
        TEST(find(s, x));
        printf("(x = %d) ", x);
        TEST(verify(insert(s, 2*x)));
        printf("(x = %d) ", x);
        TEST(verify(erase(s, x)));
    }

}

    // Custom lists.
{
    EMPTY e;
    LIST<int> xs = e;
    for (int i = 0; i < 10; i++)
    {
        CONS<int> node = {i, xs};
        xs = node;
    }
    printf("\n\33[33mxs = %s\33[0m\n", c_str(show(xs)));

    TEST(index(xs) == 1);
    TEST(compare(show(xs), string("[9,8,7,6,5,4,3,2,1,0]")) == 0);
    TEST(compare(show(reverse(xs)), string("[0,1,2,3,4,5,6,7,8,9]")) == 0);
}

    putchar('\n');
    printf("total=%zu; passed=%zu (%.2f%%)\n", total, passed,
        ((double)passed / (double)total) * 100.0);

    return 0;
}

