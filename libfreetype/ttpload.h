/***************************************************************************/
/*                                                                         */
/*  ttpload.h                                                              */
/*                                                                         */
/*    TrueType glyph data/program tables loader (specification).           */
/*                                                                         */
/*  Copyright 1996-2001, 2002 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __TTPLOAD_H__
#define __TTPLOAD_H__



#include <freetype/internal/tttypes.h>


#ifdef __cplusplus
extern "C" {
#endif


  FT_LOCAL( FT_Error )
  tt_face_load_loca( TT_Face    face,
                     FT_Stream  stream );

  FT_LOCAL( FT_Error )
  tt_face_load_cvt( TT_Face    face,
                    FT_Stream  stream );

  FT_LOCAL( FT_Error )
  tt_face_load_fpgm( TT_Face    face,
                     FT_Stream  stream );


#ifdef __cplusplus
}
#endif

#endif /* __TTPLOAD_H__ */


/* END */
