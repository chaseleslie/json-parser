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


#ifndef JSON_INTROSPECT_C
#define JSON_INTROSPECT_C


#define JSON_TOP_LVL 1


#include "json_types.h"
#include "json_introspect.h"
#include "json_parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <limits.h>


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


const size_t JSON_STR_BUFF_INIT_SIZE = 32;
const double JSON_STR_BUFF_INCR_SIZE = 1.5;


//Loop through string:value pairs in the passed object, call iter callback passing object, string and value
//Returns nonzero if passed object or callback is null, zero otherwise
//Callback should return truthy value, or zero to stop iterating
int json_object_foreach(json_value* val, json_object_foreach_cb iter) {
	int retVal = 1;
	if (!val || !iter) {
		return retVal;
	}
	
	if (val->valueType != object_value) {
		return retVal;
	}
	
	return json_object_foreach_obj(val->value, iter);
}
int json_object_foreach_obj(json_object* obj, json_object_foreach_cb iter) {
	if (!obj || !iter) {
		return 1;
	}
	
	size_t k = 0, numElems = obj->size;
	while (k < numElems && iter(obj, obj->names[k], obj->values[k])) {
		k += 1;
	}
	
	return 0;
}

//Loop through elements in the passed array, call iter callback passing array and element
//Returns nonzero if passed array or callback is null, zero otherwise
//Callback should return truthy value, or zero to stop iterating
int json_array_foreach(json_value* val, json_array_foreach_cb iter) {
	int retVal = 1;
	if (!val || !iter) {
		return retVal;
	}
	
	if (val->valueType != array_value) {
		return retVal;
	}
	
	return json_array_foreach_arr(val->value, iter);
}
int json_array_foreach_arr(json_array* arr, json_array_foreach_cb iter) {
	if (!arr || !iter) {
		return 1;
	}
	
	size_t k = 0, numElems = arr->size;
	while (k < numElems && iter(arr, arr->values[k])) {
		k += 1;
	}
	
	return 0;
}

/* JSON Pointer functions */

typedef struct reference_token {
	char* token;
	size_t tokenLen;
} reference_token;

