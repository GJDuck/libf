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

#ifndef _FSHOW_H
#define _FSHOW_H

#include "fbase.h"
#include "fvalue.h"

#include "fstring_defs.h"

namespace F
{

/**
 * Show a pointer.
 */
extern PURE String show(const void *_x);

/**
 * Show a Boolean.
 */
extern PURE String show(bool _x);

/**
 * Show a signed character.
 */
extern PURE String show(signed char _x);

/**
 * Show an unsigned character.
 */
extern PURE String show(unsigned char _x);

/**
 * Show a character.
 */
extern PURE String show(char _x);

/**
 * Show a short integer.
 */
extern PURE String show(short _x);

/**
 * Show an unsigned short integer.
 */
extern PURE String show(unsigned short _x);

/**
 * Show a signed integer.
 */
extern PURE String show(int _x);

/**
 * Show an unsigned integer.
 */
extern PURE String show(unsigned _x);

/**
 * Show a long signed integer.
 */
extern PURE String show(long int _x);

/**
 * Show a long unsigned integer.
 */
extern PURE String show(unsigned long int _x);

/**
 * Show a long long signed integer.
 */
extern PURE String show(long long int _x);

/**
 * Show a long long unsigned integer.
 */
extern PURE String show(unsigned long long int _x);

/**
 * Show a float.
 */
extern PURE String show(float _x);

/**
 * Show a double.
 */
extern PURE String show(double _x);

}           /* namespace F */

#endif      /* _FSHOW_H */
