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

#include <stdio.h>
#include <string.h>

#include "fcompare.h"

namespace F
{

extern PURE int compare(const void *x, const void *y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

extern PURE int compare(bool x, bool y)
{
    return (int)x - (int)y;
}

extern PURE int compare(signed char x, signed char y)
{
    return (int)x - (int)y;
}

extern PURE int compare(unsigned char x, unsigned char y)
{
    return (int)x - (int)y;
}

extern PURE int compare(short x, short y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

extern PURE int compare(unsigned short x, unsigned short y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

extern PURE int compare(int x, int y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

extern PURE int compare(unsigned x, unsigned y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

extern PURE int compare(long int x, long int y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

extern PURE int compare(unsigned long int x, unsigned long int y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

extern PURE int compare(long long int x, long long int y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

extern PURE int compare(unsigned long long int x, unsigned long long int y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

extern PURE int compare(float x, float y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    Any a = cast<Any>(x);
    Any b = cast<Any>(y);
    return compare(a, b);
}

extern PURE int compare(double x, double y)
{
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    Any a = cast<Any>(x);
    Any b = cast<Any>(y);
    return compare(a, b);
}

extern PURE int compare(Any x, Any y)
{
    if (x._val < y._val)
        return -1;
    if (x._val > y._val)
        return 1;
    return 0;
}

}

