import os
from t import LIB9, CC, DEFINES, ROOT, INCLUDES, LIBS

OUT="limbo.exe"

DEFINES+=' -DINCPATH="""%s/module"""' % ROOT

INCLUDES+=r"""
libmp\port
"""

FNAMES=LIB9
FNAMES+=r"""
limbo\asm.c
limbo\com.c
limbo\decls.c
limbo\dis.c
limbo\dtocanon.c
limbo\ecom.c
limbo\gen.c
limbo\lex.c
limbo\nodes.c
limbo\optab.c
limbo\optim.c
limbo\sbl.c
limbo\stubs.c
limbo\typecheck.c
limbo\types.c
limbo\y.tab.c

lib9\sprint.c
lib9\fprint.c
lib9\argv0.c
lib9\exits.c
lib9\cleanname.c
lib9\getwd-Nt.c
lib9\create-Nt.c

libmath\g_fmt.c
libmath\dtoa.c
libmath\FPcontrol-Nt.c

libsec\port\md5.c
libsec\port\md5block.c

libbio\boffset.c
libbio\binit.c
libbio\bprint.c
libbio\bflush.c
libbio\bgetc.c
libbio\bputc.c
libbio\bputrune.c
libbio\bgetrune.c
libbio\bwrite.c
"""

cmd=r'iyacc.exe -d %s/limbo/limbo.y"' % ROOT

os.system(cmd)
try: os.remove('%s/limbo/y.tab.c' % ROOT); 
except: pass
os.rename('y.tab.c', '%s/limbo/y.tab.c' % ROOT)

try: os.remove('%s/limbo/y.tab.h' % ROOT); 
except: pass
os.rename('y.tab.h', '%s/limbo/y.tab.h' % ROOT)


FNAMES = ' '.join( (os.path.join(ROOT,x) for x in FNAMES.split()) )
INCLUDES = ' '.join( (('-I'+os.path.join(ROOT,x)) for x in INCLUDES.split()) )
LIBS = ' '.join( (('-l'+x) for x in LIBS.split()) )

cmd=r'%s -o %s %s %s %s %s' % (CC, OUT, DEFINES, INCLUDES, LIBS, FNAMES)
print cmd

os.system(cmd)
