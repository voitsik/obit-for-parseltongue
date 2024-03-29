/* $Id$   */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2004-2022                                          */
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

/*#include "glib/gqsort.h"*/
#include "ObitTableCCUtil.h"
#include "ObitMem.h"
#include "ObitBeamShape.h"
#include "ObitSpectrumFit.h"

/*----------------Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitTableCCUtil.c
 * ObitTableCC class utility function definitions.
 */

/*----------------------Private function prototypes----------------------*/
/** Private: Form sort structure for a table */
static ofloat* 
MakeCCSortStruct (ObitTableCC *in, olong *number, olong *size, olong *ncomp,
		  ofloat *parms, ObitErr *err);

/** Private: Form sort structure for a table with selection by row  */
static ofloat* 
MakeCCSortStructSel (ObitTableCC *in, olong startComp, olong endComp, 
		     olong *size, olong *number, olong *ncomp, ofloat *parms, 
		     ObitErr *err);

/** Private: Form sort structure for a table with selection by row w/ mixed Gaussians */
static ofloat* 
MakeCCSortStructSel2 (ObitTableCC *in, olong startComp, olong endComp, 
		      olong *size, olong *number, olong *ncomp,
		      ObitSkyModelCompType *type, ObitErr *err);

/** Private: Sort comparison function for positions */
static gint CCComparePos (gconstpointer in1, gconstpointer in2, 
			  gpointer ncomp);

/** Private: Sort comparison function for Flux density */
static gint CCCompareFlux (gconstpointer in1, gconstpointer in2, 
		     gpointer ncomp);

/** Private: Merge entries in Sort structure */
static void CCMerge (ofloat *base, olong size, olong number); 

/** Private: Merge spectral entries in Sort structure */
static void CCMergeSpec (ofloat *base, olong size, olong number, 
			 gboolean doSpec, gboolean doTSpec, 
			 gboolean doZ); 

/** Private: Merge spectral entries in Sort structure w/ mixed Gaussians */
static void CCMergeSpec2 (ofloat *base, olong size, olong number, 
			 gboolean doSpec, gboolean doTSpec, 
			 gboolean doZ); 

/** Private: reorder table based on Sort structure */
static ObitIOCode 
ReWriteTable(ObitTableCC *out, ofloat *base, olong size, olong number, 
	     ofloat *parms, ObitErr *err);
/*----------------------Public functions---------------------------*/

/**
 * Grid components as points onto grid.  The image from which the components is
 * derived is described in desc.  The output grid is padded by a factor OverSample.
 * If the components are Gaussians, their parameters are returned in gaus.
 * \param in         Table to grid
 * \param OverSample Expansion factor for output image
 * \param first      First component (1-rel) to include, 0=>1, filled in if changed
 * \param last       Last component (1-rel) to include, 0=>all, filled in if changed
 * \param noNeg      Ignore first negative flux component and after
 * \param factor     factor to multiply timec fluxes
 * \param minFlux    Minimum abs. value flux density to include (before factor)
 * \param maxFlux    Maximum abs. value flux density to include (before factor)
 * \param desc       Descriptor for image from which components derived
 * \param grid       [out] filled in array, created, resized if necessary
 * \param gparm      [out] Gaussian/expDisk parameters (major, minor, PA all in deg)
 *                   if the components in in are Gaussians or expDisk, else, -1.
 * \param ncomp      [out] number of components gridded.
 * \param err        ObitErr error stack.
 * \return I/O Code  OBIT_IO_OK = OK.
 */
ObitIOCode ObitTableCCUtilGrid (ObitTableCC *in, olong OverSample, 
				olong *first, olong *last, gboolean noNeg,
				ofloat factor, ofloat minFlux, ofloat maxFlux,
				ObitImageDesc *desc, ObitFArray **grid, 
				ofloat gparm[3], olong *ncomp, 
				ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  ObitTableCCRow *CCRow = NULL;
  olong itestX, itestY;
  ofloat ftestX, ftestY, maxX, minX, maxY, minY;
  ofloat *array, xpoff, ypoff;
  olong j, irow, xPix, yPix, iAddr;
  ofloat iCellX, iCellY, fNx, fNy;
  olong ndim, naxis[2], nx, ny, count = 0, badCnt = 0;
  ObitCCCompType mtype = OBIT_CC_Unknown;
  gboolean isGauss=FALSE, isExpDisk=FALSE;
  gchar *routine = "ObitTableCCUtilGrid";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return retCode;
  g_assert (ObitTableCCIsA(in));

  gparm[0] = gparm[1] = gparm[2] = -1.0; /* init Gaussian */
  
  /* Create/resize output if necessary */
  ndim = 2;
  naxis[0] = OverSample*desc->inaxes[desc->jlocr];
  naxis[1] = OverSample*desc->inaxes[desc->jlocd];
  /* (re)allocate memory for plane */
  if (*grid!=NULL) *grid = ObitFArrayRealloc(*grid, ndim, naxis);
  else *grid = ObitFArrayCreate("ModelImage", ndim, naxis);

  /* Zero fill */
  ObitFArrayFill (*grid, 0.0);

  /* Get pointer to in->plane data array */
  naxis[0] = 0; naxis[1] = 0; 
  array = ObitFArrayIndex(*grid, naxis);
  
  /* Image size as float */
  nx  = OverSample*desc->inaxes[desc->jlocr];
  ny  = OverSample*desc->inaxes[desc->jlocd];
  fNx = (ofloat)nx;
  fNy = (ofloat)ny;
  /* allowed range of X */
  minX = (-fNx/2.0) * fabs(desc->cdelt[desc->jlocr]);
  maxX = ((fNx/2.0) - 1.0) * fabs(desc->cdelt[desc->jlocr]);
  /* allowed range of Y */
  minY = (-fNy/2.0) * fabs(desc->cdelt[desc->jlocd]);
  maxY = ((fNy/2.0) - 1.0) * fabs(desc->cdelt[desc->jlocd]);

  /* Open CC table */
  retCode = ObitTableCCOpen (in, OBIT_IO_ReadWrite, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, in->name, retCode);
  
  /* Create table row */
  if (!CCRow) CCRow = newObitTableCCRow (in);

  /* Field specific stuff */
  /* Inverse Cell spacings */
  if (desc->cdelt[desc->jlocr]!=0.0) iCellX = 1.0/desc->cdelt[desc->jlocr];
  else iCellX = 1.0;
  if (desc->cdelt[desc->jlocd]!=0.0) iCellY = 1.0/desc->cdelt[desc->jlocd];
  else iCellY = 1.0;

  /*    Get reference pixel offsets from (nx/2+1, ny/2+1) */
  xpoff = (desc->crpix[desc->jlocr] - (desc->inaxes[desc->jlocr]/2) - 1) *
    desc->cdelt[desc->jlocr];
  ypoff = (desc->crpix[desc->jlocd] - (desc->inaxes[desc->jlocd]/2) - 1) *
    desc->cdelt[desc->jlocd];

  /* loop over CCs */
  count = 0;  /* Count of components */
  if (*first<=0) *first = 1;
  if (*last<=0) *last = in->myDesc->nrow;
  *last = MIN (*last, in->myDesc->nrow);
  for (j=*first; j<=*last; j++) {
    irow = j;
    retCode = ObitTableCCReadRow (in, irow, CCRow, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) 
      Obit_traceback_val (err, routine, in->name, retCode);
    
    /* Get any Gaussian parameters on first, else check */
    if (j==*first) {
      /* Is this a Gaussian component? */
      if ((in->parmsCol>=0) &&
	  (in->myDesc->dim[in->parmsCol][0]>=4)) {
	mtype = (ObitCCCompType)CCRow->parms[3]+0.5;
	if ((mtype==OBIT_CC_GaussMod) ||
	    (mtype==OBIT_CC_CGaussMod) ||
	    (mtype==OBIT_CC_GaussModSpec) ||
	    (mtype==OBIT_CC_CGaussModSpec) ||
	    (mtype==OBIT_CC_GaussModTSpec) ||
	    (mtype==OBIT_CC_CGaussModTSpec)) {
	  gparm[0] = CCRow->parms[0];
	  gparm[1] = CCRow->parms[1];
	  gparm[2] = CCRow->parms[2];
	  isGauss = TRUE;
	}
      }
    } else if (isGauss) {
      /* All Gaussians MUST be the same */
      if ((CCRow->parms[0]!=gparm[0]) || (CCRow->parms[1]!=gparm[1]) || 
	  (CCRow->parms[2]!=gparm[2])) {
	Obit_log_error(err, OBIT_Error,"%s: All Gaussians MUST have same size",
		       routine);
	return retCode ;
      }
    } /* end of Gaussian Check */

    /* Get any Exponential disk parameters on first, else check */
    if (j==*first) {
      /* Is this a expDisk component? */
      if ((in->parmsCol>=0) &&
	  (in->myDesc->dim[in->parmsCol][0]>=4)) {
	mtype = (ObitCCCompType)CCRow->parms[3]+0.5;
	if ((mtype==OBIT_CC_expDiskMod) ||
	    (mtype==OBIT_CC_expDiskModSpec) ||
	    (mtype==OBIT_CC_expDiskModTSpec)) {
	  gparm[0] = CCRow->parms[0];
	  isExpDisk = TRUE;
	}
      }
    } else if (isExpDisk) {
      /* All expDisks MUST be the same */
      if (CCRow->parms[0]!=gparm[0]) {
	Obit_log_error(err, OBIT_Error,"%s: All exp. Disks MUST have same size",
		       routine);
	return retCode ;
      }
    } /* end of expDisk Check */

    /* Only to first negative? */
    if (noNeg && (CCRow->Flux<0.0)) break;
  
    /* Process component */
    CCRow->Flux *= factor;     /* Apply factor */
    CCRow->DeltaX += xpoff;    /* Reference pixel offset from  (nx/2,ny/2)*/
    CCRow->DeltaY += ypoff;

    /* Component wanted? larger than in->minFlux and not zero */
    if ((fabs(CCRow->Flux)<minFlux)  || (CCRow->Flux==0.0)) continue;
    /* Nothing too big */
    if (fabs(CCRow->Flux)>maxFlux) continue;
    
    /* Check that comps are on cells */
    ftestX = CCRow->DeltaX * iCellX; /* Convert to cells */
    ftestY = CCRow->DeltaY * iCellY;
    if (ftestX>0.0) itestX = (olong)(ftestX + 0.5);
    else itestX = (olong)(ftestX - 0.5);
    if (ftestY>0.0) itestY = (olong)(ftestY + 0.5);
    else itestY = (olong)(ftestY - 0.5);
  
    /* Count bad cells */
    if ((fabs(ftestX-itestX)>0.1) || (fabs(ftestY-itestY)>0.1)) {
      badCnt++;
      /* Warn but keep going */
      if (badCnt<50) {
	Obit_log_error(err, OBIT_InfoWarn, "%s Warning: Bad cell %f %f in %s", 
		       routine, CCRow->DeltaX*iCellX, CCRow->DeltaY*iCellY, in->name);
      }
    }

    /* Clip range of X,Y */
    CCRow->DeltaX = MIN (maxX, MAX (CCRow->DeltaX, minX));
    CCRow->DeltaY = MIN (maxY, MAX (CCRow->DeltaY, minY));

    /* X,Y to cells */
    CCRow->DeltaX *= iCellX;
    CCRow->DeltaY *= iCellY;
    /* 0-rel pixel numbers */
    xPix = (olong)(CCRow->DeltaX + nx/2 + 0.5);
    yPix = (olong)(CCRow->DeltaY + ny/2 + 0.5);

    /* Sum into image */
    iAddr = xPix + nx * yPix;
    array[iAddr] += CCRow->Flux;

    count++;        /* how many */
  } /* end loop over components */

  /* How many? */
  *ncomp = count;
  
  /* Close Table */
  retCode = ObitTableCCClose (in, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, in->name, retCode);
  
  /* Release table/row */
  CCRow   = ObitTableCCRowUnref (CCRow);
  
  return retCode;
} /* end ObitTableCCUtilGrid */

/**
 * Grid spectral components as points onto grid.  The image from which the components is
 * derived is described in desc.  The output grid is padded by a factor OverSample.
 * If the components are Gaussians, their parameters are returned in gaus.
 * Output image is spectral term iterm times the flux density.
 * Works for both parameterized spectra (Param[3} 10-19) 
 * or tabulated spectra (Param[3} 20-29).
 * \param in         Table to grid
 * \param OverSample Expansion factor for output image
 * \param iterm      Spectral term to grid, 0=flux, 1=flux*si, 2=flux*curve...
 *                   For tabulated spectra this is the spectrum
 * \param first      First component (1-rel) to include, 0=>1, filled in if changed
 * \param last       Last component (1-rel) to include, 0=>all, filled in if changed
 * \param noNeg      Ignore first negative flux component and after
 * \param factor     factor to multiply timec fluxes
 * \param minFlux    Minimum abs. value flux density to include (before factor)
 * \param maxFlux    Maximum abs. value flux density to include (before factor)
 * \param desc       Descriptor for image from which components derived
 * \param grid       [out] filled in array, created, resized if necessary
 * \param gparm      [out] Gaussian/expDisk parameters (major, minor, PA all in deg) if
 *                   the components in in are Gaussians, else, -1.
 * \param ncomp      [out] number of components gridded.
 * \param err        ObitErr error stack.
 * \return I/O Code  OBIT_IO_OK = OK.
 */