//Assumptions:
//jsonPtrStrLen > 1,
//jsonPtrStr[0] == '/'
//jsonPtrStr[jsonPtrStrLen - 1] != '/'
static reference_token* parse_json_pointer(
	json_allocator* jsAlloc,
	const char* jsonPtrStr,
	const size_t jsonPtrStrLen,
	size_t* tokensLen
) {
	reference_token* tokens = NULL;
	
	size_t numTokens = 0;
	int haveSep = 0;
	for (size_t k = 0; k < jsonPtrStrLen; k += 1) {
		if (haveSep) {
			if (jsonPtrStr[k] == '/') {
				return tokens;
			}
			haveSep = 0;
			numTokens += 1;
		} else {
			if (jsonPtrStr[k] == '/') {
				haveSep = 1;
			}
		}
	}
	
	tokens = jsAlloc->malloc(sizeof(reference_token) * numTokens);
	if (!tokens) {
		return tokens;
	}
	
	size_t tokenIndex = 0;
	const char* lastTokenStart = jsonPtrStr;
	for (size_t k = 0; k < jsonPtrStrLen; k += 1) {
		if ((k && jsonPtrStr[k] == '/') || k == (jsonPtrStrLen - 1)) {
			size_t off = (k == (jsonPtrStrLen - 1)) ? 1 : 0;
			const size_t tokenLen = (jsonPtrStr + k + off) - (lastTokenStart + 1);
			reference_token* token = tokens + tokenIndex;
			token->tokenLen = tokenLen;
			token->token = jsAlloc->malloc(sizeof(char) * tokenLen);
			if (!token->token) {
				for (size_t iK = 0; iK < tokenIndex; iK += 1) {
					jsAlloc->free(tokens[iK].token);
				}
				jsAlloc->free(tokens);
				return NULL;
			}
			
			const char* ptr = lastTokenStart + 1;
			for (size_t iK = 0; iK < tokenLen; iK += 1) {
				token->token[iK] = ptr[iK];
			}
			
			lastTokenStart = jsonPtrStr + k;
			tokenIndex += 1;
		}
	}
	
	//Replace all "~1" with '/'
	for (size_t k = 0; k < numTokens; k += 1) {
		const size_t tokenLen = tokens[k].tokenLen;
		char* ptr = tokens[k].token;
		size_t tokenLenUnescaped = tokenLen;
		for (size_t iK = 0; iK < tokenLen; iK += 1) {
			if (ptr[iK] == '~') {
				if (
					(iK + 1) >= tokenLen ||
					(ptr[iK + 1] != '0' && ptr[iK + 1] != '1')
				) {//Incomplete/invalid escaped token
					for (size_t n = 0; n < numTokens; n += 1) {
						jsAlloc->free(tokens[n].token);
					}
					jsAlloc->free(tokens);
					return NULL;
				} else {
					if (ptr[iK + 1] == '1') {
						ptr[iK] = '/';
						for (size_t n = iK + 2, m = iK + 1; n < tokenLenUnescaped; n += 1, m += 1) {
							ptr[m] = ptr[n];
						}
						
						tokenLenUnescaped -= 1;
					}
				}
			}
		}
		tokens[k].tokenLen = tokenLenUnescaped;
	}
	
	//Replace all "~0" with '~'
	for (size_t k = 0; k < numTokens; k += 1) {
		const size_t tokenLen = tokens[k].tokenLen;
		char* ptr = tokens[k].token;
		size_t tokenLenUnescaped = tokenLen;
		for (size_t iK = 0; iK < tokenLen; iK += 1) {
			if (ptr[iK] == '~') {
				if (
					(iK + 1) >= tokenLen ||
					(ptr[iK + 1] != '0' && ptr[iK + 1] != '1')
				) {//Incomplete/invalid escaped token
					for (size_t n = 0; n < numTokens; n += 1) {
						jsAlloc->free(tokens[n].token);
					}
					jsAlloc->free(tokens);
					return NULL;
				} else {
					if (ptr[iK + 1] == '0') {
						ptr[iK] = '~';
						for (size_t n = iK + 2, m = iK + 1; n < tokenLenUnescaped; n += 1, m += 1) {
							ptr[m] = ptr[n];
						}
						
						tokenLenUnescaped -= 1;
					}
				}
			}
		}
		tokens[k].tokenLen = tokenLenUnescaped;
	}
	
	*tokensLen = numTokens;
	return tokens;
}

json_value* json_value_query(
	json_parser_state* parserState,
	json_value* value,
	const char* query,
	const size_t queryLen
) {
	json_value* val = NULL;
	if (
		!parserState || !value
		|| !query || !queryLen
		|| *query != '/'
		|| query[queryLen - 1] == '/'
		|| (value->valueType != object_value
		&& value->valueType != array_value)
	) {
		return val;
	} else if (queryLen == 1 && *query == '/') {
		return value;
	}
	
	size_t numTokens = 0;
	reference_token* tokens = parse_json_pointer(
		parserState->JSON_Allocator,
		query,
		queryLen,
		&numTokens
	);
	if (!tokens) {
		return val;
	}
	
	int err = 0;
	val = value;
	for (size_t k = 0; k < numTokens; k += 1) {
		reference_token* token = tokens + k;
		if (val->valueType == array_value) {
			json_array* arr = val->value;
			const size_t maxDigits = 32;
			char digits[32];
			if (!arr->size || token->tokenLen > maxDigits - 1) {
				err = 1;
				val = NULL;
			} else {
				switch (token->token[0]) {
					case '0': {
						if (token->tokenLen != 1) {
							err = 1;
							val = NULL;
						} else {
							val = arr->values[0];
						}
					}
					break;
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9': {
						char* ptr = token->token;
						const size_t tokenLen = token->tokenLen;
						for (size_t iK = 0; iK < tokenLen; iK += 1) {
							digits[iK] = ptr[iK];
						}
						digits[tokenLen] = 0;
						long index = strtol(digits, &ptr, 10);
						if (!index || index == LONG_MAX || index >= arr->size || (ptr - digits) != tokenLen) {
							err = 1;
							val = NULL;
						} else {
							val = arr->values[index];
						}
					}
					break;
					case '-':
					default: {
						err = 1;
						val = NULL;
					}
					break;
				}
			}
		} else if (val->valueType == object_value) {
			json_object* obj = val->value;
			bool matched = false;	//Did any obj name match
			for (size_t iK = 0, numNames = obj->size; iK < numNames; iK += 1) {
				const char* ptr = obj->names[iK]->value;
				const size_t ptrLen = obj->names[iK]->valueLen;
				bool noMatch = false;	//Did this obj name match
				
				if (token->tokenLen == ptrLen) {
					for (size_t n = 0; n < token->tokenLen && n < ptrLen; n += 1) {
						if (ptr[n] != token->token[n]) {
							noMatch = true;
							break;
						}
					}
					if (noMatch) {
						continue;
					}
				} else {
					continue;
				}
				
				matched = true;
				val = obj->values[iK];
				break;
			}
			if (!matched) {
				err = 1;
				val = NULL;
			}
		} else {
			err = 1;
			val = NULL;
		}
		
		if (err) {
			break;
		}
	}
	
	for (size_t k = 0; k < numTokens; k += 1) {
		parserState->JSON_Allocator->free(tokens[k].token);
	}
	parserState->JSON_Allocator->free(tokens);
	return val;
}

