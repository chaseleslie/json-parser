#ifndef JSON_TYPES_CC
#define JSON_TYPES_CC


#define JSON_TOP_LVL 1

#include "json_types.h"
#include "json_parser.h"

#include <stdlib.h>
#include <string.h>


#define JSON_OBJ_INIT_SIZE 8
#define JSON_OBJ_INCR_SIZE 2
#define JSON_ARRAY_INIT_SIZE 8
#define JSON_ARRAY_INCR_SIZE 2


#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


static void* json_allocator_alloc(size_t);
static void json_allocator_free(void*);
static void json_allocator_free_noop(void*);


const char* const JSON_VALUE_NAMES[] = {
	"unspecified",
	"string",
	"number",
	"object",
	"array",
	"true",
	"false",
	"null"
};

const char* const JSON_PARSER_STATE_NAMES[] = {
	"init",
	"",
	"complete",
	"",
	"error"
};

const char JSON_TOKEN_NAMES[] = {
	'"',
	'+',
	'-',
	'{',
	'}',
	'[',
	']',
	':',
	',',
	'\\'
};


int json_types_init(void* (*allocFunction)(size_t), void (*freeFunction)(void*), json_allocator** jsAllocOut, json_factory** jsFactoryOut) {
	//If both alloc and free functions passed by user, use those internally
	//If only alloc function passed, assume user will take care of freeing memory and use free_noop internally
	//If only free function or no alloc and free function passed, use defaults (malloc and free)
	int ret = 1;
	json_allocator* jsonAlloc = NULL;
	json_factory* jsonFact = NULL;
	if (allocFunction) {
		jsonAlloc = (json_allocator*) allocFunction(sizeof(json_allocator));
		if (!jsonAlloc) {
			return ret;
		}
		jsonFact = (json_factory*) allocFunction(sizeof(json_factory));
		if (!jsonFact) {
			return ret;
		}
		jsonAlloc->malloc = allocFunction;
		jsonFact->allocator = jsonAlloc;
		if (freeFunction) {
			jsonAlloc->free = freeFunction;
		} else {
			jsonAlloc->free = json_allocator_free_noop;
		}
	} else {
		jsonAlloc = (json_allocator*) json_allocator_alloc(sizeof(json_allocator));
		if (!jsonAlloc) {
			return ret;
		}
		jsonFact = (json_factory*) json_allocator_alloc(sizeof(json_factory));
		if (!jsonFact) {
			return ret;
		}
		jsonAlloc->free = json_allocator_free;
		jsonAlloc->malloc = json_allocator_alloc;
		jsonFact->allocator = jsonAlloc;
	}
	
	jsonFact->new_json_object = json_factory_new_json_object;
	jsonFact->new_json_value = json_factory_new_json_value;
	jsonFact->new_json_string = json_factory_new_json_string;
	jsonFact->new_json_number = json_factory_new_json_number;
	jsonFact->new_json_array = json_factory_new_json_array;
	jsonFact->new_json_true = json_factory_new_json_true;
	jsonFact->new_json_false = json_factory_new_json_false;
	jsonFact->new_json_null = json_factory_new_json_null;
	
	*jsAllocOut = jsonAlloc;
	*jsFactoryOut = jsonFact;
	
	ret = 0;
	return ret;
}

/* Default JSON Alloc and Free functions */

static void* json_allocator_alloc(size_t size) {
	return malloc(size);
}
static void json_allocator_free(void* ptr) {
	return free(ptr);
}
static void json_allocator_free_noop(void* ptr) {
	//Do Nothing
}

/* JSON Factory functions */

