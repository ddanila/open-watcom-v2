/****************************************************************************
*
*                            Open Watcom Project
*
* Copyright (c) 2002-2026 The Open Watcom Contributors. All Rights Reserved.
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


#include "asmglob.h"
#if defined( _STANDALONE_ )
#include "directiv.h"
#endif
#include <ctype.h>


typedef union {
    float   f;
    long    l;
} NUMBERFL;

#ifdef DEBUG_OUT
const char              *CurrString; // Current Input Line
#endif

#if defined( _STANDALONE_ )

bool                    EnumDirective;

#endif

static unsigned         radix_base = 10;

static bool get_float( asm_tok *tok, const char **input, char **output )
/**********************************************************************/
{
    /* valid floats look like:  (int)[.(int)][e(int)][r] */

    bool            got_decimal;
    bool            got_e;
    const char      *ptr;
    size_t          len;
    int             c;
    unsigned char   suffix_len;

    suffix_len = 0;
    got_e = false;
    got_decimal = false;
    for( ptr = *input; (c = *ptr) != '\0'; ptr++ ) {
        if( isdigit( c ) )
            continue;
        if( isspace( c ) )
            break;
        if( c == '.' ) {
            if( got_decimal || got_e )
                break;
            got_decimal = true;
            continue;
        }
        c = tolower( c );
        if( c == 'r' ) {
            suffix_len = 1;
            break;
        }
        if( c != 'e' || got_e )
            break;
        // c == 'e'
        got_e = true;
        /* accept e+2 / e-4 /etc. */
        c = *(ptr + 1);
        if( c == '+' || c == '-' ) {
            ptr++;
        }
    }
    tok->class = TC_FLOAT;
    /* copy the string, fix input & output pointers */
    tok->string_ptr = *output;
    len = ptr - *input + suffix_len;
    memcpy( *output, *input, len );
    *output += len;
    *(*output)++ = '\0';
    *input = ptr + suffix_len;

    tok->u.float_value = (float)atof( tok->string_ptr );
    return( RC_OK );
}

static void array_mul_add( unsigned char *buf, unsigned base, const char *dig, unsigned size )
{
    unsigned long   num;

    if( isdigit( *dig ) ) {
        num = *dig - '0';
    } else {
        num = tolower( *dig ) - 'a' + 10;
    }
    while( size-- > 0 ) {
        num = num + *buf * base;
        *buf = (unsigned char)num;
        num >>= 8;
        buf++;
    }
}

static bool get_bracket_triple( asm_tok *tok, const char **input, char **output,
                                char symbol_o, char symbol_c,
                                tok_class open_class, tok_class close_class,
                                int *extra_tokens )
/****************************************************************************
 * Emit a three-token bracket triple at tok[0..2]:
 *     tok[0]: open_class,   string_ptr -> "<" or "{"
 *     tok[1]: TC_RAW_TEXT,  string_ptr -> body (nested same-form brackets
 *                                          preserved verbatim, via level)
 *     tok[2]: close_class,  string_ptr -> ">" or "}"
 * On return *extra_tokens = 2 so the AsmScan caller knows to advance its
 * token index past the body and closer in addition to its usual i++.
 */
{
    int         count;
    int         level;
    asm_tok     *body;
    asm_tok     *close;

    tok->class = open_class;
    tok->string_ptr = *output;
    tok->delim = 0;
    *(*output)++ = symbol_o;
    *(*output)++ = '\0';
    (*input)++;     /* skip the opener char */

    body = tok + 1;
    body->class = TC_RAW_TEXT;
    body->string_ptr = *output;
    body->delim = 0;

    count = 0;
    level = 0;
    while( count < MAX_TOK_LEN ) {
        if( **input == symbol_o ) {
            level++;
        } else if( **input == symbol_c ) {
            if( level ) {
                level--;
            } else {
                (*input)++; /* skip the closing delimiter */
                break;
            }
        } else if( **input == '\0' || **input == '\n' ) {
            *(*output)++ = '\0';
            body->u.value = count;
            /* Set *extra_tokens to the count we've written so far so the
             * AsmScan caller's SetFinalToken doesn't overwrite the partial
             * opener+body we just emitted. The body is well-formed (just
             * truncated); flagging that to the user is more useful than the
             * old generic SYNTAX_ERROR. */
            *extra_tokens = 1;
            AsmError( UNTERMINATED_BRACKETED_TEXT );
            return( RC_ERROR );
        }
        *(*output)++ = *(*input)++;
        count++;
    }
    *(*output)++ = '\0';
    body->u.value = count;

    close = tok + 2;
    close->class = close_class;
    close->string_ptr = *output;
    close->delim = 0;
    *(*output)++ = symbol_c;
    *(*output)++ = '\0';

    *extra_tokens = 2;
    return( RC_OK );
}

