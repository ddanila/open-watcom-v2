/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2024-2026 The Open Watcom Contributors. All Rights Reserved.
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#ifndef _ASMOPS_H_
#define _ASMOPS_H_

#include "asmops.gh"

typedef enum asm_cpu {
    P_EMPTY = -1,
    /*
     * bit count from left: ( need at least 7 bits )
     * bit 0-2:   Math coprocessor
     * bit 3:     Protected mode
     * bit 4-6:   cpu type
     * bit 7-11;  extension set
     */
    P_NO87  = 0x0000,         /* no FPU */
    P_87    = 0x0001,         /* 8087 */
    P_287   = 0x0002,         /* 80287 */
    P_387   = 0x0004,         /* 80387 */

    P_PM    = 0x0008,         /* protect-mode */

    P_86    = 0x0000,         /* 8086, default */
    P_186   = 0x0010,         /* 80186 */
    P_286   = 0x0020,         /* 80286 */
    P_286p  = P_286 | P_PM,   /* 80286, protected mode */
    P_386   = 0x0030,         /* 80386 */
    P_386p  = P_386 | P_PM,   /* 80386, protected mode */
    P_486   = 0x0040,         /* 80486 */
    P_486p  = P_486 | P_PM,   /* 80486, protected mode */
    P_586   = 0x0050,         /* pentium */
    P_586p  = P_586 | P_PM,   /* pentium, protected mode */
    P_686   = 0x0060,         /* pentium */
    P_686p  = P_686 | P_PM,   /* pentium, protected mode */

    P_MMX   = 0x0080,         /* MMX extension instructions */
    P_K3D   = 0x0100,         /* 3DNow extension instructions */
    P_SSE   = 0x0200,         /* SSE extension instructions */
    P_SSE2  = 0x0400,         /* SSE extension instructions */
    P_SSE3  = 0x0800,         /* SSE extension instructions */

    NO_OPPRFX  = P_MMX | P_SSE | P_SSE2 | P_SSE3,

    P_FPU_MASK = 0x0007,
    P_CPU_MASK = 0x0070,
    P_EXT_MASK = 0x0F80
} asm_cpu;


/*
 * Token classes for string-like content
 * --------------------------------------
 *
 * The lexer treats "stringy" content as three distinct things:
 *
 *   TC_STRING         '...' or "..." -- a real string literal. Emitted as a
 *                     single token. The opener is recorded on the token's
 *                     .delim field so the macro re-wrap path can round-trip
 *                     the original quote ('m '/'' -> 'DB '/'').
 *
 *   <...> / {...}     opaque bracketed raw text. Emitted as a *triple*:
 *
 *                         TC_OP_ANGLE  TC_RAW_TEXT  TC_CL_ANGLE
 *                         TC_OP_BRACE  TC_RAW_TEXT  TC_CL_BRACE
 *
 *                     The opener/closer tokens carry the bracket char in
 *                     their string_ptr; the inner TC_RAW_TEXT carries the
 *                     body. Directives that accept bracketed forms walk the
 *                     three tokens explicitly (see Include, IRP, struct
 *                     initialisers, etc.). This mirrors the form Jiri
 *                     described in PR #1622 review.
 *
 *   TC_RAW_TEXT       outside a bracket triple, only appears as the
 *                     undelimited-bareword fallback path in get_string.
 *
 *   TC_COMMENT_TEXT   line tail following the COMMENT directive. Read
 *                     directly by CommentDirective; not a "string" in any
 *                     other sense.
 *
 * The IS_OPEN_BRACKET / IS_CLOSE_BRACKET macros below help walk the triple.
 * IS_STRING_TOKEN is a narrow shim retained only for sites that treat
 * TC_STRING and a bareword TC_RAW_TEXT the same way (e.g. blank-arg checks).
 */

typedef enum tok_class {
    TC_FINAL,
    TC_INSTR,
    TC_RES_ID,
    TC_ID,
    TC_REG,
    TC_STRING,           /* '...' or "..." quoted string literal */
    TC_RAW_TEXT,         /* body of a bracket triple, or undelim bareword */
    TC_COMMENT_TEXT,     /* body of the COMMENT directive */
    TC_DIRECTIVE,
    TC_DIRECT_EXPR,
    TC_NUM,
    TC_FLOAT,
    TC_NOOP,                 /* No operation */

    TC_POSITIVE,
    TC_NEGATIVE,
    TC_ID_IN_BACKQUOTES,
    TC_PATH,
    TC_UNARY_OPERATOR,
    TC_RELATION_OPERATOR,
    TC_ARITH_OPERATOR,
    TC_BAD_NUM,

    TC_OP_BRACKET,       // '(',
    TC_OP_SQ_BRACKET,    // '[',
    TC_OP_BRACE,         // '{',  -- raw-text opener or ENUM body opener
    TC_OP_ANGLE,         // '<',  -- raw-text opener (paired with TC_CL_ANGLE)
    TC_CL_BRACKET,       // ')',
    TC_CL_SQ_BRACKET,    // ']',
    TC_CL_BRACE,         // '}',  -- raw-text closer or ENUM body closer
    TC_CL_ANGLE,         // '>',  -- raw-text closer (paired with TC_OP_ANGLE)
    TC_COMMA,            // ',',
    TC_COLON,            // ':',
    TC_COLON_EQU,        // ':=',
    TC_SEMI_COLON,       // ';',
    TC_PLUS,             // '+',
    TC_MINUS,            // '-',
    TC_DOT,              // '.',
    TC_QUESTION_MARK,    // '?',
    TC_PERCENT,          // '%',
} tok_class;

typedef struct asm_tok {
    tok_class           class;
    char                *string_ptr;
    char                delim;          /* see tok_class block comment above; 0 if not applicable */
    union {
        long            value;
        float           float_value;
        unsigned char   bytes[10];
        asm_token       token;
    } u;
} asm_tok;

#define IS_STRING_TOKEN( cls )      ((cls) == TC_STRING || (cls) == TC_RAW_TEXT)
#define IS_OPEN_BRACKET( cls )      ((cls) == TC_OP_ANGLE || (cls) == TC_OP_BRACE)
#define IS_CLOSE_BRACKET( cls )     ((cls) == TC_CL_ANGLE || (cls) == TC_CL_BRACE)

/* True if tokens[i] is the start of a logical "string-like value" --
 * either a TC_STRING / standalone TC_RAW_TEXT (size 1) or a bracket triple
 * (size 3, body at i+1). */
#define IS_STRING_VALUE( tokens, i ) \
    ( IS_STRING_TOKEN( (tokens)[i].class ) || IS_OPEN_BRACKET( (tokens)[i].class ) )

/* Body string of the value at tokens[i]; assumes IS_STRING_VALUE is true. */
#define STRING_VALUE_BODY( tokens, i ) \
    ( IS_OPEN_BRACKET( (tokens)[i].class ) ? (tokens)[(i) + 1].string_ptr : (tokens)[i].string_ptr )

/* How many tokens the logical value at tokens[i] spans: 3 for a bracket
 * triple, 1 otherwise. */
#define STRING_VALUE_LEN( tokens, i ) \
    ( IS_OPEN_BRACKET( (tokens)[i].class ) ? 3 : 1 )

#endif
