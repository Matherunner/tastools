#!/usr/bin/env python3

import sys
from argparse import ArgumentParser
from math import copysign

parser = ArgumentParser()
parser.add_argument('--prepend', metavar='CMDS', help='prepend CMDS to the output')
parser.add_argument('--append', metavar='CMDS', help='append CMDS to the output')
parser.add_argument('--hfrval', metavar='FRAMETIME', default='0.0001', help='value for the host_framerate before the final wait')
parser.add_argument('--noendhfr', action='store_true', help='do not print a host_framerate before the final wait')
parser.add_argument('--save', help='save the game to SAVE at the end of output')
parser.add_argument('--record', metavar='DEMO', help='record the entire script to DEMO')
args = parser.parse_args()

AM_U_2 = 360 / 65536 / 2
IN_ATTACK = 1 << 0
IN_JUMP = 1 << 1
IN_DUCK = 1 << 2
IN_FORWARD = 1 << 3
IN_BACK = 1 << 4
IN_USE = 1 << 5
IN_MOVELEFT = 1 << 9
IN_MOVERIGHT = 1 << 10
IN_ATTACK2 = 1 << 11
IN_RELOAD = 1 << 13

ftime = 0
yawspeed_str = ''
backspd_sign = '-'
commands = [0] * 10
pitch = None

for line in sys.stdin:
    if line.rstrip() == 'CL_SignonReply: 2':
        break
else:
    print('ERROR: Couldn\'t find "CL_SignonReply: 2"', file=sys.stderr)
    sys.exit(1)

if args.prepend is not None:
    print(args.prepend)
if args.record is not None:
    print('record', args.record)
print('+left')
print('cl_yawspeed 0')
print('cl_forwardspeed 10000')
print('cl_backspeed 10000')
print('cl_sidespeed 10000')
print('cl_upspeed 10000')

for line in sys.stdin:
    if line.startswith('prethink'):
        new_ftime = float(line.rsplit(maxsplit=1)[1])
        if not new_ftime:
            continue
        if new_ftime != ftime:
            print('host_framerate', new_ftime)
            ftime = new_ftime
        print('wait')
        print(yawspeed_str, end='')

    elif line.startswith('cl_yawspeed'):
        yawspeed_str = line

    elif line.startswith('weapon_'):
        print(line, end='')

    elif line.startswith('usercmd'):
        s = line.split()
        new_pitch = float(s[3])
        if new_pitch != pitch:
            adjpitch = new_pitch + copysign(AM_U_2, new_pitch)
            print('cl_pitchup', -adjpitch)
            print('cl_pitchdown', adjpitch)
            pitch = new_pitch

        buttons = int(s[2])
        for i, b, s in [(0, IN_FORWARD, 'forward'),
                        (1, IN_MOVERIGHT, 'moveright'),
                        (2, IN_MOVELEFT, 'moveleft'),
                        (3, IN_BACK, 'back'),
                        (4, IN_USE, 'use'),
                        (5, IN_ATTACK, 'attack'),
                        (6, IN_ATTACK2, 'attack2'),
                        (7, IN_RELOAD, 'reload'),
                        (8, IN_DUCK, 'duck'),
                        (9, IN_JUMP, 'jump')]:
            newv = buttons & b
            if newv == commands[i]:
                continue
            print(('+' if newv else '-') + s)
            commands[i] = newv

    elif commands[3] and line.startswith('fsu'):
        s = line.split(maxsplit=2)
        new_backspd_sign = s[1][0]
        if new_backspd_sign != backspd_sign:
            print('cl_backspeed', '10000' if new_backspd_sign == '-' else '-10000')
            backspd_sign = new_backspd_sign

if not args.noendhfr:
    print('host_framerate', args.hfrval)
print('wait')
print('-use')
print('-attack')
print('-attack2')
print('-reload')
print('-jump')
print('-duck')
print('-left')
print('-forward')
print('-moveleft')
print('-moveright')
print('-back')
if args.record is not None:
    print('stop')
if args.save is not None:
    print('save', args.save)
print('echo TASEND')
if args.append is not None:
    print(args.append)
