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
#include <stdbool.h>
#include <limits.h>


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


#endif	//#ifndef JSON_INTROSPECT_C