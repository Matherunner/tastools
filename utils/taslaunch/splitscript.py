#!/usr/bin/env python3

import sys
from argparse import ArgumentParser
from os.path import basename

parser = ArgumentParser()
parser.add_argument('N', type=int, help='number of lines per file')
parser.add_argument('prefix', help='output file prefix (numbers followed by .cfg will be appended to this prefix)')
args = parser.parse_args()

if args.N < 1:
    print('The number of lines must be >= 1.')
    sys.exit(1)

nlines = 0
filenum = 1
outfile = open(args.prefix + '.cfg', 'w')
for line in sys.stdin:
    print(line, end='', file=outfile)
    nlines += 1
    if nlines < args.N:
        continue
    newname = '{}{}.cfg'.format(args.prefix, filenum)
    filenum += 1
    print('exec "{}"'.format(basename(newname)), file=outfile)
    outfile.close()
    outfile = open(newname, 'w')
    nlines = 0
outfile.close()
