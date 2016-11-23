#!/usr/bin/python
#Usage:
# python test.py [test name(s)]
# ./test.py [test name(s)]
#If you don't specify any tests, all tests will be run.
#Test name(s) can be specified with regex - TODO, NOT HERE  YET!
####################################################
#HOW TO NAME YOUR TESTS:
#Compose the filename of tags, that describe what functionality is
#tested. For example, a source with + and - operators on int and 
#double values could be named as plus_minus_double_int.ifj16
#Alternatively, you could name a source code for computing say factorial
#function as factorial.ifj16. If you can't come up with an appropriate name,
#use other[UNIQUE ID].ifj16. 
#It's recommended to use Python 2.7. Have fun with debugging!
import sys, os, subprocess, re

ALL=[f for f in os.listdir('./tests') if f != 'logs']
REGEXLIST=[]
TESTSLIST=[]

class colors:
	HEADER = '\033[95m'
	OK = '\033[92m'
	WARNING = '\033[93m'
	FAIL = '\033[91m'
	ENDC = '\033[0m'
	BOLD = '\033[1m'
  
def ok(test):
	print colors.HEADER + test + colors.OK + ' - OK' + colors.ENDC

def notOK(test, expected, got):
	print colors.HEADER + test + colors.FAIL + ' - FAIL: ' + \
	'Expected ' + expected + ', but got', got, colors.ENDC

def createTestList():
	if sys.argv[1:] == ['all']:
		REGEXLIST=ALL
	else:
		REGEXLIST=sys.argv[1:]
	#Creating TESTLIST
	for r in REGEXLIST:
		for f in ALL:
			res = re.search(r, f)
			if res:
				TESTSLIST.append(f)

def main():
	createTestList();
	for test in TESTSLIST:
		source = "tests/"+test
		os.environ["IFJ16"] = source
		with open('tests/logs/'+test+'.out', 'w+') as out:
			ret = subprocess.call(['./ifj16'], stdout=out, stderr=out)
		with open(source, 'r') as f:
			expected = f.readline().strip('/\n')
		if ret == int(expected):
			ok(test)
		else:
			notOK(test, expected, ret)

main()