/***************************************************************************/
/*                                                                         */
/*  tterrors.h                                                             */
/*                                                                         */
/*    TrueType error codes (specification only).                           */
/*                                                                         */
/*  Copyright 2001 by                                                      */
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
  /* This file is used to define the TrueType error enumeration            */
  /* constants.                                                            */
  /*                                                                       */
  /*************************************************************************/

#ifndef __TTERRORS_H__
#define __TTERRORS_H__

#include <freetype/ftmoderr.h>

#undef __FTERRORS_H__

#define FT_ERR_PREFIX  TT_Err_
#define FT_ERR_BASE    FT_Mod_Err_TrueType

#include <freetype/fterrors.h>

#undef FT_ERR_PREFIX
#undef FT_ERR_BASE

#endif /* __TTERRORS_H__ */

/* END */
