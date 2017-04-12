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

#ifndef _FCOMPARE_H
#define _FCOMPARE_H

#include "fbase.h"

namespace F
{

/**
 * Compare pointers.
 */
extern PURE int compare(const void *_x, const void *_y);

/**
 * Compare Booleans.
 */
extern PURE int compare(bool _x, bool _y);

/**
 * Compare signed characters.
 */
extern PURE int compare(signed char _x, signed char _y);

/**
 * Compare unsigned characters.
 */
extern PURE int compare(unsigned char _x, unsigned char _y);

/**
 * Compare signed short integers.
 */
extern PURE int compare(short _x, short _y);

/**
 * Compare unsigned short integers.
 */
extern PURE int compare(unsigned short _x, unsigned short _y);

/**
 * Compare signed integers.
 */
extern PURE int compare(int _x, int _y);

/**
 * Compare unsigned integers.
 */
extern PURE int compare(unsigned _x, unsigned _y);

/**
 * Compare signed long integers.
 */
extern PURE int compare(long int _x, long int _y);

/**
 * Compare unsigned long integers.
 */
extern PURE int compare(unsigned long int _x, unsigned long int _y);

/**
 * Compare signed long long integers.
 */
extern PURE int compare(long long int _x, long long int _y);

/**
 * Compare unsigned long long integers.
 */
extern PURE int compare(unsigned long long int _x, unsigned long long int _y);

/**
 * Compare floats.
 */
extern PURE int compare(float _x, float _y);

/**
 * Compare doubles.
 */
extern PURE int compare(double _x, double _y);

}               /* namespace F */

#endif          /* _FCOMPARE_H */
