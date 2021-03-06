/* $Id$    */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2003-2015                                          */
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
#ifndef OBITUVDESC_H 
#define OBITUVDESC_H 
#include "Obit.h"
#include "ObitErr.h"
#include "ObitInfoList.h"
#include "ObitImageDesc.h"
#include "ObitSkyGeom.h"

/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitUVDesc.h
 * ObitUVDesc Obit uv data descriptor class definition.
 *
 * This class is derived from the #Obit class.
 *
 * This contains information about the observations and the size and 
 * structure of the data.
 *
 * \section ObitUVDescUsage Usage
 * Instances can be obtained using the #newObitUVDesc constructor
 * the #ObitUVDescCopy copy constructor or a pointer duplicated using 
 * the #ObitUVDescRef function.
 * When an instance is no longer needed, use the #ObitUVDescUnref macro
 * to release it.
 */

/*----------------- Macroes ---------------------------*/
/** 
 * Macro to unreference (and possibly destroy) an ObitUVDesc
 * returns a ObitUVDesc* (NULL).
 * \li in = object to unreference.
 */
#define ObitUVDescUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitUVDesc.
 * returns a ObitUVDesc*.
 * in = object to reference
 */
#define ObitUVDescRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitUVDescIsA(in) ObitIsA (in, ObitUVDescGetClass())

/** Maximum number of dimensions in regular data array */
#define UV_MAXDIM 7       /* maximum array dimension */
/** Maximum number of "random" parameters */
#define UV_MAX_RANP 14       /* maximum array dimension */
/** Maximum length of descriptor string value */
#define UVLEN_VALUE 41
/** Maximum length of descriptor keyword  */
#define UVLEN_KEYWORD 21

/*--------------Class definitions-------------------------------------*/
/**
 * ObitUVDesc Class structure.
 *
 * This class contains descriptions of interferometric visibility data.
 */  
typedef struct {
#include "ObitUVDescDef.h"  /* Actual definitions */
} ObitUVDesc;

/*---------------Public functions---------------------------*/
/** Public: Class initializer. */
void ObitUVDescClassInit (void);

/** Public: Constructor. */
ObitUVDesc* newObitUVDesc (gchar *name);

/** Public: Copy UVDesc */
ObitUVDesc* ObitUVDescCopy (ObitUVDesc* in, ObitUVDesc* out,
			    ObitErr *err);

/** Public: Return class pointer. */
gconstpointer ObitUVDescGetClass (void);

/** Public: Copy descriptive (nonstructural) information. */
void ObitUVDescCopyDesc (ObitUVDesc* in, ObitUVDesc* out,
			 ObitErr *err);

/** Public: Copy Frequency information arrays. */
void ObitUVDescCopyFreq (ObitUVDesc* in, ObitUVDesc* out,
			 ObitErr *err);

/** Public: Index for easier access */
void ObitUVDescIndex (ObitUVDesc* in);

/** Public: Find the indices correspondoning to regular parameters. */
olong ObitUVDescRegularIndices(ObitUVDesc* in);

/** Public: Get Frequency arrays */
void ObitUVDescGetFreq (ObitUVDesc* in, Obit *fqtab, odouble *SouIFOff,
			ObitErr *err);

/** Public: Convert Date string to Julian Date */
void ObitUVDescDate2JD (const gchar* date, odouble *JD);

/** Public: Convert Julian Date to Date string */
void ObitUVDescJD2Date (odouble JD, gchar *date);

/** Public: Get position phase shift parameters from image descriptor */
void ObitUVDescShiftPhase (ObitUVDesc* uvDesc, 
			   ObitImageDesc* imDesc, 
			   ofloat dxyzc[3], ObitErr *err);

/** Public: Get position phase shift parameters from a shift */
void ObitUVDescShiftPosn (ObitUVDesc* uvDesc, 
			  ofloat xShift, ofloat yShift, 
			  ofloat dxyzc[3], ObitErr *err);
