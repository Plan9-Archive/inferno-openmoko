import os
import time
from t import LIB9, CC, DEFINES, ROOT, INCLUDES, LIBS

OUT="emu.exe"

LIBS+=r"""
winmm ws2_32 advapi32 mpr user32 gdi32
"""
DEFINES+=r""" -DEMU=1 -DSTACK=0 -DKERNDATE=%d""" % int(time.time())
INCLUDES+=r"""
emu\port
libinterp
libmp\port
include\freetype
libkeyring
libtk
"""

FNAMES=LIB9
FNAMES+=r"""
emu\Nt\audio.c
emu\Nt\cmd.c
emu\Nt\devarch.c
emu\Nt\deveia.c
emu\Nt\devfs.c
emu\Nt\ipif.c
emu\Nt\os.c
emu\Nt\win.c

emu\port\error.c
emu\port\dev.c
emu\port\chan.c
emu\port\main.c
emu\port\sysfile.c
emu\port\devmnt.c
emu\port\lock.c
emu\port\qio.c
emu\port\cache.c
emu\port\env.c
emu\port\proc.c
emu\port\dis.c
emu\port\exception.c
emu\port\devaudio.c
emu\port\devprog.c
emu\port\devsrv.c
emu\port\devcons.c
emu\port\devpointer.c
emu\port\inferno.c
emu\port\pgrp.c
emu\port\random.c
emu\port\parse.c
emu\port\errstr.c
emu\port\alloc-nt.c
emu\port\latin1.c
emu\port\dial.c
emu\port\exportfs.c
emu\port\uqid.c
emu\port\ipaux.c
emu\port\devroot.c
emu\port\devenv.c
emu\port\devpipe.c
emu\port\devdup.c
emu\port\devssl.c
emu\port\devcap.c
emu\port\devcmd.c
emu\port\devmem.c
emu\port\devdraw.c
emu\port\devsnarf.c
emu\port\devip.c
emu\port\devindir.c
emu\port\srv.c
emu\port\discall.c

libinterp\alt.c
libinterp\xec.c
libinterp\string.c
libinterp\raise.c
libinterp\heap.c
libinterp\gc.c
libinterp\link.c
libinterp\load.c
libinterp\sign.c
libinterp\readmod.c
libinterp\comp-386.c
libinterp\dlm-Nt.c
libinterp\runt.c
libinterp\conv.c
libinterp\tk.c
libinterp\draw.c
libinterp\math.c
libinterp\keyring.c
libinterp\loader.c
libinterp\freetype.c
libinterp\ipint.c
libinterp\geom.c

libtk\buton.c
libtk\canvs.c
libtk\canvu.c
libtk\carcs.c
libtk\cbits.c
libtk\cimag.c
libtk\cline.c
libtk\colrs.c
libtk\coval.c
libtk\cpoly.c
libtk\crect.c
libtk\ctext.c
libtk\cwind.c
libtk\ebind.c
libtk\entry.c
libtk\extns.c
libtk\frame.c
libtk\grids.c
libtk\image.c
libtk\label.c
libtk\listb.c
libtk\menus.c
libtk\packr.c
libtk\panel.c
libtk\parse.c
libtk\scale.c
libtk\scrol.c
libtk\textu.c
libtk\textw.c
libtk\tindx.c
libtk\tmark.c
libtk\ttags.c
libtk\twind.c
libtk\utils.c
libtk\windw.c
libtk\xdata.c

lib9\exits.c
lib9\runestrlen.c
lib9\fmtquote.c
lib9\convM2D.c
lib9\convD2M.c
lib9\convM2S.c
lib9\convS2M.c
lib9\seprint.c
lib9\vsmprint.c
lib9\utfrune.c
lib9\smprint.c
lib9\fprint.c
lib9\sprint.c
lib9\strecpy.c
lib9\fmtprint.c
lib9\fmtstr.c
lib9\fcallfmt.c
lib9\cleanname.c
lib9\strtoll.c
lib9\tokenize.c
lib9\lock-Nt-386.c
lib9\u16.c
lib9\u32.c
lib9\u64.c
lib9\getfields.c
lib9\fmtvprint.c
lib9\utfrrune.c

libdraw\arith.c
libdraw\alloc.c
libdraw\chan.c
libdraw\bytesperline.c
libdraw\string.c
libdraw\stringwidth.c
libdraw\ellipse.c
libdraw\bezier.c
libdraw\poly.c
libdraw\line.c
libdraw\font.c
libdraw\drawrepl.c
libdraw\rgb.c
libdraw\readimage.c
libdraw\window.c
libdraw\getdefont.c
libdraw\loadimage.c
libdraw\draw.c
libdraw\buildfont.c
libdraw\border.c
libdraw\init.c
libdraw\unloadimage.c
libdraw\stringbg.c
libdraw\allocimagemix.c
libdraw\openfont.c
libdraw\writeimage.c
libdraw\defont.c
libdraw\rectclip.c
libdraw\replclipr.c
libdraw\creadimage.c
libdraw\subfont.c
libdraw\getsubfont.c
libdraw\readsubfont.c
libdraw\computil.c

libmemdraw\arc.c
libmemdraw\line.c
libmemdraw\poly.c
libmemdraw\fillpoly.c
libmemdraw\draw.c
libmemdraw\hwdraw.c
libmemdraw\alloc.c
libmemdraw\cmap.c
libmemdraw\icossin.c
libmemdraw\icossin2.c
libmemdraw\ellipse.c
libmemdraw\unload.c
libmemdraw\cload.c
libmemdraw\load.c

libmemlayer\load.c
libmemlayer\line.c
libmemlayer\lalloc.c
libmemlayer\ldelete.c
libmemlayer\lorigin.c
libmemlayer\unload.c
libmemlayer\ltofront.c
libmemlayer\ltorear.c
libmemlayer\lsetrefresh.c
libmemlayer\lhide.c
libmemlayer\layerop.c
libmemlayer\draw.c

libsec\port\genrandom.c
libsec\port\des.c	libsec\port\desmodes.c
libsec\port\idea.c
libsec\port\rc4.c
libsec\port\md4.c
libsec\port\md5.c	libsec\port\md5block.c
libsec\port\sha1.c      libsec\port\sha1block.c
libsec\port\hmac.c
libsec\port\rsagen.c	libsec\port\rsaalloc.c
libsec\port\rsafill.c
libsec\port\rsaencrypt.c
libsec\port\rsadecrypt.c
libsec\port\dsagen.c	libsec\port\dsaalloc.c	libsec\port\dsasign.c	libsec\port\dsaverify.c
libsec\port\eggen.c     libsec\port\egalloc.c   libsec\port\egsign.c    libsec\port\egverify.c
libsec\port\genprime.c
libsec\port\gensafeprime.c
libsec\port\aes.c
libsec\port\blowfish.c
libsec\port\dsaprivtopub.c
libsec\port\egprivtopub.c
libsec\port\rsaprivtopub.c
libsec\port\dsaprimes.c
libsec\port\probably_prime.c
libsec\port\smallprimetest.c
libsec\port\prng.c
libsec\port\fastrand.c

libkeyring\egalg.c
libkeyring\dsaalg.c
libkeyring\rsaalg.c

libmath\FPcontrol-Nt.c
libmath\fdim.c
libmath\dtoa.c
libmath\blas.c
libmath\gemm.c
libmath\pow10.c
libmath\gfltconv.c
libmath\g_fmt.c

libmath\fdlibm\e_acos.c
libmath\fdlibm\e_acosh.c
libmath\fdlibm\e_asin.c
libmath\fdlibm\e_atan2.c
libmath\fdlibm\e_atanh.c
libmath\fdlibm\e_cosh.c
libmath\fdlibm\e_exp.c
libmath\fdlibm\e_fmod.c
libmath\fdlibm\e_hypot.c
libmath\fdlibm\e_j0.c
libmath\fdlibm\e_j1.c
libmath\fdlibm\e_jn.c
libmath\fdlibm\e_lgamma_r.c
libmath\fdlibm\e_log.c
libmath\fdlibm\e_log10.c
libmath\fdlibm\e_pow.c
libmath\fdlibm\e_rem_pio2.c
libmath\fdlibm\e_remainder.c
libmath\fdlibm\e_sinh.c
libmath\fdlibm\e_sqrt.c
libmath\fdlibm\k_cos.c
libmath\fdlibm\k_rem_pio2.c
libmath\fdlibm\k_sin.c
libmath\fdlibm\k_tan.c
libmath\fdlibm\s_asinh.c
libmath\fdlibm\s_atan.c
libmath\fdlibm\s_cbrt.c
libmath\fdlibm\s_ceil.c
libmath\fdlibm\s_copysign.c
libmath\fdlibm\s_cos.c
libmath\fdlibm\s_erf.c
libmath\fdlibm\s_expm1.c
libmath\fdlibm\s_fabs.c
libmath\fdlibm\s_finite.c
libmath\fdlibm\s_floor.c
libmath\fdlibm\s_ilogb.c
libmath\fdlibm\s_isnan.c
libmath\fdlibm\s_log1p.c
libmath\fdlibm\s_modf.c
libmath\fdlibm\s_nextafter.c
libmath\fdlibm\s_rint.c
libmath\fdlibm\s_scalbn.c
libmath\fdlibm\s_sin.c
libmath\fdlibm\s_tan.c
libmath\fdlibm\s_tanh.c


libmp\port\mptoi.c
libmp\port\mpadd.c
libmp\port\mpsub.c
libmp\port\mpmul.c
libmp\port\mpdiv.c
libmp\port\mpmod.c
libmp\port\mpcmp.c
libmp\port\mpright.c
libmp\port\mpleft.c
libmp\port\mpvecsub.c
libmp\port\mpvecadd.c
libmp\port\mpveccmp.c
libmp\port\mpdigdiv.c
libmp\port\mpvecdigmuladd.c
libmp\port\mpaux.c
libmp\port\mprand.c
libmp\port\mpexp.c
libmp\port\mpextendedgcd.c
libmp\port\strtomp.c
libmp\port\mpinvert.c
libmp\port\mpfmt.c
libmp\port\mptobe.c
libmp\port\mptoui.c
libmp\port\betomp.c
libmp\port\letomp.c

libfreetype\freetype.c
libfreetype\ftinit.c
libfreetype\ftsystem_inf.c
libfreetype\type1.c
libfreetype\type42.c
libfreetype\pfr.c
libfreetype\ftutil.c
libfreetype\ftgloadr.c
libfreetype\ftsmooth.c
libfreetype\ftobjs.c
libfreetype\ftstream.c
libfreetype\ftoutln.c
libfreetype\ftrend1.c
libfreetype\ftgrays.c
libfreetype\ahmodule.c
libfreetype\ahhint.c
libfreetype\ftglyph.c
libfreetype\cffdrivr.c
libfreetype\psauxmod.c
libfreetype\psmodule.c
libfreetype\psobjs.c
libfreetype\sfdriver.c
libfreetype\ttload.c
libfreetype\cffobjs.c
libfreetype\ahglyph.c
libfreetype\ttsbit.c
libfreetype\t1decode.c
libfreetype\cffcmap.c
libfreetype\t1cmap.c
libfreetype\ahglobal.c
libfreetype\ttpost.c
libfreetype\ttcmap.c
libfreetype\sfobjs.c
libfreetype\pshmod.c
libfreetype\ttcmap0.c
libfreetype\pshrec.c
libfreetype\pshglob.c
libfreetype\ahangles.c
libfreetype\ttdriver.c
libfreetype\ftcalc.c
libfreetype\cffload.c
libfreetype\ttgload.c
libfreetype\cffparse.c
libfreetype\ftraster.c
libfreetype\pshalgo3.c
libfreetype\cffgload.c
libfreetype\fttrigon.c
libfreetype\ttobjs.c
libfreetype\ttpload.c
""" + """
libinterp\dec.c
emu\Nt\emu.c
emu\Nt\emu.root.c
"""

