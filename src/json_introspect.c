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

json_value* json_value_query(json_parser_state* parserState, json_value* value, const char* query) {
	return NULL;
}


#endif	//#ifndef JSON_INTROSPECT_C