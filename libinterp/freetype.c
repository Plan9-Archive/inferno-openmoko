#include <lib9.h>
#include <kernel.h>
#include "isa.h"
#include "interp.h"
#include "runt.h"
#include "raise.h"
#include "freetypemod.h"
#include "freetype.h"


typedef struct Face Face;
struct Face {
	Freetype_Face	freetypeface;		/* limbo part */
	FTface		ftface;			/* private parts */
};

Type*	TMatrix;
Type*	TVector;
Type*	TFace;
Type*	TGlyph;

static char	Matrixmap[] = Freetype_Matrix_map;
static char	Vectormap[] = Freetype_Vector_map;
static char	Facemap[] = Freetype_Face_map;
static char	Glyphmap[] = Freetype_Glyph_map;

static void		freeface(Heap*, int);
static Face*	ckface(Freetype_Face*);

void
freetypemodinit(void)
{
	builtinmod("$Freetype", Freetypemodtab, Freetypemodlen);
	TMatrix = dtype(freeheap, sizeof(Freetype_Matrix), Matrixmap, sizeof(Matrixmap), "Freetype->Matrix" );
	TVector = dtype(freeheap, sizeof(Freetype_Vector), Vectormap, sizeof(Vectormap), "Freetype->Vector" );
	TFace = dtype(freeface, sizeof(Face), Facemap, sizeof(Facemap), "Freetype->Face" );
	TGlyph = dtype(freeheap, sizeof(Freetype_Glyph), Glyphmap, sizeof(Glyphmap), "Freetype->Glyph" );
}

DISAPI(Face_haschar)
{
	Face *face;

	*f->ret = 0;
	face = ckface(f->face);
	release();
	*f->ret = fthaschar(face->ftface, f->c);
	acquire();
}

DISAPI(Face_loadglyph)
{
	Heap *h;
	Face *face;
	Freetype_Glyph *g;
	FTglyph ftg;
	int n, i, s1bpr, s2bpr;
	char *err;

	face = ckface(f->face);

	destroy(*f->ret);
	*f->ret = (Freetype_Glyph *)H;

	release();
	err = ftloadglyph(face->ftface, f->c, &ftg);
	acquire();
	if (err != nil) {
		kwerrstr(err);
		return;
	}

	h = heap(TGlyph);
	if (h == H) {
		kwerrstr(exNomem);
		return;
	}
	g = H2D(Freetype_Glyph*, h);
	n = ftg.width*ftg.height;
	h = heaparray(&Tbyte, n);
	if (h == H) {
		destroy(g);
		kwerrstr(exNomem);
		return;
	}
	g->bitmap = H2D(Array*, h);
	g->top = ftg.top;
	g->left = ftg.left;
	g->height = ftg.height;
	g->width = ftg.width;
	g->advance.x = ftg.advx;
	g->advance.y = ftg.advy;

	s1bpr = ftg.width;
	s2bpr = ftg.bpr;
	for (i = 0; i < ftg.height; i++)
		memcpy(g->bitmap->data+(i*s1bpr), ftg.bitmap+(i*s2bpr), s1bpr);
	*f->ret = g;
}

DISAPI(Freetype_newface)
{
	Heap *h;
	Face *face;
	Freetype_Face *limboface;
	FTfaceinfo finfo;
	char *path;
	char *err;

	destroy(*f->ret);
	*f->ret = (Freetype_Face *)H;

	h = heapz(TFace);
	if (h == H) {
		kwerrstr(exNomem);
		return;
	}

	face = H2D(Face*, h);
	limboface = (Freetype_Face*)face;
	*f->ret = limboface;
	path = strdup(string2c(f->path));	/* string2c() can call error() */
	release();
	err = ftnewface(path, f->index, &face->ftface, &finfo);
	acquire();
	free(path);
	if (err != nil) {
		*f->ret = (Freetype_Face *)H;
		destroy(face);
		kwerrstr(err);
		return;
	}
	limboface->nfaces = finfo.nfaces;
	limboface->index = finfo.index;
	limboface->style = finfo.style;
	limboface->height = finfo.height;
	limboface->ascent = finfo.ascent;
	limboface->familyname = c2string(finfo.familyname, strlen(finfo.familyname));
	limboface->stylename = c2string(finfo.stylename, strlen(finfo.stylename));
	*f->ret = limboface;
}

DISAPI(Freetype_newmemface)
{
	destroy(*f->ret);
	*f->ret = (Freetype_Face *)H;

	kwerrstr("not implemented");
}

DISAPI(Face_setcharsize)
{
	Face *face;
	Freetype_Face *limboface;
	FTfaceinfo finfo;
	char *err;

	face = ckface(f->face);
	limboface = (Freetype_Face*)face;
	release();
	err = ftsetcharsize(face->ftface, f->pts, f->hdpi, f->vdpi, &finfo);
	acquire();
	if (err == nil) {
		limboface->height = finfo.height;
		limboface->ascent = finfo.ascent;
	}
	retstr(err, f->ret);
}

DISAPI(Face_settransform)
{
	FTmatrix *m = nil;
	FTvector *v = nil;
	Face *face;

	face = ckface(f->face);

	/*
	 * ftsettransform() has no error return
	 * we have one for consistency - but always nil for now
	 */
	destroy(*f->ret);
	*f->ret = (String*)H;

	if (f->m != H)
		m = (FTmatrix*)(f->m);
	if (f->v != H)
		v = (FTvector*)(f->v);
	release();
	ftsettransform(face->ftface, m, v);
	acquire();
}

static void
freeface(Heap *h, int swept)
{
	Face *face = H2D(Face*, h);

	if (!swept) {
		destroy(face->freetypeface.familyname);
		destroy(face->freetypeface.stylename);
	}
	release();
	ftdoneface(face->ftface);
	acquire();
	memset(&face->ftface, 0, sizeof(face->ftface));
}

static Face*
ckface(Freetype_Face *face)
{
	if (face == nil || face == H)
		error("nil Face");
	if (D2H(face)->t != TFace)
		error(exType);
	return (Face*)face;
}
