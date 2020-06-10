#!/usr/bin/python3

import subprocess
import argparse
from os import path

parser = argparse.ArgumentParser(description='Vulkan and OpenGL benchmark.')
parser.add_argument('--vulkan', action='store_true',
                    help='Run the Vulkan program for timings')
parser.add_argument('--gl', action='store_true',
                    help='Run the OpenGL program for timings')
parser.add_argument('-d', dest='debug', action='store_true', help='Debug mode.')
parser.add_argument('--num_cubes', default=40000, help='Number of cubes to render.')
parser.add_argument('--num_frames', default=1000, help='Number of frames to render.')
parser.add_argument('--file', default='time.csv', help='The file to save the result in.')
parser.add_argument('--num_times', default=1, type=int, help='If more than 1, then run the program many times.')
parser.add_argument('--num_threads', default=0, type=int, help='Run the program with the specified number of threads.')
args = parser.parse_args()

class Args:
    num_frames=-1
    num_threads=1
    num_cubes=16
    fileName="time.csv"
    overwrite=False

    def __init__(self, f=-1, t=1, c=16, o=False):
        self.num_frames=f
        self.num_threads=t
        self.num_cubes=c
        self.overwrite=o

    def build(self):
        a = [
            "-num_cubes", str(self.num_cubes),
            "-num_threads", str(self.num_threads),
            "-file", self.fileName
        ]
        if self.overwrite:
            a += ["-overwrite"]
        if self.num_frames>0:
            a += ["-num_frames", str(self.num_frames)]
        return a

def run_process(py_args, program=None):
    args=Args(py_args.num_frames,1,py_args.num_cubes,True)
    executable_path="./gl/gl"
    fileName=path.join("csv",program,py_args.file)
    if path.exists(fileName) and (py_args.file != "/dev/null"):
        cont = input("File {} already exists. Overwrite? [y/n] ".format(fileName))
        if not (cont=='y'):
            print("Exiting.")
            exit(0)
    args.fileName=fileName
    if program=="vulkan":
        executable_path="./vulkan/vulkan"
    if py_args.debug:
        args=Args(100,1,16,True)
    if py_args.num_threads > 0:
        args.num_threads=py_args.num_threads
        a=args.build()
        a = [executable_path] + a
        print(' '.join(a))
        subprocess.call(a)
        return
    for i in range(1,5):
        for j in range(0,py_args.num_times):
            if py_args.num_times > 1 and args.num_frames > 1:
                print("Resetting num_frames - can only have num_frames==1 when num_times > 1")
                args.num_frames=1
            args.num_threads = i
            if j > 0:
                args.overwrite=False
            a=args.build()
            a = [executable_path] + a
            print(' '.join(a))
            subprocess.call(a)

# See a good convergence at 1000 frames, with 40000 cubes.
subprocess.call(["make","-j","4"])
if args.vulkan:
    run_process(args,"vulkan")
if args.gl:
    run_process(args, "gl")
