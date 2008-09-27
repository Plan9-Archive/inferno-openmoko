#define ARM
#include	<windows.h>
#include	"dat.h"
#include	"fns.h"
#include	"error.h"



typedef struct	User	User;
typedef struct  Gmem	Gmem;
typedef	struct	Stat	Stat;
typedef	struct Fsinfo	Fsinfo;
typedef	WIN32_FIND_DATA	Fsdir;

#ifndef INVALID_SET_FILE_POINTER
#define	INVALID_SET_FILE_POINTER	((DWORD)-1)
#endif

struct Fsinfo
{
	int	uid;
	int	gid;
	int	mode;
	int	fd;
	vlong	offset;
	QLock	oq;
	char*	spec;
	Cname*	name;	/* Windows' idea of the file name */
	Fsdir*	de;	/* non-nil for saved entry from last dirread at offset */
};
#define	FS(c)	((Fsinfo*)(c)->aux)


extern	DWORD	PlatformId;
	char    rootdir[MAXROOT] = "\\inferno";
static	Qid	rootqid;
static	int	file_share_delete;
static	uchar	isntfrog[256];
static	void		fsremove(Chan*);
	wchar_t	*widen(char *s);
	char		*narrowen(wchar_t *ws);
	int		widebytes(wchar_t *ws);

static char Etoolong[] = "file name too long";

/*
 * these lan manager functions are not supplied
 * on windows95, so we have to load the dll by hand
 */

extern	int		nth2fd(HANDLE);
extern	HANDLE		ntfd2h(int);
static	int		cnisroot(Cname*);
static	int		fsisroot(Chan*);
static	int		okelem(char*, int);
static	int		fsexist(char*, Qid*);
static	char*	fspath(Cname*, char*, char*, char*);
static	Cname*	fswalkpath(Cname*, char*, int);
static	char*	fslastelem(Cname*);
static	long		fsdirread(Chan*, uchar*, int, vlong);
static	ulong		fsqidpath(char*);
static	int		fsomode(int);
static	int		fsdirset(char*, int, WIN32_FIND_DATA*, char*, Chan*, int isdir);
static 	int		fsdirsize(WIN32_FIND_DATA*, char*, Chan*);
static	void		fssettime(char*, long, long);
static	long		unixtime(FILETIME);
static	FILETIME	wintime(ulong);
	int		runeslen(Rune*);
	Rune*		runesdup(Rune*);
	Rune*		utftorunes(Rune*, char*, int);
	char*		runestoutf(char*, Rune*, int);
	int		runescmp(Rune*, Rune*);


int
winfilematch(char *path, WIN32_FIND_DATA *data)
{
	char *p;
	wchar_t *wpath;
	int r;

	p = path+strlen(path);
	while(p > path && p[-1] != '\\')
		--p;
	wpath = widen(p);
	r = (data->cFileName[0] == '.' && runeslen(data->cFileName) == 1)
			|| runescmp(data->cFileName, wpath) == 0;
	free(wpath);
	return r;
}

int
winfileclash(char *path)
{
	HANDLE h;
	WIN32_FIND_DATA data;
	wchar_t *wpath;

	wpath = widen(path);
	h = FindFirstFileW(wpath, &data);
	free(wpath);
	if (h != INVALID_HANDLE_VALUE) {
		FindClose(h);
		return !winfilematch(path, &data);
	}
	return 0;
}


/*
 * this gets called to set up the environment when we switch users
 */
void
setid(char *name, int owner)
{
	if(owner && !iseve())
		return;

	kstrdup(&up->env->user, name);
}

static void
fsfree(Chan *c)
{
	cnameclose(FS(c)->name);
	if(FS(c)->de != nil)
		free(FS(c)->de);
	free(FS(c));
}

