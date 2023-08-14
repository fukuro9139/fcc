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

# Multiple input files
rm -f $tmp/foo.o $tmp/bar.o
echo 'int x;' > $tmp/foo.c
echo 'int y;' > $tmp/bar.c
(cd $tmp; $OLDPWD/fcc $tmp/foo.c $tmp/bar.c)
[ -f $tmp/foo.o ] && [ -f $tmp/bar.o ]
check 'multiple input files (-o)'

rm -f $tmp/foo.s $tmp/bar.s
echo 'int x;' > $tmp/foo.c
echo 'int y;' > $tmp/bar.c
(cd $tmp; $OLDPWD/fcc -S $tmp/foo.c $tmp/bar.c)
[ -f $tmp/foo.s ] && [ -f $tmp/bar.s ]
check 'multiple input files (-S)'

echo OK