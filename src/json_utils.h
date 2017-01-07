/* Copyright (C) 2015-2016 Chase
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 *  @file json_utils.h
 *  @brief JSON parser library utility functions
 *
 *  This header declares utility functions for the library.
 */


#ifndef JSON_UTILS_H
#define JSON_UTILS_H


#ifndef JSON_TOP_LVL
#error "The file json_utils.h must not be included directly. Include 'json.h' instead."
#endif	//#ifndef JSON_TOP_LVL


#include "json_parser.h"
#include "json_types.h"

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus

/**
 *  @brief Report an error message
 *
 *  Report an error message passed as a C-string through @p err.
 *
 *  @param[in] err A C-string containing an error message to report
 */
void json_error(const char* err);

/**
 *  @brief Report a formatted error message with line and column number
 *
 *  This function reports an error message along with the line and column number of
 *  the current position of the parser. The first argument @p err should be a printf-like
 *  format string indicating two @c size_t values, e.g. @c "Error: %zu:%zu".
 *
 *  @param[in] err A C-string indicating two @c size_t values
 *  @param[in] parserState Pointer to the parser's json_parser_state instance
 *
 *  @see printf(3)
 */
void json_error_lineno(const char* err, json_parser_state* parserState);

/**
 *  @brief Unescape a JSON string value
 *
 *  This function unescapes a JSON string to a UTF-8 encoded string. The basic escape sequences
 *  as well as Unicode escape sequences are unescaped, including UTF-16 surrogate pairs. The string
 *  memory is allocated from the @p parserState json_allocator, which is initialized with the call
 *  to json_parser_init().
 *
 *  The argument @p state is used to signal the success of the operation. Zero is placed in @p state
 *  on success, and nonzero on failure.
 *
 *  This function returns @c NULL if memory allocation fails, or the allocated string otherwise. If an
 *  error other than from memory allocation is encountered, a partially processed string is returned.
 *  If the returned string is not @c NULL and @p indicates failure, the string may not be @c null
 *  terminated. Only @p n characters are read from @p str. If the end of @p str is reached before
 *  the function is done processing, the partially unescaped string will be returned with @p state
 *  set to nonzero. If @p state is zero, the returned string will be @c null terminated and the length
 *  of the string will be returned in @p unescapedLen. If @p state is nonzero, the length of the
 *  allocated string is returned in @p unescapedLen, but it will be incomplete.
 *
 *  If @p str is @c NULL or @p n is zero, an empty string will be allocated and returned. In this
 *  case, @p unescapedLen will be set to zero but an allocated empty string is returned.
 *
 *  Note that JSON strings can contain Unicode escape sequences for control characters, which
 *  means that @c "'\u0000'" in a string will be unescaped to Unicode @c null char in memory. It is
 *  therefore safest to use the returned length of the string for processing and to not rely on
 *  it being a C string; though if no error occurs a string containing the Unicode @c null char will
 *  still be terminated with @c null char at the end.
 *
 *  @param[in] parserState Pointer to the parser's json_parser_state instance
 *  @param[in] str The JSON string value to unescape
 *  @param n Length of the JSON string @p str
 *  @param[out] state Pointer to an integer to receive state of operation, zero for success and nonzero on failure
 *  @param[out] unescapedLen Pointer to @c size_t to receive length of the unescaped string returned
 *  @return Pointer to the allocated UTF-8 encoded string, or @c NULL on failure
 *
 *  @see https://tools.ietf.org/html/rfc7159#section-7
 */
char* json_utils_unescape_string(json_parser_state* parserState, const char* str, size_t n, int* state, size_t* unescapedLen);


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_UTILS_H
