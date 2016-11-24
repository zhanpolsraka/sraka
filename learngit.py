#!/usr/bin/python
#Usage:
#chmod 755 learngit.py
#./leargit.py [--pres|--master] [...FILES]
#--pres for preserving files with conflicts. Fixed files will be saved eith fixed_ prefix
#--master for solving in favor of 'master' instead of 'HEAD'
#FILES are optional. If no files are provided, everything in the working dir willl be fixed (including leargit.py)
import sys
import os

FILES=""
MODE="<<<<<<< HEAD\n"
ERASEMOD=">>>>>>> 2b8323f5e27322d40ab27f04d22ea0184b21c9bc\n"
SEP="=======\n"
ERASE=0

if len([f for f in sys.argv[1:] if os.path.isfile(f)]) == 0:
	FILES = [f for f in os.listdir('.') if os.path.isfile(f)]
else:
	FILES = [f for f in sys.argv[1:] if os.path.isfile(f)]
if "--master" in sys.argv:
	tmp = MODE
	MODE=ERASEMOD
	ERASEMOD = tmp

print("FILES: " + str(FILES) + "\nMODE: " + MODE)
for filename in FILES:
	print("fixing file "+filename)
	with open(filename) as file, open("fixed_"+filename, 'w+') as tmp:
		for line in file:
			if line == SEP or line == ERASEMOD:
				if ERASE == 0:
					ERASE = 1
				else:
					ERASE = 0
			elif line != MODE and ERASE == 0:
				tmp.write(line)
	if "--pres" not in sys.argv:
		os.rename("fixed_"+filename, filename)