ObitIOCode ObitTableCCUtilGridSpect (ObitTableCC *in, olong OverSample, olong iterm,
				     olong *first, olong *last, gboolean noNeg,
				     ofloat factor, ofloat minFlux, ofloat maxFlux,
				     ObitImageDesc *desc, ObitFArray **grid, 
				     ofloat gparm[3], olong *ncomp, 
				     ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  ObitTableCCRow *CCRow = NULL;
  olong itestX, itestY;
  ofloat ftestX, ftestY, maxX, minX, maxY, minY;
  ofloat *array, xpoff, ypoff;
  olong j, irow, xPix, yPix, iAddr;
  ofloat iCellX, iCellY, fNx, fNy, spectTerm;
  olong ndim, naxis[2], nx, ny, parmoff, count = 0, badCnt = 0;
  gboolean doSpec=TRUE, doTSpec=FALSE, isGauss=FALSE, isExpDisk=FALSE;
  ObitCCCompType mtype = OBIT_CC_Unknown;
  gchar *routine = "ObitTableCCUtilGridSpect";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return retCode;
  g_assert (ObitTableCCIsA(in));

  gparm[0] = gparm[1] = gparm[2] = -1.0; /* init Gaussian */
  
  /* Create/resize output if necessary */
  ndim = 2;
  naxis[0] = OverSample*desc->inaxes[desc->jlocr];
  naxis[1] = OverSample*desc->inaxes[desc->jlocd];
  /* (re)allocate memory for plane */
  if (*grid!=NULL) *grid = ObitFArrayRealloc(*grid, ndim, naxis);
  else *grid = ObitFArrayCreate("ModelImage", ndim, naxis);

  /* Zero fill */
  ObitFArrayFill (*grid, 0.0);

  /* Get pointer to in->plane data array */
  naxis[0] = 0; naxis[1] = 0; 
  array = ObitFArrayIndex(*grid, naxis);
  
  /* Image size as float */
  nx  = OverSample*desc->inaxes[desc->jlocr];
  ny  = OverSample*desc->inaxes[desc->jlocd];
  fNx = (ofloat)nx;
  fNy = (ofloat)ny;
  /* allowed range of X */
  minX = (-fNx/2.0) * fabs(desc->cdelt[desc->jlocr]);
  maxX = ((fNx/2.0) - 1.0) * fabs(desc->cdelt[desc->jlocr]);
  /* allowed range of Y */
  minY = (-fNy/2.0) * fabs(desc->cdelt[desc->jlocd]);
  maxY = ((fNy/2.0) - 1.0) * fabs(desc->cdelt[desc->jlocd]);

  /* Open CC table */
  retCode = ObitTableCCOpen (in, OBIT_IO_ReadWrite, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, in->name, retCode);
  
  /* Create table row */
  if (!CCRow) CCRow = newObitTableCCRow (in);

  /* Field specific stuff */
  /* Inverse Cell spacings */
  if (desc->cdelt[desc->jlocr]!=0.0) iCellX = 1.0/desc->cdelt[desc->jlocr];
  else iCellX = 1.0;
  if (desc->cdelt[desc->jlocd]!=0.0) iCellY = 1.0/desc->cdelt[desc->jlocd];
  else iCellY = 1.0;

  /*    Get reference pixel offsets from (nx/2+1, ny/2+1) */
  xpoff = (desc->crpix[desc->jlocr] - (desc->inaxes[desc->jlocr]/2) - 1) *
    desc->cdelt[desc->jlocr];
  ypoff = (desc->crpix[desc->jlocd] - (desc->inaxes[desc->jlocd]/2) - 1) *
    desc->cdelt[desc->jlocd];

  /* Where is spectral term? */
  parmoff = 3;

  /* loop over CCs */
  count = 0;  /* Count of components */
  if (*first<=0) *first = 1;
  if (*last<=0) *last = in->myDesc->nrow;
  *last = MIN (*last, in->myDesc->nrow);
  for (j=*first; j<=*last; j++) {
    irow = j;
    retCode = ObitTableCCReadRow (in, irow, CCRow, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) 
      Obit_traceback_val (err, routine, in->name, retCode);
    
    /* Make sure this has a spectrum */
    Obit_retval_if_fail((CCRow->parms && (CCRow->parms[3]>=10.)), err, retCode, 
			"%s: CCs do not contain spectra", routine);
    
    /* Get any Gaussian parameters on first, else check */
    if (j==*first) {
      /* Is this a Gaussian component? */
      if ((in->parmsCol>=0) &&
	  (in->myDesc->dim[in->parmsCol][0]>=4)) {
	mtype = (ObitCCCompType)CCRow->parms[3]+0.5;
	if ((mtype==OBIT_CC_GaussMod) ||
	    (mtype==OBIT_CC_CGaussMod) ||
	    (mtype==OBIT_CC_GaussModSpec) ||
	    (mtype==OBIT_CC_CGaussModSpec) ||
	    (mtype==OBIT_CC_GaussModTSpec) ||
	    (mtype==OBIT_CC_CGaussModTSpec)) {
	  gparm[0] = CCRow->parms[0];
	  gparm[1] = CCRow->parms[1];
	  gparm[2] = CCRow->parms[2];
	  isGauss = TRUE;
	}
      }
    } else if (isGauss) {
      /* All Gaussians MUST be the same */
      if ((CCRow->parms[0]!=gparm[0]) || (CCRow->parms[1]!=gparm[1]) || 
	  (CCRow->parms[2]!=gparm[2])) {
	Obit_log_error(err, OBIT_Error,"%s: All Gaussians MUST have same size",
		       routine);
	return retCode ;
      }
    } /* end of Gaussian Check */

    /* Get any Exponential disk parameters on first, else check */
    if (j==*first) {
      /* Is this a expDisk component? */
      if ((in->parmsCol>=0) &&
	  (in->myDesc->dim[in->parmsCol][0]>=4)) {
	mtype = (ObitCCCompType)CCRow->parms[3]+0.5;
	if ((mtype==OBIT_CC_expDiskMod) ||
	    (mtype==OBIT_CC_expDiskModSpec) ||
	    (mtype==OBIT_CC_expDiskModTSpec)) {
	  gparm[0] = CCRow->parms[0];
	  isExpDisk = TRUE;
  
	}
      }
    } else if (isExpDisk) {
      /* All expDisks MUST be the same */
      if (CCRow->parms[0]!=gparm[0]) {
	Obit_log_error(err, OBIT_Error,"%s: All exp. Disks MUST have same size",
		       routine);
	return retCode ;
      }
    } /* end of expDisk Check */

    /* Get spectrum type */
    doSpec  = (CCRow->parms[3]>=9.9)  && (CCRow->parms[3]<=19.0);
    doTSpec = (CCRow->parms[3]>=19.9) && (CCRow->parms[3]<=29.0);
    
    /* Only to first negative? */
    if (noNeg && (CCRow->Flux<0.0)) break;
  
    /* Process component */
    CCRow->Flux *= factor;     /* Apply factor */
    CCRow->DeltaX += xpoff;    /* Reference pixel offset from  (nx/2,ny/2)*/
    CCRow->DeltaY += ypoff;

    /* Component wanted? larger than in->minFlux and not zero */
    if ((fabs(CCRow->Flux)<minFlux)  || (CCRow->Flux==0.0)) continue;
    /* Nothing too big */
    if (fabs(CCRow->Flux)>maxFlux) continue;
    
    /* Check that comps are on cells */
    ftestX = CCRow->DeltaX * iCellX; /* Convert to cells */
    ftestY = CCRow->DeltaY * iCellY;
    if (ftestX>0.0) itestX = (olong)(ftestX + 0.5);
    else itestX = (olong)(ftestX - 0.5);
    if (ftestY>0.0) itestY = (olong)(ftestY + 0.5);
    else itestY = (olong)(ftestY - 0.5);
  
    /* Count bad cells */
    if ((fabs(ftestX-itestX)>0.1) || (fabs(ftestY-itestY)>0.1)) {
      badCnt++;
      /* Warn but keep going */
      if (badCnt<50) {
	Obit_log_error(err, OBIT_InfoWarn, "%s Warning: Bad cell %f %f in %s", 
		       routine, CCRow->DeltaX*iCellX, CCRow->DeltaY*iCellY, in->name);
      }
    }

    /* Clip range of X,Y */
    CCRow->DeltaX = MIN (maxX, MAX (CCRow->DeltaX, minX));
    CCRow->DeltaY = MIN (maxY, MAX (CCRow->DeltaY, minY));

    /* X,Y to cells */
    CCRow->DeltaX *= iCellX;
    CCRow->DeltaY *= iCellY;
    /* 0-rel pixel numbers */
    xPix = (olong)(CCRow->DeltaX + nx/2 + 0.5);
    yPix = (olong)(CCRow->DeltaY + ny/2 + 0.5);

    /* Spectral term parameter or tabulated */
    if (doSpec) {
      spectTerm = 1.0;
      if (iterm==1) spectTerm = CCRow->parms[1+parmoff];
      if (iterm==2) spectTerm = CCRow->parms[2+parmoff];
      if (iterm==3) spectTerm = CCRow->parms[3+parmoff];
    } else if (doTSpec) {
      spectTerm = CCRow->parms[iterm+parmoff] * factor;
    } else  spectTerm = 0.0;  /* Something went wrong */

    /* Sum into image */
    iAddr = xPix + nx * yPix;
    if (doSpec) {
      array[iAddr] += CCRow->Flux*spectTerm;
    } else if (doTSpec) {
     array[iAddr] += spectTerm;
    }

    count++;        /* how many */
  } /* end loop over components */

  /* How many? */
  *ncomp = count;
  
  /* Tell? */
  if ((iterm==1) && (err->prtLv>=2)) {
    Obit_log_error(err, OBIT_InfoErr, "Gridded %d components from %s", 
		   count, in->name);
  }

  /* Close Table */
  retCode = ObitTableCCClose (in, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, in->name, retCode);
  
  /* Release table/row */
  CCRow   = ObitTableCCRowUnref (CCRow);
  
  return retCode;
} /* end ObitTableCCUtilGridSpect */

/**
 * Return an ObitFArray containing the list of components in the CC table
 * from one image which appear in another.
 * Returned array has component values on the first axis and one row per
 * overlapping component.  (X cell (0-rel), Y cell (0-rel), flux, Z).  
 * CCs on cells within 0.5 pixels of outDesc are included.
 * Components in the same cell are combined.
 * If the components are Gaussians, their parameters are returned in gparm.
 * \param in         Table of CCs
 * \param inDesc     Descriptor for image from which components derived
 * \param outDesc    Descriptor for output image 
 * \param gparm      [out] Gaussian parameters (major, minor, PA (all deg)) 
 *                   if the components in in are Gaussians, else, -1.
 *                   These are the values from the first CC.
 * \param ncomp      [out] number of components in output list (generally less 
 *                   than size of FArray).
 * \param err        ObitErr error stack.
 * \return pointer to list of components, may be NULL on failure, 
 *  MUST be Unreffed.
 */
