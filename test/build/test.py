#!/usr/bin/python3
import sys
import os

sys.path.append('../')
import common

def run(project_root, test_root):
	if os.system(f"tcc {project_root}/*.c -o lllc.exe") != 0:
		raise common.TestFailureException()
	if os.system(f"tcc {project_root}/main.c -DONE_SOURCE -o lllc.exe") != 0:
		raise common.TestFailureException()
