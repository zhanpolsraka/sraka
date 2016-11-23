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
#use other[UNIQUE ID].ifj16. Have fun with debugging!
import sys
import os

if sys.argv[1:] == ['all']:
	TESTSLIST=[f for f in os.listdir('./tests')]
else:
	TESTSLIST=sys.argv[1:]

for test in TESTSLIST:
	os.environ["IFJ16"] = "tests/"+test
	#print os.environ["IFJ16"]
	os.system('./ifj16 '+os.environ["IFJ16"])