/*
 * Copyright (c) 2017 The National University of Singapore.
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
#include <math.h>

#include "fshow.h"
#include "fstring.h"

namespace F
{

#define MAX_INT_BUF             64
#define MAX_DOUBLE_BUF          128
#define MAX_PTR_BUF             64
#define MAX_ESCAPE_CHAR_BUF     32

#define DOUBLE_MIN_PRECISION    15
#define DOUBLE_MAX_PRECISION    (DOUBLE_MIN_PRECISION+2)

static String show_char(char c);
static String show_float(double x);

extern PURE String show(const void *p)
{
    char buf[MAX_PTR_BUF];
    int r = snprintf(buf, sizeof(buf)-1, "%pp", p);
    if (r <= 0 || r >= sizeof(buf)-1)
        error("snprintf failed");
    return string(buf);
}

extern PURE String show(bool b)
{
    if (b)
        return string("true");
    else
        return string("false");
}

static String show_char(char c)
{
    printf("show_char\n");
    switch (c)
    {
        case '\0':
            return string("'\\0'");
        case '\a':
            return string("'\\a'");
        case '\f':
            return string("'\\f'");
        case '\n':
            return string("'\\n'");
        case '\r':
            return string("'\\r'");
        case '\t':
            return string("'\\t'");
        case '\v':
            return string("'\\v'");
        case '\\':
            return string("'\\\\'");
        case '\'':
            return string("'\\\''");
        default:
            break;
    }

    if (!isprint(c))
    {
        char buf[MAX_ESCAPE_CHAR_BUF];
        int r = snprintf(buf, sizeof(buf)-1, "'\\x%.2x'",
            ((unsigned)c) & 0xFF);
        if (r <= 0 || r >= sizeof(buf)-1)
            error("snprintf failed");
        return string(buf);
    }
    else
    {
        char buf[4] = {'\'', c, '\'', '\0'};
        return string(buf);
    }
}

extern PURE String show(unsigned char c)
{
    return show_char((char)c);
}

extern PURE String show(signed char c)
{
    return show_char((char)c);
}

extern PURE String show(char c)
{
    return show_char((char)c);
}

extern PURE String show(short x)
{
    char buf[MAX_INT_BUF];
    int r = snprintf(buf, sizeof(buf)-1, "%hd", x);
    if (r <= 0 || r >= sizeof(buf)-1)
        error("snprintf failed");
    return string(buf);
}

extern PURE String show(unsigned short x)
{
    char buf[MAX_INT_BUF];
    int r = snprintf(buf, sizeof(buf)-1, "%hu", x);
    if (r <= 0 || r >= sizeof(buf)-1)
        error("snprintf failed");
    return string(buf);
}

extern PURE String show(int x)
{
    char buf[MAX_INT_BUF];
    int r = snprintf(buf, sizeof(buf)-1, "%d", x);
    if (r <= 0 || r >= sizeof(buf)-1)
        error("snprintf failed");
    return string(buf);
}

extern PURE String show(unsigned x)
{
    char buf[MAX_INT_BUF];
    int r = snprintf(buf, sizeof(buf)-1, "%u", x);
    if (r <= 0 || r >= sizeof(buf)-1)
        error("snprintf failed");
    return string(buf);
}

extern PURE String show(long int x)
{
    char buf[MAX_INT_BUF];
    int r = snprintf(buf, sizeof(buf)-1, "%ld", x);
    if (r <= 0 || r >= sizeof(buf)-1)
        error("snprintf failed");
    return string(buf);
}

extern PURE String show(unsigned long int x)
{
    char buf[MAX_INT_BUF];
    int r = snprintf(buf, sizeof(buf)-1, "%lu", x);
    if (r <= 0 || r >= sizeof(buf)-1)
        error("snprintf failed");
    return string(buf);
}

extern PURE String show(long long int x)
{
    char buf[MAX_INT_BUF];
    int r = snprintf(buf, sizeof(buf)-1, "%lld", x);
    if (r <= 0 || r >= sizeof(buf)-1)
        error("snprintf failed");
    return string(buf);
}

extern PURE String show(unsigned long long int x)
{
    char buf[MAX_INT_BUF];
    int r = snprintf(buf, sizeof(buf)-1, "%llu", x);
    if (r <= 0 || r >= sizeof(buf)-1)
        error("snprintf failed");
    return string(buf);
}

extern PURE String show(float x)
{
    return show_float((double)x);
}

extern PURE String show(double x)
{
    return show_float(x);
}

static String show_float(double x)
{
    if (isnan(x))
        return string("NaN");
    if (isinf(x))
        return string("inf");
    
    char buf[MAX_DOUBLE_BUF];
    double x1;
    int p = DOUBLE_MIN_PRECISION;
    do
    {
        int r = snprintf(buf, sizeof(buf)-1, "%.*g", p, x);
        if (r <= 0 || r >= sizeof(buf)-1)
            error("snprintf failed");
        if (sscanf(buf, "%lf", &x1) != 1)
            error("sscanf failed");
        p++;
    }
    while (x != x1);

    return string(buf);
}

}

