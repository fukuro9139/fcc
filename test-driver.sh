#!/bin/sh
tmp=`mktemp -d /tmp/fcc-test-XXXXXX`
trap 'rm -rf $tmp' INT TERM HUP EXIT
echo > $tmp/empty.c

check() {
    if [ $? -eq 0 ]; then
        echo "testing $1 ... passed"
    else
        echo "testing $1 ... failed"
        exit 1
    fi
}

# -o
rm -f $tmp/out
./fcc -o $tmp/out ""
[ -f $tmp/out ]
check -o

# --help
./fcc --help 2>&1 | grep -q fcc
check --help

echo OK