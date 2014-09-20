#!/usr/bin/env python3

import sys
from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument('gamecfg', help='path to game.cfg')
parser.add_argument('N1', type=int, help='number of waits before pause')
parser.add_argument('N2', type=int, help='number of waits after pause')
parser.add_argument('--save', help='save the game to SAVE at the end of game.cfg, usually used for saving during level transition')
parser.add_argument('--nolog', action='store_true', help='do not activate logging at the beginning')
parser.add_argument('--trigger', action='store_true')
args = parser.parse_args()

if args.trigger:
    for line in sys.stdin:
        if line.startswith('GAME SKILL LEVEL'):
            break

try:
    with open(args.gamecfg, 'w') as f:
        if not args.nolog:
            print('sv_taslog 1', file=f)
        print('wait\n' * args.N1, end='', file=f)
        print('pause', file=f)
        print('wait\n' * args.N2, end='', file=f)
        if args.save is not None:
            print('save "{}"'.format(args.save), file=f)
except OSError as e:
    print('Failed to write to game.cfg:', e, file=sys.stderr)
    sys.exit(1)
