ifndef CC
    CC := gcc
endif

ifndef CC_FLAGS
    CC_FLAGS := -std=c11 -Wall
endif


BUILD_DIR := dist
OBJ_FILES := json.o json_types.o json_parser.o json_utils.o
CC_FILES := json_types.c json_parser.c json_utils.c
DLIB_FILE := libjson.so
SLIB_FILE := libjson.a

all: build libjson libjson_static

build:
	mkdir -p $(BUILD_DIR)

json: json_types json_parser json_utils
	ld -o json.o -r json_types.o json_parser.o json_utils.o

json_types: json_types.c json_types.h
	$(CC) $(CC_FLAGS) -o json_types.o -c json_types.c

json_parser: json_parser.c json_parser.h
	$(CC) $(CC_FLAGS) -o json_parser.o -c json_parser.c

json_utils: json_utils.c json_utils.h
	$(CC) $(CC_FLAGS) -o json_utils.o -c json_utils.c


libjson: json_types.c json_types.h json_parser.c json_parser.h json_utils.c json_utils.h
	$(CC) $(CC_FLAGS) -o $(BUILD_DIR)/$(DLIB_FILE) -fPIC -shared $(CC_FILES)

libjson_static: json json.o
	ar rcs $(BUILD_DIR)/$(SLIB_FILE) json.o

clean:
	rm $(OBJ_FILES)
	rm -r $(BUILD_DIR)
