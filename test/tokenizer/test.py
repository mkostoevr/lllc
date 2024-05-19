#!/usr/bin/python3
import sys
import os
import filecmp

sys.path.append('../')
import common

def run(project_root, test_root):
	test_log = f"{test_root}/test.log"
	test_c = f"{test_root}/test.c"
	test_txt = f"{test_root}/test.txt"
	if os.system(f"tcc -DONE_SOURCE -run {test_c} {test_txt} {test_log}") != 0:
		raise common.TestFailureException()
	if not filecmp.cmp(test_log, f"{test_log}.expect"):
		raise common.TestFailureException()
