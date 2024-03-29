/* $Id$  */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2002-2022                                          */
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
/*; Correspondence about this software should be addressed as follows:*/
/*;         Internet email: bcotton@nrao.edu.                         */
/*;         Postal address: William Cotton                            */
/*;                         National Radio Astronomy Observatory      */
/*;                         520 Edgemont Road                         */
/*;                         Charlottesville, VA 22903-2475 USA        */
/*--------------------------------------------------------------------*/
/*  Define the basic components of the ObitUVGrid ClassInfo structure */
/* This is intended to be included in a classInfo structure definition*/
#include "ObitClassDef.h"  /* Parent class ClassInfo definition file */
/** Function pointer to Create/initialize structures. */
ObitUVGridSetupFP ObitUVGridSetup;
/** Function pointer to Read/grid uv data. */
ObitUVGridReadUVFP ObitUVGridReadUV;
/** Function pointer to Parallel Read/grid uv data. */
ObitUVGridReadUVParFP ObitUVGridReadUVPar;
/** Function pointer to transform/correct. */
ObitUVGridFFT2ImFP ObitUVGridFFT2Im;
/** Function pointer to Parallel transform/correct. */
ObitUVGridFFT2ImParFP ObitUVGridFFT2ImPar;

/* Private functions for derived classes */
/** Function pointer to Prep buffer. */
PrepBufferFP PrepBuffer;
/** Function pointer to Grid buffer. */
GridBufferFP GridBuffer;
/** Function pointer to Gridding correction. */
GridCorrFnFP  GridCorrFn;
/** Function pointer to get max number of parallel grids */
ObitUVGridGetNumParFP ObitUVGridGetNumPar;