/**  Public: Tell rotation angle of uv data */
ofloat ObitUVDescRotate(ObitUVDesc *in);

/**  Public: Phase and UV re-projection matrices for 3D imaging */
gboolean ObitUVDescShift3DMatrix (ObitUVDesc *uvDesc, ObitImageDesc* imDesc,
				  ofloat URot3D[3][3], ofloat PRot3D[3][3]);

/**  Public: Phase and UV re-projection matrices for 3D imaging for a given posn. offset */
gboolean ObitUVDescShift3DPos (ObitUVDesc *uvDesc,  ofloat shift[2], ofloat mrotat,
			       gboolean do3D, ofloat URot3D[3][3], ofloat PRot3D[3][3]);

/**  Public: Determine number of vis per IO not to exceed memory limit */
olong ObitUVDescSetNVis (ObitUVDesc *uvDesc, ObitInfoList* info, olong nvis);

/** Public: Determine projection type */
ObitSkyGeomProj ObitUVDescGetProj(ObitUVDesc *uvDesc);

/**
 * Public: inline function to get antenna numbers, subaray from data buffer
 * Allows access to either data with "BASELINE" random parameters or
 * "ANTENNA1" and "ANTENNA2" (with 0.01*(subarray-1)).
 * \param uvDesc  Data descriptor
 * \param buffer  UV data buffer for single visibility
 * \param ant1    [out] First antenna number (1-rel)
 * \param ant2    [out] Second antenna number (1-rel)
 * \param suba    [out] Subarray number (1-rel)
 */
static inline void ObitUVDescGetAnts(ObitUVDesc *uvDesc, ofloat *buffer,
				     olong *ant1, olong *ant2, olong *suba)
{
  ofloat cbase;

  /* Which type? */
  if (uvDesc->ilocb>=0) {  /* Baseline */
    cbase = buffer[uvDesc->ilocb];
    *ant1 = (cbase / 256.0) + 0.001;
    *ant2 = (cbase - *ant1 * 256) + 0.001;
    *suba = (olong)(100.0 * (cbase -  *ant1 * 256 - *ant2) + 1.5);
  } else { /* Antennas */
    *ant1 = (olong)(buffer[uvDesc->iloca1]+0.5);
    *ant2 = (olong)(buffer[uvDesc->iloca2]+0.5);
    *suba = (olong)(buffer[uvDesc->ilocsa]+0.5);
  }
} /* end ObitUVDescGetAnts */

/**
 * Public: inline function to set antenna numbers, subaray in data buffer
 * Allows access to either data with "BASELINE" random parameters or
 * "ANTENNA1" and "ANTENNA2" (with 0.01*(subarray-1)).
 * \param uvDesc  Data descriptor
 * \param buffer  UV data buffer for single visibility
 * \param ant1    First antenna number (1-rel)
 * \param ant2    Second antenna number (1-rel)
 * \param suba    Subarray number (1-rel)
 */
static inline void ObitUVDescSetAnts(ObitUVDesc *uvDesc, ofloat *buffer,
				     olong ant1, olong ant2, olong suba)
{
  /* Which type? */
  if (uvDesc->ilocb>=0) {  /* Baseline */
    buffer[uvDesc->ilocb] = (ofloat)(ant1*256 + ant2 + 0.01*(suba-1));
  } else { /* Antennas */
    buffer[uvDesc->iloca1] = (ofloat)(ant1);
    buffer[uvDesc->iloca2] = (ofloat)(ant2);
    buffer[uvDesc->ilocsa] = (ofloat)(suba);
  }
} /* end ObitUVDescSetAnts */
/*-------------------Class Info--------------------------*/
/**
 * ClassInfo Structure.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitUVDescClassDef.h" /* Actual definition */
} ObitUVDescClassInfo; 


#endif /* OBITUVDESC_H */ 

