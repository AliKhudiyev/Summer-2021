#!/bin/zsh

g++ -std=c++11 $1.cpp -o $1 && leaks -atExit -- ./$1
