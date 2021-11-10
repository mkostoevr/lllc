#!/usr/bin/python3
import sys
import os
import filecmp

sys.path.append('../')
import common

def run():
	if os.system("tcc -DONE_SOURCE -run test.c > test.log") != 0:
		raise common.TestFailureException()
	if not filecmp.cmp("test.log", "test.log.expect"):
		raise common.TestFailureException()