void
fsinit(void)
{
	int n, isvol;
	ulong attr;
	char *p, tmp[MAXROOT];
	wchar_t *wp, *wpath;
	wchar_t wrootdir[MAXROOT];

	isntfrog['/'] = 1;
	isntfrog['\\'] = 1;
	isntfrog[':'] = 1;
	isntfrog['*'] = 1;
	isntfrog['?'] = 1;
	isntfrog['"'] = 1;
	isntfrog['<'] = 1;
	isntfrog['>'] = 1;

	/*
	 * vet the root
	 */
	strcpy(tmp, rootdir);
	for(p = tmp; *p; p++)
		if(*p == '/')
			*p = '\\';
	if(tmp[0] != 0 && tmp[1] == ':') {
		if(tmp[2] == 0) {
			tmp[2] = '\\';
			tmp[3] = 0;
		}
		else if(tmp[2] != '\\') {
			/* don't allow c:foo - only c:\foo */
			panic("illegal root pathX");
		}
	}
	wrootdir[0] = '\0';
	wpath = widen(tmp);
	for(wp = wpath; *wp; wp++) {
		if(*wp < 32 || (*wp < 256 && isntfrog[*wp] && *wp != '\\' && *wp != ':'))
			panic("illegal root path");
	}
//	n = GetFullPathName(wpath, MAXROOT, wrootdir, &last);
//??????????
	n = wcslen(wpath);
	wcscpy(wrootdir, wpath);

	free(wpath);
	runestoutf(rootdir, wrootdir, MAXROOT);
	if(n >= MAXROOT || n == 0)
		panic("illegal root path");

	/* get rid of trailing \ */
	while(rootdir[n-1] == '\\') {
		if(n <= 2) {
			panic("illegal root path");
		}
		rootdir[--n] = '\0';
	}

	isvol = 0;
	if(rootdir[1] == ':' && rootdir[2] == '\0')
		isvol = 1;
	else if(rootdir[0] == '\\' && rootdir[1] == '\\') {
		p = strchr(&rootdir[2], '\\');
		if(p == nil)
			panic("inferno root can't be a server");
		isvol = strchr(p+1, '\\') == nil;
	}

	if(strchr(rootdir, '\\') == nil)
		strcat(rootdir, "\\.");
	attr = GetFileAttributesW(wrootdir);
	if(attr == 0xFFFFFFFF)
		panic("root path '%s' does not exist", narrowen(wrootdir));
	rootqid.path = fsqidpath(rootdir);
	if(attr & FILE_ATTRIBUTE_DIRECTORY)
		rootqid.type |= QTDIR;
	rootdir[n] = '\0';

	rootqid.vers = time(0);

	/*
	 * set up for nt file security checking
	 */
	/* Windows Mobile 6.0 and up */
	/* file_share_delete = FILE_SHARE_DELETE;	/* sensible handling of shared files by delete and rename */
}

Chan*
fsattach(char *spec)
{
	Chan *c;
	static int devno;
	static Lock l;
	char *drive = (char *)spec;

	if (!emptystr(drive) && (drive[1] != ':' || drive[2] != '\0'))
		error(Ebadspec);

	c = devattach('U', spec);
	lock(&l);
	c->dev = devno++;
	unlock(&l);
	c->qid = rootqid;
	c->aux = smalloc(sizeof(Fsinfo));
	if(!emptystr(spec)) {
		char *s = smalloc(strlen(spec)+1);
		strcpy(s, spec);
		FS(c)->spec = s;
		c->qid.path = fsqidpath(spec);
		c->qid.type = QTDIR;
		c->qid.vers = 0;
	}
	FS(c)->name = newcname("/");
	return c;
}

Walkqid*
fswalk(Chan *c, Chan *nc, char **name, int nname)
{
	int j, alloc;
	Walkqid *wq;
	char path[MAX_PATH], *p;
	Cname *ph;
	Cname *current, *next;

	if(nname > 0)
		isdir(c);

	alloc = 0;
	current = nil;
	wq = smalloc(sizeof(Walkqid)+(nname-1)*sizeof(Qid));
	if(waserror()){
		if(alloc && wq->clone != nil)
			cclose(wq->clone);
		cnameclose(current);
		free(wq);
		return nil;
	}
	if(nc == nil){
		nc = devclone(c);
		nc->type = 0;
		alloc = 1;
	}
	wq->clone = nc;
	current = FS(c)->name;
	if(current != nil)
		incref(&current->r);
	for(j = 0; j < nname; j++){
		if(!(nc->qid.type&QTDIR)){
			if(j==0)
				error(Enotdir);
			break;
		}
		if(!okelem(name[j], 0)){
			if(j == 0)
				error(Efilename);
			break;
		}
		p = fspath(current, name[j], path, FS(c)->spec);

		if(strcmp(name[j], "..") == 0) {
			if(fsisroot(c))
				nc->qid = rootqid;
			else{
				ph = fswalkpath(current, "..", 1);
				if(cnisroot(ph)){
					nc->qid = rootqid;
					current = ph;
					if(current != nil)
						incref(&current->r);
				}
				else {
					fspath(ph, 0, path, FS(c)->spec);
					if(!fsexist(path, &nc->qid)){
						cnameclose(ph);
						if(j == 0)
						{
							o("%s:%d %s", __FILE__, __LINE__, __FUNCTION__);
							error(Enonexist);
						}
						break;
					}
				}
				next = fswalkpath(current, name[j], 1);
				cnameclose(current);
				current = next;
				cnameclose(ph);
			}
		}
		else{
			if(!fsexist(path, &nc->qid)){
				if(j == 0)
				{
					o("%s:%d %s", __FILE__, __LINE__, __FUNCTION__);
					error(Enonexist);
				}
				break;
			}
			next = fswalkpath(current, name[j], 1);
			cnameclose(current);
			current = next;
		}
		wq->qid[wq->nqid++] = nc->qid;
	}
	poperror();
	if(wq->nqid < nname){
		cnameclose(current);
		if(alloc)
			cclose(wq->clone);
		wq->clone = nil;
	}else if(wq->clone){
		nc->aux = smalloc(sizeof(Fsinfo));
		nc->type = c->type;
		FS(nc)->spec = FS(c)->spec;
		FS(nc)->name = current;
	}
	return wq;
}

