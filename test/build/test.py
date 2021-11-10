#!/usr/bin/python3
import sys
import os

sys.path.append('../')
import common

def run():
	if os.system("tcc ../../*.c -o lllc.exe") != 0:
		raise common.TestFailureException()
	if os.system("tcc ../../main.c -DONE_SOURCE -o lllc.exe") != 0:
		raise common.TestFailureException()
