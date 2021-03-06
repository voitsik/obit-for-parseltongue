/* $Id$ */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2010-2020                                          */
/*;  Associated Universities, Inc. Washington DC, USA.                */
/*;  This program is free software; you can redistribute it and/or    */
/*;  modify it under the terms of the GNU General Public License as   */
/*;  published by the Free Software Foundation; either version 2 of   */
/*;  the License, or (at your option) any later version.              */
/*;                                                                   */
/*;  This program is distributed in the hope that it will be useful,  */
/*;  but WITHOUT ANY WARRANTY; without even the implied warranty of   */
/*;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    */
/*;  GNU General Public License for more details.                     */
/*;                                                                   */
/*;  You should have received a copy of the GNU General Public        */
/*;  License along with this program; if not, write to the Free       */
/*;  Software Foundation, Inc., 675 Massachusetts Ave, Cambridge,     */
/*;  MA 02139, USA.                                                   */
/*;                                                                   */
/*;Correspondence about this software should be addressed as follows: */
/*;         Internet email: bcotton@nrao.edu.                         */
/*;         Postal address: William Cotton                            */
/*;                         National Radio Astronomy Observatory      */
/*;                         520 Edgemont Road                         */
/*;                         Charlottesville, VA 22903-2475 USA        */
/*--------------------------------------------------------------------*/
/*  Define the basic components of the ObitDConCleanVisMF structure   */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitDConCleanVisMFDef.h
 * ObitDConCleanVisMF structure members for this and any derived classes.
 */
#include "ObitDConCleanVisDef.h"  /* Parent class definitions */
/** Order of the imaging 
    Spectral index only = 1, plus curvature = 2 */
olong order;
/** minimum residual flux for 1st and second order */
ofloat OrdFlux[5];
/** Doing dual Q&U polarization? */
gboolean isDual;
/* Secondary (UPol) SkyModel */
ObitSkyModel *skyModel2;
/* Secondary Pixel (UPol) list */
ObitDConCleanPxList *Pixels2;
