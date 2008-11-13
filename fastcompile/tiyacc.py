import os
from t import LIB9, CC, DEFINES, ROOT, INCLUDES, LIBS

OUT="iyacc.exe"

FNAMES=LIB9
FNAMES+=r"""
utils\iyacc\yacc.c

lib9\sprint.c
lib9\fprint.c
lib9\argv0.c
lib9\exits.c

lib9\create-Nt.c
libbio\binit.c
libbio\bprint.c
libbio\bflush.c
libbio\bgetc.c
libbio\bputc.c
libbio\bputrune.c
libbio\bgetrune.c
libbio\bwrite.c
"""

FNAMES = ' '.join( (os.path.join(ROOT,x) for x in FNAMES.split()) )
INCLUDES = ' '.join( (('-I'+os.path.join(ROOT,x)) for x in INCLUDES.split()) )
LIBS = ' '.join( (('-l'+x) for x in LIBS.split()) )

cmd=r'%s -o %s %s %s %s %s' % (CC, OUT, DEFINES, INCLUDES, LIBS, FNAMES)
print cmd

os.system(cmd)
