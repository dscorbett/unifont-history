#!/usr/bin/env python3

# Copyright 2020 David Corbett
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


import binascii
import fileinput
import os
import re
import sys


def byte_to_binary(n):
    return ''.join(str((n & (1 << i)) and 1) for i in reversed(range(8)))


def hex_to_binary(h):
    return ''.join(byte_to_binary(b) for b in binascii.a2b_hex(h))


def draw(source, output):
    for line in source:
        line = line.strip()
        if not line:
            continue
        fields = line.split(':')
        cp = fields[0]
        cp6 = cp.zfill(6)
        plane = cp6[:2]
        row = cp6 = cp6[2:4]
        glyph = hex_to_binary(fields[1])
        width = len(glyph) / 16
        output_file = os.path.join(output, plane, row, f'{cp}.glyph')
        if os.path.exists(output_file):
            continue
        os.makedirs(os.path.dirname(output_file), exist_ok=True)
        with open(output_file, 'w') as f:
            f.write(fields[0])
            f.write('\n')
            for i, bit in enumerate(glyph):
                if bit == '1':
                    f.write('#')
                else:
                    f.write(' ')
                if i % width == width - 1:
                    f.write('\n')
            f.write(fields[1])
            f.write('\n')


def usage():
    raise Exception(f'Usage: {sys.argv[0]} HEX_FILE')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        usage()
    hex_file = sys.argv[1]
    if not os.path.exists(hex_file):
        raise Exception(f'No such file: {hex_file}')
    hex_directory = f'{hex_file}.d'
    if os.path.exists(hex_directory):
        raise Exception(f'Already exists: {hex_directory}')
    draw(fileinput.input(hex_file), hex_directory)
