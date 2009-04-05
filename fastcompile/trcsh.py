import os
from t import LIB9, CC, DEFINES, ROOT, INCLUDES, LIBS

OUT="rcsh.exe"


INCLUDES+=r"""
utils\awk
"""

FNAMES=LIB9
FNAMES+=r"""
utils\rcsh\Nt.c
utils\rcsh\code.c
utils\rcsh\exec.c
utils\rcsh\glob.c
utils\rcsh\here.c
utils\rcsh\io.c
utils\rcsh\lex.c
utils\rcsh\main.c
utils\rcsh\pcmd.c
utils\rcsh\pfnc.c
utils\rcsh\simple.c
utils\rcsh\trap.c
utils\rcsh\tree.c
utils\rcsh\var.c
utils\rcsh\word.c
utils\rcsh\y.tab.c

lib9\sprint.c
lib9\fprint.c
lib9\exits.c
lib9\seek.c
lib9\create-Nt.c
lib9\dirstat-Nt.c
"""

cmd=r'iyacc.exe -d %s/utils/rcsh/syn.y"' % ROOT
os.system(cmd)

try: os.remove('%s/utils/rcsh/y.tab.c' % ROOT)
except: pass
os.rename('y.tab.c', '%s/utils/rcsh/y.tab.c' % ROOT)

try: os.remove('%s/utils/rcsh/y.tab.h' % ROOT)
except: pass
os.rename('y.tab.h', '%s/utils/rcsh/y.tab.h' % ROOT)

#sys.exit()

FNAMES = ' '.join( (os.path.join(ROOT,x) for x in FNAMES.split()) )
INCLUDES = ' '.join( (('-I'+os.path.join(ROOT,x)) for x in INCLUDES.split()) )
LIBS = ' '.join( (('-l'+x) for x in LIBS.split()) )

cmd=r'%s -o %s %s %s %s %s' % (CC, OUT, DEFINES, INCLUDES, LIBS, FNAMES)
print cmd

os.system(cmd)
