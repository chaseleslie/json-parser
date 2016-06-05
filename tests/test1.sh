#! /bin/sh

#Test parsing general JSON string

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

printf "$JSON_STR" | ./test_libjson --stdin
exit $?
