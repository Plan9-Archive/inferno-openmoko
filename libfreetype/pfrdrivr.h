/***************************************************************************/
/*                                                                         */
/*  pfrdrivr.h                                                             */
/*                                                                         */
/*    High-level Type PFR driver interface (specification).                */
/*                                                                         */
/*  Copyright 2002 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __PFRDRIVR_H__
#define __PFRDRIVR_H__



#include <freetype/internal/ftdriver.h>


#ifdef __cplusplus
extern "C" {
#endif


  FT_EXPORT_VAR( const FT_Driver_ClassRec )  pfr_driver_class;


#ifdef __cplusplus
}
#endif


#endif /* __PFRDRIVR_H__ */


/* END */