static bool get_string( asm_tok *tok, const char **input, char **output, int *extra_tokens )
/******************************************************************************************
 * On '/'":  emit one TC_STRING; .delim records the opener (' or ").
 * On </{ :  emit a bracket triple via get_bracket_triple (sets *extra_tokens=2).
 * Otherwise: emit one TC_RAW_TEXT for the undelimited bareword fallback
 *            (reachable only when get_id, get_number etc. give up and
 *            decide the run of chars is opaque text).
 */
{
    char    symbol_o;
    int     count;

    *extra_tokens = 0;
    symbol_o = **input;

    switch( symbol_o ) {
    case '"':
    case '\'':
        tok->class = TC_STRING;
        tok->string_ptr = *output;
        tok->delim = symbol_o;
        (*input)++;     /* skip the opener */
        for( count = 0; count < MAX_TOK_LEN; count++ ) {
            if( **input == symbol_o ) {
                if( *( *input + 1 ) == symbol_o ) {
                    /* doubled delim -- literal */
                    (*input)++; /* skip the 1st, keep the 2nd */
                } else {
                    (*input)++; /* skip the closing delimiter */
                    break;
                }
            } else if( **input == '\0' || **input == '\n' ) {
                *(*output)++ = '\0';
                AsmError( SYNTAX_ERROR );
                return( RC_ERROR );
            }
            *(*output)++ = *(*input)++;
        }
        *(*output)++ = '\0';
        tok->u.value = count;
        return( RC_OK );

    case '<':
        return( get_bracket_triple( tok, input, output, '<', '>',
                                    TC_OP_ANGLE, TC_CL_ANGLE, extra_tokens ) );

    case '{':
        return( get_bracket_triple( tok, input, output, '{', '}',
                                    TC_OP_BRACE, TC_CL_BRACE, extra_tokens ) );

    default:
        /* Undelimited fallback. TC_BAREWORD keeps this distinct from a
         * triple body (which is always TC_RAW_TEXT) so the structural
         * invariant "TC_RAW_TEXT only appears inside <>/{}" holds. */
        tok->class = TC_BAREWORD;
        tok->string_ptr = *output;
        tok->delim = 0;
        for( count = 0; **input != '\0' && !isspace( **input ) && **input != ','; count++ ) {
            *(*output)++ = *(*input)++;
        }
        *(*output)++ = '\0';
        tok->u.value = count;
        return( RC_OK );
    }
}

