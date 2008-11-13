import os
from t import LIB9, CC, DEFINES, ROOT, INCLUDES, LIBS

OUT="md5sum.exe"


INCLUDES+=r"""
libmp\port
"""

FNAMES=LIB9
FNAMES+=r"""
utils\md5sum\md5sum.c

lib9\sprint.c
lib9\fprint.c
lib9\argv0.c
lib9\exits.c

libsec\port\md5.c
libsec\port\md5block.c
"""

FNAMES = ' '.join( (os.path.join(ROOT,x) for x in FNAMES.split()) )
INCLUDES = ' '.join( (('-I'+os.path.join(ROOT,x)) for x in INCLUDES.split()) )
LIBS = ' '.join( (('-l'+x) for x in LIBS.split()) )

cmd=r'%s -o %s %s %s %s %s' % (CC, OUT, DEFINES, INCLUDES, LIBS, FNAMES)
print cmd

os.system(cmd)