Chan*
fsopen(Chan *c, int mode)
{
	HANDLE h;
	int m, isdir, aflag, cflag;
	char path[MAX_PATH];
	wchar_t *wpath;

	isdir = c->qid.type & QTDIR;
	if(isdir && mode != OREAD)
		error(Eperm);
	fspath(FS(c)->name, 0, path, FS(c)->spec);

	c->mode = openmode(mode);
	if(isdir)
		FS(c)->fd = nth2fd(INVALID_HANDLE_VALUE);
	else {
		m = fsomode(mode & 3);
		cflag = OPEN_EXISTING;
		if(mode & OTRUNC)
			cflag = TRUNCATE_EXISTING;
		aflag = FILE_FLAG_RANDOM_ACCESS;
		if(mode & ORCLOSE)
			aflag |= FILE_FLAG_DELETE_ON_CLOSE;
		if (winfileclash(path))
			error(Eexist);
		wpath = widen(path);
		//o("CreateFileW(%S)\n", wpath);
		h = CreateFileW(wpath, m, FILE_SHARE_READ|FILE_SHARE_WRITE|file_share_delete, 0, cflag, aflag, 0);
		free(wpath); // bug: it may change lasterror, todo: check all sources for such a bug
		if(h == INVALID_HANDLE_VALUE)
		{
			oserror();
		}
		FS(c)->fd = nth2fd(h);
	}

	c->offset = 0;
	FS(c)->offset = 0;
	c->flag |= COPEN;
	return c;
}

void
fscreate(Chan *c, char *name, int mode, ulong perm)
{
	HANDLE h;
	int m, aflag;
	BY_HANDLE_FILE_INFORMATION hi;
	char *p, path[MAX_PATH];
	wchar_t *wpath;
	if(!okelem(name, 1))
		error(Efilename);

	m = fsomode(mode & 3);
	p = fspath(FS(c)->name, name, path, FS(c)->spec);

	if(perm & DMDIR) {
		if(mode != OREAD) {
			error(Eisdir);
		}
		wpath = widen(path);
		if(!CreateDirectoryW(wpath, 0) || !fsexist(path, &c->qid)) {
			free(wpath);

			oserror();
		}
		free(wpath);
		FS(c)->fd = nth2fd(INVALID_HANDLE_VALUE);
	}
	else {
		aflag = 0;
		if(mode & ORCLOSE)
			aflag = FILE_FLAG_DELETE_ON_CLOSE;
		if (winfileclash(path))
			error(Eexist);
		wpath = widen(path);
		h = CreateFileW(wpath, m, FILE_SHARE_READ|FILE_SHARE_WRITE|file_share_delete, /*&sa*/0, CREATE_ALWAYS, aflag, 0);
		free(wpath);
		if(h == INVALID_HANDLE_VALUE) {

			oserror();
		}
		FS(c)->fd = nth2fd(h);
		c->qid.path = fsqidpath(path);
		c->qid.type = 0;
		c->qid.vers = 0;
		if(GetFileInformationByHandle(h, &hi))
			c->qid.vers = unixtime(hi.ftLastWriteTime);
	}

	c->mode = openmode(mode);
	c->offset = 0;
	FS(c)->offset = 0;
	c->flag |= COPEN;
	FS(c)->name = fswalkpath(FS(c)->name, name, 0);
}