json_object* json_factory_new_json_object(json_factory* jsonFact, json_value* objParentValue) {
	json_object* obj = (json_object*) jsonFact->allocator->malloc( sizeof(json_object) );
	if (!obj) {
		return NULL;
	}
	
	obj->names = NULL;
	obj->values = NULL;
	obj->size = 0;
	obj->capacity = 0;
	obj->parentValue = objParentValue;
	
	return obj;
}
json_value* json_factory_new_json_value(json_factory* jsonFact, JSON_VALUE valValueType, void* valValue, JSON_VALUE valParentValueType, void* valParentValue) {
	json_value* value = (json_value*) jsonFact->allocator->malloc( sizeof(json_value) );
	if (!value) {
		return NULL;
	}
	
	value->valueType = valValueType;
	value->value = valValue;
	
	value->parentValueType = valParentValueType;
	value->parentValue = valParentValue;
	
	return value;
}
json_string* json_factory_new_json_string(json_factory* jsonFact, const char* strValue, json_value* strParentValue) {
	json_string* str = (json_string*) jsonFact->allocator->malloc( sizeof(json_string) );
	if (!str) {
		return NULL;
	}
	
	str->value = strValue;
	str->parentValue = strParentValue;
	
	return str;
}
json_number* json_factory_new_json_number(json_factory* jsonFact, double numValue, json_value* numParentValue) {
	json_number* num = (json_number*) jsonFact->allocator->malloc( sizeof(json_number) );
	if (!num) {
		return NULL;
	}
	
	num->value = numValue;
	num->parentValue = numParentValue;
	
	return num;
}
json_array* json_factory_new_json_array(json_factory* jsonFact, json_value* arrParentValue) {
	json_array* arr = (json_array*) jsonFact->allocator->malloc( sizeof(json_array) );
	if (!arr) {
		return NULL;
	}
	
	arr->values = NULL;
	arr->size = 0;
	arr->capacity = 0;
	arr->parentValue = arrParentValue;
	
	return arr;
}
json_true* json_factory_new_json_true(json_factory* jsonFact, json_value* truParentValue) {
	json_true* tru = (json_true*) jsonFact->allocator->malloc( sizeof(json_true) );
	if (!tru) {
		return NULL;
	}
	
	tru->parentValue = truParentValue;
	
	return tru;
}
json_false* json_factory_new_json_false(json_factory* jsonFact, json_value* falParentValue) {
	json_false* fal = (json_false*) jsonFact->allocator->malloc( sizeof(json_false) );
	if (!fal) {
		return NULL;
	}
	
	fal->parentValue = falParentValue;
	
	return fal;
}
json_null* json_factory_new_json_null(json_factory* jsonFact, json_value* nulParentValue) {
	json_null* nul = (json_null*) jsonFact->allocator->malloc( sizeof(json_null) );
	if (!nul) {
		return NULL;
	}
	
	nul->parentValue = nulParentValue;
	
	return nul;
}

/* JSON Value manipulation functions */

//Returns number of pairs added to passed obj, or zero on error
size_t json_object_add_pair(json_factory* jsonFact, json_object* obj, json_string* name, json_value* value) {
	if (!obj || !name || !value) {//Invalid Args
		return 0;
	} else if (!obj->capacity) {//Uninitialized
		obj->names = (json_string**) jsonFact->allocator->malloc( sizeof(json_string*) * JSON_OBJ_INIT_SIZE );
		if (!obj->names) {
			return 0;
		}
		obj->values = (json_value**) jsonFact->allocator->malloc( sizeof(json_value*) * JSON_OBJ_INIT_SIZE );
		if (!obj->values) {
			return 0;
		}
		obj->capacity = JSON_OBJ_INIT_SIZE;
		obj->names[0] = name;
		obj->values[0] = value;
		obj->size = 1;
	} else if (obj->size < obj->capacity ) {//Size < Capacity
		obj->names[obj->size] = name;
		obj->values[obj->size] = value;
		obj->size += 1;
	} else {//Realloc
		size_t sizeIncr = obj->capacity * JSON_OBJ_INCR_SIZE;
		json_string** names = (json_string**) jsonFact->allocator->malloc( sizeof(json_string*) * sizeIncr );
		if (!names) {
			return 0;
		}
		json_value** values = (json_value**) jsonFact->allocator->malloc( sizeof(json_value*) * sizeIncr );
		if (!values) {
			jsonFact->allocator->free(names);
			return 0;
		}
		memcpy(names, obj->names, sizeof(obj->names) * obj->capacity);
		memcpy(values, obj->values, sizeof(obj->values) * obj->capacity);
		jsonFact->allocator->free(obj->names);
		jsonFact->allocator->free(obj->values);
		obj->names = names;
		obj->values = values;
		obj->capacity = sizeIncr;
		
		obj->names[obj->size] = name;
		obj->values[obj->size] = value;
		obj->size += 1;
	}
	return 1;
}

//Returns number of elements added to passed array, or zero on error
size_t json_array_add_element(json_factory* jsonFact, json_array* arr, json_value* value) {
	if (!arr || !value) {//Invalid Args
		return 0;
	} else if (!arr->capacity) {//Uninitialized
		arr->values = (json_value**) jsonFact->allocator->malloc( sizeof(json_value*) * JSON_ARRAY_INIT_SIZE );
		if (!arr->values) {
			return 0;
		}
		arr->capacity = JSON_ARRAY_INIT_SIZE;
		arr->values[0] = value;
		arr->size = 1;
	} else if (arr->size < arr->capacity ) {//Size < Capacity
		arr->values[arr->size] = value;
		arr->size += 1;
	} else {//Realloc
		size_t sizeIncr = arr->capacity * JSON_ARRAY_INCR_SIZE;
		json_value** values = (json_value**) jsonFact->allocator->malloc( sizeof(json_value*) * sizeIncr );
		if (!values) {
			return 0;
		}
		
		memcpy(values, arr->values, sizeof(arr->values) * arr->capacity);
		jsonFact->allocator->free(arr->values);
		arr->values = values;
		arr->capacity = sizeIncr;
		
		arr->values[arr->size] = value;
		arr->size += 1;
	}
	return 1;
}

