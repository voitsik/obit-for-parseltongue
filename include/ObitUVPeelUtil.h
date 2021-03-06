/* $Id$   */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2007-2010                                          */
/*;  Associated Universities, Inc. Washington DC, USA.                */
/*;                                                                   */
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
#ifndef OBITUVPEELUTIL_H 
#define OBITUVPEELUTIL_H 

#include "ObitInfoList.h"
#include "ObitUV.h"
#include "ObitDConCleanVis.h"
#include "ObitErr.h"

/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitUVPeelUtil.h
 * ObitUVPeelUtil module definition.
 *
 * This utility utility contains utility functions for "peeling" uv data.
 * This technique is to subtract individual sources after self calibrating on them.
 */

/*---------------Public functions---------------------------*/
/** Public: Peel strong source from UV data based on previous CLEAN */
olong ObitUVPeelUtilPeel (ObitInfoList* myInput, ObitUV* inUV, 
			 ObitDConCleanVis *myClean, olong *donePeel,  ObitErr* err);

/** Public: Loop over sources to be peeled */
void ObitUVPeelUtilLoop (ObitInfoList* myInput, ObitUV* inUV, 
			 ObitDConCleanVis *myClean, 
			 olong *nfield, olong **ncomp,  ObitErr* err);

#endif /* OBITIUVPEELUTIL_H */ 
