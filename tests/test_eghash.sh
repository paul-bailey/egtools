#!/bin/sh

cat alice.txt | tr ' ' '\n' | sort | uniq | ./test_eghash