//Loop through string:value pairs in the passed object, call iter callback passing object, string and value
//Returns nonzero if passed object or callback is null, zero otherwise
//Callback should return truthy value, or zero to stop iterating
int json_object_foreach(json_object* obj, json_object_foreach_cb iter) {
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
int json_array_foreach(json_array* arr, json_array_foreach_cb iter) {
	if (!arr || !iter) {
		return 1;
	}
	
	size_t k = 0, numElems = arr->size;
	while (k < numElems && iter(arr, arr->values[k])) {
		k += 1;
	}
	
	return 0;
}

//Convenience function to get a string representing the value
const char* json_value_get_type(json_value* value) {
	return (value) ? JSON_VALUE_NAMES[value->valueType] : NULL;
}

/*
 * Functions to free the structures allocated during runtime
 * Return zero on success, nonzero on failure
 */

int json_visitor_free_all(json_parser_state* parserState, json_value* topVal) {
	int ret = 1;
	if (!parserState || !topVal) {
		return ret;
	}
	
	return json_visitor_free_value(parserState->JSON_Factory, topVal);
}

int json_visitor_free_value(json_factory* jsonFact, json_value* value) {
	int ret = (value) ? 0 : 1;
	if (ret) {
		return ret;
	}
	
	switch (value->valueType) {
		default:
		case unspecified_value:
			ret = 1;
		break;
		case string_value:
			ret = json_visitor_free_string(jsonFact, value->value);
		break;
		case number_value:
			ret = json_visitor_free_number(jsonFact, value->value);
		break;
		case object_value:
			ret = json_visitor_free_object(jsonFact, value->value);
		break;
		case array_value:
			ret = json_visitor_free_array(jsonFact, value->value);
		break;
		case true_value:
			ret = json_visitor_free_true(jsonFact, value->value);
		break;
		case false_value:
			ret = json_visitor_free_false(jsonFact, value->value);
		break;
		case null_value:
			ret = json_visitor_free_null(jsonFact, value->value);
		break;
	}
	
	if (!ret) {
		jsonFact->allocator->free(value);
	}
	
	return ret;
}
int json_visitor_free_object(json_factory* jsonFact, json_object* obj) {
	int ret = (!obj || (obj->size > 0 && (!obj->names || !obj->values))) ? 1 : 0;
	
	if (!ret) {
		for (size_t k = 0, n = obj->size; k < n; k += 1) {
			ret = json_visitor_free_value(jsonFact, obj->values[k]);
			if (ret) {
				break;
			}
			ret = json_visitor_free_string(jsonFact, obj->names[k]);
			if (ret) {
				break;
			}
		}
	}
	
	if (!ret) {
		jsonFact->allocator->free(obj->values);
		jsonFact->allocator->free(obj->names);
		jsonFact->allocator->free(obj);
	}
	
	return ret;
}
int json_visitor_free_array(json_factory* jsonFact, json_array* arr) {
	int ret = (!arr || (arr->size > 0 && !arr->values)) ? 1 : 0;
	
	if (!ret) {
		for (size_t k = 0, n = arr->size; k < n; k += 1) {
			ret = json_visitor_free_value(jsonFact, arr->values[k]);
			if (ret) {
				break;
			}
		}
	}
	
	if (!ret) {
		jsonFact->allocator->free(arr->values);
		jsonFact->allocator->free(arr);
	}
	
	return ret;
}
int json_visitor_free_string(json_factory* jsonFact, json_string* str) {
	int ret = (str) ? 0 : 1;
	
	if (!ret) {
		jsonFact->allocator->free((void*) str->value);
		jsonFact->allocator->free(str);
	}
	
	return ret;
}
int json_visitor_free_number(json_factory* jsonFact, json_number* num) {
	int ret = (num) ? 0 : 1;
	
	if (!ret) {
		jsonFact->allocator->free(num);
	}
	
	return ret;
}
int json_visitor_free_true(json_factory* jsonFact, json_true* tru) {
	int ret = (tru) ? 0 : 1;
	
	if (!ret) {
		jsonFact->allocator->free(tru);
	}
	
	return ret;
}
int json_visitor_free_false(json_factory* jsonFact, json_false* fals) {
	int ret = (fals) ? 0 : 1;
	
	if (!ret) {
		jsonFact->allocator->free(fals);
	}
	
	return ret;
}
int json_visitor_free_null(json_factory* jsonFact, json_null* nul) {
	int ret = (nul) ? 0 : 1;
	
	if (!ret) {
		jsonFact->allocator->free(nul);
	}
	
	return ret;
}


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_TYPES_CC
