AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = libjson.la
libjson_la_SOURCES = json_types.c json_parser.c json_utils.c
libjson_la_LDFLAGS = -version-info 0:0:0
libjson_la_CPPFLAGS = -std=c11 -Wall
nobase_include_HEADERS = json.h json_types.h json_parser.h json_utils.h

