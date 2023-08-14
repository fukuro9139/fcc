#!/bin/bash
tmp=`mktemp -d /tmp/fcc-test-XXXXXX`
trap 'rm -rf $tmp' INT TERM HUP EXIT

echo > $tmp/empty.c
echo 'int main() {}' > $tmp/main.c

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
./fcc -o $tmp/out $tmp/empty.c
[ -f $tmp/out ]
check -o

# --help
./fcc --help 2>&1 | grep -q fcc
check --help

# -S
./fcc -S -o - $tmp/main.c | grep -q 'main:'
check -S

# Default output file
rm -f $tmp/out.o $tmp/out.s
echo 'int main() {}' > $tmp/out.c
(cd $tmp; $OLDPWD/fcc out.c)
[ -f $tmp/out.o ]
check 'default output file (-o)'

(cd $tmp; $OLDPWD/fcc -S out.c)
[ -f $tmp/out.s ]
check 'default output file (-S)'

echo OK