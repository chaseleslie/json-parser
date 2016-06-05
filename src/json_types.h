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

#ifndef JSON_TYPES_H
#define JSON_TYPES_H


#ifndef JSON_TOP_LVL
#error "The file json_types.h must not be included directly. Include 'json.h' instead."
#endif	//#ifndef JSON_TOP_LVL


//TODO: Add up-reference to parent nodes from value (child) nodes


#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


typedef enum JSON_VALUE {
	unspecified_value = 0,
	string_value,
	number_value,
	object_value,
	array_value,
	true_value,
	false_value,
	null_value,
	JSON_VALUE_COUNT
} JSON_VALUE;

extern const char* const JSON_VALUE_NAMES[];

typedef enum JSON_PARSER_STATE {
	init_state = 0,
	complete_state,
	error_state,
	JSON_PARSER_STATE_MAX
} JSON_PARSER_STATE;

extern const char* const JSON_PARSER_STATE_NAMES[];

typedef enum JSON_TOKEN {
	json_token_quote = 0,
	json_token_plus,
	json_token_minus,
	json_token_lbrace,
	json_token_rbrace,
	json_token_lbrack,
	json_token_rbrack,
	json_token_colon,
	json_token_comma,
	json_token_backslash,
	JSON_TOKEN_COUNT
} JSON_TOKEN;

extern const char JSON_TOKEN_NAMES[];

struct json_parser_state;
typedef struct json_parser_state json_parser_state;

struct json_allocator;
typedef struct json_allocator json_allocator;
struct json_factory;
typedef struct json_factory json_factory;

struct json_object;
typedef struct json_object json_object;
struct json_value;
typedef struct json_value json_value;

struct json_string;
typedef struct json_string json_string;
struct json_number;
typedef struct json_number json_number;
struct json_array;
typedef struct json_array json_array;
struct json_true;
typedef struct json_true json_true;
struct json_false;
typedef struct json_false json_false;
struct json_null;
typedef struct json_null json_null;


int json_types_init(void* (*allocFunction)(size_t), void (*freeFunction)(void*), json_allocator** jsAllocOut, json_factory** jsFactoryOut);

json_object* json_factory_new_json_object(json_factory* jsonFact, json_value* objParentValue);
json_value* json_factory_new_json_value(json_factory* jsonFact, JSON_VALUE valValueType, void* valValue, JSON_VALUE valParentValueType, void* valParentValue);
json_string* json_factory_new_json_string(json_factory* jsonFact, const char* strValue, json_value* strParentValue);
json_number* json_factory_new_json_number(json_factory* jsonFact, double numValue, json_value* numParentValue);
json_array* json_factory_new_json_array(json_factory* jsonFact, json_value* arrParentValue);
json_true* json_factory_new_json_true(json_factory* jsonFact, json_value* truParentValue);
json_false* json_factory_new_json_false(json_factory* jsonFact, json_value* falParentValue);
json_null* json_factory_new_json_null(json_factory* jsonFact, json_value* nulParentValue);

size_t json_object_add_pair(json_factory* jsonFact, json_object* obj, json_string* name, json_value* value);
size_t json_array_add_element(json_factory* jsonFact, json_array* arr, json_value* value);

typedef int (*json_object_foreach_cb)(json_object*, json_string*, json_value*);
typedef int (*json_array_foreach_cb)(json_array*, json_value*);

int json_object_foreach(json_object* obj, json_object_foreach_cb iter);
int json_array_foreach(json_array* arr, json_array_foreach_cb iter);
const char* json_value_get_type(json_value* value);

int json_visitor_free_all(json_parser_state* parserState, json_value* topVal);
int json_visitor_free_value(json_factory* jsonFact, json_value* value);
int json_visitor_free_object(json_factory* jsonFact, json_object* obj);
int json_visitor_free_array(json_factory* jsonFact, json_array* arr);
int json_visitor_free_string(json_factory* jsonFact, json_string* str);
int json_visitor_free_number(json_factory* jsonFact, json_number* num);
int json_visitor_free_true(json_factory* jsonFact, json_true* tru);
int json_visitor_free_false(json_factory* jsonFact, json_false* fals);
int json_visitor_free_null(json_factory* jsonFact, json_null* nul);


typedef struct json_allocator {
	void* (*malloc)(size_t);
	void (*free)(void*);
} json_allocator;

typedef struct json_factory {
	json_allocator* allocator;
	
	json_object* (*new_json_object)(json_factory* jsonFact, json_value* objParentValue);
	json_value* (*new_json_value)(json_factory* jsonFact, JSON_VALUE valValueType, void* valValue, JSON_VALUE valParentValueType, void* valParentValue);
	json_string* (*new_json_string)(json_factory* jsonFact, const char* strValue, json_value* strParentValue);
	json_number* (*new_json_number)(json_factory* jsonFact, double numValue, json_value* numParentValue);
	json_array* (*new_json_array)(json_factory* jsonFact, json_value* arrParentValue);
	json_true* (*new_json_true)(json_factory* jsonFact, json_value* truParentValue);
	json_false* (*new_json_false)(json_factory* jsonFact, json_value* falParentValue);
	json_null* (*new_json_null)(json_factory* jsonFact, json_value* nulParentValue);
} json_factory;


typedef struct json_object {
	//TODO: Have json_object contain json_string in a json_value ??
	json_string** names;
	json_value** values;
	size_t size;
	size_t capacity;
	json_value* parentValue;
} json_object;

typedef struct json_value {
	JSON_VALUE valueType;
	void* value;
	
	JSON_VALUE parentValueType;
	void* parentValue;
} json_value;

typedef struct json_string {
	const char* value;
	json_value* parentValue;
} json_string;

typedef struct json_number {
	double value;
	json_value* parentValue;
} json_number;

typedef struct json_array {
	json_value** values;
	size_t size;
	size_t capacity;
	json_value* parentValue;
} json_array;

typedef struct json_true {
	json_value* parentValue;
} json_true;

typedef struct json_false {
	json_value* parentValue;
} json_false;

typedef struct json_null {
	json_value* parentValue;
} json_null;


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_TYPES_H