static bool get_number( asm_tok *tok, const char **input, char **output, bool base10, int *extra_tokens )
/******************************************************************************************************
 * extra_tokens is an out-param used only when get_number falls through to
 * get_string (no leading digits and not a 0-prefixed literal). See get_string.
 */
{
    const char          *ptr;
    const char          *dig_start;
    bool                first_char_0;
    unsigned char       suffix_len;
    size_t              len;
    unsigned            base = 0;
    unsigned            digits_seen;
    int                 c;
    int                 c2;

#define VALID_BINARY    0x0003
#define VALID_OCTAL     0x00ff
#define VALID_DECIMAL   0x03ff
#define OK_NUM( t )     ((digits_seen & ~VALID_##t) == 0)

    digits_seen = 0;
    first_char_0 = false;
    suffix_len = 0;
    ptr = *input;
    if( *(unsigned char *)ptr == '0' ) {
        ++ptr;
        if( tolower( *(unsigned char *)ptr ) == 'x' ) {
            ++ptr;
            base = 16;
        } else {
            first_char_0 = true;
        }
    }
    dig_start = ptr;
    for( ; (c = *(unsigned char *)ptr) != '\0'; ++ptr ) {
        if( isdigit( c ) ) {
            digits_seen |= 1 << (c - '0');
            continue;
        } else if( isspace( c ) ) {
            break;
        } else if( c == '.' ) {
            /* note that a float MUST contain a dot
             * OR be ended with an "r"
             * 1234e78 is NOT a valid float */
            return( get_float( tok, input, output ) );
        }
        c = tolower( c );
        if( isxdigit( c ) ) {
            /*
             * process only a-fA-F characters, 0-9 are already processed
             */
            if( base != 16 && radix_base != 16 ) {
                if( c == 'b' ) {
                    if( base == 0 && OK_NUM( BINARY ) ) {
                        c2 = ((unsigned char *)ptr)[1];
                        if( !isxdigit( c2 ) && tolower( c2 ) != 'h' ) {
                            base = 2;
                            suffix_len = 1;
                            break;
                        }
                    }
                } else if( c == 'd' ) {
                    if( base == 0 && OK_NUM( DECIMAL ) ) {
                        c2 = ptr[1];
                        if( !isxdigit( c2 ) && tolower( c2 ) != 'h' ) {
                            if( !isalnum( c2 ) && c2 != '_' ) {
                                base = 10;
                                suffix_len = 1;
                            }
                            break;
                        }
                    }
                }
            }
            digits_seen |= 1 << (c - 'a' + 10);
            continue;
        } else if( c == 'h' ) {
            base = 16;
            suffix_len = 1;
        } else if( c == 't' ) {
            base = 10;
            suffix_len = 1;
        } else if( c == 'o' || c == 'q' ) {
            base = 8;
            suffix_len = 1;
        } else if( c == 'r' ) {
            /* note that a float MUST contain a dot
             * OR be ended with an "r"
             * 1234e78 is NOT a valid float */
            return( get_float( tok, input, output ) );
        } else if( c == 'y' ) {
            base = 2;
            suffix_len = 1;
        }
        break;
    }
    if( digits_seen == 0 ) {
        if( !first_char_0 ) {
            return( get_string( tok, input, output, extra_tokens ) );
        }
        digits_seen |= 1;
        first_char_0 = false;
        dig_start = *input;
    }
#if defined( _STANDALONE_ )
    if( !Options.allow_c_octals ) {
        first_char_0 = false;
    }
#endif
    tok->class = TC_NUM;
    if( base == 0 ) {
        if( base10 ) {
            base = 10;
        } else {
            base = radix_base;
        }
    }
    switch( base ) {
    case 10:
        if( OK_NUM( DECIMAL ) )
            break;
        /* fall through */
    case 8:
        if( OK_NUM( OCTAL ) )
            break;
        /* fall through */
    case 2:
        if( OK_NUM( BINARY ) )
            break;
        /* fall through */
        //AsmError( INVALID_NUMBER_DIGIT );
        /* swallow remainder of token */
        c = *(unsigned char *)ptr;
        while( IS_VALID_ID_CHAR( c ) ) {
            c = *(unsigned char *)(++ptr);
        }
        tok->class = TC_BAD_NUM;
        break;
    }
    /* copy the string, fix input & output pointers */
    tok->string_ptr = *output;
    len = ptr - *input + suffix_len;
    memcpy( *output, *input, len );
    *output += len;
    *(*output)++ = '\0';
    *input = ptr + suffix_len;
    memset( tok->u.bytes, 0, sizeof( tok->u.bytes ) );
    while( dig_start < ptr ) {
        array_mul_add( tok->u.bytes, base, dig_start, sizeof( tok->u.bytes ) );
        ++dig_start;
    }
    return( RC_OK );
} /* get_number */

static bool get_id_in_backquotes( asm_tok *tok, const char **input, char **output )
/*********************************************************************************/
{
    tok->string_ptr = *output;
    tok->class = TC_ID;
    tok->u.value = 0;

    /* copy char from input to output & inc both */
    (*input)++;             // strip off the backquotes
    for( ; **input != '`'; ) {
        *(*output)++ = *(*input)++;
        if( **input == '\0' || **input == ';' ) {
            AsmError( SYNTAX_ERROR );
            return( RC_ERROR );
        }
    }
    (*input)++;         /* don't output the last '`' */
    *(*output)++ = '\0';
    return( RC_OK );
}

