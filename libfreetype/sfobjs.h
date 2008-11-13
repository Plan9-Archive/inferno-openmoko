/***************************************************************************/
/*                                                                         */
/*  sfobjs.h                                                               */
/*                                                                         */
/*    SFNT object management (specification).                              */
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


#ifndef __SFOBJS_H__
#define __SFOBJS_H__



#include <freetype/internal/sfnt.h>
#include <freetype/internal/ftobjs.h>


#ifdef __cplusplus
extern "C" {
#endif


  FT_LOCAL( FT_Error )
  sfnt_init_face( FT_Stream      stream,
                  TT_Face        face,
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params );

  FT_LOCAL( FT_Error )
  sfnt_load_face( FT_Stream      stream,
                  TT_Face        face,
                  FT_Int         face_index,
                  FT_Int         num_params,
                  FT_Parameter*  params );

  FT_LOCAL( void )
  sfnt_done_face( TT_Face  face );


#ifdef __cplusplus
}
#endif

#endif /* __SFDRIVER_H__ */


/* END */
