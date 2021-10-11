import os, sys, getopt, argparse

ap = argparse.ArgumentParser()

ap.add_argument('--info', required=False, action='store_true', help='Info')
ap.add_argument('-t', '--init', required=False, default='system', help='Initialize system directory')
ap.add_argument('-v', '--visual', required=False, action='store_true', help='Visual mode')
ap.add_argument('-V', '--verbose', required=False, action='store_true', help='Verbose mode')
ap.add_argument('-i', '--system_in', required=False, default='in.sys', help='System input file path')
ap.add_argument('-p' '--options_in', required=False, default='in.opt', help='Options input file path')
ap.add_argument('-I', '--data_in', required=False, default='in.csv', help='Dataset input file path')
ap.add_argument('-o', '--system_out', required=False, default='out.sys', help='System output file path')
ap.add_argument('-P' '--options_out', required=False, default='out.opt',help='Options output file path')
ap.add_argument('-O', '--data_out', required=False, default='out.csv', help='Dataset output file path')
ap.add_argument('-m', '--mode', required=False, default=0, help='0: Fit & Predict, 1: Fit, 2: Predict')
ap.add_argument('-s', '--speed', required=False, default=5, help='Speed of the animation [0, 9]')

'''
mode:
    fit & predict   - showing both system diagram and truth table comparison diagram
    fit             - showing only system diagram
    predict         - showing only truth table comparison diagram
'''

args = vars(ap.parse_args())
print(args)

if args['info']:
    print('This is the info')

print(f"Initializing system directory '{args['init']}'")
status = os.system(f"./init.sh {args['init']}")
if status:
    print('Terminated')
else:
    print('Done')
