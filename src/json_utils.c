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

#ifndef JSON_UTILS_C
#define JSON_UTILS_C


#define JSON_TOP_LVL 1


#include "json_utils.h"
#include "json_parser.h"
#include "json_types.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


//Maps hexadecimal ascii digits to corresponding integral value
static const uint8_t xascii_to_uint8_map[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
	0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static char uni_buffer[16];


static inline uint16_t uni_str_to_num(const char* str);
static char* uni_num_to_utf8_str(uint32_t uni, size_t* numBytes);
static inline int uni_is_valid_surrogate_pair(uint16_t uni1, uint16_t uni2);
static inline char* uni_surrogate_pair_to_utf8_str(uint16_t uni1, uint16_t uni2, size_t* numBytes);


void json_error(const char* err) {
	fprintf(stderr, "%s\n", err);
}

void json_error_lineno(const char* err, json_parser_state* parserState) {
	if (!parserState->errorStream) {
		return;
	}
	const char* ptr = parserState->jsonStr;
	size_t count = 0, n = parserState->jsonStrPos, lastNL = 0;
	for (size_t k = 0; k < n; k += 1) {
		if (ptr[k] == '\n') {
			count += 1;
			lastNL = k;
		}
	}
	fprintf(parserState->errorStream, err, count, n - lastNL);
}

//Unescape the passed json string of given size to a null-terminated UTF-8 string
//Allocates the returned string from JSON_Allocator::malloc
//TODO: Check that all UTF-16 surrogate pairs are properly encoded to UTF-8
char* json_utils_unescape_string(json_parser_state* parserState, const char* str, size_t n, int* ret, size_t* unescapedLen) {
	char* unescaped = NULL;
	*ret = 1;
	*unescapedLen = 0;
	if (!str || !n) {
		unescaped = (char*) parserState->JSON_Allocator->malloc( sizeof(char) );
		if (unescaped) {
			unescaped[0] = 0;
			*ret = 0;
		}
		return unescaped;
	}
	
	unescaped = (char*) parserState->JSON_Allocator->malloc( sizeof(char) * n + 1 );
	if (!unescaped) {
		return unescaped;
	}
	*unescapedLen = n;
	
	size_t j = 0, k = 0;
	
	while (k < n) {
		if (str[k] == JSON_TOKEN_NAMES[json_token_backslash]) {
			if (k + 1 >= n) {//Reached eos
				return unescaped;
			}
			
			char c = 0;
			switch (str[k + 1]) {
				case '"':
				case '\\':
				case '/':
					c = str[k + 1];
				break;
				case 'b':
					c = 0x08;
				break;
				case 'f':
					c = 0x0C;
				break;
				case 'n':
					c = 0x0A;
				break;
				case 'r':
					c = 0x0D;
				break;
				case 't':
					c = 0x09;
				break;
				case 'u': {
					uint16_t uni1 = 0, uni2 = 0;
					const char* uniBuff = NULL;
					int isSurrogatePair = 0;
					
					if (//Reached eos, or bad unicode escape
						k + 5 >= n
						|| !isxdigit(str[k + 2])
						|| !isxdigit(str[k + 3])
						|| !isxdigit(str[k + 4])
						|| !isxdigit(str[k + 5])
					) {
						return unescaped;
					}
					
					size_t numBytes = 0;
					if (//This escape sequence and the following form a valid surrogate pair
						(k + 11 <= n)
						&& (str[k + 6] == '\\')
						&& (str[k + 7] == 'u')
						&& isxdigit(str[k + 8])
						&& isxdigit(str[k + 9])
						&& isxdigit(str[k + 10])
						&& isxdigit(str[k + 11])
						&& (uni1 = uni_str_to_num(str + k + 2))
						&& (uni2 = uni_str_to_num(str + k + 8))
						&& uni_is_valid_surrogate_pair(uni1, uni2)
					) {
						uniBuff = uni_surrogate_pair_to_utf8_str(uni1, uni2, &numBytes);
						isSurrogatePair = 1;
					} else {//Single unicode escape sequence
						uni1 = uni_str_to_num(str + k + 2);
						uniBuff = uni_num_to_utf8_str(uni1, &numBytes);
					}
					
					size_t m = 0;
					while (m < numBytes) {
						unescaped[j] = uniBuff[m];
						j += 1;
						m += 1;
					}
					k += (isSurrogatePair) ? 12 : 6;
					continue;
				}
				break;
				default:
					return unescaped;
				break;
			}
			unescaped[j] = c;
			j += 1;
			k += 2;
			continue;
		}
		unescaped[j] = str[k];
		j += 1;
		k += 1;
	}
	unescaped[j] = 0;
	
	*ret = 0;
	*unescapedLen = j;
	return unescaped;
}

//Returns a 16 bit integer from the 4-character unicode escape sequence given
static inline uint16_t uni_str_to_num(const char* str) {
	return (
		(xascii_to_uint8_map[ (unsigned char) str[0] ] << 12)
		+ (xascii_to_uint8_map[ (unsigned char) str[1] ] << 8)
		+ (xascii_to_uint8_map[ (unsigned char) str[2] ] << 4)
		+ (xascii_to_uint8_map[ (unsigned char) str[3] ])
	);
}

//Returns a UTF-8 encoded string from the given 32 bit integer
static char* uni_num_to_utf8_str(uint32_t uni, size_t* numBytes) {
	size_t pos = 0;
	if (uni < 0x80) {
		uni_buffer[pos] = uni;
		pos += 1;
	} else if (uni < 0x800) {
		uni_buffer[pos] = 0xC0 | ( (uni & 0x1F00) >> 8);
		pos += 1;
		uni_buffer[pos] = 0x80 | (uni & 0x3F);
		pos += 1;
	} else if (uni < 0xFFFF) {
		uni_buffer[pos] = 0xE0 | ( (uni & 0xF000) >> 12);
		pos += 1;
		uni_buffer[pos] = 0x80 | ( (uni & 0xFC0) >> 6);
		pos += 1;
		uni_buffer[pos] = 0x80 | (uni & 0x3F);
		pos += 1;
	} else {
		uni_buffer[pos] = 0xF0 | ( (uni & 0x1C0000) >> 18);
		pos += 1;
		uni_buffer[pos] = 0x80 | ( (uni & 0x3F000) >> 12);
		pos += 1;
		uni_buffer[pos] = 0x80 | ( (uni & 0xFC0) >> 6);
		pos += 1;
		uni_buffer[pos] = 0x80 | (uni & 0x3F);
		pos += 1;
	}
	uni_buffer[pos] = 0;
	if (numBytes) {
		*numBytes = pos;
	}
	return uni_buffer;
}

//Checks if the two given 16 bit integers form a valid UTF-16 surrogate pair
static inline int uni_is_valid_surrogate_pair(uint16_t uni1, uint16_t uni2) {
	return (
		(uni1 >= 0xD800 && uni1 <= 0xDBFF)
		&& (uni2 >= 0xDC00 && uni2 <= 0xDFFF)
	);
}
//Returns a UTF-8 encoded string from the surrogate pair in the two given 16 bit integers
static inline char* uni_surrogate_pair_to_utf8_str(uint16_t uni1, uint16_t uni2, size_t* numBytes) {
	return uni_num_to_utf8_str(
		((uni1 & 0x3FF) << 10)
		+ (uni2 & 0x3FF)
		+ 0x10000,
		numBytes
	);
}


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_UTILS_C