ObitFArray* 
ObitTableCCUtilCrossList (ObitTableCC *inCC, ObitImageDesc *inDesc,  
			  ObitImageDesc *outDesc, ofloat gparm[3], 
			  olong *ncomps, ObitErr *err)
{
  ObitFArray *outArray=NULL;
  ObitIOCode retCode;
  ObitTableCCRow *CCRow = NULL;
  olong i, count, number, nout, irow, lrec;
  olong ncomp;
  olong nrow, ndim, naxis[2], size, fsize, tsize;
  ofloat *table, inPixel[2], outPixel[2], *SortStruct = NULL;
  ofloat *entry;
  gboolean wanted, doSpec=TRUE, doTSpec=FALSE, doZ=FALSE;
  gchar *outName;
  ObitCCCompType modType;
  gchar *routine = "ObitTableCCUtilCrossList";
  
  /* error checks */
  if (err->error) return outArray;

  /* Open */
  retCode = ObitTableCCOpen (inCC, OBIT_IO_ReadOnly, err);
  /* If this fails try ReadWrite */
  if (err->error) { 
    ObitErrClearErr(err);  /* delete failure messages */
    retCode = ObitTableCCOpen (inCC, OBIT_IO_ReadWrite, err);
  }
  if ((retCode != OBIT_IO_OK) || (err->error))
      Obit_traceback_val (err, routine, inCC->name, outArray);

  /* Does the CC table have a DeltaZ column? */
  doZ = inCC->DeltaZCol>=0;

 /* Create sortStruct 
     element size */
  nrow = inCC->myDesc->nrow;
  /* Normal CCs or with spectra? */
  doSpec = (inCC->noParms>4);
  if (doSpec) fsize = 4;
  else fsize = 3;
  if (doZ) fsize++;  /* One for deltaZ */
  size = fsize * sizeof(ofloat);
  /*   Total size of structure in case all rows valid */
  tsize = size * (nrow+10);
  /* create output structure */
  SortStruct = ObitMemAlloc0Name (tsize, "CCSortStructure");

  /* Create table row */
  CCRow = newObitTableCCRow (inCC);

  /* Initialize */
  gparm[0] = gparm[1] = gparm[2] = -1.0;  /* No Gaussian yet */
  count = 0;         /* How many CCs accepted */
  /* If only 3 col, or parmsCol 0 size then this is a point model */
  if ((inCC->myDesc->nfield==3) || 
      (inCC->parmsCol<0) ||
      (inCC->myDesc->dim[inCC->parmsCol]<=0)) 
    modType = OBIT_CC_PointMod;
  else  
    modType = OBIT_CC_Unknown; /* Model type not yet known */

  /* Get spectrum type */
  doSpec  = (CCRow->parms[3]>=9.9)  && (CCRow->parms[3]<=19.0);
  doTSpec = (CCRow->parms[3]>=19.9) && (CCRow->parms[3]<=29.0);
  
  /* Loop over table reading CCs */
  for (i=1; i<=nrow; i++) {

    irow = i;
    retCode = ObitTableCCReadRow (inCC, irow, CCRow, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

    /* Get model type  */
    if (modType == OBIT_CC_Unknown) {
      modType = (olong)(CCRow->parms[3] + 0.5);
      /* If Gaussian take model */
      if ((modType==OBIT_CC_GaussMod)     || (modType==OBIT_CC_CGaussMod) ||
	  (modType==OBIT_CC_GaussModSpec) || (modType==OBIT_CC_CGaussModSpec)) {
	gparm[0] = CCRow->parms[0];
	gparm[1] = CCRow->parms[1];
	gparm[2] = CCRow->parms[2];
      }
      /* If neither a point nor Gaussian - barf */
      if ((modType!=OBIT_CC_GaussMod) && (modType!=OBIT_CC_CGaussMod) && 
	  (modType!=OBIT_CC_PointMod) && (modType!=OBIT_CC_GaussModSpec) && 
	  (modType!=OBIT_CC_CGaussModSpec) && (modType!=OBIT_CC_PointModSpec) &&
	  (modType!=OBIT_CC_GaussModTSpec) && (modType!=OBIT_CC_CGaussModTSpec) && 
	  (modType!=OBIT_CC_PointModTSpec)) {
	Obit_log_error(err, OBIT_Error,
		       "%s: Model type %d neither point nor Gaussian in %s",
		       routine, modType, inCC->name);
	goto cleanup;
      }
    } /* end model type checking */

    /* Is this one within 3 pixels of outDesc? */
    inPixel[0] = CCRow->DeltaX / inDesc->cdelt[0] + inDesc->crpix[0];
    inPixel[1] = CCRow->DeltaY / inDesc->cdelt[1] + inDesc->crpix[1];
    wanted = ObitImageDescCvtPixel (inDesc, outDesc, inPixel, outPixel, err);
    if (err->error) goto cleanup;

    if (wanted) { /* yes */
      /* add to structure */
      entry = (ofloat*)(SortStruct + count * fsize);  /* set pointer to entry */
      entry[0] = outPixel[0] - 1.0;  /* Make zero rel. pixels */
      entry[1] = outPixel[1] - 1.0;
      entry[2] = CCRow->Flux;
      if (doZ) entry[3] = CCRow->DeltaZ;
      count++; /* How many? */
    }
  } /* end loop over TableCC */

  /* Close */
  retCode = ObitTableCCClose (inCC, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Release table row */
  CCRow = ObitTableCCRowUnref (CCRow);

  /* Catch anything? */
  *ncomps = count;
  if (count<=0) goto cleanup;
    
  /* Sort */
  number = count; /* Total number of entries */
  ncomp  = 2;     /* number of values to compare */
  g_qsort_with_data (SortStruct, number, size, CCComparePos, &ncomp);

  /* Merge entries - Normal or with spectra? */
  doSpec = (inCC->noParms>4);
  if (doSpec || doTSpec) 
    CCMergeSpec (SortStruct, fsize, number, doSpec, doTSpec, doZ);
  else
    CCMerge (SortStruct, fsize, number);
  
  /* Sort to descending merged flux densities */
  ncomp = 1;
  g_qsort_with_data (SortStruct, number, size, CCCompareFlux, &ncomp);

  /* Count number of valid entries left */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {
    if (entry[0]>-1.0e19) count++;
    entry += fsize;  /* pointer in table */
  }

  /* Create FArray list large enough for merged CCs */
  ndim = 2; naxis[0] = 3; naxis[1] = count;
  if (doZ) naxis[0]++;   /* One for delta Z*/
  nout = naxis[1];
  lrec = naxis[0];   /* size of table row */
  nout = naxis[1];   /* Size of output array */
  outName =  g_strconcat ("CC List:", inCC->name, NULL);
  outArray = ObitFArrayCreate (outName, ndim, naxis);
  g_free (outName);  /* deallocate name */

  /* Get pointer to array */
  naxis[0] = naxis[1] = 0;
  table = ObitFArrayIndex (outArray, naxis);

  /* Copy structure to output array */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {

    /* Deleted? */
    if (entry[0]>-1.0e19) {
      /* Check that array not blown */
      if (count>nout) {
	Obit_log_error(err, OBIT_Error,"%s: Internal array overrun",
		       routine);
	goto cleanup;
      }
      /* copy to out */
      table[0] = entry[0];
      table[1] = entry[1];
      table[2] = entry[2];
      if (doZ) table[3] = entry[3];
      table += lrec;
      count++;
    } /* end of contains value */
    entry += fsize;  /* pointer in table */
  } /* end loop over array */
  
  /* How many? */
  *ncomps = count;

  /* Cleanup */
 cleanup:
  if (SortStruct) ObitMemFree(SortStruct);
  if (err->error) Obit_traceback_val (err, routine, inCC->name, outArray);

  return outArray;
} /*  end ObitTableCCUtilCrossList */

/**
 * Return an ObitFArray containing the list of spectral components in the 
 * CC table from one image which appear in another.
 * Returned array has component values on the first axis and one row per
 * overlapping component.  (X cell (0-rel), Y cell (0-rel), flux, spectral terms).  
 * CCs on cells within 0.5 pixels of outDesc are included.
 * Components in the same cell are combined.
 * If the components are Gaussians, their parameters are returned in gparm.
 * Works for both parameterized spectra (Param[3} 10-19) 
 * or tabulated spectra (Param[3} 20-29).
 * For tabulated spectra, selected channel components are summed and saved as 
 * element 4 in the output array.
 * For paramertized spectra, element 4 is flux weighted spectral term, except for 
 * iterm=0 in which it's the sum of the flux density .
 * \param in         Table of CCs
 * \param inDesc     Descriptor for image from which components derived
 * \param outDesc    Descriptor for output image 
 * \param gparm      [out] Gaussian parameters (major, minor, PA (all deg)) 
 *                   if the components in in are Gaussians, else, -1.
 *                   These are the values from the first CC.
 * \param ncomp      [out] number of components in output list (generally less 
 *                   than size of FArray).
 * \param iterm      Select spectral term, 0=flux, 1=si, 2=curvature...
 *                   For tabulated spectra this is the spectrum
 * \param err        ObitErr error stack.
 * \return pointer to list of components, may be NULL on failure, 
 *  MUST be Unreffed.
 */
ObitFArray* 
ObitTableCCUtilCrossListSpec (ObitTableCC *inCC, ObitImageDesc *inDesc,  
			      ObitImageDesc *outDesc, ofloat gparm[3], 
			      olong *ncomps, olong iterm, ObitErr *err)
{
  ObitFArray *outArray=NULL;
  ObitIOCode retCode;
  ObitTableCCRow *CCRow = NULL;
  olong i, count, number, nout, irow, lrec, toff;
  olong ncomp, parmoff=3;
  olong nrow, ndim, naxis[2], size, fsize, tsize;
  ofloat *table, inPixel[2], outPixel[2], *SortStruct = NULL;
  ofloat *entry, spectTerm;
  gboolean wanted;
  gboolean doSpec=TRUE, doTSpec=FALSE, doZ=FALSE;
  gchar *outName;
  ObitCCCompType modType;
  gchar *routine = "ObitTableCCUtilCrossListSpec";
  
  /* error checks */
  if (err->error) return outArray;

  /* Open */
  retCode = ObitTableCCOpen (inCC, OBIT_IO_ReadOnly, err);
  /* If this fails try ReadWrite */
  if (err->error) { 
    ObitErrClearErr(err);  /* delete failure messages */
    retCode = ObitTableCCOpen (inCC, OBIT_IO_ReadWrite, err);
  }
  if ((retCode != OBIT_IO_OK) || (err->error))
      Obit_traceback_val (err, routine, inCC->name, outArray);

  /* Does the CC table have a DeltaZ column? */
  doZ = inCC->DeltaZCol>=0;
  if (doZ) toff = 4;
  else     toff = 3;
  
 /* Create sortStruct 
     element size */
  nrow = inCC->myDesc->nrow;
  fsize = 4;
  if (doZ) fsize++;  /* One for deltaZ */
  size = fsize * sizeof(ofloat);
  /*   Total size of structure in case all rows valid */
  tsize = size * (nrow+10);
  /* create output structure */
  SortStruct = ObitMemAlloc0Name (tsize, "CCSortStructure");

  /* Create table row */
  CCRow = newObitTableCCRow (inCC);

  /* Initialize */
  gparm[0] = gparm[1] = gparm[2] = -1.0;  /* No Gaussian yet */
  count = 0;         /* How many CCs accepted */
  /* If only 3 col, or parmsCol 0 size then this is a point model */
  if ((inCC->myDesc->nfield==3) || 
      (inCC->parmsCol<0) ||
      (inCC->myDesc->dim[inCC->parmsCol]<=0)) 
    modType = OBIT_CC_PointMod;
  else  
    modType = OBIT_CC_Unknown; /* Model type not yet known */
  
  /* Loop over table reading CCs */
  for (i=1; i<=nrow; i++) {

    irow = i;
    retCode = ObitTableCCReadRow (inCC, irow, CCRow, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

    /* Get model type  */
    if (modType == OBIT_CC_Unknown) {
      modType = (olong)(CCRow->parms[3] + 0.5);
      /* Get spectrum type */
      doSpec  = (CCRow->parms[3]>=9.9)  && (CCRow->parms[3]<=19.0);
      doTSpec = (CCRow->parms[3]>=19.9) && (CCRow->parms[3]<=29.0);
      /* If Gaussian take model */
      if ((modType==OBIT_CC_GaussMod)      || (modType==OBIT_CC_CGaussMod) ||
	  (modType==OBIT_CC_GaussModSpec)  || (modType==OBIT_CC_CGaussModSpec) ||
	  (modType==OBIT_CC_GaussModTSpec) || (modType==OBIT_CC_CGaussModTSpec)) {
	gparm[0] = CCRow->parms[0];
	gparm[1] = CCRow->parms[1];
	gparm[2] = CCRow->parms[2];
      }
      /* If neither a point nor Gaussian - barf */
      if ((modType!=OBIT_CC_PointMod)       && (modType!=OBIT_CC_PointModSpec)  &&
	  (modType!=OBIT_CC_PointModTSpec)  &&
	  (modType!=OBIT_CC_CGaussMod)      && (modType!=OBIT_CC_GaussMod)      && 
	  (modType!=OBIT_CC_CGaussModSpec)  && (modType!=OBIT_CC_GaussModSpec)  && 
	  (modType!=OBIT_CC_CGaussModTSpec) && (modType!=OBIT_CC_GaussModTSpec)) {
	Obit_log_error(err, OBIT_Error,
		       "%s: Model type %d neither point nor Gaussian in %s",
		       routine, modType, inCC->name);
	goto cleanup;
      }
    } /* end model type checking */

    /* Is this one within 3 pixels of outDesc? */
    inPixel[0] = CCRow->DeltaX / inDesc->cdelt[0] + inDesc->crpix[0];
    inPixel[1] = CCRow->DeltaY / inDesc->cdelt[1] + inDesc->crpix[1];
    wanted = ObitImageDescCvtPixel (inDesc, outDesc, inPixel, outPixel, err);
    if (err->error) goto cleanup;

    if (wanted) { /* yes */
    /* Spectral term parameter or tabulated */
    if (doSpec) {
      spectTerm = 1.0;
      if (iterm==1) spectTerm = CCRow->parms[1+parmoff];
      if (iterm==2) spectTerm = CCRow->parms[2+parmoff];
      if (iterm==3) spectTerm = CCRow->parms[3+parmoff];
    } else if (doTSpec) {
      spectTerm = CCRow->parms[iterm+parmoff];
    } else  spectTerm = 0.0;  /* Something went wrong */

    /* add to structure */
    entry = (ofloat*)(SortStruct + count * fsize);  /* set pointer to entry */
    entry[0] = outPixel[0] - 1.0;  /* Make zero rel. pixels */
    entry[1] = outPixel[1] - 1.0;
    entry[2] = CCRow->Flux;
    if (doZ) entry[3] = CCRow->DeltaZ;
    entry[toff] = spectTerm;
    count++; /* How many? */
    }
  } /* end loop over TableCC */

  /* Close */
  retCode = ObitTableCCClose (inCC, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Release table row */
  CCRow = ObitTableCCRowUnref (CCRow);

  /* Catch anything? */
  *ncomps = count;
  if (count<=0) goto cleanup;
    
  /* Sort */
  number = count; /* Total number of entries */
  ncomp  = 2;     /* number of values to compare */
  g_qsort_with_data (SortStruct, number, size, CCComparePos, &ncomp);

  /* Merge entries */
  CCMergeSpec (SortStruct, fsize, number, doSpec, doTSpec, doZ);
  
  /* Sort to descending merged flux densities */
  ncomp = 1;
  g_qsort_with_data (SortStruct, number, size, CCCompareFlux, &ncomp);

  /* Count number of valid entries left */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {
    if (entry[0]>-1.0e19) count++;
    entry += fsize;  /* pointer in table */
  }

  /* Create FArray list large enough for merged CCs */
  ndim = 2; naxis[0] = fsize; naxis[1] = count;
  nout = naxis[1];
  lrec = naxis[0];   /* size of table row */
  nout = naxis[1];   /* Size of output array */
  outName =  g_strconcat ("CC List:", inCC->name, NULL);
  outArray = ObitFArrayCreate (outName, ndim, naxis);
  g_free (outName);  /* deallocate name */

  /* Get pointer to array */
  naxis[0] = naxis[1] = 0;
  table = ObitFArrayIndex (outArray, naxis);

  /* Copy structure to output array */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {

    /* Deleted? */
    if (entry[0]>-1.0e19) {
      /* Check that array not blown */
      if (count>nout) {
	Obit_log_error(err, OBIT_Error,"%s: Internal array overrun",
		       routine);
	goto cleanup;
      }
      /* copy to out */
      table[0] = entry[0];
      table[1] = entry[1];
      if (doSpec || doTSpec) table[2] = entry[toff];
      else                   table[2] = entry[2];
      table[3] = entry[3];
      if (doZ) table[4] = entry[4];
      table += lrec;
      count++;
    } /* end of contains value */
    entry += fsize;  /* pointer in table */
  } /* end loop over array */

  /* Flux weighted flux is sum of flux */
  if (iterm==0) {
    table = ObitFArrayIndex (outArray, naxis);
    for (i=0; i<count; i++) {
      table[3] = table[2];
      table += lrec;
    }
  } /* end of flux only */
  
  /* How many? */
  *ncomps = count;

  /* Cleanup */
 cleanup:
  if (SortStruct) ObitMemFree(SortStruct);
  if (err->error) Obit_traceback_val (err, routine, inCC->name, outArray);

  return outArray;
} /*  end ObitTableCCUtilCrossListSpec */

/**
 * Return an ObitTableCC with the components in the CC table
 * from one image which appear in another (outIm).
 * Returned Table is a new table attached to outIm.
 * CCs on cells within 0.5 pixels of outDesc are included.
 * \param in         Table of CCs
 * \param inDesc     Descriptor for image from which components derived
 * \param outImage   Output image 
 * \param ncomp      [out] number of components in output list (generally less 
 *                   than size of FArray).
 * \param err        ObitErr error stack.
 * \return pointer to new table, may be NULL on failure or no components in common, 
 *  MUST be Unreffed.
 */
ObitTableCC* 
ObitTableCCUtilCrossTable (ObitTableCC *inCC, ObitImageDesc *inDesc,  
			   ObitImage *outIm, olong *ncomps, 
			   ObitErr *err)
{
  ObitIOCode retCode;
  ObitTableCC *outCC=NULL;
  ObitTableCCRow *CCRow=NULL;
  olong i, count, irow, orow, nrow, ver;
  ofloat inPixel[2], outPixel[2], *parms=NULL;
  gboolean dummyParms=FALSE, wanted;
  gchar *routine = "ObitTableCCUtilCrossTable";
  
  /* error checks */
  *ncomps = 0;   /* In case */
  if (err->error) return outCC;

  /* Open */
  retCode = ObitTableCCOpen (inCC, OBIT_IO_ReadOnly, err);
  /* If this fails try ReadWrite */
  if (err->error) { 
    ObitErrClearErr(err);  /* delete failure messages */
    retCode = ObitTableCCOpen (inCC, OBIT_IO_ReadWrite, err);
  }
  if ((retCode != OBIT_IO_OK) || (err->error))
      Obit_traceback_val (err, routine, inCC->name, outCC);

  /* Anything? */
  nrow = inCC->myDesc->nrow;
  if (nrow<=0) {
   retCode = ObitTableCCClose (inCC, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) 
    Obit_traceback_val (err, routine, inCC->name, outCC);
  return outCC;
  }
  
  /* Create new output CC table */
  ver = 0 ;
  outCC = newObitTableCCValue ("SelectedCC", (ObitData*)outIm,
			       &ver, OBIT_IO_WriteOnly, inCC->noParms, 
			       err);
  if (err->error) goto cleanup;
  /* Open */
  retCode = ObitTableCCOpen (outCC, OBIT_IO_WriteOnly, err);
  /* If this fails try ReadWrite */
  if (err->error) { 
    ObitErrClearErr(err);  /* delete failure messages */
    retCode = ObitTableCCOpen (outCC, OBIT_IO_ReadWrite, err);
  }
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

  /* Create table row */
  CCRow = newObitTableCCRow (outCC);
  ObitTableCCSetRow (outCC, CCRow, err);
  if (err->error) goto cleanup;

  /* Dummy output parms if needed */
  dummyParms = ((inCC->parmsCol<0) && (outCC->parmsCol>0));
  if (dummyParms) parms = g_malloc0(outCC->myDesc->repeat[outCC->parmsCol]*sizeof(ofloat));
  
  /* Initialize */
  count = 0;         /* How many CCs accepted */
  /* Loop over table reading CCs */
  for (i=1; i<=nrow; i++) {

    irow = i;
    retCode = ObitTableCCReadRow (inCC, irow, CCRow, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
    if (CCRow->status<0) continue;  /* Skip deselected record */

    /* Is this one within outIm? */
    inPixel[0] = CCRow->DeltaX / inDesc->cdelt[0] + inDesc->crpix[0];
    inPixel[1] = CCRow->DeltaY / inDesc->cdelt[1] + inDesc->crpix[1];
    wanted = ObitImageDescCvtPixel (inDesc, outIm->myDesc, inPixel, outPixel, err);
    if (err->error) goto cleanup;

    if (wanted) { /* yes - write */
      if (err->error) goto cleanup;
      /* Dummy parms? */
      if (dummyParms) CCRow->parms = parms;
      orow = -1;
      retCode = ObitTableCCWriteRow (outCC, orow, CCRow, err);
      if (err->error) goto cleanup;
      count++; /* How many? */
    }
  } /* end loop over TableCC */

  /* Release table row */
  CCRow = ObitTableCCRowUnref (CCRow);
  
  /* How many? */
  *ncomps = count;

  /* Cleanup */
 cleanup:
  retCode = ObitTableCCClose (inCC, err); /* Close */
  retCode = ObitTableCCClose (outCC, err); /* Close */
  if (parms) g_free(parms);
  if (err->error) Obit_traceback_val (err, routine, inCC->name, outCC);

  return outCC;
} /*  end ObitTableCCUtilCrossTable */

/**
 * Merge elements of an ObitTableCC on the same position.
 * First sorts table, collapses, sorts to desc. flux
 * \param in      Table to sort
 * \param out     Table to write output to
 * \param err     ObitErr error stack.
 * \return I/O Code  OBIT_IO_OK = OK.
 */
ObitIOCode ObitTableCCUtilMerge (ObitTableCC *in, ObitTableCC *out, 
				 ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  olong i, size, fsize, number=0, ncomp;
  ofloat parms[20];
  ofloat *SortStruct = NULL;
  gboolean doSpec=TRUE, doTSpec=FALSE, doZ=FALSE;
  gchar *routine = "ObitTableCCUtilMerge";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return retCode;
  g_assert (ObitTableCCIsA(in));

  /* Open table */
  retCode = ObitTableCCOpen (in, OBIT_IO_ReadOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, in->name, retCode);
  
  /* Must be something in the table else just return */
  if (in->myDesc->nrow<=0) {
    retCode = ObitTableCCClose (in, err);
    if ((retCode != OBIT_IO_OK) || (err->error))
      Obit_traceback_val (err, routine, in->name, retCode);
    return OBIT_IO_OK;
  }

  /* Does the CC table have a DeltaZ column? */
  doZ = in->DeltaZCol>=0;
 
  /* build sort structure from table */
  for (i=0; i<20; i++) parms[i] = 0.0;
  SortStruct = MakeCCSortStruct (in, &size, &number, &ncomp, parms, err);
  if (err->error) goto cleanup;

  /* Close table */
  retCode = ObitTableCCClose (in, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, in->name, retCode);

  /* Sort */
  g_qsort_with_data (SortStruct, number, size, CCComparePos, &ncomp);

  /* Get spectrum type */
  doSpec  = (parms[3]>=9.9)  && (parms[3]<=19.0);
  doTSpec = (parms[3]>=19.9) && (parms[3]<=29.0);

  /* Merge entries - Normal or with spectra? */
  fsize = size/sizeof(ofloat);
  if (doSpec || doTSpec) 
    CCMergeSpec (SortStruct, fsize, number, doSpec, doTSpec, doZ);
  else
    CCMerge (SortStruct, fsize, number);
  
  /* Sort to descending merged flux densities */
  ncomp = 1;
  g_qsort_with_data (SortStruct, number, size, CCCompareFlux, &ncomp);

  /* Clone output table from input */
  out = (ObitTableCC*)ObitTableClone ((ObitTable*)in, (ObitTable*)out);

  /* Write output table */
  retCode = ReWriteTable (out, SortStruct, fsize, number, parms, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Cleanup */
 cleanup:
  if (SortStruct) ObitMemFree(SortStruct);
  if (err->error) Obit_traceback_val (err, routine, in->name, retCode);

  return OBIT_IO_OK;
} /* end ObitTableCCUtilMerge */

/**
 * Merge elements of an ObitTableCC on the same position.
 * with selection by row number.
 * First sorts table, collapses, sorts to desc. flux
 * \param in        Table to sort
 * \param startComp First component to select 
 * \param endComp   Last component to select, 0=> all
 * \param parms     [out] Non-point parameters (MUST all be the same ) dimen. at least 4.
 *                  parms[3] = type
 *                  0 => point, no parameters
 *                  1 = Sky Gaussian, [0:3]=maj, min, PA
 *                  2 = Convolved Gaussian, [0:3]=maj axis, min axis, PA (all deg)
 *                  3 = Uniform Sphere [0] = radius (deg)
 *                  4 = Exponential disk [0] = scale length (deg)
 * \param err       ObitErr error stack.
 * \return FArray containing merged CC table contents; MUST be Unreffed.
 *                Will contain flux, X, Y, + any spectral terms
 * \li Flux
 * \li Delta X
 * \li Delta Y
 */
ObitFArray* ObitTableCCUtilMergeSel (ObitTableCC *in, olong startComp, 
				     olong endComp, ofloat *parms, 
				     ObitErr *err)
{
  ObitFArray *out = NULL;
  olong i, j, count, lout, nout, ndim, naxis[2];
  ObitIOCode retCode;
  olong size, fsize, number=0, ncomp, nterms, toff;
  ofloat lparms[20];
  ofloat *entry, *outArray, *SortStruct = NULL;
  gboolean doSpec=TRUE, doTSpec=FALSE, doZ=FALSE;
  gchar *routine = "ObitTableCCUtilMergeSel";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return out;
  g_assert (ObitTableCCIsA(in));

  /* Open table */
  retCode = ObitTableCCOpen (in, OBIT_IO_ReadOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Does the CC table have a DeltaZ column? */
  doZ = in->DeltaZCol>=0;
  if (doZ) {lout = 4; toff = 4;}
  else     {lout = 3; toff = 3;}

  /* Must be something in the table else just return */
  if (in->myDesc->nrow<=0) {
    retCode = ObitTableCCClose (in, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
    return NULL;
  }

  /* Check range of components */
  startComp = MAX (1, startComp);
  endComp   = MIN (in->myDesc->nrow, endComp);

  /* build sort structure from table */
  for (i=0; i<20; i++) lparms[i] = 0;
  SortStruct = MakeCCSortStructSel (in, startComp, endComp, 
				    &size, &number, &ncomp, lparms, 
				    err);
  if (err->error) goto cleanup;

  /* Close table */
  retCode = ObitTableCCClose (in, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

  /* Sort */
  g_qsort_with_data (SortStruct, number, size, CCComparePos, &ncomp);

  /* Get spectrum type */
  doSpec  = (lparms[3]>=9.9)  && (lparms[3]<=19.0);
  doTSpec = (lparms[3]>=19.9) && (lparms[3]<=29.0);

  /* Merge entries */
  fsize = size/sizeof(ofloat);
  if (doSpec || doTSpec) 
    CCMergeSpec (SortStruct, fsize, number, doSpec, doTSpec, doZ);
  else
    CCMerge (SortStruct, fsize, number);
  
  /* Sort to descending merged flux densities */
  ncomp = 1;
  g_qsort_with_data (SortStruct, number, size, CCCompareFlux, &ncomp);

  /* Count number of valid entries left */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {
    if (entry[0]>-1.0e19) count++;
    entry += fsize;  /* pointer in table */
  }


  /* Create output Array */
  /* Need room for spectral terms? */
  if (in->noParms>4)  nterms = in->noParms-4;
  else nterms = 0;
  if (in->noParms>4) lout += nterms;

  ndim = 2; naxis[0] = lout; naxis[1] = count;
  nout = count;
  out      = ObitFArrayCreate ("MergedCC", ndim, naxis);
  naxis[0] = naxis[1] = 0;
  outArray = ObitFArrayIndex(out, naxis);

  /* Any model parameters */
  for (i=0; i<4; i++) parms[i] = 0.0;
  for (i=0; i<MIN (4,in->noParms); i++) parms[i] = lparms[i];

  /* Copy structure to output array */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {

    /* Deleted? */
    if (entry[0]>-1.0e19) {
      /* Check that array not blown */
      if (count>nout) {
	Obit_log_error(err, OBIT_Error,"%s: Internal array overrun",
		       routine);
	goto cleanup;
      }
      /* copy to out */
      outArray[0] = entry[2];
      outArray[1] = entry[0];
      outArray[2] = entry[1];
      if (doZ) outArray[3] = entry[3];  /* DeltaZ */
      for (j=0; j<nterms; j++) outArray[toff+j] = entry[toff+j];
      outArray += lout;
      count++;
    } /* end of contains value */
    entry += fsize;  /* pointer in table */
  } /* end loop over array */
  
  /* Cleanup */
 cleanup:
  if (SortStruct) ObitMemFree(SortStruct);
  if (err->error) Obit_traceback_val (err, routine, in->name, out);

  return out;
} /* end ObitTableCCUtilMergeSel */

/**
 * Merge elements of an ObitTableCC on the same position.
 * with selection by row number.
 * First sorts table, collapses, sorts to desc. flux
 * \param in        Table to sort
 * \param startComp First component to select 
 * \param endComp   Last component to select, 0=> all
 * \param parms     [out] Type of CC entries
 * \param err       ObitErr error stack.
 * \return FArray containing merged CC table contents; MUST be Unreffed.
 *                Will contain flux, X, Y, + any spectral terms
 * \li Flux
 * \li Delta X
 * \li Delta Y
 * \li model 1 (major axis FWHM deg)
 * \li model 2 (minor axis FWHM deg)
 * \li model 3 (PA deg)
 * \li ... spectralcomponents
 */
ObitFArray* ObitTableCCUtilMergeSel2 (ObitTableCC *in, olong startComp, 
				     olong endComp, ObitSkyModelCompType *type,
				     ObitErr *err)
{
  ObitFArray *out = NULL;
  olong i, j, count, lout, nout, ndim, off, naxis[2];
  ObitIOCode retCode;
  olong size, fsize, number=0, ncomp, nterms, toff;
  ofloat lparms[20];
  ofloat *entry, *outArray, *SortStruct = NULL;
  gboolean doSpec=TRUE, doTSpec=FALSE, doZ=FALSE;
  gchar *routine = "ObitTableCCUtilMergeSel";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return out;
  g_assert (ObitTableCCIsA(in));

  /* Open table */
  retCode = ObitTableCCOpen (in, OBIT_IO_ReadOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Does the CC table have a DeltaZ column? */
  doZ = in->DeltaZCol>=0;
  if (doZ) {lout = 7; toff = 7;}
  else     {lout = 6; toff = 6;}

  /* Must be something in the table else just return */
  if (in->myDesc->nrow<=0) {
    retCode = ObitTableCCClose (in, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
    return NULL;
  }

  /* Check range of components */
  startComp = MAX (1, startComp);
  endComp   = MIN (in->myDesc->nrow, endComp);

  /* build sort structure from table */
  for (i=0; i<20; i++) lparms[i] = 0;
  SortStruct = MakeCCSortStructSel2 (in, startComp, endComp, 
				    &size, &number, &ncomp, type, 
				    err);
  if (err->error) goto cleanup;

  /* Close table */
  retCode = ObitTableCCClose (in, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

  /* Sort */
  g_qsort_with_data (SortStruct, number, size, CCComparePos, &ncomp);

  /* Get spectrum type */
  doSpec  = (lparms[3]>=9.9)  && (lparms[3]<=19.0);
  doTSpec = (lparms[3]>=19.9) && (lparms[3]<=29.0);

  /* Merge entries */
  fsize = size/sizeof(ofloat);
  if (doSpec || doTSpec) 
    CCMergeSpec2 (SortStruct, fsize, number, doSpec, doTSpec, doZ);
  else
    CCMerge (SortStruct, fsize, number);
  
  /* Sort to descending merged flux densities */
  ncomp = 1;
  g_qsort_with_data (SortStruct, number, size, CCCompareFlux, &ncomp);

  /* Count number of valid entries left */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {
    if (entry[0]>-1.0e19) count++;
    entry += fsize;  /* pointer in table */
  }

  /* Create output Array */
  /* Need room for spectral terms? */
  if (in->noParms>4)  nterms = in->noParms-4;
  else nterms = 0;
  if (in->noParms>4) lout += nterms;

  ndim = 2; naxis[0] = lout; naxis[1] = count;
  nout = count;
  out      = ObitFArrayCreate ("MergedCC", ndim, naxis);
  naxis[0] = naxis[1] = 0;
  outArray = ObitFArrayIndex(out, naxis);

  /* Copy structure to output array */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {

    /* Deleted? */
    if (entry[0]>-1.0e19) {
      /* Check that array not blown */
      if (count>nout) {
	Obit_log_error(err, OBIT_Error,"%s: Internal array overrun",
		       routine);
	goto cleanup;
      }
      /* copy to out */
      outArray[0] = entry[2];
      outArray[1] = entry[0];
      outArray[2] = entry[1]; off = 2;
      if (doZ) {outArray[3] = entry[3]; off++;} /* DeltaZ */
      outArray[off+1] = entry[off+1];  /* Gaussian comps */
      outArray[off+2] = entry[off+2];
      outArray[off+3] = entry[off+3];
      for (j=0; j<nterms; j++) outArray[toff+j] = entry[toff+j];
      outArray += lout;
      count++;
    } /* end of contains value */
    entry += fsize;  /* pointer in table */
  } /* end loop over array */
  
  /* Cleanup */
 cleanup:
  if (SortStruct) ObitMemFree(SortStruct);
  if (err->error) Obit_traceback_val (err, routine, in->name, out);

  return out;
} /* end ObitTableCCUtilMergeSel2 */


/**
 * Merge spectral elements of an ObitTableCC on the same position.
 * with selection by row number.
 * Spectral components are flux weighted average
 * First sorts table, collapses, sorts to desc. flux
 * \param in        Table to sort
 * \param startComp First component to select 
 * \param endComp   Last component to select, 0=> all
 * \param parms     [out] Non-point parameters (MUST all be the same ) dimen. at least 4.
 *                  parms[3] = type
 *                  0 => point, no parameters
 *                  1 = Sky Gaussian, [0:3]=maj, min, PA
 *                  2 = Convolved Gaussian, [0:3]=maj axis, min axis, PA (all deg)
 *                  3 = Uniform Sphere [0] = radius (deg)
 * \param err       ObitErr error stack.
 * \return FArray containing merged CC table contents; MUST be Unreffed.
 *                Will contain flux, X, Y, + any spectral terms
 * \li Flux
 * \li Delta X
 * \li Delta Y
 * \li Parms if [3]>=10 [4-?] are spectral components
 */
ObitFArray* ObitTableCCUtilMergeSelSpec (ObitTableCC *in, olong startComp, 
					  olong endComp, ofloat *parms, 
					  ObitErr *err)
{
  ObitFArray *out = NULL;
  olong i, j, count, lout, nout, toff, ndim, naxis[2];
  ObitIOCode retCode;
  olong size, fsize, number=0, ncomp, nterms;
  ofloat lparms[20];
  gboolean doSpec=TRUE, doTSpec=FALSE, doZ=FALSE;
  ofloat *entry, *outArray, *SortStruct = NULL;
  gchar *routine = "ObitTableCCUtilMergeSelSpec";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return out;
  g_assert (ObitTableCCIsA(in));

  /* Open table */
  retCode = ObitTableCCOpen (in, OBIT_IO_ReadOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Must be something in the table else just return */
  if (in->myDesc->nrow<=0) {
    retCode = ObitTableCCClose (in, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
    return out;
  }

  /* Check range of components */
  startComp = MAX (1, startComp);
  endComp   = MIN (in->myDesc->nrow, endComp);

  /* Does the CC table have a DeltaZ column? */
  doZ = in->DeltaZCol>=0;
  
  /* build sort structure from table */
  for (i=0; i<20; i++) lparms[i] = 0;
  SortStruct = MakeCCSortStructSel (in, startComp, endComp, 
				    &size, &number, &ncomp, lparms, 
				    err);
  if (err->error) goto cleanup;

  /* Close table */
  retCode = ObitTableCCClose (in, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

  /* Sort */
  g_qsort_with_data (SortStruct, number, size, CCComparePos, &ncomp);

  /* Get spectrum type */
  doSpec  = (lparms[3]>=9.9)  && (lparms[3]<=19.0);
  doTSpec = (lparms[3]>=19.9) && (lparms[3]<=29.0);

  /* Merge entries */
  fsize = size/sizeof(ofloat);
  CCMergeSpec (SortStruct, fsize, number, doSpec, doTSpec, doZ);
  
  /* Sort to descending merged flux densities */
  ncomp = 1;
  g_qsort_with_data (SortStruct, number, size, CCCompareFlux, &ncomp);

  /* Count number of valid entries left */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {
    if (entry[0]>-1.0e19) count++;
    entry += fsize;  /* pointer in table */
  }

  /* Create output Array */
  if (doZ) {lout = 4; toff = 4;}
  else     {lout = 3; toff = 3;}

  /* Need room for spectral terms? */
  if (in->noParms>4)  nterms = in->noParms-4;
  else nterms = 0;
  if (in->noParms>4) lout += nterms;

  ndim = 2; naxis[0] = lout; naxis[1] = count;
  nout = count;
  out      = ObitFArrayCreate ("MergedCC", ndim, naxis);
  naxis[0] = naxis[1] = 0;
  outArray = ObitFArrayIndex(out, naxis);

  /* Any model parameters */
  for (i=0; i<4; i++) parms[i] = 0.0;
  for (i=0; i<MIN (4,in->noParms); i++) parms[i] = lparms[i];

  /* Copy structure to output array */
  entry = SortStruct;
  count = 0;
  for (i=0; i<number; i++) {

    /* Deleted? */
    if (entry[0]>-1.0e19) {
      /* Check that array not blown */
      if (count>nout) {
	Obit_log_error(err, OBIT_Error,"%s: Internal array overrun",
		       routine);
	goto cleanup;
      }
      /* copy to out */
      outArray[0] = entry[2];
      outArray[1] = entry[0];
      outArray[2] = entry[1];
      if (doZ) outArray[3] = entry[3];  /* DeltaZ */
      for (j=0; j<nterms; j++) outArray[toff+j] = entry[toff+j];
      outArray += lout;
      count++;
    } /* end of contains value */
    entry += fsize;  /* pointer in table */
  } /* end loop over array */
  
  /* Cleanup */
 cleanup:
  if (SortStruct) ObitMemFree(SortStruct);
  if (err->error) Obit_traceback_val (err, routine, in->name, out);

  return out;
} /* end ObitTableCCUtilMergeSelSpec */


/**
 * Merge selected entries in a CC Table then write values with 
 * absolute values of the flux densities into an output table
 * \param image     input image with input CC table
 * \param inCCver   input CC table
 * \param outCCver  Desired output CC table on image, if 0 then new
 *                  value used returned.
 * \param startComp First component to select 
 * \param endComp   Last component to select, 0=> all
 * \param range     Max and min abs value of flux densities to accept
 * \param err       ObitErr error stack.
 * \return Merged CC table, or NULL if empty.
 */
ObitTableCC* 
ObitTableCCUtilMergeSel2Tab (ObitImage *image, olong inCCver, olong *outCCver,
			     olong startComp, olong endComp, 
			     ofloat range[2], ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  ObitFArray *Merged=NULL;
  ObitTable *tempTable=NULL;
  ObitTableCC *inCCTable = NULL, *outCCTable = NULL;
  ObitTableCCRow *CCRow = NULL;
  gchar *tabType = "AIPS CC";
  olong naxis[2], warray, larray, i, j, ver, orow, offset, toff, nSpec=0;
  ofloat *array, parms[20];
  gboolean doSpec=FALSE, doTSpec=FALSE, doZ=FALSE;
  gchar *routine = "bitTableCCUtilMergeSel2Tab";

   /* error checks */
  if (err->error) return outCCTable;
  g_assert (ObitImageIsA(image));

  /* Get input CC table */
  ver = inCCver;
  tempTable = newObitImageTable (image,OBIT_IO_ReadOnly, tabType, &ver, err);
  if ((tempTable==NULL) || (err->error)) 
     Obit_traceback_val (err, routine, image->name, outCCTable);
  inCCTable = ObitTableCCConvert(tempTable);
  tempTable = ObitTableUnref(tempTable);
  if (err->error) Obit_traceback_val (err, routine, image->name, outCCTable);
  
  /* Open input */
  retCode = ObitTableCCOpen (inCCTable, OBIT_IO_ReadOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, image->name, outCCTable);

  /* Select and merge input table */
  Merged = ObitTableCCUtilMergeSel (inCCTable, startComp, endComp, parms, err);
  if (err->error) goto cleanup;

  /* Anything there? */
  if (Merged==NULL) return NULL;

  naxis[0] = 0; naxis[1]=0; 
  array = ObitFArrayIndex(Merged, naxis);
  warray = Merged->naxis[0];
  larray = Merged->naxis[1];
  
  /* Get spectrum type */
  if (inCCTable->noParms>4) {
    doSpec  = (parms[3]>=9.9)  && (parms[3]<=19.0);
    doTSpec = (parms[3]>=19.9) && (parms[3]<=29.0);
    nSpec = inCCTable->noParms - 4;
  }
  
  /* Create output CC table */
  ver = *outCCver;
  outCCTable = newObitTableCCValue ("Merged/selected", (ObitData*)image,
				    &ver, OBIT_IO_WriteOnly, inCCTable->noParms, 
				    err);
  if (err->error) goto cleanup;
  *outCCver = ver;  /* save if defaulted (0) */
      
  /* Open output */
  retCode = ObitTableCCOpen (outCCTable, OBIT_IO_ReadWrite, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

  /* Create table row */
  CCRow = newObitTableCCRow (outCCTable);

   /* Does the CC table have a DeltaZ column? */
  doZ = inCCTable->DeltaZCol>=0;
  if (doZ) toff = 4;
  else     toff = 3;
 /* Copy Parms to row */
  for (j=0; j<MIN(4,inCCTable->noParms); j++) CCRow->parms[j] = parms[j];
  
  offset = 4;
  /* loop over table */
  for (j=1; j<=larray; j++) {
    /* Want this one? */
    if ((fabs(array[0])>=range[0]) && (fabs(array[0])<=range[1])) {
      /* Copy row data */
      CCRow->Flux   = array[0];
      CCRow->DeltaX = array[1];
      CCRow->DeltaY = array[2];
      if (doZ) CCRow->DeltaZ =  array[3];
      /* Copy any spectra */
      if (doSpec) {
	for (i=0; i<nSpec; i++) CCRow->parms[i+offset] = array[toff+i];
      } else if (doTSpec) {
	for (i=0; i<nSpec; i++) CCRow->parms[i+offset] = array[toff+i];
      }
      
      /* Write output */
      orow = -1;
      retCode = ObitTableCCWriteRow (outCCTable, orow, CCRow, err);
    }
    if  (err->error) goto cleanup;
    
    /* Update */
    array += warray;
  } /* end loop over table */

  /* Close/cleanup */
 cleanup:
  retCode   = ObitTableCCClose (outCCTable, err);
  inCCTable = ObitTableUnref(inCCTable);
  CCRow     = ObitTableRowUnref(CCRow);
  Merged    = ObitFArrayUnref(Merged);
  if  (err->error) Obit_traceback_val (err, routine, image->name, outCCTable);

 return outCCTable;
} /* end ObitTableCCUtilMergeSel2Tab */

/**
 * Scale flux densities of  elements of an ObitTableCC.
 * \param in        Table to scale
 * \param startComp First component to select 
 * \param endComp   Last component to select, 0=> all
 * \param scale     Factor to multiply times flux densities
 * \param err       ObitErr error stack.
 */
void ObitTableCCUtilScale (ObitTableCC *in, olong startComp, 
			   olong endComp, ofloat scale, ObitErr *err)
{
  ObitTableCCRow *row=NULL;
  olong irow;
  ObitIOCode retCode;
  gchar *routine = "ObitTableCCUtilScale";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return;
  g_assert (ObitTableCCIsA(in));

  /* Open table */
  retCode = ObitTableCCOpen (in, OBIT_IO_ReadWrite, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Must be something in the table else just return */
  if (in->myDesc->nrow<=0) {
    retCode = ObitTableCCClose (in, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
    return;
  }

  /* Check range of components */
  startComp = MAX (1, startComp);
  endComp   = MIN (in->myDesc->nrow, endComp);
  if (endComp<=0) endComp = in->myDesc->nrow;

  if (err->error) goto cleanup;

  /* Create table row */
  row = newObitTableCCRow (in);

  /* Loop over table */
  for (irow=startComp; irow<=endComp; irow++) {
    retCode = ObitTableCCReadRow (in, irow, row, err);
    if (row->status<0) continue;  /* Skip deselected record */
    if (err->error) goto cleanup;
    row->Flux *= scale;
    retCode = ObitTableCCWriteRow (in, irow, row, err);
    if (err->error) goto cleanup;
  } /* End loop over table */

  /* Close table */
  retCode = ObitTableCCClose (in, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

  /* Cleanup */
 cleanup:
  row = ObitTableCCRowUnref (row); /* Release table row */
  if (err->error) Obit_traceback_msg (err, routine, in->name);
} /* end ObitTableCCUtilScale */

/**
 * Append the selected components of one table onto another
 * \param inCC      Table to copy from
 * \param outCC     Table to copy to
 * \param startComp First component to select , 0=>1
 * \param endComp   Last component to select, 0=> all
 * \param err       ObitErr error stack.
 */
void ObitTableCCUtilAppend  (ObitTableCC *inCC, ObitTableCC *outCC, 
			     olong startComp, olong endComp, ObitErr *err)
{
  ObitTableCCRow *row=NULL;
  olong irow, orow;
  ObitIOCode retCode;
  gboolean dummyParms=FALSE;
  ofloat *parms=NULL;
  gchar *routine = "ObitTableCCUtilAppend";

  /* error checks */
  if (err->error) return;
  g_assert (ObitTableCCIsA(inCC));
  g_assert (ObitTableCCIsA(outCC));

  /* Open input table */
  retCode = ObitTableCCOpen (inCC, OBIT_IO_ReadOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Must be something in the table else just return */
  if (inCC->myDesc->nrow<=0) {
    retCode = ObitTableCCClose (inCC, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
    return;
  }

  /* Open output table */
  retCode = ObitTableCCOpen (outCC, OBIT_IO_ReadWrite, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

   /* Dummy output parms if needed */
  dummyParms = ((inCC->parmsCol<0) && (outCC->parmsCol>0));
  if (dummyParms) parms = g_malloc0(outCC->myDesc->repeat[outCC->parmsCol]*sizeof(ofloat));

  /* Check range of components */
  startComp = MAX (1, startComp);
  endComp   = MIN (inCC->myDesc->nrow, endComp);
  if (endComp<=0) endComp = inCC->myDesc->nrow;

  /* Create table row */
  row = newObitTableCCRow (outCC);
  ObitTableCCSetRow (outCC, row, err);
  if (err->error) goto cleanup;

  /* Loop over table */
  for (irow=startComp; irow<=endComp; irow++) {
    retCode = ObitTableCCReadRow (inCC, irow, row, err);
    if (row->status<0) continue;  /* Skip deselected record */
    if (err->error) goto cleanup;
    /* Dummy parms? */
    if (dummyParms) row->parms = parms;
    orow = -1;
    retCode = ObitTableCCWriteRow (outCC, orow, row, err);
    if (err->error) goto cleanup;
  } /* End loop over table */

  /* Close tables */
  retCode = ObitTableCCClose (inCC, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  retCode = ObitTableCCClose (outCC, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

  /* Cleanup */
 cleanup:
  if (parms) g_free(parms);
  row = ObitTableCCRowUnref (row); /* Release table row */
  if (err->error) Obit_traceback_msg (err, routine, inCC->name);
} /* end ObitTableCCUtilAppend */

/*
 * Append the selected components of one table onto another
 * \param inCC      Table to copy from
 * \param outCC     Table to copy to
 * \param uvDesc    UVData descriptor fro data from which the image was made.
 * \param imDesc    Output Image descriptor
 * \param startComp First component to select , 0=>1
 * \param endComp   Last component to select, 0=> all
 * \param err       ObitErr error stack.
 */
void ObitTableCCUtilAppendShift (ObitTableCC *inCC, ObitTableCC *outCC, 
				 ObitUVDesc *uvDesc, ObitImageDesc *imDesc, 
				 olong startComp, olong endComp, ObitErr *err)
{
  ObitTableCCRow *row=NULL;
  olong irow, orow;
  ObitIOCode retCode;
  gboolean dummyParms=FALSE, do3Dmul;
  ofloat xxoff, yyoff, zzoff, *parms=NULL, umat[3][3], pmat[3][3], konst;
  ofloat dxyzc[3], maprot, uvrot, ccrot, ssrot, xpoff, ypoff, xyz[3], xp[3];
  gchar *routine = "ObitTableCCUtilAppendShift";

  /* error checks */
  if (err->error) return;
  g_assert (ObitTableCCIsA(inCC));
  g_assert (ObitTableCCIsA(outCC));

  /* Open input table */
  retCode = ObitTableCCOpen (inCC, OBIT_IO_ReadOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Must be something in the table else just return */
  if (inCC->myDesc->nrow<=0) {
    retCode = ObitTableCCClose (inCC, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
    return;
  }

  /* Open output table */
  retCode = ObitTableCCOpen (outCC, OBIT_IO_ReadWrite, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

   /* Dummy output parms if needed */
  dummyParms = ((inCC->parmsCol<0) && (outCC->parmsCol>0));
  if (dummyParms) parms = g_malloc0(outCC->myDesc->repeat[outCC->parmsCol]*sizeof(ofloat));

  /* Check range of components */
  startComp = MAX (1, startComp);
  endComp   = MIN (inCC->myDesc->nrow, endComp);
  if (endComp<=0) endComp = inCC->myDesc->nrow;

  /* Create table row */
  row = newObitTableCCRow (outCC);
  ObitTableCCSetRow (outCC, row, err);
  if (err->error) goto cleanup;

  /* Shift parameters -  rotation */
  maprot = ObitImageDescRotate(imDesc);
  uvrot  = ObitUVDescRotate(uvDesc);
  ssrot = sin (DG2RAD * (uvrot - maprot));
  ccrot = cos (DG2RAD * (uvrot - maprot));

  /* projection rotation matrix if needed */
  do3Dmul = ObitUVDescShift3DMatrix (uvDesc, imDesc, umat, pmat);
  
  /* Phase shift */
  ObitUVDescShiftPhase(uvDesc, imDesc, dxyzc, err);
  if (err->error) Obit_traceback_msg (err, routine, inCC->name);
 
  /* rotate phase shift */
  xxoff = dxyzc[0] * ccrot + dxyzc[1] * ssrot;
  yyoff = dxyzc[1] * ccrot - dxyzc[0] * ssrot;
  zzoff = dxyzc[2];
  /* undo factor to be applied later */
  konst = DG2RAD * 2.0 * G_PI;
  xxoff /= konst;
  yyoff /= konst;
  zzoff /= konst;

  /* Pixel offset */
  xpoff = imDesc->xPxOff * imDesc->cdelt[imDesc->jlocr];
  ypoff = imDesc->yPxOff * imDesc->cdelt[imDesc->jlocd];

  /* Loop over table */
  for (irow=startComp; irow<=endComp; irow++) {
    retCode = ObitTableCCReadRow (inCC, irow, row, err);
    if (row->status<0) continue;  /* Skip deselected record */
    if (err->error) goto cleanup;
    /* Dummy parms? */
    if (dummyParms) row->parms = parms;
    /* Apply position shift */
    xp[0]  = row->DeltaX + xpoff;
    xp[1]  = row->DeltaY + ypoff;
    xp[2]  = 0.0;
    if (do3Dmul) {
      xyz[0] = xp[0]*umat[0][0] + xp[1]*umat[1][0];
      xyz[1] = xp[0]*umat[0][1] + xp[1]*umat[1][1];
      xyz[2] = xp[0]*umat[0][2] + xp[1]*umat[1][2];
      /* PRJMUL (2, XP, UMAT, XYZ); */
    } else {  /* no rotn matrix */
      xyz[0] = ccrot * xp[0] + ssrot * xp[1];
      xyz[1] = ccrot * xp[1] - ssrot * xp[0];
      xyz[2] = 0.0;
    }
    /* May need to undo any field rotation */
    row->DeltaX = xyz[0] + xxoff;
    row->DeltaY = xyz[1] + yyoff;
    if (inCC->DeltaZCol>=0) row->DeltaZ = xyz[2] +  zzoff;
    orow = -1;
    retCode = ObitTableCCWriteRow (outCC, orow, row, err);
    if (err->error) goto cleanup;
  } /* End loop over table */

  /* Close tables */
  retCode = ObitTableCCClose (inCC, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  retCode = ObitTableCCClose (outCC, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;

  /* Cleanup */
 cleanup:
  if (parms) g_free(parms);
  row = ObitTableCCRowUnref (row); /* Release table row */
  if (err->error) Obit_traceback_msg (err, routine, inCC->name);
} /* end ObitTableCCUtilAppendShift */

/**
 * Filter out weak, isolated components in a table 
 * Zeroes any component for which the sum of the flux densities of all
 * components within radius is less than minFlux.
 * \param CCTab     CC table object to filter. 
 * \param radius    Radius within which to consider components. (deg) 
 * \param minFlux   Minimum acceptable summed flux
 * \param err       Error/message stack
 * \return True if any components zeroed
 */
gboolean ObitTableCCUtilFiltCC (ObitTableCC *CCTab, ofloat radius, ofloat minFlux, 
				ObitErr* err)  
{
  gboolean out = FALSE;
  ObitTableCCRow *CCRow = NULL;
  olong   irow, nrow, ncc, i, j;
  ofloat mxdis, sum, dis;
  ofloat *xpos=NULL, *ypos=NULL, *flux=NULL;
  olong  *row=NULL;
  gboolean checkMore;
  gchar *routine = "ObitTableCCUtilFiltCC";

  /* Error checks */
  if (err->error) return out;  /* previous error? */

  /* Open table */
  ObitTableCCOpen (CCTab, OBIT_IO_ReadWrite, err);
  if (err->error) goto cleanup;

  /* allocate storage */
  nrow = CCTab->myDesc->nrow;
  if (nrow<1) goto closeit;
  xpos = g_malloc0(nrow*sizeof(ofloat));
  ypos = g_malloc0(nrow*sizeof(ofloat));
  flux = g_malloc0(nrow*sizeof(ofloat));
  row  = g_malloc0(nrow*sizeof(olong));

  /* Copy table to arrays */
  CCRow = newObitTableCCRow (CCTab);
  ncc = 0;
  nrow = CCTab->myDesc->nrow;
  for (irow=1; irow<=nrow; irow++) {
    ObitTableCCReadRow (CCTab, irow, CCRow, err);
    if (err->error) goto cleanup;
    if (CCRow->status==-1) continue;  /* Deselected? */
    row[ncc]    = irow;
    xpos[ncc]   = CCRow->DeltaX;
    ypos[ncc]   = CCRow->DeltaY;
    flux[ncc++] = CCRow->Flux;
  } /* end loop reading table */

  /* Loop to convergence */
  mxdis = radius * radius;
  checkMore = TRUE;
  while (checkMore) {
    /* Sum fluxes within radius */
    checkMore = FALSE;
    for (i=0; i<ncc; i++) { 
      if (flux[i]==0.0) continue;    /* need to check? */
      sum = 0.0;
      for (j=0; j<ncc; j++) {
	if (flux[j]==0.0) continue;  /* need to check? */
	dis = (xpos[i]-xpos[j])*(xpos[i]-xpos[j]) + (ypos[i]-ypos[j])*(ypos[i]-ypos[j]);
	if (dis <= mxdis) sum += flux[j];
      } /* end loop  summing */
      if (sum < minFlux)  {  /* If too small, replace with zero flux */
	out       = TRUE;
	checkMore = TRUE;
	flux[i]   = 0.0;
	ObitTableCCReadRow (CCTab, row[i], CCRow, err);
	CCRow->Flux = 0.0;
	ObitTableCCWriteRow (CCTab, row[i], CCRow, err);
      } 
    } /* end loop over table */
  } /* End iteration */

  /* If out, mark as unsorted */
  if (out) {
    CCTab->myDesc->sort[0] = 0;
    CCTab->myDesc->sort[1] = 0;
  }

  /* Close table */
 closeit:
  ObitTableCCClose (CCTab, err);
  if (err->error) goto cleanup;
      
 cleanup:
   /* deallocate storage */
  CCRow = ObitTableCCRowUnref(CCRow);  
  if (xpos) g_free(xpos); 
  if (ypos) g_free(ypos); 
  if (flux) g_free(flux); 
  if (row)  g_free(row); 
  if (err->error) Obit_traceback_val (err, routine, CCTab->name, out);
  return out;
} /* end of routine ObitTableCCUtilFiltCC */ 

/**
 * Determine data type of the CCs in a specified table 
 * \param file      Data object with CC Table attached
 * \param ver       CCTable version
 * \param err       Error/message stack
 * \return ObitCCCompType of component type
 */
ObitCCCompType ObitTableCCUtilGetType (ObitData *file, olong ver, ObitErr* err)
{
  ObitCCCompType out=OBIT_CC_Unknown;
  ObitTableCC *CCTab=NULL;
  ObitTableCCRow *CCRow = NULL;
  olong noParms=0, iver=ver, row, ttype;
  gchar *routine = "ObitTableCCUtilGetType";

  /* Error */
  if (err->error) return out;

  /* Ensure file fully instantiated and OK */
  ObitDataFullInstantiate (file, TRUE, err);
  if (err->error) Obit_traceback_val (err, routine, file->name, out);

  /* Get table */ 
  CCTab =  newObitTableCCValue ("CC", file, &iver, OBIT_IO_ReadOnly, noParms, err);
  if (err->error) Obit_traceback_val (err, routine, file->name, out);

  /* Get it? */
  if (CCTab==NULL) {
    Obit_log_error(err, OBIT_Error,"%s: NO AIPS CC Table %d on %s",
		   routine, ver, file->name);
    return out;
  }

  /* Open table */
  ObitTableCCOpen (CCTab, OBIT_IO_ReadWrite, err);
  if (err->error) Obit_traceback_val (err, routine, file->name, out);

  row = 1;
  CCRow = newObitTableCCRow (CCTab);
  ObitTableCCReadRow (CCTab, row, CCRow, err);
  if (err->error) Obit_traceback_val (err, routine, file->name, out);
  ttype = (olong)(CCRow->parms[3]+0.5);

  ObitTableCCClose (CCTab, err);
  if (err->error) Obit_traceback_val (err, routine, file->name, out);

  /* What is it? */
  if (CCTab->noParms<=0)      out = OBIT_CC_PointMod;
  else if (CCTab->noParms>=0) out = ttype;

  /* Cleanup */
  CCRow = ObitTableCCRowUnref(CCRow);  
  CCTab = ObitTableCCUnref(CCTab);  
  return out;
} /* end ObitTableCCUtilGetType */

/**
 * Convert  CC Table with tabulated spectra to spectral fits
 * Antenna beam pattern weighed fitting; all model types supported.
 * \param image    Input ObitImageMF with attached CC table
 *                 Must have freq axis type = "SPECLNMF"
 *                 possible control parameter:
 * \li "dropNeg" OBIT_boolean if True, drop negative components [True] 
 * \li "doPBCor" OBIT_boolean if True, Primary beam corr [False] 
 * \param outImage Output ObitImageWB with attached CC table
 * \param nTerm    Number of output Spectral terms, 2=SI, 3=also curve.
 * \param inCCVer  input CC table version
 * \param outCCver output CC table version number, 
 *                 0=> create new in which case the actual value is returned
 * \param startCC  [in] the desired first CC number (1-rel)
 *                 [out] the actual first CC number in returned table
 * \param endCC    [in] the desired highest CC number, 0=> to end of table
 *                 [out] the actual highest CC number in returned table
 * \param err      Obit error stack object.
 */
void ObitTableCCUtilT2Spec  (ObitImage *image, ObitImageWB *outImage,
			     olong nTerm, olong *inCCVer, olong *outCCVer,
			     olong startCC, olong endCC, ObitErr *err)
{
  ObitTableCC *inCCTable=NULL, *outCCTable=NULL;
  ObitTable *tempTable=NULL;
  ObitTableCCRow *inCCRow=NULL, *outCCRow=NULL;
  ObitImageDesc *imDesc=NULL;
  ObitBeamShape *BeamShape=NULL;
  ObitBeamShapeClassInfo *BSClass;
  ObitIOCode retCode;
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  ObitInfoType type;
  union ObitInfoListEquiv InfoReal; 
  ofloat *flux=NULL, *sigma=NULL, *fitResult=NULL, pbmin=0.01, *PBCorr=NULL;
  ofloat *FreqFact=NULL, *sigmaField=NULL;
  ofloat *RMS=NULL, alpha=0.0, antSize = 25.0, fblank =  ObitMagicF();
  odouble *Freq=NULL, refFreq;
  odouble Angle=0.0;
  gpointer fitArg=NULL;
  olong irow, orow, ver, i, j, offset, nSpec, sCC, eCC, noParms, nterm;
  olong planeNo[5] = {1,1,1,1,1};
  gchar keyword[20];
  gboolean doZ, dropNeg, doPBCor;
  gchar *tabType = "AIPS CC";
  gchar *routine = "ObitTableCCUtilT2Spec";

  /* error checks */
  if (err->error) return;
  
  /* Make sure this is an ObitImage */
  Obit_return_if_fail((ObitImageIsA(image)), err, 
		      "%s: Image %s NOT an ObitImage", 
		      routine, image->name);

  /* Make sure this is really an ObitImageMF - freq axis = "SPECLNMF" */
  imDesc = image->myDesc;
  Obit_return_if_fail((!strncmp (imDesc->ctype[imDesc->jlocf],"SPECLNMF", 8)), err, 
		      "%s: Image %s NOT an ObitImageMF - no SPECLNMF axis", 
		      routine, image->name);

  /* Drop negatives? */
  dropNeg = TRUE;
  ObitInfoListGetTest(image->info, "dropNeg", &type, dim, &dropNeg);
  
  /* Primary Beam correction? */
  doPBCor = FALSE;
  ObitInfoListGetTest(image->info, "doPBCor", &type, dim, &doPBCor);
  
  /* Create spectrum info arrays */
  nSpec = 1;
  ObitInfoListGetTest(imDesc->info, "NSPEC", &type, dim, &nSpec);
  nterm = 1;
  ObitInfoListGetTest(imDesc->info, "NTERM", &type, dim, &nterm);
  /* Reference frequency from output */
  refFreq = outImage->myDesc->crval[outImage->myDesc->jlocf];
  Freq    = g_malloc0(nSpec*sizeof(odouble));
  FreqFact= g_malloc0(nSpec*sizeof(ofloat));
  RMS     = g_malloc0(nSpec*sizeof(ofloat));
  /* get number of and channel frequencies for CC spectra from 
     CC table on image  */
  if (nSpec>1) {
    for (i=0; i<nSpec; i++) {
      Freq[i] = 1.0;
      sprintf (keyword, "FREQ%4.4d",i+1);
      ObitInfoListGetTest(imDesc->info, keyword, &type, dim, &Freq[i]);
    }
  }
    
  /* Prior spectral index */
  InfoReal.flt = 0.0;   type = OBIT_float;
  ObitInfoListGetTest(imDesc->info, "ALPHA", &type, dim, &InfoReal);
  if (type==OBIT_double) alpha = (ofloat)InfoReal.dbl;
  if (type==OBIT_float)  alpha = (ofloat)InfoReal.flt;
  
  /* Log Freq ratio */
  for (i=0; i<nSpec; i++)  FreqFact[i] = log(Freq[i]/refFreq);

  /* Setup for fitting */
  offset     = 4;
  flux       = g_malloc0(nSpec*sizeof(ofloat));
  sigma      = g_malloc0(nSpec*sizeof(ofloat));
  sigmaField = g_malloc0(nSpec*sizeof(ofloat));
  PBCorr     = g_malloc0(nSpec*sizeof(ofloat));
  BeamShape  = ObitBeamShapeCreate ("BS", (ObitImage*)image, pbmin, antSize, TRUE);
  BSClass    = (ObitBeamShapeClassInfo*)(BeamShape->ClassInfo);
  fitArg     = ObitSpectrumFitMakeArg (nSpec, nTerm, refFreq, Freq, FALSE, 
				       &fitResult, err);
  for (i=0; i<nTerm; i++) fitResult[i]  = 0.0;
  for (i=0; i<nSpec; i++) sigmaField[i] = -1.0;
  if  (err->error) goto cleanup;
  
  /* Get image RMSes */  
  retCode = ObitImageOpen (image, OBIT_IO_ReadOnly, err);
  for (i=0; i<nSpec; i++) {
    planeNo[0] = i + nterm + 1; /* Note: 1 rel */
    ObitImageGetPlane (image, NULL, planeNo, err);
    RMS[i] = ObitFArrayRMS(image->image);
  }
  retCode = ObitImageClose (image, err);
  if (err->error) Obit_traceback_msg (err, routine, image->name);

  /* Make sure output image descriptor correct */
  retCode = ObitImageOpen ((ObitImage*)outImage, OBIT_IO_ReadWrite, err);
  if (err->error) Obit_traceback_msg (err, routine, outImage->name);
  outImage->myDesc->crval[outImage->myDesc->jlocf] = refFreq;
  retCode = ObitImageClose ((ObitImage*)outImage, err);
  if (err->error) Obit_traceback_msg (err, routine, outImage->name);


  /* Get input CC table */
  ver = *inCCVer;
  tempTable = newObitImageTable (image,OBIT_IO_ReadOnly, tabType, &ver, err);
  if ((tempTable==NULL) || (err->error)) 
     Obit_traceback_msg (err, routine, image->name);
  inCCTable = ObitTableCCConvert(tempTable);
  tempTable = ObitTableUnref(tempTable);
  if (err->error) Obit_traceback_msg (err, routine, image->name);
  
  /* Open input */
  retCode = ObitTableCCOpen (inCCTable, OBIT_IO_ReadOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_msg (err, routine, image->name);
  *inCCVer = ver;

  /* Create table row */
  inCCRow = newObitTableCCRow (inCCTable);

  /* Create output CC table */
  ver = *outCCVer;
  noParms = 3 + nTerm;
  outCCTable = newObitTableCCValue ("SpecT table", (ObitData*)outImage,
				    &ver, OBIT_IO_WriteOnly, noParms, 
				    err);
  if (err->error) goto cleanup;
  *outCCVer = ver;  /* save if defaulted (0) */
  
  /* Open output */
  retCode = ObitTableCCOpen (outCCTable, OBIT_IO_ReadWrite, err);
  if ((retCode != OBIT_IO_OK) || (err->error)) goto cleanup;
  
  /* Create table row */
  outCCRow = newObitTableCCRow (outCCTable);
  
    /* Does the CC table have a DeltaZ column? */
  doZ = inCCTable->DeltaZCol>=0;

  offset = 4;
  sCC = MAX (1, startCC);
  if (endCC>0) eCC = MIN (endCC, inCCTable->myDesc->nrow);
  else eCC = inCCTable->myDesc->nrow;
  /* loop over table */
  for (j=sCC; j<=eCC; j++) {
    irow = j;
    retCode = ObitTableCCReadRow (inCCTable, irow, inCCRow, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) 
      Obit_traceback_msg (err, routine, inCCTable->name);
    
    /* Make sure this has a tabulated spectrum */
    Obit_return_if_fail((inCCRow->parms && (inCCRow->parms[3]>=20.)), err, 
			"%s: CCs do not contain tab. spectra", routine);
    
    /* Copy row data */
    outCCRow->DeltaX   = inCCRow->DeltaX;
    outCCRow->DeltaY   = inCCRow->DeltaY;
    if (doZ) outCCRow->DeltaZ   = inCCRow->DeltaZ;
    outCCRow->parms[0] = inCCRow->parms[0]; /* Model parameters */
    outCCRow->parms[1] = inCCRow->parms[1];
    outCCRow->parms[2] = inCCRow->parms[2];
    outCCRow->parms[3] = inCCRow->parms[3] - 10.0;  /* Change type */

    /* Fit spectrum */
    /* Set field sigmas to RMS */
    if (sigmaField[0]<0.0) {
      for (i=0; i<nSpec; i++) sigmaField[i] = RMS[i];
    }

    /* Primary beam stuff - Distance from Center  */
    Angle = ObitImageDescAngle(image->myDesc, inCCRow->DeltaX, inCCRow->DeltaY);
    
    /* Loop over spectral channels get corrected flux, sigma */
    for (i=0; i<nSpec; i++) {
      if (doPBCor) {
	BeamShape->refFreq = Freq[i];  /* Set frequency */
	PBCorr[i] = BSClass->ObitBeamShapeGainSym(BeamShape, Angle);
	flux[i]   = inCCRow->parms[offset+i] / PBCorr[i];
	if ((RMS[i]>0.0) && (RMS[i]!=fblank))
	  sigma[i]  = sigmaField[i] / (PBCorr[i]*PBCorr[i]);
	else
	  sigma[i] = -1;   /* Bad plane */
      } else {  /* No PB Corr */
	flux[i]   = inCCRow->parms[offset+i];
	if ((RMS[i]>0.0) && (RMS[i]!=fblank))
	  sigma[i]  = sigmaField[i];
	else
	  sigma[i] = -1;   /* Bad plane */
      }
    } /* End channel loop */
    
    /* Fit spectrum */
    ObitSpectrumFitSingleArg (fitArg, flux, sigma, fitResult);
    
    /* Prior spectral index correction */
    if (nTerm>=2) fitResult[1] += alpha;
    
    /* Replace channel fluxes with fitted spectrum */
    outCCRow->Flux     = fitResult[0];
    for (i=0; i<nTerm; i++) outCCRow->parms[offset+i] = fitResult[i+1];
    
    /* Write output */
    if (dropNeg && (outCCRow->Flux<0.0)) continue;  /* Want this one? */
    orow = -1;
    retCode = ObitTableCCWriteRow (outCCTable, orow, outCCRow, err);
    if  (err->error) goto cleanup;
    
  } /* end loop over table */

  /* Close/cleanup */
 cleanup:
  retCode   = ObitTableCCClose (inCCTable, err);
  retCode   = ObitTableCCClose (outCCTable, err);
  inCCTable = ObitTableUnref(inCCTable);
  outCCTable= ObitTableUnref(outCCTable);
  inCCRow   = ObitTableRowUnref(inCCRow);
  outCCRow  = ObitTableRowUnref(outCCRow);
  BeamShape = ObitBeamShapeUnref(BeamShape);
  ObitSpectrumFitKillArg(fitArg);
  if (flux)       g_free(flux);
  if (sigma)      g_free(sigma);
  if (sigmaField) g_free(sigmaField);
  if (PBCorr)     g_free(PBCorr);
  if (Freq)       g_free(Freq);
  if (FreqFact)   g_free(FreqFact);
  if (RMS)        g_free(RMS);
  if (fitResult)  g_free(fitResult);
  if (err->error) Obit_traceback_msg (err, routine, image->name);
}  /* end ObitTableCCUtilT2Spec */

/**
 * Force average TSpec spectrum to a given spectrum
 * Antenna beam pattern included; all model types supported.
 * \param image    Input ObitImage(MF) with attached CC table
 *                 Must have freq axis type = "SPECLNMF"
 *                 possible control parameters:
 * \li "Limit" OBIT_float scalar maximum distance [deg] from pointing for 
 *      CCs in the normalization, default = very large
 * \param inCCVer  input CC table version
 * \param refFreq  Reference frequency for spectrum (Hz)
 * \param nterm    Number of terms in spectrum
 * \param terms    Spectral terms, 0=flux density, 1=spectral index, 2=curvature
 * \param startCC  [in] the desired first CC number (1-rel)
 *                 [out] the actual first CC number in returned table
 * \param endCC    [in] the desired highest CC number, 0=> to end of table
 *                 [out] the actual highest CC number in returned table
 * \param err      Obit error stack object.
 */
void ObitTableCCUtilFixTSpec (ObitImage *inImage, olong *inCCVer, 
			      odouble refFreq, olong nterm, ofloat *terms,
			      olong startCC, olong endCC, ObitErr *err)
{
  ObitTableCC *inCCTable=NULL;
  ObitTable *tempTable=NULL;
  ObitTableCCRow *inCCRow=NULL;
  ObitImageDesc *imDesc=NULL;
  ObitIOCode retCode;
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  ObitInfoType type;
  ofloat *FreqFact=NULL, ll, lll, arg, alpha=0.0, specFact, *sumFlux=NULL;
  ofloat fblank = ObitMagicF();
  ofloat Limit, dist;
  odouble *Freq=NULL, rfAlpha;
  olong irow, orow, ver, i, j, iterm, offset, nSpec, sCC, eCC;
  union ObitInfoListEquiv InfoReal; 
  gchar *tabType = "AIPS CC";
  gchar keyword[20];
  gchar *routine = "ObitTableCCUtilFixTSpec";

  /* error checks */
  if (err->error) return;
  
  /* Make sure this is an ObitImage */
  Obit_return_if_fail((ObitImageIsA(inImage)), err, 
		      "%s: Image %s NOT an ObitImage", 
		      routine, inImage->name);

  /* Make sure this is an ObitImageMF - freq axis = "SPECLNMF" */
  imDesc = inImage->myDesc;
  Obit_return_if_fail((!strncmp (imDesc->ctype[imDesc->jlocf],"SPECLNMF", 8)), err, 
		      "%s: Image %s NOT an ObitImageMF - no SPECLNMF axis", 
		      routine, inImage->name);

  /* Limit on distance from pointing */
  Limit = 1.0e20;
  ObitInfoListGetTest(inImage->info, "Limit", &type, dim, &Limit);
  if (Limit<=0.0) Limit = 1.0e20;
  
  /* Numbers of things */
  nSpec = 1;
  ObitInfoListGetTest(imDesc->info, "NSPEC", &type, dim, &nSpec);
  /*nterm = 1;
    ObitInfoListGetTest(imDesc->info, "NTERM", &type, dim, &nterm);*/
  sumFlux = g_malloc0(nSpec*sizeof(ofloat));

  /* Get input CC table */
  ver = *inCCVer;
  tempTable = newObitImageTable (inImage,OBIT_IO_ReadOnly, tabType, &ver, err);
  if ((tempTable==NULL) || (err->error)) 
     Obit_traceback_msg (err, routine, inImage->name);
  inCCTable = ObitTableCCConvert(tempTable);
  tempTable = ObitTableUnref(tempTable);
  if (err->error) Obit_traceback_msg (err, routine, inImage->name);
  
  /* Open input */
  retCode = ObitTableCCOpen (inCCTable, OBIT_IO_ReadWrite, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_msg (err, routine, inImage->name);
  *inCCVer = ver;
  /* Create table row */
  inCCRow = newObitTableCCRow (inCCTable);

  offset = 4;
  sCC = MAX (1, startCC);
  if (endCC>0) eCC = MIN (endCC, inCCTable->myDesc->nrow);
  else eCC = inCCTable->myDesc->nrow;
  /* loop over table */
  for (j=sCC; j<=eCC; j++) {
    irow = j;
    retCode = ObitTableCCReadRow (inCCTable, irow, inCCRow, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) 
      Obit_traceback_msg (err, routine, inCCTable->name);
    
    /* Make sure this has a spectrum */
    Obit_return_if_fail((inCCRow->parms && (inCCRow->parms[3]>=20.)), err, 
			"%s: CCs do not contain tab. spectra", routine);

    /* Close enough? */
    dist = sqrt(inCCRow->DeltaX*inCCRow->DeltaX + inCCRow->DeltaY*inCCRow->DeltaY);
    if (dist>Limit) continue;

    /* Sum */
    for (i=0; i<nSpec; i++) {
      if (inCCRow->parms[offset+i]!=fblank) 
	sumFlux[i] += MAX (0.0, inCCRow->parms[offset+i]);
    }
  
  } /* end loop over table summing */

  /* Create spectrum info arrays */
  Freq    = g_malloc0(nSpec*sizeof(odouble));
  FreqFact= g_malloc0(nSpec*sizeof(ofloat));
  /* get number of and channel frequencies for CC spectra from 
     CC table on first image in mosaic */
  if (nSpec>1) {
    for (i=0; i<nSpec; i++) {
      Freq[i] = 1.0;
      sprintf (keyword, "FREQ%4.4d",i+1);
      ObitInfoListGetTest(imDesc->info, keyword, &type, dim, &Freq[i]);
    }
  }
    
  /* Prior spectral index */
  InfoReal.flt = 0.0;   type = OBIT_float;
  ObitInfoListGetTest(imDesc->info, "ALPHA", &type, dim, &InfoReal);
  if (type==OBIT_double) alpha = (ofloat)InfoReal.dbl;
  if (type==OBIT_float)  alpha = (ofloat)InfoReal.flt;
  rfAlpha = refFreq;
  ObitInfoListGetTest(imDesc->info, "RFALPHA", &type, dim, &rfAlpha);
  
  /* Log Freq ratio */
  for (i=0; i<nSpec; i++)  FreqFact[i] = log(Freq[i]/refFreq);

  /* Get correction factors per channel in sumFlux */
  for (i=0; i<nSpec; i++) {
    /* Frequency dependent term */
    lll = ll = FreqFact[i];
    arg = 0.0;
    for (iterm=1; iterm<nterm; iterm++) {
      arg += terms[iterm] * lll;
      lll *= ll;
    }
    /* Convert sum of flux to correction factor */
    specFact = exp(arg);
    if (sumFlux[i]!=0.0) sumFlux[i] = specFact*terms[0] / sumFlux[i];
    else sumFlux[i] = 1.0;
    /* Correct for prior alpha */
    specFact = exp(-alpha * log(Freq[i]/rfAlpha));
    sumFlux[i] *= specFact;
  }

  offset = 4;
  sCC = MAX (1, startCC);
  if (endCC>0) eCC = MIN (endCC, inCCTable->myDesc->nrow);
  else eCC = inCCTable->myDesc->nrow;
  /* loop over table */
  for (j=sCC; j<=eCC; j++) {
    irow = j;
    retCode = ObitTableCCReadRow (inCCTable, irow, inCCRow, err);
    if ((retCode != OBIT_IO_OK) || (err->error)) 
      Obit_traceback_msg (err, routine, inCCTable->name);
    
    /* Correct channel fluxes with fitted spectrum */
    for (i=0; i<nSpec; i++) inCCRow->parms[offset+i] *= sumFlux[i];
      
    /* Write output */
    orow = j;
    retCode = ObitTableCCWriteRow (inCCTable, orow, inCCRow, err);
    if  (err->error) goto cleanup;
  
  } /* end loop over table */

  /* Close/cleanup */
 cleanup:
  retCode   = ObitTableCCClose (inCCTable, err);
  inCCTable = ObitTableUnref(inCCTable);
  inCCRow   = ObitTableRowUnref(inCCRow);
  if (Freq)       g_free(Freq);
  if (FreqFact)   g_free(FreqFact);
  if (sumFlux)    g_free(sumFlux);
  if  (err->error) Obit_traceback_msg (err, routine, inImage->name);
} /* end ObitTableCCUtiTl2Spec */

/**
 * Combine multiple simple CCTables into a tabulated spectrum CC table
 * Output Table will have nCCVer tabulated spectral points and 
 * the sum of the values as the "Flux" in the table.
 * Each of the tables CC version inCCVer to inCCVer+nCCVer-1 have 
 * each entry promoted into a spectrum with the flux density in entry
 * CCVer-inCCVer+1 and the rest zero.
 * All components can be forced to Gaussians (point=zero size) using
 * doGaus.  The operation amy fail for mixed point and Gaussian input
 * tables if doGaus=FALSE.
 * \param image    Input ObitImage with attached CC tables
 * \param inCCVer  first input CC table version
 * \param nCCVer   Number of input CC table version and size of output spectrum
 * \param outCCVer Output TSpec table
 * \param bcopy    first component (1-rel) in each input table to copy
 * \param bcomp    [out] first new component in outCCVer
 * \param ecomp    [out] highest new component in outCCVer
 * \param doGaus   Promote points to zero size Gaussians?.
 * \param err      Obit error stack object.
 */
void ObitTableCCUtilCombTSpec (ObitImage *inImage, olong inCCVer, olong nCCVer,
			       olong outCCVer, olong *bcopy, olong *bcomp, olong *ecomp, 
			       gboolean doGaus, ObitErr *err)
{
  ObitTableCC *inCCTab=NULL, *outCCTab;
  ObitTableCCRow *inCCRow=NULL, *outCCRow=NULL;
  olong i, nSpec, ver, noParms, orow, irow, itab, ncopy;
  ofloat  modType;
  gboolean inPoint, doZ;
  gchar *routine = "ObitTableCCUtilCombTSpec";

  /* error checks */
  if (err->error) return;
  
  /* Make sure this is an ObitImage */
  Obit_return_if_fail((ObitImageIsA(inImage)), err, 
		      "%s: Image %s NOT an ObitImage", 
		      routine, inImage->name);

  /* Numbers of things */
  nSpec = nCCVer;
  noParms = 4 + nSpec;
  if (doGaus) modType = (ofloat)OBIT_CC_GaussModTSpec;
  else        modType = (ofloat)OBIT_CC_PointModTSpec;
  
  /* Create  output CC table */
  ver = outCCVer ;
  outCCTab = newObitTableCCValue ("OutputTSpecCC", (ObitData*)inImage,
				  &ver, OBIT_IO_WriteOnly, noParms, 
				  err);
  if (err->error) Obit_traceback_msg (err, routine, inImage->name);
 
  /* Create output table row */
  outCCRow = newObitTableCCRow (outCCTab);
  
  /* Open output */
  ObitTableCCOpen (outCCTab, OBIT_IO_ReadWrite, err);
  ObitTableCCSetRow (outCCTab, outCCRow, err);
  if (err->error) Obit_traceback_msg (err, routine, inImage->name);
 
  /* Check compatability - it may be an existing table */
  Obit_return_if_fail((outCCTab->noParms==noParms), err, 
		      "%s: Existing CC table incompatable %d %d", 
		      routine, outCCTab->noParms, noParms);
  
  /* Existing rows? */
  orow = outCCTab->myDesc->nrow+1;
  *bcomp = orow;

  /* Loop over input Tables */
  for (itab=0; itab<nCCVer; itab++) {
    /* Zero parms */
    for (i=0; i<noParms; i++) outCCRow->parms[i] = 0;
    
    ver = inCCVer + itab;
    i = 0;
    inCCTab = newObitTableCCValue ("InputCC", (ObitData*)inImage,
				   &ver, OBIT_IO_ReadOnly, i, err);
    if (err->error) Obit_traceback_msg (err, routine, inImage->name);
    
    /* Create output table row */
    inCCRow = newObitTableCCRow (inCCTab);
    
    /* Open input */
    ObitTableCCOpen (inCCTab, OBIT_IO_ReadOnly, err);
    if (err->error) Obit_traceback_msg (err, routine, inImage->name);
    inPoint = inCCTab->noParms<=0;

    /* Does the CC table have a DeltaZ column? */
    doZ = inCCTab->DeltaZCol>=0;

    /* Copy Rows */
    ncopy = inCCTab->myDesc->nrow;
    if (ncopy<=0) goto done;
    for (irow=bcopy[itab]; irow<=ncopy; irow++) {
      ObitTableCCReadRow (inCCTab, irow, inCCRow, err);
      if (err->error) Obit_traceback_msg (err, routine, inCCTab->name);
      /* Copy info */
      outCCRow->DeltaX   = inCCRow->DeltaX;
      outCCRow->DeltaY   = inCCRow->DeltaY;
      if (doZ) outCCRow->DeltaZ   = inCCRow->DeltaZ;
      outCCRow->Flux   = inCCRow->Flux;
      /* Set output type and parameters if given */
      if (inPoint) outCCRow->parms[3] = modType;
      else  for (i=0; i<4; i++) outCCRow->parms[i] = inCCRow->parms[i];
      /* One point in spectrum */
      outCCRow->parms[4+itab] = inCCRow->Flux;
      ObitTableCCWriteRow (outCCTab, orow, outCCRow, err);
      orow++;   /* Count rows */
      if (err->error) Obit_traceback_msg (err, routine, outCCTab->name);
  } /* end loop over rows in input table */
    /* Cleanup input Table */
  done:
    ObitTableCCClose (inCCTab, err);
    inCCTab = ObitTableUnref(inCCTab);
    inCCRow = ObitTableRowUnref(inCCRow);
    if (err->error) Obit_traceback_msg (err, routine, inImage->name);
  } /* end loop over tables */

    /* Cleanup output Table */
  ObitTableCCClose (outCCTab, err);
  outCCTab = ObitTableUnref(outCCTab);
  outCCRow = ObitTableRowUnref(outCCRow);
  if (err->error) Obit_traceback_msg (err, routine, inImage->name);

  *ecomp = orow-1;  /* Highest actual output row number */
} /* end ObitTableCCUtilCombTSpec */

/*----------------------Private functions---------------------------*/
/**
 * Create/fill sort structure for a CC table
 * The sort structure has one "entry" per row which contains 
 * \li Delta X
 * \li Delta Y
 * \li Flux
 * \li [optional] spectral terms +
 *
 * Each valid row in the table has an entry.
 * \param in     Table to sort, assumed already open;
 * \param size   [out] Number of bytes in entry
 * \param number [out] Number of entries
 * \param ncomp  [out] Number of values to compare
 * \param parms  [out] Parms of first element if they exist
 * \param err     ObitErr error stack.
 * \return sort structure, should be ObitMemFreeed when done.
 */
static ofloat* 
MakeCCSortStruct (ObitTableCC *in, olong *size, olong *number, olong *ncomp,
		  ofloat *parms, ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  ofloat *out = NULL;
  ObitTableCCRow *row = NULL;
  ofloat *entry;
  olong irow, nrow, count, i, j, toff;
  gulong tsize;
  olong nterms, fsize;
  gboolean haveDeltaZ=FALSE;
  gchar *routine = "MakeCCSortStruct";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return out;
  g_assert (ObitTableCCIsA(in));

  /* Get table info */
  nrow = in->myDesc->nrow;

  /* Does the CC table have a DeltaZ column? */
  haveDeltaZ = in->DeltaZCol>=0;
  
  /* element size */
  if (haveDeltaZ) {fsize = 4; toff = 4;}
  else            {fsize = 3; toff = 3;}
  /* Need room for spectral terms? */
  if (in->noParms>4)  nterms = in->noParms-4;
  else nterms = 0;
  fsize += nterms;
  *size = fsize * sizeof(ofloat);

  /* Total size of structure in case all rows valid */
  tsize = *size;
  tsize *= (nrow+10);
  /* create output structure */
  out = ObitMemAlloc0Name (tsize, "CCSortStructure");
  
  /* Compare 2  (X, Y pos) */
  *ncomp = 2;

  /* Create table row */
  row = newObitTableCCRow (in);

 /* loop over table */
  irow = 0;
  count = 0;
  retCode = OBIT_IO_OK;
  while (retCode==OBIT_IO_OK) {
    irow++;
    retCode = ObitTableCCReadRow (in, irow, row, err);
    if (retCode == OBIT_IO_EOF) break;
    if ((retCode != OBIT_IO_OK) || (err->error)) 
      Obit_traceback_val (err, routine, in->name, out);
    if (row->status<0) continue;  /* Skip deselected record */

    /* add to structure */
    entry = (ofloat*)(out + count * fsize);  /* set pointer to entry */
    entry[0] = row->DeltaX;
    entry[1] = row->DeltaY;
    entry[2] = row->Flux;
    if (haveDeltaZ) entry[3] = row->DeltaZ;
    /* First 4 parms are model, following are spectral parameters */
    for (j=0; j<nterms; j++) entry[toff+j] = row->parms[4+j];

    /* Save parms if any for first record */
    if ((count<=0) && (in->noParms>0)) {
      for (i=0; i<MIN (4,in->noParms); i++) parms[i] = row->parms[i];
    }
    
    count++;  /* How many valid */
  } /* end loop over file */
  
  /* check for errors */
  if ((retCode > OBIT_IO_EOF) || (err->error))
    Obit_traceback_val (err, routine, in->name, out);
  
  /* Release table row */
  row = ObitTableCCRowUnref (row);
  
  /* Actual number */
  *number = count;

  return out;
} /* end MakeCCSortStruc */ 

/**
 * Create/fill sort structure for a CC table selecting by row
 * The sort structure has one "entry" per row which contains 
 * \li Delta X
 * \li Delta Y
 * \li Delta Flux
 *
 * Each valid row in the table has an entry.
 * \param in        Table to sort, assumed already open;
 * \param startComp First component to select 
 * \param endComp   Last component to select, 0=> all
 * \param size      [out] Number of bytes in entry
 * \param number    [out] Number of entries
 * \param ncomp     [out] Number of values to compare
 * \param parms     [out] Parms of first element if they exist
 * \param err        ObitErr error stack.
 * \return sort structure, should be ObitMemFreeed when done.
 */
static ofloat* 
MakeCCSortStructSel (ObitTableCC *in, olong startComp, olong endComp, 
		     olong *size, olong *number, olong *ncomp, ofloat *parms, 
		     ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  ofloat *out = NULL;
  ObitTableCCRow *row = NULL;
  ofloat *entry;
  olong irow, nrow, tsize, count, i, j, toff;
  olong nterms, fsize;
  gboolean haveDeltaZ=FALSE;
  gchar *routine = "MakeCCSortStructSel";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return out;
  g_assert (ObitTableCCIsA(in));

  /* Get table info */
  nrow = in->myDesc->nrow;

  /* Does the CC table have a DeltaZ column? */
  haveDeltaZ = in->DeltaZCol>=0;
  
  /* element size */
  if (haveDeltaZ) {fsize = 4; toff = 4;}
  else            {fsize = 3; toff = 3;}
  /* Need room for spectral terms? */
  if (in->noParms>4)  nterms = in->noParms-4;
  else nterms = 0;
  fsize += nterms;
  *size = fsize * sizeof(ofloat);

  /* Total size of structure in case all rows valid */
  tsize = (*size) * (nrow+10);
  /* create output structure */
  out = ObitMemAlloc0Name (tsize, "CCSortStructure");
  
  /* Compare 2  (X, Y pos) */
  *ncomp = 2;

  /* Create table row */
  row = newObitTableCCRow (in);

  /* loop over table */
  irow = startComp-1;
  count = 0;
  retCode = OBIT_IO_OK;
  while ((irow<endComp) && (retCode==OBIT_IO_OK)) {
    irow++;
    retCode = ObitTableCCReadRow (in, irow, row, err);
    if (retCode == OBIT_IO_EOF) break;
    if ((retCode != OBIT_IO_OK) || (err->error)) 
      Obit_traceback_val (err, routine, in->name, out);
    if (row->status<0) continue;  /* Skip deselected record */

    /* add to structure */
    entry = (ofloat*)(out + count * fsize);  /* set pointer to entry */
    entry[0] = row->DeltaX;
    entry[1] = row->DeltaY;
    entry[2] = row->Flux;
    if (haveDeltaZ) entry[3] = row->DeltaZ;
    /* First 4 parms are model, following are spectral parameters */
    for (j=0; j<nterms; j++) entry[toff+j] = row->parms[4+j];

    /* Save 1st 4 parms if any for first record */
    if ((count<=0) && (in->noParms>0)) {
      for (i=0; i<MIN (4,in->noParms); i++) parms[i] = row->parms[i];
    }
    
    count++;  /* How many valid */
  } /* end loop over file */
  
  /* check for errors */
  if ((retCode > OBIT_IO_EOF) || (err->error))
    Obit_traceback_val (err, routine, in->name, out);
  
  /* Release table row */
  row = ObitTableCCRowUnref (row);
  
  /* Actual number */
  *number = count;

  return out;
} /* end MakeSortStrucSel */ 

/**
 * Create/fill sort structure for a CC table selecting by row
 * The sort structure has one "entry" per row which contains 
 * \li Delta X
 * \li Delta Y
 * \li Delta Flux
 *
 * Each valid row in the table has an entry.
 * \param in        Table to sort, assumed already open;
 * \param startComp First component to select 
 * \param endComp   Last component to select, 0=> all
 * \param size      [out] Number of bytes in entry
 * \param number    [out] Number of entries
 * \param ncomp     [out] Number of values to compare
 * \param type      [out] Type of components
 * \param err        ObitErr error stack.
 * \return sort structure, should be ObitMemFreeed when done.
 */
static ofloat* 
MakeCCSortStructSel2 (ObitTableCC *in, olong startComp, olong endComp, 
		      olong *size, olong *number, olong *ncomp, 
		      ObitSkyModelCompType *type, ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  ofloat *out = NULL;
  ObitTableCCRow *row = NULL;
  ofloat *entry;
  olong irow, nrow, tsize, count, j, toff, off;
  olong nterms, fsize, tmpType;
  gboolean haveDeltaZ=FALSE;
  ObitSkyModelCompType  maxType = OBIT_SkyModel_PointMod;
  gchar *routine = "MakeCCSortStructSel2";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return out;
  g_assert (ObitTableCCIsA(in));

  /* Get table info */
  nrow = in->myDesc->nrow;

  /* Does the CC table have a DeltaZ column? */
  haveDeltaZ = in->DeltaZCol>=0;
  
  /* element size - allow 3 Gaussian components */
  if (haveDeltaZ) {fsize = 7; toff = 7;}
  else            {fsize = 6; toff = 6;}
  /* Need room for spectral terms? */
  if (in->noParms>4)  nterms = in->noParms-4;
  else nterms = 0;
  fsize += nterms;
  *size = fsize * sizeof(ofloat);

  /* Total size of structure in case all rows valid */
  tsize = (*size) * (nrow+10);
  /* create output structure */
  out = ObitMemAlloc0Name (tsize, "CCSortStructure");
  
  /* Compare 2  (X, Y pos) */
  *ncomp = 2;

  /* Create table row */
  row = newObitTableCCRow (in);

  /* loop over table */
  irow = startComp-1;
  count = 0;
  retCode = OBIT_IO_OK;
  while ((irow<endComp) && (retCode==OBIT_IO_OK)) {
    irow++;
    retCode = ObitTableCCReadRow (in, irow, row, err);
    if (retCode == OBIT_IO_EOF) break;
    if ((retCode != OBIT_IO_OK) || (err->error)) 
      Obit_traceback_val (err, routine, in->name, out);
    if (row->status<0) continue;  /* Skip deselected record */

    /* add to structure */
    entry = (ofloat*)(out + count * fsize);  /* set pointer to entry */
    entry[0] = row->DeltaX;
    entry[1] = row->DeltaY;
    entry[2] = row->Flux; off = 2;
    if (haveDeltaZ) {entry[3] = row->DeltaZ; off = 3;}
    if (in->noParms>=3) {
      entry[off+1] = row->parms[0];
      entry[off+2] = row->parms[1];
      entry[off+3] = row->parms[2];
    } else { /* only points */
      entry[off+1] = 0.;
      entry[off+2] = 0.;
      entry[off+3] = 0.;
   }
   /* First 4 parms are model, following are spectral parameters */
    for (j=0; j<nterms; j++) entry[toff+j] = row->parms[4+j];

    /* Save 1st 4 parms if any for first record */
    if (in->noParms>=4) {
      tmpType = (olong)row->parms[3];
      maxType = MAX(maxType, tmpType);
    }
    
    count++;  /* How many valid */
  } /* end loop over file */
  
  /* check for errors */
  if ((retCode > OBIT_IO_EOF) || (err->error))
    Obit_traceback_val (err, routine, in->name, out);
  
  /* Release table row */
  row = ObitTableCCRowUnref (row);
  
  /* Actual number */
  *number = count;
  *type   = maxType;  /* output type */
  return out;
} /* end MakeSortStrucSel2 */ 

/**
 * Compare two lists of floats
 * Conformant to function type GCompareDataFunc
 * \param in1   First list
 * \param in2   Second list
 * \param ncomp Number of values to compare (2)
 * \return <0 -> in1 < in2; =0 -> in1 == in2; >0 -> in1 > in2; 
 */
static gint CCComparePos (gconstpointer in1, gconstpointer in2, 
			  gpointer ncomp)
{
  gint out = 0;
  ofloat *float1, *float2;

  /* get correctly typed local values */
  float1 = (float*)(in1);
  float2 = (float*)(in2);

  if (float1[0]<float2[0])      out = -1;
  else if (float1[0]>float2[0]) out = 1;
  else                          out = 0;
  if (!out) { /* compare second needed? */
    if (float1[1]<float2[1])      out = -1;
    else if (float1[1]>float2[1]) out = 1;
    else                          out = 0;
  }

  return out;
} /* end CCComparePos */


/**
 * Compare fluxes, to give descending abs order.
 * Conformant to function type GCompareDataFunc
 * \param in1   First list
 * \param in2   Second list
 * \param ncomp Number of values to compare (1)
 * \return <0 -> in1 < in2; =0 -> in1 == in2; >0 -> in1 > in2; 
 */
static gint CCCompareFlux (gconstpointer in1, gconstpointer in2, 
			   gpointer ncomp)
{
  gint out = 0;
  ofloat *float1, *float2;

  /* get correctly typed local values */
  float1 = (float*)(in1 + 2*sizeof(ofloat));
  float2 = (float*)(in2 + 2*sizeof(ofloat));
  if (fabs(*float1)<fabs(*float2))      out =  1;
  else if (fabs(*float1)>fabs(*float2)) out = -1;
  else                          out =  0;
  return out;
} /* end CCCompareFlux */

/**
 * Merge entries in sort structure
 * leaves "X" entry in defunct rows -1.0e20
 * table and then copies over the input table.
 * \param base    Base address of sort structure
 * \param size    Size in gfloats of a sort element
 * \param number  Number of sort elements
 */
static void CCMerge (ofloat *base, olong size, olong number)
{
  olong i, j;
  ofloat *array = base;
  
  i = 0;
  while (i<number) {
    j=i+1;
    while (j<number) {
      if ((array[j*size]!=array[i*size]) || (array[j*size+1]!=array[i*size+1]))
	break;
      array[i*size+2] += array[j*size+2];  /* sum fluxes */
      array[j*size] = -1.0e20;             /* Don't need any more */
      j++;
    } /* end finding matches */
    i = j;   /* move on */
  } /* end loop over table */

} /* end CCMerge */

/**
 * Merge Spectral entries in sort structure
 * leaves "X" posn entry in defunct rows -1.0e20
 * table and then copies over the input table.
 * For parameterized spectra:
 * Takes flux weighted average of spectral components,
 * assumed to be entries 3+
 * For tabulated spectra:
 * Takes sums spectral components assumed to be entries 3+
 * \param base    Base address of sort structure
 * \param size    Size in gfloats of an element
 * \param number  Number of sort elements
 * \param doSpec  TRUE if parameterized spectra
 * \param doTSpec TRUE if tabulated spectra
 * \param doZ     TRUE if have delta Z in table
 */
static void CCMergeSpec (ofloat *base, olong size, olong number, 
			 gboolean doSpec, gboolean doTSpec, 
			 gboolean doZ)
{
  olong i, j, k, toff;
  ofloat *array = base;

  /* Merging doSpec data too risky */
  if (doSpec) return;
  
  if (doZ) toff = 4;
  else     toff = 3;

  /* Multiply parameterized spectral terms by flux */
  if (doSpec) {
    j = 0;
    while (j<number) {
      for (k=toff; k<size; k++) 
	array[j*size+k] *=  array[j*size+2];
      j++;
    }
  }

  i = 0;
  while (i<number) {
    j=i+1;
    while (j<number) {
      if ((array[j*size]!=array[i*size]) || (array[j*size+1]!=array[i*size+1]))
	break;
      /* Sum spectral components  flux */
      for (k=toff; k<size; k++) 
	array[i*size+k] += array[j*size+k]; 
      array[i*size+2] += array[j*size+2];  /* sum fluxes */
      array[j*size] = -1.0e20;             /* Don't need any more */
      j++;
    } /* end finding matches */
    i = j;   /* move on */
  } /* end loop over table */

  /* Normalize parameterized spectra by sum of flux */
  if (doSpec) {
    i = 0;
    while (i<number) {
      if ((array[i*size]>-1.0e-19) && (fabs(array[i*size+3])>0.0)) {
	for (k=toff; k<size; k++) 
	  array[i*size+k] /= array[i*size+2]; 
      }
      i++;
    }
  }

} /* end CCMergeSpec */

/**
 * Merge Spectral entries in sort structure allowing mixed Gaussians
 * leaves "X" posn entry in defunct rows -1.0e20
 * table and then copies over the input table.
 * For parameterized spectra:
 * Takes flux weighted average of spectral components,
 * assumed to be entries 3+
 * For tabulated spectra:
 * Takes sums spectral components assumed to be entries 3+
 * \param base    Base address of sort structure
 * \param size    Size in gfloats of an element
 * \param number  Number of sort elements
 * \param doSpec  TRUE if parameterized spectra
 * \param doTSpec TRUE if tabulated spectra
 * \param doZ     TRUE if have delta Z in table
 */
static void CCMergeSpec2 (ofloat *base, olong size, olong number, 
			  gboolean doSpec, gboolean doTSpec, 
			  gboolean doZ)
{
  olong i, j, k, toff;
  ofloat *array = base;

  /* Merging doSpec data too risky */
  if (doSpec) return;
  
  if (doZ) toff = 7;
  else     toff = 6;

  /* Multiply parameterized spectral terms by flux */
  if (doSpec) {
    j = 0;
    while (j<number) {
      for (k=toff; k<size; k++) 
	array[j*size+k] *=  array[j*size+2];
      j++;
    }
  }

  i = 0;
  while (i<number) {
    j=i+1;
    while (j<number) {
      if ((array[j*size]!=array[i*size]) || (array[j*size+1]!=array[i*size+1]))
	break;
      /* only combine like sized Gaussians (and same DELTAZ) */
      if ((array[j*size+3]==array[i*size+3]) && 
	  (array[j*size+4]==array[i*size+4]) && 
	  (array[j*size+5]==array[i*size+5])) {
	/* Sum spectral components  flux */
	for (k=toff; k<size; k++) array[i*size+k] += array[j*size+k]; 
	array[i*size+2] += array[j*size+2];  /* sum fluxes */
	array[j*size]    = -1.0e20;          /* Don't need any more */
      }
      j++;
    } /* end finding matches */
    i = j;   /* move on */
  } /* end loop over table */

  /* Normalize parameterized spectra by sum of flux */
  if (doSpec) {
    i = 0;
    while (i<number) {
      if ((array[i*size]>-1.0e-19) && (fabs(array[i*size+3])>0.0)) {
	for (k=toff; k<size; k++) 
	  array[i*size+k] /= array[i*size+2]; 
      }
      i++;
    }
  }

} /* end CCMergeSpec2 */

/**
 * Write valid entries in sort structure
 * \param out     Table write
 * \param base    Base address of sort structure
 * \param size    Size in floats of a sort element
 * \param number  Number of sort elements
 * \param parms   Parms of components
 * \param err     ObitErr error stack.
 * \return I/O Code  OBIT_IO_OK = OK.
 */
static ObitIOCode 
ReWriteTable(ObitTableCC *out, ofloat *base, olong size, olong number, 
	     ofloat *parms, ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  ObitTableCCRow *row = NULL;
  ofloat *entry;
  olong irow, i, nterms, count, toff;
  gboolean doZ;
  gchar *routine = "ReWriteTable";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return retCode;
  g_assert (ObitTableCCIsA(out));

  /* Open table */
  retCode = ObitTableCCOpen (out, OBIT_IO_WriteOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, out->name, retCode);

  /* Does the CC table have a DeltaZ column? */
  doZ = out->DeltaZCol>=0;
  if (doZ) toff = 4;
  else     toff = 3;

   /* Mark as sorted by descending flux */
  out->myDesc->sort[0] = -(out->FluxCol+257);
  out->myDesc->sort[1] = 0;

  /* Fooey!  This one counts */
 ((ObitTableDesc*)out->myIO->myDesc)->sort[0] = -(out->FluxCol+257);
 ((ObitTableDesc*)out->myIO->myDesc)->sort[1] = 0;
  
  /* Need to copy any  spectral terms? */
  if (out->noParms>4) nterms = out->noParms-4;
  else nterms = 0;

  /* Create row structure */
  row = newObitTableCCRow (out);

  /* loop over table */
  retCode = OBIT_IO_OK;
  entry = (ofloat*)base;
  irow  = 0;
  count = 0;
  while (count<number) {

    /* Deleted? */
    if (entry[0]>-1.0e19) {

      /* copy to row */
      row->DeltaX = entry[0];
      row->DeltaY = entry[1];
      row->Flux   = entry[2];
      if (doZ) row->DeltaZ = entry[3];
            /* copy any model parms - only one of these */
      if (out->noParms>0) {
	for (i=0; i<MAX(4, out->noParms); i++) row->parms[i] = parms[i];
      }
      /* any spectral terms - one per entry */
      for (i=0; i<nterms; i++) row->parms[4+i] = entry[toff+i];

      /* Write */
      irow++;
      retCode = ObitTableCCWriteRow (out, irow, row, err);
      if ((retCode != OBIT_IO_OK) || (err->error)) 
	Obit_traceback_val (err, routine, out->name, retCode);
    }
    count++;
    entry += size;  /* pointer in table */
  } /* end loop over file */
  
  /* Close table */
  retCode = ObitTableCCClose (out, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, out->name, retCode);

  /* Release table row */
  row = ObitTableCCRowUnref (row);

  /* Tell what you've done */
  if (err->prtLv>=2) 
    Obit_log_error(err, OBIT_InfoErr,
		   "Merged %d CC components into %d for %s",
		   number, irow, out->name);

  return retCode;
} /* end ReWriteTable */

