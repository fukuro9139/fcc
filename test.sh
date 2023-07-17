#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./fcc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 2 "4/2"
assert 6 "2*3"
assert 9 "( 1 + 2) * 3"
assert 7 "1 + 2 * 3"

echo OK