static bool get_id( asm_tok *tok, const char **input, char **output )
/********************************************************************
 * get_id could change buf_index, if a COMMENT directive is found
 */
{
    int             c;
    const asm_ins   ASMI86FAR *ins;

    tok->string_ptr = *output;
    if( **input == '\\' ) {
        tok->class = TC_PATH;
    } else {
        tok->class = TC_ID;
    }
    tok->u.value = 0;

    *(*output)++ = *(*input)++;
    for( ; ; ) {
        c = *(unsigned char *)*input;
        /* if character is part of a valid name, add it */
        if( IS_VALID_ID_CHAR( c ) ) {
            *(*output)++ = *(*input)++;
        } else if( c == '\\' ) {
            *(*output)++ = *(*input)++;
            tok->class = TC_PATH;
        } else  {
            break;
        }
    }
    *(*output)++ = '\0';

    /* now decide what to do with it */

    if( tok->class == TC_PATH )
        return( RC_OK );
    ins = get_instruction( tok->string_ptr );
    if( ins == NULL ) {
        if( tok->string_ptr[1] == '\0' && tok->string_ptr[0] == '?' ) {
            tok->class = TC_QUESTION_MARK;
        }
    } else {
        tok->u.token = ins->token;
#if defined( _STANDALONE_ )
        switch( ins->token ) {
        // MASM6 keywords
        case T_FOR:
        case T_FORC:
            if( Options.mode & (MODE_MASM5 | MODE_TASM) ) {
                return( RC_OK );
            }
            break;
        // TASM keywords - initialization
        case T_MASM:
        case T_IDEAL:
            if( (Options.mode_init & MODE_TASM) == 0 ) {
                return( RC_OK );
            }
            break;
        // TASM keywords
        case T_ARG:
        case T_ENUM:
        case T_LOCALS:
        case T_NOLOCALS:
        case T_NOWARN:
        case T_SMART:
        case T_WARN:
            if( (Options.mode & MODE_TASM) == 0 ) {
                return( RC_OK );
            }
            break;
        // TASM IDEAL keywords
        case T_CODESEG:
        case T_CONST:
        case T_DATASEG:
        case T_ERR:
        case T_ERRIFB:
        case T_ERRIFDEF:
        case T_ERRIFDIF:
        case T_ERRIFDIFI:
        case T_ERRIFE:
        case T_ERRIFIDN:
        case T_ERRIFIDNI:
        case T_ERRIFNB:
        case T_ERRIFNDEF:
        case T_EXITCODE:
        case T_FARDATA:
        case T_MODEL:
        case T_P186:
        case T_P286:
        case T_P286N:
        case T_P286P:
        case T_P287:
        case T_P386:
        case T_P386P:
        case T_P387:
        case T_P486:
        case T_P486P:
        case T_P586:
        case T_P586P:
        case T_P686:
        case T_P686P:
        case T_P8086:
        case T_P8087:
        case T_PK3D:
        case T_PMMX:
        case T_PXMM:
        case T_PXMM2:
        case T_PXMM3:
        case T_STACK:
        case T_STARTUPCODE:
        case T_UDATASEG:
        case T_UFARDATA:
            if( (Options.mode & MODE_IDEAL) == 0 ) {
                return( RC_OK );
            }
            break;
        }
#endif
        tok->class = TC_INSTR;
        if( ins->opnd_type1 == OP_SPECIAL ) {
            switch( ins->rm_byte ) {
            case OP_REGISTER:
                tok->class = TC_REG;
                break;
            case OP_RES_ID:
                tok->class = TC_RES_ID;
                if( tok->u.token == T_DP ) {
                    tok->u.token = T_DF;
                }
                break;
            case OP_RES_ID_PTR_MODIF:
                tok->class = TC_RES_ID;
                if( tok->u.token == T_PWORD ) {
                    tok->u.token = T_FWORD;
                }
                break;
            case OP_UNARY_OPERATOR:
                tok->class = TC_UNARY_OPERATOR;
                break;
#if defined( _STANDALONE_ )
            case OP_RELATION_OPERATOR:
                tok->class = TC_RELATION_OPERATOR;
                break;
#endif
            case OP_ARITH_OPERATOR:
                tok->class = TC_ARITH_OPERATOR;
                break;
            case OP_DIRECTIVE:
                tok->class = TC_DIRECTIVE;
#if defined( _STANDALONE_ )
                if( ins->token == T_ENUM ) {
                    EnumDirective = true;
                }
#endif
                break;
#if defined( _STANDALONE_ )
            case OP_DIRECT_EXPR:
                tok->class = TC_DIRECT_EXPR;
                break;
#endif
            }
        }
    }
    return( RC_OK );
}