/* JSON stringify functions */

typedef struct json_string_buffer {
	char* string;
	size_t size;
	size_t capacity;
	const char* indent;
	size_t indentLen;
	size_t indentLevel;
	int flags;
} json_string_buffer;

int json_value_stringify_value(json_parser_state* parserState, json_string_buffer* strBuff, json_value* value);
int json_value_stringify_object(json_parser_state* parserState, json_string_buffer* strBuff, json_object* obj);
int json_value_stringify_array(json_parser_state* parserState, json_string_buffer* strBuff, json_array* arr);
int json_value_stringify_string(json_parser_state* parserState, json_string_buffer* strBuff, json_string* str);
int json_value_stringify_number(json_parser_state* parserState, json_string_buffer* strBuff, json_number* num);

//Resize and copy contents over if necessary
//addSize is the additional size needed for the string
static int json_string_buffer_resize(json_parser_state* parserState, json_string_buffer* strBuff, const size_t addSize) {
	int retVal = 1;
	const size_t neededSize = strBuff->size + addSize;
	
	if (neededSize > strBuff->capacity) {
		char* tmpStr = NULL;
		size_t newCap = align_offset(strBuff->capacity * JSON_STR_BUFF_INCR_SIZE, 16);
		
		//Try to maintain constant growth of capacity
		//If inadequate, grow from neededSize
		if (newCap < neededSize) {
			newCap = align_offset(neededSize * JSON_STR_BUFF_INCR_SIZE, 16);
		}
		
		tmpStr = parserState->JSON_Allocator->malloc(sizeof(char) * newCap);
		
		if (!tmpStr) {
			return retVal;
		}
		
		char* ptr = strBuff->string;
		for (size_t k = 0, n = strBuff->size; k < n; k += 1) {
			tmpStr[k] = ptr[k];
		}
		
		strBuff->capacity = newCap;
		parserState->JSON_Allocator->free(ptr);
		strBuff->string = tmpStr;
	}
	
	retVal = 0;
	return retVal;
}

//Append to the buffer
static int json_string_buffer_append(json_parser_state* parserState, json_string_buffer* strBuff, const char* str, const size_t strLen) {
	int retVal = 1;
	
	retVal = json_string_buffer_resize(parserState, strBuff, strLen);
	if (retVal) {
		return retVal;
	}
	
	char* ptr = strBuff->string;
	for (size_t k = strBuff->size, n = k + strLen, m = 0; k < n; k += 1, m += 1) {
		ptr[k] = str[m];
	}
	
	strBuff->size += strLen;
	
	retVal = 0;
	return retVal;
}

