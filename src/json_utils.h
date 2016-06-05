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


void json_error(const char* err);

void json_error_lineno(const char* err, json_parser_state* parserState);

char* json_utils_unescape_string(json_parser_state* parserState, const char* str, size_t n, int* state);


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_UTILS_H
