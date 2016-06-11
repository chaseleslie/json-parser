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

#Test parsing incomplete JSON text

#Object missing value
JSON_STR=$(cat <<'EOF'
	{
		"n":
	}
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin FAIL
if $?; then
	exit $?
fi

#Object missing second value
JSON_STR=$(cat <<'EOF'
	{
		"n": 1,
		"t":
	}
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin FAIL
if $?; then
	exit $?
fi

#Object missing closing brace
JSON_STR=$(cat <<'EOF'
	{
		"n": 1
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin FAIL
if $?; then
	exit $?
fi

#Array missing closing bracket
JSON_STR=$(cat <<'EOF'
	{
		"n": [
			1, 2, 3
	}
EOF
)

printf "$JSON_STR" | ./test_libjson --stdin FAIL
if $?; then
	exit $?
fi

exit 0
