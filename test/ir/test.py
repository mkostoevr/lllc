#!/usr/bin/python3
import sys
import os
import filecmp

sys.path.append('../')
import common

def run(project_root, test_root):
	test_c = f"{test_root}/test.c"
	test_lll = f"{test_root}/test.lll"
	test_log = f"{test_root}/test.log"
	if os.system(f"tcc -run {test_c} {test_lll} {test_log}") != 0:
		raise common.TestFailureException()
	if not filecmp.cmp(test_log, f"{test_log}.expect"):
		raise common.TestFailureException()
