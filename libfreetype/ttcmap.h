/***************************************************************************/
/*                                                                         */
/*  ttcmap.h                                                               */
/*                                                                         */
/*    TrueType character mapping table (cmap) support (specification).     */
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


#ifndef __TTCMAP_H__
#define __TTCMAP_H__



#include <freetype/internal/tttypes.h>


#ifdef __cplusplus
extern "C" {
#endif


  FT_LOCAL( FT_Error )
  tt_face_load_charmap( TT_Face       face,
                        TT_CMapTable  cmap,
                        FT_Stream     input );

  FT_LOCAL( FT_Error )
  tt_face_free_charmap( TT_Face       face,
                        TT_CMapTable  cmap );


#ifdef __cplusplus
}
#endif

#endif /* __TTCMAP_H__ */


/* END */
