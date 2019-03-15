#!/bin/bash

python ../Confucius-Mencius/py_tools/base/utf8_convertor.py . .h .cpp
../Confucius-Mencius/sh_tools/base/dos2unix.sh .
../Confucius-Mencius/sh_tools/base/chmod.sh .
