#!/usr/bin/python3
import sys
import os

sys.path.append('../')
import common

def run(project_root, test_root):
	lllc0_exe = f"{test_root}/lllc0.exe"
	lllc1_exe = f"{test_root}/lllc1.exe"
	if os.system(f"tcc {project_root}/*.c -o {lllc0_exe}") != 0:
		raise common.TestFailureException()
	if os.system(f"tcc {project_root}/main.c -DONE_SOURCE -o {lllc1_exe}") != 0:
		raise common.TestFailureException()
