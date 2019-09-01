#!/bin/bash
pushd ../compile
make clean && QEMU=1 make

