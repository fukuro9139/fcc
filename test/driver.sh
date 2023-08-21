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
./fcc -c -o $tmp/out $tmp/empty.c
[ -f $tmp/out ]
check -o

# --help
./fcc --help 2>&1 | grep -q fcc
check --help

# -S
./fcc -S -o - -xc $tmp/main.c | grep -q 'main:'
check -S

# Default output file
rm -f $tmp/out.o $tmp/out.s
echo 'int main() {}' > $tmp/out.c
(cd $tmp; $OLDPWD/fcc -c out.c)
[ -f $tmp/out.o ]
check 'default output file (-o)'

(cd $tmp; $OLDPWD/fcc -c -S out.c)
[ -f $tmp/out.s ]
check 'default output file (-S)'

# Multiple input files
rm -f $tmp/foo.o $tmp/bar.o
echo 'int x;' > $tmp/foo.c
echo 'int y;' > $tmp/bar.c
(cd $tmp; $OLDPWD/fcc -c $tmp/foo.c $tmp/bar.c)
[ -f $tmp/foo.o ] && [ -f $tmp/bar.o ]
check 'multiple input files (-o)'

rm -f $tmp/foo.s $tmp/bar.s
echo 'int x;' > $tmp/foo.c
echo 'int y;' > $tmp/bar.c
(cd $tmp; $OLDPWD/fcc -c -S $tmp/foo.c $tmp/bar.c)
[ -f $tmp/foo.s ] && [ -f $tmp/bar.s ]
check 'multiple input files (-S)'

# Run linker
rm -f $tmp/foo
echo 'int main() { return 0; }' > $tmp/foo.cc
./fcc -o $tmp/foo -xc -xc $tmp/foo.cc
$tmp/foo
check linker

rm -f $tmp/foo
echo 'int bar(); int main() { return bar(); }' > $tmp/foo.c
echo 'int bar() { return 42; }' > $tmp/bar.c
./fcc -o $tmp/foo $tmp/foo.c $tmp/bar.c
$tmp/foo
[ "$?" = 42 ]
check linker

# a.out
rm -f $tmp/a.out
echo 'int main() {}' > $tmp/foo.c
(cd $tmp; $OLDPWD/fcc foo.c)
[ -f $tmp/a.out ]
check a.out

# -E
echo foo > $tmp/out.h
echo "#include \"$tmp/out.h\"" > $tmp/out.cc
./fcc -E -xc $tmp/out.cc | grep -q foo
check -E

echo foo > $tmp/out1.h
echo "#include \"$tmp/out1.h\"" > $tmp/out1.cc
./fcc -E -o $tmp/out2 -xc $tmp/out1.cc
cat $tmp/out2 | grep -q foo
check '-E and -o'

# -I
mkdir $tmp/dir
echo foo > $tmp/dir/i-option-test
echo "#include \"i-option-test\"" > $tmp/i-option-test.c
./fcc -I$tmp/dir -E -xc $tmp/i-option-test.c | grep -q foo
check -I

# -x
echo 'int x;' | ./fcc -c -xc -o $tmp/foo.o -
check -xc
echo 'x:' | ./fcc -c -x assembler -o $tmp/foo.o -
check '-x assembler'

echo 'int x;' > $tmp/foo.c
./fcc -c -x assembler -x none -o $tmp/foo.o $tmp/foo.c
check '-x none'

echo OK