//Append to the buffer escaped
static int json_string_buffer_append_escaped(json_parser_state* parserState, json_string_buffer* strBuff, const char* str) {
	int retVal = 1;
	
	const size_t buffLen = 32;
	char buff[32];
	const uint8_t* ptr = (uint8_t*) str;
	uint32_t c1 = ptr[0];
	bool withinBMP = true;
	uint32_t codePoint = 0;
	
	if (c1 < 0x80) {
		codePoint = c1;
	} else if (c1 >= 0xC0 && c1 < 0xE0) {
		codePoint = ((c1 & 0x1F) << 6) + (ptr[1] & 0x3F);
	} else if (c1 >= 0xE0 && c1 < 0xF0) {
		codePoint = ((c1 & 0x0F) << 12) + ((ptr[1] & 0x3F) << 6) + (ptr[2] & 0x3F);
	} else if (c1 >= 0xF0) {
		withinBMP = false;
		codePoint = ((c1 & 0x07) << 18) + ((ptr[1] & 0x3F) << 12) + ((ptr[2] & 0x3F) << 6) + (ptr[3] & 0x3F);
	} else {
		return retVal;
	}
	
	int ret = 0;
	if (withinBMP) {
		ret = snprintf(buff, buffLen, "\\u%04X", codePoint);
		if (ret < 0 || ret >= buffLen) {
			return retVal;
		}
	} else {
		const uint32_t cp = codePoint - 0x10000;
		const uint16_t high = 0xD800 + (cp >> 10);
		const uint16_t low = 0xDC00 + (cp & 0x3FF);
		ret = snprintf(buff, buffLen, "\\u%04" PRIX16 "\\u%04" PRIX16, high, low);
		if (ret < 0 || ret >= buffLen) {
			return retVal;
		}
	}
	
	return json_string_buffer_append(parserState, strBuff, buff, ret);
}

//Indent the buffer
static int json_string_buffer_indent(json_parser_state* parserState, json_string_buffer* strBuff, const char* indent, const size_t indentLen, const size_t num) {
	int retVal = 1;
	
	retVal = json_string_buffer_resize(parserState, strBuff, indentLen * num);
	if (retVal) {
		return retVal;
	}
	
	for (size_t k = 0; k < num; k += 1) {
		retVal = json_string_buffer_append(parserState, strBuff, indent, indentLen);
		if (retVal) {
			return retVal;
		}
	}
	
	retVal = 0;
	return retVal;
}

char* json_value_stringify(
	json_parser_state* parserState,
	json_value* value,
	const char* indent,
	int flags,
	size_t* strLen
) {
	if (!parserState || !value || !strLen) {
		return NULL;
	}
	
	json_string_buffer strBuff = {NULL, 0, 0, indent ? indent : "\t", 0, 0, flags};
	strBuff.string = parserState->JSON_Allocator->malloc(sizeof(char) * JSON_STR_BUFF_INIT_SIZE);
	if (!strBuff.string) {
		return NULL;
	}
	strBuff.capacity = JSON_STR_BUFF_INIT_SIZE;
	const char* tmp = strBuff.indent;
	while (*tmp) {
		strBuff.indentLen += 1;
		tmp += 1;
	}
	
	int retVal = json_value_stringify_value(parserState, &strBuff, value);
	if (retVal) {
		parserState->JSON_Allocator->free(strBuff.string);
		return NULL;
	}
	
	retVal = json_string_buffer_append(parserState, &strBuff, "\0", 1);
	if (retVal) {
		parserState->JSON_Allocator->free(strBuff.string);
		return NULL;
	}
	
	*strLen = strBuff.size;
	return strBuff.string;
}

