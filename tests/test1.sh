#! /bin/sh

# Copyright (C) 2015-2016 Chase
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#Test parsing general JSON texts

#General JSON text test
JSON_STR=$(cat <<'EOF'
	{
		"obj": {
			"arr": [
				1.0e10, null, true, false, "Some string here"
			]
		}
	}
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin PASS
if $?; then
	exit $?
fi

#Memcheck with valgrind
HAVE_VALGRIND_IN_PATH=$(command -v valgrind >/dev/null 2>&1 && printf 'yes' || printf 'no')
if [ "$HAVE_VALGRIND_IN_PATH" = "yes" ]; then
	libtool --mode=execute valgrind ./.libs/test_libjson
	if $?; then
		exit $?
	fi
fi

#Test parsing empty string
printf "\0" | ./test_libjson --stdin FAIL
if $?; then
	exit $?
fi

#Test parsing invalid control chars in string
printf "{\"test\": \"An\x00Invalid\x1FString\"}" | ./test_libjson --stdin FAIL
if $?; then
	exit $?
fi

#Parse toplevel object
JSON_STR=$(cat <<'EOF'
	{}
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin PASS
if $?; then
	exit $?
fi

#Parse toplevel array
JSON_STR=$(cat <<'EOF'
	[]
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin PASS
if $?; then
	exit $?
fi

#Parse toplevel number
JSON_STR=$(cat <<'EOF'
	3.14159
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin PASS
if $?; then
	exit $?
fi

#Parse toplevel string
JSON_STR=$(cat <<'EOF'
	"Some\u0020string\u0020here."
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin PASS
if $?; then
	exit $?
fi

#Parse toplevel false
JSON_STR=$(cat <<'EOF'
	false
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin PASS
if $?; then
	exit $?
fi

#Parse toplevel null
JSON_STR=$(cat <<'EOF'
	null
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin PASS
if $?; then
	exit $?
fi

#Parse toplevel true
JSON_STR=$(cat <<'EOF'
	true
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin PASS
if $?; then
	exit $?
fi

exit $?
