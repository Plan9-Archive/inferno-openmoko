import os
from t import LIB9, CC, DEFINES, ROOT, INCLUDES, LIBS

OUT="awk.exe"


INCLUDES+=r"""
utils\awk
"""

FNAMES=r"""
utils\awk\b.c
utils\awk\main.c
utils\awk\parse.c
utils\awk\proctab.c
utils\awk\tran.c
utils\awk\lib.c
utils\awk\run.c
utils\awk\lex.c
utils\awk\ytab.c
"""
"""
utils\awk\missing95.c
"""

FNAMES = ' '.join( (os.path.join(ROOT,x) for x in FNAMES.split()) )
INCLUDES = ' '.join( (('-I'+os.path.join(ROOT,x)) for x in INCLUDES.split()) )
LIBS = ' '.join( (('-l'+x) for x in LIBS.split()) )

cmd=r'%s -o %s %s %s %s %s' % (CC, OUT, DEFINES, INCLUDES, LIBS, FNAMES)
print cmd

os.system(cmd)