int json_value_stringify_value(
	json_parser_state* parserState,
	json_string_buffer* strBuff,
	json_value* value
) {
	int retVal = 1;
	if (!parserState || !strBuff || !value) {
		return retVal;
	}
	const int flags = strBuff->flags;
	
	switch (value->valueType) {
		case object_value: {
			retVal = json_string_buffer_append(parserState, strBuff, &JSON_TOKEN_NAMES[json_token_lbrace], 1);
			if (retVal) {
				return retVal;
			}
			
			strBuff->indentLevel += 1;
			retVal = json_value_stringify_object(parserState, strBuff, value->value);
			if (retVal) {
				return retVal;
			}
			strBuff->indentLevel -= 1;
			
			if (flags & json_stringify_indent) {
				retVal = json_string_buffer_indent(parserState, strBuff, strBuff->indent, strBuff->indentLen, strBuff->indentLevel);
				if (retVal) {
					return retVal;
				}
			}
			retVal = json_string_buffer_append(parserState, strBuff, &JSON_TOKEN_NAMES[json_token_rbrace], 1);
			if (retVal) {
				return retVal;
			}
		}
		break;
		case array_value: {
			retVal = json_string_buffer_append(parserState, strBuff, &JSON_TOKEN_NAMES[json_token_lbrack], 1);
			if (retVal) {
				return retVal;
			}
			
			strBuff->indentLevel += 1;
			retVal = json_value_stringify_array(parserState, strBuff, value->value);
			if (retVal) {
				return retVal;
			}
			strBuff->indentLevel -= 1;
			
			if (flags & json_stringify_indent) {
				retVal = json_string_buffer_indent(parserState, strBuff, strBuff->indent, strBuff->indentLen, strBuff->indentLevel);
				if (retVal) {
					return retVal;
				}
			}
			retVal = json_string_buffer_append(parserState, strBuff, &JSON_TOKEN_NAMES[json_token_rbrack], 1);
			if (retVal) {
				return retVal;
			}
		}
		break;
		case string_value: {
			retVal = json_value_stringify_string(parserState, strBuff, value->value);
			if (retVal) {
				return retVal;
			}
		}
		break;
		case number_value: {
			retVal = json_value_stringify_number(parserState, strBuff, value->value);
			if (retVal) {
				return retVal;
			}
		}
		break;
		case true_value: {
			retVal = json_string_buffer_append(parserState, strBuff, "true", 4);
			if (retVal) {
				return retVal;
			}
		}
		break;
		case false_value: {
			retVal = json_string_buffer_append(parserState, strBuff, "false", 5);
			if (retVal) {
				return retVal;
			}
		}
		break;
		case null_value: {
			retVal = json_string_buffer_append(parserState, strBuff, "null", 4);
			if (retVal) {
				return retVal;
			}
		}
		break;
		default:
			return retVal;
		break;
	}
	
	retVal = 0;
	return retVal;
}

int json_value_stringify_object(
	json_parser_state* parserState,
	json_string_buffer* strBuff,
	json_object* obj
) {
	int retVal = 1;
	const int flags = strBuff->flags;
	
	if (flags & json_stringify_indent) {
		retVal = json_string_buffer_append(parserState, strBuff, "\n", 1);
		if (retVal) {
			return retVal;
		}
	}
	
	for (size_t k = 0, n = obj->size; k < n; k += 1) {
		if (flags & json_stringify_indent) {
			retVal = json_string_buffer_indent(parserState, strBuff, strBuff->indent, strBuff->indentLen, strBuff->indentLevel);
			if (retVal) {
				return retVal;
			}
		}
		
		retVal = json_value_stringify_string(parserState, strBuff, obj->names[k]);
		
		if (flags & json_stringify_spaces) {
			retVal = json_string_buffer_append(parserState, strBuff, ": ", 2);
			if (retVal) {
				return retVal;
			}
		} else {
			retVal = json_string_buffer_append(parserState, strBuff, ":", 1);
			if (retVal) {
				return retVal;
			}
		}
		
		retVal = json_value_stringify_value(parserState, strBuff, obj->values[k]);
		if (retVal) {
			return retVal;
		}
		
		if (k < n - 1) {
			if (flags & json_stringify_spaces && !(flags & json_stringify_indent)) {
				retVal = json_string_buffer_append(parserState, strBuff, ", ", 2);
				if (retVal) {
					return retVal;
				}
			} else if (flags & json_stringify_indent) {
				retVal = json_string_buffer_append(parserState, strBuff, ",\n", 2);
				if (retVal) {
					return retVal;
				}
			} else {
				retVal = json_string_buffer_append(parserState, strBuff, ",", 1);
				if (retVal) {
					return retVal;
				}
			}
		}
	}
	
	if (flags & json_stringify_indent) {
		retVal = json_string_buffer_append(parserState, strBuff, "\n", 1);
		if (retVal) {
			return retVal;
		}
	}
	
	retVal = 0;
	return retVal;
}

