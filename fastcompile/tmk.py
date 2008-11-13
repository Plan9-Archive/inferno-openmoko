import os
from t import LIB9, CC, DEFINES, ROOT, INCLUDES, LIBS

OUT="mk.exe"

INCLUDES+=r"""
utils\include
"""

FNAMES=LIB9
FNAMES+=r"""
utils\mk\Nt.c
utils\mk\arc.c
utils\mk\archive.c
utils\mk\bufblock.c
utils\mk\env.c
utils\mk\file.c
utils\mk\graph.c
utils\mk\job.c
utils\mk\lex.c
utils\mk\main.c
utils\mk\match.c
utils\mk\mk.c
utils\mk\parse.c
utils\mk\rc.c
utils\mk\recipe.c
utils\mk\rule.c
utils\mk\run.c
utils\mk\shprint.c
utils\mk\symtab.c
utils\mk\var.c
utils\mk\varsub.c
utils\mk\word.c

utils\libregexp\regaux.c
utils\libregexp\regcomp.c
utils\libregexp\regsub.c
utils\libregexp\regexec.c

lib9\sprint.c
lib9\fprint.c
lib9\argv0.c
lib9\exits.c
lib9\getfields.c
lib9\utfrune.c
lib9\dirstat-Nt.c
lib9\create-Nt.c
lib9\seek.c

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