void
fsclose(Chan *c)
{
	HANDLE h;

	if(c->flag & COPEN){
		h = ntfd2h(FS(c)->fd);
		if(h != INVALID_HANDLE_VALUE){
			if(c->qid.type & QTDIR)
				FindClose(h);
			else
				CloseHandle(h);
		}
	}
	if(c->flag & CRCLOSE){
		if(!waserror()){
			fsremove(c);
			poperror();
		}
		return;
	}
	fsfree(c);
}

/*
 * 64-bit seeks, using SetFilePointer because SetFilePointerEx
 * is not supported by NT
 */
static void
fslseek(HANDLE h, vlong offset)
{
	LONG hi;

	if(offset <= 0x7fffffff){
		if(SetFilePointer(h, (LONG)offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{

			oserror();
		}
	}else{
		hi = offset>>32;
		if(SetFilePointer(h, (LONG)offset, &hi, FILE_BEGIN) == INVALID_SET_FILE_POINTER &&
		   GetLastError() != NO_ERROR)
		{

			oserror();
		}
	}
}

long
fsread(Chan *c, void *va, long n, vlong offset)
{
	DWORD n2;
	HANDLE h;

	qlock(&FS(c)->oq);
	if(waserror()){
		qunlock(&FS(c)->oq);
		nexterror();
	}
	if(c->qid.type & QTDIR) {
		n2 = fsdirread(c, va, n, offset);
	}
	else {
		h = ntfd2h(FS(c)->fd);
		if(FS(c)->offset != offset){
			fslseek(h, offset);
			FS(c)->offset = offset;
		}
		if(!ReadFile(h, va, n, &n2, NULL))
		{

			oserror();
		}
		FS(c)->offset += n2;
	}
	qunlock(&FS(c)->oq);
	poperror();
	return n2;
}

long
fswrite(Chan *c, void *va, long n, vlong offset)
{
	DWORD n2;
	HANDLE h;

	qlock(&FS(c)->oq);
	if(waserror()){
		qunlock(&FS(c)->oq);
		nexterror();
	}
	h = ntfd2h(FS(c)->fd);
	if(FS(c)->offset != offset){
		fslseek(h, offset);
		FS(c)->offset = offset;
	}
	if(!WriteFile(h, va, n, &n2, NULL))
	{

		oserror();
	}
	FS(c)->offset += n2;
	qunlock(&FS(c)->oq);
	poperror();
	return n2;
}

int
fsstat(Chan *c, uchar *buf, int n)
{
	WIN32_FIND_DATAW data;
	char path[MAX_PATH];
	wchar_t *wpath;

	o("fsstat(%s)\n", FS(c)->name->s);
	/*
	 * have to fake up a data for volumes like
	 * c: and \\server\share since you can't FindFirstFile them
	 */
	if(fsisroot(c)){
		strcpy(path, rootdir);
		if(strchr(path, '\\') == nil)
			strcat(path, "\\.");
		wpath = widen(path);
		o("path=%s wpath=%S\n", path, wpath);
		data.dwFileAttributes = GetFileAttributesW(wpath);
		free(wpath);
		if(data.dwFileAttributes == 0xFFFFFFFF)
		{

			oserror();
		}
		data.ftCreationTime =
		data.ftLastAccessTime =
		data.ftLastWriteTime = wintime(0);
		data.nFileSizeHigh = 0;
		data.nFileSizeLow = 0;
		utftorunes(data.cFileName, ".", MAX_PATH);
	} else {
		HANDLE h = INVALID_HANDLE_VALUE;

		fspath(FS(c)->name, 0, path, FS(c)->spec);
		if (c->flag & COPEN)
			h = ntfd2h(FS(c)->fd);
		o("path=%s h=%ux\n", path, h);
		if (h != INVALID_HANDLE_VALUE) {
			BY_HANDLE_FILE_INFORMATION fi;
			if (c->mode & OWRITE)
				FlushFileBuffers(h);
			if (!GetFileInformationByHandle(h, &fi))
			{

				oserror();
			}
			data.dwFileAttributes = fi.dwFileAttributes;
			data.ftCreationTime = fi.ftCreationTime;
			data.ftLastAccessTime = fi.ftLastAccessTime;
			data.ftLastWriteTime = fi.ftLastWriteTime;;
			data.nFileSizeHigh = fi.nFileSizeHigh;
			data.nFileSizeLow = fi.nFileSizeLow;
		} else {
			wpath = widen(path);
			o("path=%s wpath=%S\n", path, wpath);
			h = FindFirstFileW(wpath, &data);
			free(wpath);
			if(h == INVALID_HANDLE_VALUE)
			{

				oserror();
			}
			if (!winfilematch(path, &data)) {
				FindClose(h);
				o("%s:%d %s", __FILE__, __LINE__, __FUNCTION__);
				error(Enonexist);
			}
			FindClose(h);
		}
		utftorunes(data.cFileName, fslastelem(FS(c)->name), MAX_PATH);
	}

	return fsdirset(buf, n, &data, path, c, 0);
}

int
fswstat(Chan *c, uchar *buf, int n)
{
	int wsd;
	Dir dir;
	Cname * volatile ph;
	HANDLE h;
	ulong attr;
	User *ou, *gu;
	WIN32_FIND_DATAW data;
	char *last, path[MAX_PATH], newpath[MAX_PATH], strs[4*256];
	wchar_t wspath[MAX_PATH], wsnewpath[MAX_PATH];
	wchar_t *wpath;
	int nmatch;

	n = convM2D(buf, n, &dir, strs);
	if(n == 0)
		error(Eshortstat);

	last = fspath(FS(c)->name, 0, path, FS(c)->spec);
	utftorunes(wspath, path, MAX_PATH);

	if(fsisroot(c)){
		if(dir.atime != ~0)
			data.ftLastAccessTime = wintime(dir.atime);
		if(dir.mtime != ~0)
			data.ftLastWriteTime = wintime(dir.mtime);
		utftorunes(data.cFileName, ".", MAX_PATH);
	}else{
		h = FindFirstFileW(wspath, &data);
		if(h == INVALID_HANDLE_VALUE)
		{

			oserror();
		}
		if (!winfilematch(path, &data)) {
			FindClose(h);
			o("%s:%d %s", __FILE__, __LINE__, __FUNCTION__);
			error(Enonexist);
		}
		FindClose(h);
	}

	wsd = 0;
	ou = nil;
	gu = nil;
	wpath = widen(dir.name);
	nmatch = runescmp(wpath, data.cFileName);
	free(wpath);
	if(!emptystr(dir.name) && nmatch != 0){
		if(!okelem(dir.name, 1))
			error(Efilename);
		ph = fswalkpath(FS(c)->name, "..", 1);
		if(waserror()){
			cnameclose(ph);
			nexterror();
		}
		ph = fswalkpath(ph, dir.name, 0);
		fspath(ph, 0, newpath, FS(c)->spec);
		utftorunes(wsnewpath, newpath, MAX_PATH);
		if(GetFileAttributesW(wpath) != 0xffffffff && !winfileclash(newpath))
			error("file already exists");
		if(fsisroot(c))
			error(Eperm);
		poperror();
		cnameclose(ph);
	}

	if(dir.atime != ~0 && unixtime(data.ftLastAccessTime) != dir.atime
	|| dir.mtime != ~0 && unixtime(data.ftLastWriteTime) != dir.mtime)
		fssettime(path, dir.atime, dir.mtime);

	attr = data.dwFileAttributes;
	if(dir.mode & 0222)
		attr &= ~FILE_ATTRIBUTE_READONLY;
	else
		attr |= FILE_ATTRIBUTE_READONLY;
	if(!fsisroot(c)
	&& attr != data.dwFileAttributes
	&& (attr & FILE_ATTRIBUTE_READONLY))
		SetFileAttributesW(wspath, attr);
	if(!fsisroot(c)
	&& attr != data.dwFileAttributes
	&& !(attr & FILE_ATTRIBUTE_READONLY))
		SetFileAttributesW(wspath, attr);

	/* do last so path is valid throughout */
	wpath = widen(dir.name);
	nmatch = runescmp(wpath, data.cFileName);
	free(wpath);
	if(!emptystr(dir.name) && nmatch != 0) {
		ph = fswalkpath(FS(c)->name, "..", 1);
		if(waserror()){
			cnameclose(ph);
			nexterror();
		}
		ph = fswalkpath(ph, dir.name, 0);
		fspath(ph, 0, newpath, FS(c)->spec);
		utftorunes(wsnewpath, newpath, MAX_PATH);
		/*
		 * can't rename if it is open: if this process has it open, close it temporarily.
		 */
		if(!file_share_delete && c->flag & COPEN){
			h = ntfd2h(FS(c)->fd);
			if(h != INVALID_HANDLE_VALUE)
				CloseHandle(h);	/* woe betide it if ORCLOSE */
			FS(c)->fd = nth2fd(INVALID_HANDLE_VALUE);
		}
		if(!MoveFile(wspath, wsnewpath)) {

			oserror();
		} else if(!file_share_delete && c->flag & COPEN) {
			int	aflag;
			if(c->flag & CRCLOSE)
				aflag = FILE_FLAG_DELETE_ON_CLOSE;
			h = CreateFileW(wsnewpath, fsomode(c->mode & 0x3), FILE_SHARE_READ|FILE_SHARE_WRITE|file_share_delete,/*&sa*/0, OPEN_EXISTING, aflag, 0);
			if(h == INVALID_HANDLE_VALUE)
			{

				oserror();
			}
			FS(c)->fd = nth2fd(h);
		}
		cnameclose(FS(c)->name);
		poperror();
		FS(c)->name = ph;
	}
	return n;
}

static void
fsremove(Chan *c)
{
	int n;
	char *p, path[MAX_PATH];
	wchar_t wspath[MAX_PATH];

	if(waserror()){
		fsfree(c);
		nexterror();
	}
	if(fsisroot(c))
		error(Eperm);
	p = fspath(FS(c)->name, 0, path, FS(c)->spec);
	utftorunes(wspath, path, MAX_PATH);
	if(c->qid.type & QTDIR)
		n = RemoveDirectoryW(wspath);
	else
		n = DeleteFileW(wspath);
	if (!n) {
		ulong attr;
		attr = GetFileAttributesW(wspath);
		if(attr != 0xFFFFFFFF) {
			SetFileAttributesW(wspath, FILE_ATTRIBUTE_NORMAL);
			if(c->qid.type & QTDIR)
				n = RemoveDirectoryW(wspath);
			else
				n = DeleteFileW(wspath);
			if (!n) {
				SetFileAttributesW(wspath, attr);
			}
		}
	}
	if(!n)
	{

		oserror();
	}
	poperror();
	fsfree(c);
}

/*
 * check elem for illegal characters /\:*?"<>
 * ... and relatives are also disallowed,
 * since they specify grandparents, which we
 * are not prepared to handle
 */
static int
okelem(char *elem, int nodots)
{
	int c, dots;

	dots = 0;
	while((c = *(uchar*)elem) != 0){
		if(isntfrog[c])
			return 0;
		if(c == '.' && dots >= 0)
			dots++;
		else
			dots = -1;
		elem++;
	}
	if(nodots)
		return dots <= 0;
	return dots <= 2;
}

static int
cnisroot(Cname *c)
{
	return strcmp(c->s, "/") == 0;
}

static int
fsisroot(Chan *c)
{
	return strcmp(FS(c)->name->s, "/") == 0;
}

static char*
fspath(Cname *c, char *ext, char *path, char *spec)
{
	char *p, *last, *rootd;
	int extlen = 0;

	rootd = spec != nil ? spec : rootdir;
	if(ext)
		extlen = strlen(ext) + 1;
	if(strlen(rootd) + extlen >= MAX_PATH)
		error(Etoolong);
	strcpy(path, rootd);
	if(cnisroot(c)){
		if(ext) {
			strcat(path, "\\");
			strcat(path, ext);
		}
	}else{
		if(*c->s != '/') {
			if(strlen(path) + 1 >= MAX_PATH)
				error(Etoolong);
			strcat(path, "\\");
		}
		if(strlen(path) + strlen(c->s) + extlen >= MAX_PATH)
			error(Etoolong);
		strcat(path, c->s);
		if(ext){
			strcat(path, "\\");
			strcat(path, ext);
		}
	}
	last = path;
	for(p = path; *p != '\0'; p++){
		if(*p == '/' || *p == '\\'){
			*p = '\\';
			last = p;
		}
	}
	return last;
}

extern void cleancname(Cname*);

static Cname *
fswalkpath(Cname *c, char *name, int dup)
{
	if(dup)
		c = newcname(c->s);
	c = addelem(c, name);
	if(isdotdot(name))
		cleancname(c);
	return c;
}

static char *
fslastelem(Cname *c)
{
	char *p;

	p = c->s + c->len;
	while(p > c->s && p[-1] != '/')
		p--;
	return p;
}

static int
fsdirbadentry(WIN32_FIND_DATAW *data)
{
	wchar_t *s;

	s = data->cFileName;
	if(s[0] == 0)
		return 1;
	if(s[0] == '.' && (s[1] == 0 || s[1] == '.' && s[2] == 0))
		return 1;

	return 0;
}

static Fsdir*
fsdirent(Chan *c, char *path, Fsdir *data)
{
	wchar_t *wpath;
	HANDLE h;

	h = ntfd2h(FS(c)->fd);
	if(data == nil)
		data = smalloc(sizeof(*data));
	if(FS(c)->offset == 0){
		if(h != INVALID_HANDLE_VALUE)
			FindClose(h);
		wpath = widen(path);
		h = FindFirstFileW(wpath, data);
		free(wpath);
		FS(c)->fd = nth2fd(h);
		if(h == INVALID_HANDLE_VALUE){
			free(data);
			return nil;
		}
		if(!fsdirbadentry(data))
			return data;
	}
	do{
		if(!FindNextFileW(h, data)){
			free(data);
			return nil;
		}
	}while(fsdirbadentry(data));
	return data;
}

static long
fsdirread(Chan *c, uchar *va, int count, vlong offset)
{
	int i, r;
	char path[MAX_PATH], *p;
	Fsdir *de;
	vlong o;

	if(count == 0 || offset < 0)
		return 0;
	p = fspath(FS(c)->name, "*.*", path, FS(c)->spec);
	p++;
	de = nil;
	if(FS(c)->offset != offset){
		de = FS(c)->de;
		if(FS(c)->de != nil){
			free(FS(c)->de);
			FS(c)->de = nil;
		}
		FS(c)->offset = 0;
		for(o = 0; o < offset;){
			de = fsdirent(c, path, de);
			if(de == nil){
				FS(c)->offset = o;
				return 0;
			}
			runestoutf(p, de->cFileName, &path[MAX_PATH]-p);
			path[MAX_PATH-1] = '\0';
			o += fsdirsize(de, path, c);
		}
		FS(c)->offset = offset;
	}
	for(i = 0; i < count;){
		if(FS(c)->de != nil){	/* left over from previous read at offset */
			de = FS(c)->de;
			FS(c)->de = nil;
		}else{
			de = fsdirent(c, path, de);
			if(de == nil)
				break;
		}
		runestoutf(p, de->cFileName, &path[MAX_PATH]-p);
		path[MAX_PATH-1] = '\0';
		r = fsdirset(va+i, count-i, de, path, c, 1);
		if(r <= 0){
			/* won't fit; save for next read at this offset */
			FS(c)->de = de;
			break;
		}
		i += r;
		FS(c)->offset += r;
	}
	return i;
}

static ulong
fsqidpath(char *p)
{
	ulong h;
	int c;

	h = 0;
	while(*p != '\0'){
		/* force case insensitive file names */
		c = *p++;
		if(c >= 'A' && c <= 'Z')
			c += 'a'-'A';
		h = h * 19 ^ c;
	}
	return h;
}

/* TO DO: substitute fixed, made-up (unlikely) names for these */
static char* devf[] = { "aux", "com1", "com2", "lpt1", "nul", "prn", nil };

static int
devfile(char *p)
{
	char *s, *t, *u, **ss;

	if((u = strrchr(p, '\\')) != nil)
		u++;
	else if((u = strrchr(p, '/')) != nil)
		u++;
	else
		u = p;
	for(ss = devf; *ss != nil; ss++){
		for(s = *ss, t = u; *s != '\0' && *t != '\0' && *t != '.'; s++, t++)
			if(*s != *t && *s != *t+'a'-'A')
				break;
		if(*s == '\0' && (*t == '\0' || *t == '.'))
			return 1;
	}
	return 0;
}

/*
 * there are other ways to figure out
 * the attributes and times for a file.
 * perhaps they are faster
 */
static int
fsexist(char *p, Qid *q)
{
	HANDLE h;
	WIN32_FIND_DATAW data;
	wchar_t *wpath;

	if(devfile(p))
		return 0;
	wpath = widen(p);
	h = FindFirstFileW(wpath, &data);
	free(wpath);
	if(h == INVALID_HANDLE_VALUE)
		return 0;
			if (!winfilematch(p, &data)) {
				FindClose(h);
				return 0;
			}
	FindClose(h);

	q->path = fsqidpath(p);
	q->type = 0;

	if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		q->type |= QTDIR;

	q->vers = unixtime(data.ftLastWriteTime);

	return 1;
}

static int
fsdirset(char *edir, int n, WIN32_FIND_DATAW *data, char *path, Chan *c, int isdir)
{
	Dir dir;
	static char neveryone[] = "Everyone";

	dir.name = narrowen(data->cFileName);
	dir.muid = nil;
	dir.qid.path = fsqidpath(path);
	dir.qid.vers = 0;
	dir.qid.type = 0;
	dir.mode = 0;
	dir.atime = unixtime(data->ftLastAccessTime);
	dir.mtime = unixtime(data->ftLastWriteTime);
	dir.qid.vers = dir.mtime;
	dir.length = ((uvlong)data->nFileSizeHigh<<32) | ((uvlong)data->nFileSizeLow & ~((uvlong)0xFFFFFFFF<<32));
	dir.type = 'U';
	dir.dev = c->dev;

	dir.uid = neveryone;
	dir.gid = neveryone;
	dir.mode = 0777;

	if(data->dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		dir.mode &= ~0222;
	if(data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
		dir.qid.type |= QTDIR;
		dir.mode |= DMDIR;
		dir.length = 0;
	}

	if(isdir && sizeD2M(&dir) > n)
		n = -1;
	else
		n = convD2M(&dir, edir, n);
	if(dir.uid != neveryone)
		free(dir.uid);
	if(dir.gid != neveryone)
		free(dir.gid);
	free(dir.name);
	return n;
}

static int
fsdirsize(WIN32_FIND_DATA *data, char *path, Chan *c)
{
	int n;

	n = widebytes(data->cFileName);
	n += 8+8;
	return STATFIXLEN+n;
}

static void
fssettime(char *path, long at, long mt)
{
	HANDLE h;
	FILETIME atime, mtime;
	wchar_t *wpath;

	wpath = widen(path);
	h = CreateFileW(wpath, GENERIC_WRITE,
		0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	free(wpath);
	if(h == INVALID_HANDLE_VALUE)
		return;
	mtime = wintime(mt);
	atime = wintime(at);
	if(!SetFileTime(h, 0, &atime, &mtime)){

		CloseHandle(h); // bug: it can change GetLastError()
		oserror();
	}
	CloseHandle(h);
}

static int
fsomode(int m)
{
	switch(m & 0x3) {
	case OREAD:
	case OEXEC:
		return GENERIC_READ;
	case OWRITE:
		return GENERIC_WRITE;
	case ORDWR:
		return GENERIC_READ|GENERIC_WRITE;
	}
	error(Ebadarg);
	return 0;
}


time_t time(time_t* p)
{
	if(p!=0) panic("time(0)");
	return unixtime(wintime(0));
}

static long
unixtime(FILETIME ft)
{
	vlong t;

	t = (vlong)ft.dwLowDateTime + ((vlong)ft.dwHighDateTime<<32);
	t -= (vlong)10000000*134774*24*60*60;

	return (long)(t/10000000);
}

static FILETIME
wintime(ulong t)
{
	union {
	        unsigned __int64 ft_scalar;
	        FILETIME ft_struct;
        } ft;

	if(t==0)
	{
		SYSTEMTIME sysTime;
		GetSystemTime(&sysTime);
		// Error not checked as there is no way to communicate it from this API.
		SystemTimeToFileTime(&sysTime, &ft.ft_struct);
	} else
	{
		ft.ft_scalar = (vlong)t*10000000+(vlong)10000000*134774*24*60*60;
	}
	return ft.ft_struct;
}

Rune*
runesdup(Rune *r)
{
	int n;
	Rune *s;

	n = runeslen(r) + 1;
	s = malloc(n * sizeof(Rune));
	if(s == nil)
		error(Enomem);
	memmove(s, r, n * sizeof(Rune));
	return s;
}

int
runeslen(Rune *r)
{
	int n;

	n = 0;
	while(*r++ != 0)
		n++;
	return n;
}

char*
runestoutf(char *p, Rune *r, int nc)
{
	char *op, *ep;
	int n, c;

	op = p;
	ep = p + nc;
	while(c = *r++) {
		n = 1;
		if(c >= Runeself)
			n = runelen(c);
		if(p + n >= ep)
			break;
		if(c < Runeself)
			*p++ = c;
		else
			p += runetochar(p, r-1);
	}
	*p = '\0';
	return op;
}

Rune*
utftorunes(Rune *r, char *p, int nc)
{
	Rune *or, *er;

	or = r;
	er = r + nc;
	while(*p != '\0' && r + 1 < er)
		p += chartorune(r++, p);
	*r = '\0';
	return or;
}

int
runescmp(Rune *s1, Rune *s2)
{
	Rune r1, r2;

	for(;;) {
		r1 = *s1++;
		r2 = *s2++;
		if(r1 != r2) {
			if(r1 > r2)
				return 1;
			return -1;
		}
		if(r1 == 0)
			return 0;
	}
}

Dev fsdevtab = {
	'U',
	"fs",

	fsinit,
	fsattach,
	fswalk,
	fsstat,
	fsopen,
	fscreate,
	fsclose,
	fsread,
	devbread,
	fswrite,
	devbwrite,
	fsremove,
	fswstat
};