int json_value_stringify_array(
	json_parser_state* parserState,
	json_string_buffer* strBuff,
	json_array* arr
) {
	int retVal = 1;
	const int flags = strBuff->flags;
	
	if (flags & json_stringify_indent) {
		retVal = json_string_buffer_append(parserState, strBuff, "\n", 1);
		if (retVal) {
			return retVal;
		}
	}
	
	for (size_t k = 0, n = arr->size; k < n; k += 1) {
		if (flags & json_stringify_indent) {
			retVal = json_string_buffer_indent(parserState, strBuff, strBuff->indent, strBuff->indentLen, strBuff->indentLevel);
			if (retVal) {
				return retVal;
			}
		}
		retVal = json_value_stringify_value(parserState, strBuff, arr->values[k]);
		if (retVal) {
			return retVal;
		}
		
		if (k < n - 1) {
			if (flags & json_stringify_spaces && !(flags & json_stringify_indent)) {
				retVal = json_string_buffer_append(parserState, strBuff, ", ", 2);
				if (retVal) {
					return retVal;
				}
			} else if (flags & json_stringify_indent) {
				retVal = json_string_buffer_append(parserState, strBuff, ",\n", 2);
				if (retVal) {
					return retVal;
				}
			} else {
				retVal = json_string_buffer_append(parserState, strBuff, ",", 1);
				if (retVal) {
					return retVal;
				}
			}
		}
	}
	
	if (flags & json_stringify_indent) {
		retVal = json_string_buffer_append(parserState, strBuff, "\n", 1);
		if (retVal) {
			return retVal;
		}
	}
	
	retVal = 0;
	return retVal;
}

//TODO: Handle flags regarding Unicode escapes in strings
int json_value_stringify_string(
	json_parser_state* parserState,
	json_string_buffer* strBuff,
	json_string* str
) {
	int retVal = 1;
	const int flags = strBuff->flags;
	
	retVal = json_string_buffer_append(parserState, strBuff, "\"", 1);
	if (retVal) {
		return retVal;
	}
	
	const char* ptr = str->value;
	const size_t ptrLen = str->valueLen;
	size_t pos = 0;
	bool escapeNonAscii = flags & json_stringify_escape_non_ascii;
	bool escapeNonBmp = flags & json_stringify_escape_non_bmp;
	
	while (pos < ptrLen) {
		uint8_t c1 = ptr[pos];
		bool needEscape = false;
		size_t incr = 1;
		
		if (c1 < 0x1F) {
			needEscape = true;
			incr = 1;
		} else if (c1 < 0x80) {
			needEscape = false;
			incr = 1;
		} else if (c1 >= 0xC0 && c1 < 0xE0 && (pos + 1 < ptrLen)) {
			needEscape = escapeNonAscii;
			incr = 2;
		} else if (c1 >= 0xE0 && c1 < 0xF0 && (pos + 2 < ptrLen)) {
			needEscape = escapeNonAscii;
			incr = 3;
		} else if (c1 >= 0xF0 && (pos + 3 < ptrLen)) {
			needEscape = escapeNonAscii || escapeNonBmp;
			incr = 4;
		} else {
			return retVal;
		}
		
		if (needEscape) {
			retVal = json_string_buffer_append_escaped(parserState, strBuff, ptr + pos);
			if (retVal) {
				return retVal;
			}
		} else {
			retVal = json_string_buffer_append(parserState, strBuff, ptr + pos, incr);
			if (retVal) {
				return retVal;
			}
		}
		
		pos += incr;
	}
	
	retVal = json_string_buffer_append(parserState, strBuff, "\"", 1);
	if (retVal) {
		return retVal;
	}
	
	retVal = 0;
	return retVal;
}

int json_value_stringify_number(
	json_parser_state* parserState,
	json_string_buffer* strBuff,
	json_number* num
) {
	int retVal = 1;
	const size_t buffLen = 128;
	char buff[128];
	
	int bytes = snprintf(buff, buffLen, "%g", num->value);
	if (bytes < 1 || bytes >= buffLen) {
		return retVal;
	}
	
	retVal = json_string_buffer_append(parserState, strBuff, buff, bytes);
	if (retVal) {
		return retVal;
	}
	
	retVal = 0;
	return retVal;
}


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_INTROSPECT_C