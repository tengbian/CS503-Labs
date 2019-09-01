#!/usr/bin/env python
import binascii
import sys

def compute_crc32(filename):
    buf = open(filename,'rb').read()
    buf = (binascii.crc32(buf) & 0xFFFFFFFF)
    return "%08x" % buf

fn = sys.argv[1]
crc32v = compute_crc32(fn)
print crc32v