#if defined( _STANDALONE_ )

static bool get_comment( asm_tok *tok, const char **input, char **output )
{
    size_t  len;
    /* save the whole line .. we need to check
     * if the delim. char shows up 2 times */
    tok->string_ptr = *output;
    len = strlen( *input );
    memcpy( tok->string_ptr, *input, len );
    (*output) += len;
    *(*output)++ = '\0';
    *input += len;
    tok->class = TC_COMMENT_TEXT;
    tok->delim = 0;
    tok->u.value = 0;
    return( RC_OK );
}

#endif

static bool get_special_symbol( asm_tok *tok, const char **input, char **output, int *extra_tokens )
/***************************************************************************************************
 * extra_tokens is forwarded from/to get_string; non-zero only when this
 * function dispatches to get_string and the lexed form is a bracket triple.
 */
{
    char        symbol;
    tok_class   cls;

    *extra_tokens = 0;
    tok->string_ptr = *output;

    symbol = **input;
    switch( symbol ) {
    case '.' :
        cls = TC_DOT;
        break;
    case ',' :
        cls = TC_COMMA;
        break;
    case '+' :
        cls = TC_PLUS;
        break;
    case '-' :
        cls = TC_MINUS;
        break;
    case '*' :
        tok->u.token = T_OP_TIMES;
        cls = TC_ARITH_OPERATOR;
        break;
    case '/' :
        tok->u.token = T_OP_DIVIDE;
        cls = TC_ARITH_OPERATOR;
        break;
    case '[' :
        cls = TC_OP_SQ_BRACKET;
        break;
    case ']' :
        cls = TC_CL_SQ_BRACKET;
        break;
    case '(' :
        cls = TC_OP_BRACKET;
        break;
    case ')' :
        cls = TC_CL_BRACKET;
        break;
    case ':' :
#if defined( _STANDALONE_ )
        if( (*input)[1] == '=' ) {
            cls = TC_COLON_EQU;
            *(*output)++ = *(*input)++;
            break;
        }
#endif
        cls = TC_COLON;
        break;
    case '%' :
        cls = TC_PERCENT;
        break;
#if defined( _STANDALONE_ )
    case '=' :
        tok->u.token = T_EQU2;
        cls = TC_DIRECTIVE;
        break;
    case '}' :
        cls = TC_CL_BRACE;
        break;
#endif
    case '{' :
#if defined( _STANDALONE_ )
        if( EnumDirective ) {
            cls = TC_OP_BRACE;
            break;
        }
#endif
    case '\'' :
    case '"' :
    case '<' :
        /* string delimiters */
        /* fall through */
    default:
        /* anything we don't recognise we will consider a string,
         * delimited by space characters, commas, newlines or nulls
         */
        return( get_string( tok, input, output, extra_tokens ) );
    }
    tok->class = cls;
    *(*output)++ = *(*input)++;
    *(*output)++ = '\0';
    return( RC_OK );
}

#if defined( _STANDALONE_ )
static bool get_inc_path( asm_tok *tok, const char **input, char **output, int *extra_tokens )
/********************************************************************************************
 * For quoted (' or ") paths, emits a single TC_STRING. For bracketed (< or
 * {) paths, emits a bracket triple via get_string; the Include directive
 * walks the triple to find the body. For barewords, emits a single TC_PATH.
 */
{
    char symbol;

    *extra_tokens = 0;
    tok->class = TC_PATH;
    tok->u.value = 0;
    tok->string_ptr = *output;

    while( isspace( **input ) )
        (*input)++;

    symbol = **input;

    switch( symbol ) {
    case '\'' :
    case '"' :
    case '<' :
    case '{' :
        return( get_string( tok, input, output, extra_tokens ) );
    default:
        /* otherwise, just copy whatever is here */
        while( **input && !isspace( **input )  ) {
            *(*output)++ = *(*input)++;
        }
        *(*output)++ = '\0';
        return( RC_OK );
    }
}
#endif

