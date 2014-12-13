#!/usr/bin/env python3

import sys
import os
import argparse
import shlex
import subprocess
import shutil
import configparser

def print_error(line):
    print('ERROR:', line, file=sys.stderr)
    sys.exit(1)

parser = argparse.ArgumentParser()
parser.add_argument('--config', default='taslaunch.ini', help='path to taslaunch.ini')
parser.add_argument('action', choices=['sim', 'legit'], help='action to perform')
parser.add_argument('segment', help='name of segment to run')
args = parser.parse_args()

config = configparser.ConfigParser()
config.read(args.config)

config_section = None
try:
    config_section = config[args.segment]
except KeyError:
    print_error('Segment "{}" not found'.format(args.segment))

config_section['seg_name'] = args.segment
dest_prefix = config_section.get(args.action + '_dest_prefix', 'tscript')
sim_log = config_section.get('sim_log', args.segment + '_sim.log')

waitpads = None
waitpads_key = args.action + '_waitpads'
try:
    N1, N2 = config_section[waitpads_key].split()
    waitpads = (int(N1), int(N2))
except KeyError:
    print_error(waitpads_key + ' not defined in the config file')
except ValueError:
    print_error(waitpads_key + ' must be two integers')

lines_per_file = None
try:
    lines_per_file = config_section.getint('lines_per_file', 700)
    if lines_per_file < 1:
        raise ValueError
except ValueError:
    print_error('lines_per_file must be an integer >= 1')

load_cmd = ''
load_from = ''
try:
    load_cmd, load_from = config_section['load_from'].split()
    if load_cmd != 'map' and load_cmd != 'load':
        raise ValueError
    load_cmd = '+' + load_cmd
except ValueError:
    print_error('load_from must be either "map <mapname>" or "load <savename>"')
except KeyError:
    load_cmd = '+load'
    load_from = args.segment

hl_path = ''
try:
    hl_path = os.environ['HL_PATH']
except KeyError:
    print_error('$HL_PATH not set')

qcon_path = os.path.join(hl_path, 'qconsole.log')
gamecfg_path = os.path.join(hl_path, 'valve', 'game.cfg')

host_framerate = None
try:
    host_framerate = config_section.getfloat('host_framerate', 0.0001)
except ValueError:
    print_error('host_framerate must be a float')

print('Removing qconsole.log...')
try:
    os.remove(qcon_path)
except OSError as e:
    pass

print('Generating game.cfg...')
ret = subprocess.call(['gamecfg.py', gamecfg_path, str(waitpads[0]),
                       str(waitpads[1])])
if ret:
    print_error('gamecfg.py returned nonzero')

if args.action == 'sim':
    sim_src = config_section.get('sim_src_script', args.segment + '_sim.cfg')
    sim_mod = config_section.get('sim_mod', 'valve')
    dest_path = os.path.join(hl_path, sim_mod, dest_prefix)

    print('Generating simulation script...')
    try:
        with open(sim_src, 'r') as f:
            gensim = subprocess.Popen('gensim.py', stdin=f,
                                      stdout=subprocess.PIPE)
            splitscript = subprocess.Popen(
                ['splitscript.py', str(lines_per_file), dest_path],
                stdin=gensim.stdout)

            gensim.wait()
            if gensim.returncode:
                print_error('gensim.py returned nonzero')

            splitscript.wait()
            if splitscript.returncode:
                print_error('splitscript.py returned nonzero')
    except OSError as e:
        print_error('Failed to generate simulation script:' + str(e))

    sim_hl_args = shlex.split(config_section.get('sim_hl_args', ''))

    print('Executing Half-Life...')
    try:
        ret = subprocess.call(['runhl.sh', '-game', sim_mod, '-condebug',
                               '+host_framerate', str(host_framerate),
                               load_cmd, load_from] + sim_hl_args)
        if ret:
            print('Half-Life returned nonzero (ignored)', file=sys.stderr)
    except OSError as e:
        print_error('Failed to execute Half-Life:' + str(e))

    print('Copying qconsole.log...')
    try:
        shutil.copyfile(qcon_path, sim_log)
    except OSError as e:
        print_error('Failed to copy qconsole.log:' + str(e))

