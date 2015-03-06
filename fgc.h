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

#ifndef _FGC_H
#define _FGC_H

#include <stdint.h>
#include <gc.h>

#ifndef GC_INLINE
#define GC_INLINE static inline __attribute__ ((__always_inline__))
#endif      /* GC_INLINE */

#define GC_ALIGNMENT        16

namespace F
{

/*
 * GC Sizes.
 */
GC_INLINE size_t gc_size(void *_ptr)
{
    return GC_size(_ptr);
}

/*
 * GC tagged pointers.
 */
GC_INLINE void *gc_settag(void *_ptr, uint32_t _tag)
{
    return (char *)_ptr + _tag;
}
GC_INLINE uint32_t gc_gettag(void *_ptr)
{
    return ((uint32_t)(uintptr_t)_ptr) & (GC_ALIGNMENT-1);
}
GC_INLINE void *gc_deltag(void *_ptr, uint32_t _tag)
{
    return (char *)_ptr - _tag;
}
GC_INLINE void *gc_striptag(void *_ptr)
{
    return (void *)((uintptr_t)_ptr & ~(GC_ALIGNMENT-1));
}
GC_INLINE void *gc_clobbertag(void *_ptr, uint32_t _tag)
{
    return (void *)gc_settag(gc_striptag(_ptr), _tag);
}

/*
 * GC base pointer.
 */
GC_INLINE void *gc_base(void *_ptr)
{
    return GC_base(_ptr);
}

/*
 * GC enable/disable.
 */
GC_INLINE void gc_disable(void)
{
    GC_disable();
}
GC_INLINE void gc_enable(void)
{
    GC_enable();
}

/*
 * GC initialization.
 */
GC_INLINE void gc_init(void)
{
    GC_init();
}

/*
 * GC root registration.
 */
GC_INLINE void gc_root(void *_ptr, size_t _size)
{
    GC_add_roots(_ptr, (char *)_ptr+_size+1);
}

/*
 * GC memory (de)allocation.
 */
GC_INLINE void *gc_malloc(size_t _size)
{
    return GC_malloc(_size);
}
GC_INLINE void *gc_malloc_atomic(size_t _size)
{
    return GC_malloc_atomic(_size);
}
GC_INLINE void gc_free(void *_ptr)
{
    GC_free(_ptr);
}

/*
 * GC garbage collection.
 */
GC_INLINE void gc_collect(void)
{
    GC_gcollect();
}

}           /* namesLpace F */

#endif      /* _FGC_H */
