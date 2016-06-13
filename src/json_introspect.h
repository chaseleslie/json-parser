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
 *  @file json_introspect.h
 *  @brief JSON parser library functions for introspection
 *
 *  This header declares a number of functions to manipulate JSON
 *  values.
 */


#ifndef JSON_INTROSPECT_H
#define JSON_INTROSPECT_H


#ifndef JSON_TOP_LVL
#error "The file json_types.h must not be included directly. Include 'json.h' instead."
#endif	//#ifndef JSON_TOP_LVL


#include "json_types.h"


/*! Prototype for a callback function used with json_object_foreach() */
typedef int (*json_object_foreach_cb)(json_object*, json_string*, json_value*);
/*! Prototype for a callback function used with json_array_foreach() */
typedef int (*json_array_foreach_cb)(json_array*, json_value*);

/*@{ */
/**
 *  @brief Iterate over all the name/value pairs in an object
 *
 *  This function allows the caller to supply a callback function that will receive the
 *  json_string and json_value of each name/value pair in an object. The callback should
 *  match the prototype for json_object_foreach_cb and return nonzero to keep iterating
 *  or zero to stop.
 *
 *  @param[in] val Pointer to json_value containing the json_object to iterate over
 *  @param[in] iter Callback function to iterate over object
 *  @return Zero on success, nonzero on failure
 *
 *  @see json_object_foreach_cb json_object_foreach_obj()
 */
int json_object_foreach(json_value* val, json_object_foreach_cb iter);
/**
 *  @brief Iterate over all the name/value pairs in an object
 *
 *  This function behaves similarly to json_object_foreach(), taking a pointer to a
 *  json_object directly instead of it's containing json_value.
 *
 *  @param[in] obj Pointer to json_object to iterate over
 *  @param[in] iter Callback function to iterate over object
 *  @return Zero on success, nonzero on failure
 *
 *  @see json_object_foreach_cb json_object_foreach()
 */
int json_object_foreach_obj(json_object* obj, json_object_foreach_cb iter);
/**
 *  @brief Iterate over all the values in an array
 *
 *  This function allows the caller to supply a callback function that will receive the
 *  json_value of each value in an array. The callback should match the prototype for
 *  json_array_foreach_cb and return nonzero to keep iterating or zero to stop.
 *
 *  @param[in] val Pointer to json_value containing the json_array to iterate over
 *  @param[in] iter Callback function to iterate over array
 *  @return Zero on success, nonzero on failure
 *
 *  @see json_array_foreach_cb json_array_foreach_arr()
 */
int json_array_foreach(json_value* val, json_array_foreach_cb iter);
/**
 *  @brief Iterate over all the values in an array
 *
 *  This function behaves similarly to json_array_foreach(), taking a pointer to a
 *  json_array directly instead of it's containing json_value.
 *
 *  @param[in] arr Pointer to json_array to iterate over
 *  @param[in] iter Callback function to iterate over array
 *  @return Zero on success, nonzero on failure
 *
 *  @see json_array_foreach_cb json_array_foreach()
 */
int json_array_foreach_arr(json_array* arr, json_array_foreach_cb iter);
/*@} */


#endif	//#ifndef JSON_INTROSPECT_H