void SetFinalToken( token_buffer *tokbuf, token_idx count )
{
    if( count > MAX_TOKEN_COUNT )
        count = MAX_TOKEN_COUNT;
    tokbuf->tokens[count].class = TC_FINAL;
    tokbuf->tokens[count].string_ptr = NULL;
    tokbuf->tokens[count].delim = 0;
    tokbuf->count = count;
}

bool AsmScan( token_buffer *tokbuf, const char *string )
/*******************************************************
 * - perform syntax checking on scan line;
 * - pass back tokens for later use;
 * - string contains the WHOLE line to scan
 */
{
    const char          *ptr;
    char                *output_ptr;
    asm_tok             *tok;
    int                 c;
    bool                base10;
    token_idx           i;

#ifdef DEBUG_OUT
    CurrString = string;
#endif
#if defined( _STANDALONE_ )
    EnumDirective = false;
#endif

    base10 = false;
    ptr = string;
// FIXME !!
    /* skip initial spaces and expansion codes */
    while( isspace( *ptr ) || (*ptr == '%') ) {
        ptr++;
    }
    output_ptr = tokbuf->stringbuf;
    /*
     * create blank string on buffer beginning
     * for termination token
     */
    *output_ptr++ = '\0';
    tokbuf->count = 0;
    tok = tokbuf->tokens;
    i = 0;
    while( i < MAX_TOKEN_COUNT ) {
        int extra_tokens = 0;   /* non-zero if a bracket triple was emitted */
        tok[i].string_ptr = output_ptr;
        tok[i].delim = 0;
        while( isspace( *ptr ) ) {
            ptr++;
        }
        c = *(unsigned char *)ptr;
        if( c == NULLC ) {
            SetFinalToken( tokbuf, i );
            return( RC_OK );
        }

        if( IS_VALID_ID_CHAR_FIRST( c )
          || c == '\\'
          || ( c == '.' && i == 0 ) ) {
            if( get_id( tok + i, &ptr, &output_ptr ) ) {
                SetFinalToken( tokbuf, i + 1 );
                return( RC_ERROR );
            }
#if defined( _STANDALONE_ )
            if( tok[i].class == TC_DIRECTIVE ) {
                if( tok[i].u.token == T_COMMENT ) {
                    i++;
                    if( i >= MAX_TOKEN_COUNT )
                        break;
                    get_comment( tok + i, &ptr, &output_ptr );
                } else if( tok[i].u.token == T_INCLUDE || tok[i].u.token == T_INCLUDELIB ) {
                    // this mess allows include directives with undelimited file names
                    i++;
                    if( i >= MAX_TOKEN_COUNT )
                        break;
                    get_inc_path( tok + i, &ptr, &output_ptr, &extra_tokens );
                } else if( tok[i].u.token == T_DOT_RADIX ) {
                    base10 = true;
                }
            }
#endif
        } else if( isdigit( c ) ) {
            if( get_number( tok + i, &ptr, &output_ptr, base10, &extra_tokens ) ) {
                /* preserve any partial bracket-triple tokens already written */
                SetFinalToken( tokbuf, i + 1 + extra_tokens );
                return( RC_ERROR );
            }
        } else if( c == '`' ) {
            if( get_id_in_backquotes( tok + i, &ptr, &output_ptr ) ) {
                SetFinalToken( tokbuf, i + 1 );
                return( RC_ERROR );
            }
        } else {
            if( get_special_symbol( tok + i, &ptr, &output_ptr, &extra_tokens ) ) {
                /* preserve any partial bracket-triple tokens already written */
                SetFinalToken( tokbuf, i + 1 + extra_tokens );
                return( RC_ERROR );
            }
        }
        i += extra_tokens;
        i++;
    }
    AsmError( TOO_MANY_TOKENS );
    SetFinalToken( tokbuf, i );
    return( RC_ERROR );
}

void RadixSet( unsigned new_base )
{
    radix_base = new_base;
}
