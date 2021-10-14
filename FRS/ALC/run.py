import os, sys, getopt, argparse
from multiprocessing import Process, Queue
from subprocess import Popen
import threading


ap = argparse.ArgumentParser()

ap.add_argument('--info', required=False, action='store_true', help='Info')
ap.add_argument('-t', '--init', required=False, default='system', help='Initialize system directory')
ap.add_argument('-v', '--visual', required=False, action='store_true', help='Visual mode')
ap.add_argument('-V', '--verbose', required=False, action='store_true', help='Verbose mode')
ap.add_argument('-i', '--system_in', required=False, default='', help='System input file path')
ap.add_argument('-p', '--options_in', required=False, default='', help='Options input file path')
ap.add_argument('-I', '--data_in', required=True, default='', help='Dataset input file path')
ap.add_argument('-o', '--system_out', required=False, default='', help='System output file path')
ap.add_argument('-P', '--options_out', required=False, default='', help='Options output file path')
ap.add_argument('-O', '--data_out', required=False, default='', help='Dataset output file path')
ap.add_argument('-T', '--stats_out', required=False, default='', help='Stats output file path')
ap.add_argument('-m', '--mode', required=False, default=0, help='0: Fit & Predict, 1: Fit, 2: Predict')
ap.add_argument('-s', '--speed', required=False, default=5, help='Speed of the animation [1, 10]')

'''
mode:
    fit & predict   - showing both system diagram and truth table comparison diagram
    fit             - showing only system diagram
    predict         - showing only truth table comparison diagram
'''

args = vars(ap.parse_args())
print(args)


def alc():
    system_in = args['system_in']
    options_in = args['options_in']
    data_in = args['data_in']

    system_out = args['system_out']
    options_out = args['options_out']
    data_out = args['data_out']
    stats_out = args['stats_out']

    mode = args['mode']
    speed = args['speed']

    alc_args = f'--system_in "{system_in}" --options_in "{options_in}" --data_in "{data_in}" '
    alc_args += f'--system_out "{system_out}" --options_out "{options_out}" --data_out "{data_out}" '
    alc_args += f'--mode {mode} --speed {speed}'
    if args['verbose']:
        alc_args += ' --verbose'
    print('running alc core...')
    os.system(f'build/alc {alc_args} 2>/dev/null')

def visualize():
    print('running alc visualizer...')
    visualizer_args = f''
    os.system(f'python3 alc.py {visualizer_args}')


if args['info']:
    print('This is the info')

print(f"Initializing system directory '{args['init']}'")
status = os.system(f"./init.sh {args['init']}")
if status:
    print('Terminated')
else:
    print('Done')
    # proc_core = Process(target=alc)
    # proc_core.start()

    if args['visual']:
        # proc_visualizer = Process(target=visualize)
        # proc_visualizer.start()
        # proc_visualizer.join()
        pid = os.fork()
        if pid: # parent proc
            alc()
        else: # child proc
            visualize()
    else:
        alc()
    # proc_core.join()