"""
libfreetype\autohint.c
libfreetype\ftbase.c
libfreetype\ftglyph.c
libfreetype\cff.c
libfreetype\psaux.c
libfreetype\psnames.c
libfreetype\raster.c
libfreetype\sfnt.c
libfreetype\smooth.c
libfreetype\truetype.c
libfreetype\pshinter.c
"""

FNAMES = ' '.join( (os.path.join(ROOT,x) for x in FNAMES.split()) )
INCLUDES = ' '.join( (('-I'+os.path.join(ROOT,x)) for x in INCLUDES.split()) )
LIBS = ' '.join( (('-l'+x) for x in LIBS.split()) )

os.system('limbo -a          -I%s/module %s/module/runt.m > %s/libinterp/runt.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Sys      -I%s/module %s/module/runt.m > %s/libinterp/sysmod.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Keyring  -I%s/module %s/module/runt.m > %s/libinterp/keyring.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Draw     -I%s/module %s/module/runt.m > %s/libinterp/drawmod.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Prefab   -I%s/module %s/module/runt.m > %s/libinterp/prefabmod.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Tk       -I%s/module %s/module/runt.m > %s/libinterp/tkmod.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Math     -I%s/module %s/module/runt.m > %s/libinterp/mathmod.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Loader   -I%s/module %s/module/runt.m > %s/libinterp/loadermod.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Freetype -I%s/module %s/module/runt.m > %s/libinterp/freetypemod.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Bench    -I%s/module %s/module/bench.m > %s/libinterp/benchmod.h' % (ROOT,ROOT,ROOT))
os.system('limbo -a          -I%s/module %s/module/bench.m > %s/libinterp/bench.h' % (ROOT,ROOT,ROOT))
os.system('limbo -a          -I%s/module %s/module/srvrunt.b >%s/emu/port/srv.h' % (ROOT,ROOT,ROOT))
os.system('limbo -t Srv      -I%s/module %s/module/srvrunt.b >%s/emu/port/srvm.h' % (ROOT,ROOT,ROOT))


cmd=r'%s -o %s %s %s %s %s' % (CC, OUT, DEFINES, INCLUDES, LIBS, FNAMES)
#print cmd
#sys.exit()
#os.system(cmd)

import win32process, win32event
hProcess, hThread, dwPid, dwTid = win32process.CreateProcess(None, cmd, None, None, 0, 0, None, None, win32process.STARTUPINFO())
win32event.WaitForSingleObject(hProcess, win32event.INFINITE)
