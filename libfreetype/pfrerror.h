/***************************************************************************/
/*                                                                         */
/*  pfrerror.h                                                             */
/*                                                                         */
/*    PFR error codes (specification only).                                */
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


  /*************************************************************************/
  /*                                                                       */
  /* This file is used to define the PFR error enumeration constants.      */
  /*                                                                       */
  /*************************************************************************/

#ifndef __PFRERROR_H__
#define __PFRERROR_H__

#include <freetype/ftmoderr.h>

#undef __FTERRORS_H__

#define FT_ERR_PREFIX  PFR_Err_
#define FT_ERR_BASE    FT_Mod_Err_PFR

#include <freetype/fterrors.h>

#undef FT_ERR_PREFIX
#undef FT_ERR_BASE

#endif /* __PFRERROR_H__ */


/* END */