elif args.action == 'legit':
    legit_mod = config_section.get('legit_mod', 'valve')
    dest_path = os.path.join(hl_path, legit_mod, dest_prefix)
    dont_gen_legit = config_section.get('dont_gen_legit', None)

    if dont_gen_legit is None:
        print('Generating legitimate script...')
        try:
            with open(sim_log, 'r') as f:
                genlegit_args = ['genlegit.py', '--hfr', str(host_framerate)]
                if 'legit_demo' in config_section:
                    genlegit_args.append('--record')
                    genlegit_args.append(config_section['legit_demo'])
                if 'legit_save' in config_section:
                    genlegit_args.append('--save')
                    genlegit_args.append(config_section['legit_save'])
                if 'legit_prepend' in config_section:
                    genlegit_args.append('--prepend')
                    genlegit_args.append(config_section['legit_prepend'])
                if 'legit_append' in config_section:
                    genlegit_args.append('--append')
                    genlegit_args.append(config_section['legit_append'])

                genlegit = subprocess.Popen(genlegit_args, stdin=f,
                                            stdout=subprocess.PIPE)
                splitscript = subprocess.Popen(
                    ['splitscript.py', str(lines_per_file), dest_path],
                    stdin=genlegit.stdout)

                genlegit.wait()
                if genlegit.returncode:
                    print_error('genlegit.py returned nonzero')

                splitscript.wait()
                if splitscript.returncode:
                    print_error('splitscript.py returned nonzero')
        except OSError as e:
            print_error('Failed to generate legitimate script:' + str(e))

    lvlwaitpads = None
    try:
        N1, N2 = config_section['legit_lvl_waitpads'].split(maxsplit=1)
        lvlwaitpads = (int(N1), int(N2))
    except ValueError:
        print_error('legit_lvl_waitpads must be two integers')
    except KeyError:
        pass

    lvlsave = config_section.get('legit_lvl_save', None)
    legit_hl_args = shlex.split(config_section.get('legit_hl_args', ''))

    print('Executing Half-Life...')
    hl_args = ['runhl.sh', '-game', legit_mod, '-condebug',
               '+host_framerate', str(host_framerate),
               load_cmd, load_from] + legit_hl_args
    if lvlwaitpads is not None or lvlsave is not None:
        try:
            hl_proc = subprocess.Popen(hl_args, stderr=subprocess.PIPE)

            gamecfg_args = ['gamecfg.py', '--trigger', gamecfg_path]
            if lvlwaitpads is not None:
                gamecfg_args.append(str(lvlwaitpads[0]))
                gamecfg_args.append(str(lvlwaitpads[1]))
            else:
                gamecfg_args.append(str(waitpads[0]))
                gamecfg_args.append(str(waitpads[1]))

            if lvlsave is not None:
                gamecfg_args.append('--save')
                gamecfg_args.append(lvlsave)

            gamecfg_proc = subprocess.Popen(gamecfg_args, stdin=hl_proc.stderr)

            gamecfg_proc.wait()
            if gamecfg_proc.returncode:
                print_error('gamecfg.py returned nonzero')

            hl_proc.stderr.close() # prevent freezing
            hl_proc.wait()
            if hl_proc.returncode:
                print('Half-Life returned nonzero (ignored)', file=sys.stderr)
        except OSError as e:
            print_error('Failed to execute Half-Life and/or gamecfg.py:' +
                        str(e))
    else:
        try:
            ret = subprocess.call(hl_args)
            if ret:
                print('Half-Life returned nonzero (ignored)', file=sys.stderr)
        except OSError as e:
            print_error('Failed to execute Half-Life:' + str(e))
