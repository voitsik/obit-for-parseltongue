/* $Id$     */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2010,20120                                         */
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
#ifndef OBITUVIMAGERMF_H 
#define OBITUVIMAGERMF_H 

#include "Obit.h"
#include "ObitErr.h"
#include "ObitUV.h"
#include "ObitUVImager.h"
#include "ObitImageMosaicMF.h"

/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitUVImagerMF.h
 * ObitUVImagerMF Class for imaging UV data.
 *
 * This class is derived from the #ObitUVImagerMF class.
 *
  * This class contains the various utilities needed for wideband
 * imaging in the spectral imaging style
 * Fit S = S_0 exp(alpha log(nu/nu_0) + beta log(nu/nu_0)*2)
 * The result is an ImageMosaic which may optionally be flattened into a 
 * single plane.
 * 
 * \section ObitUVImagerMFaccess Creators and Destructors
 * An ObitUVImagerMF will usually be created using ObitUVImagerMFCreate which allows 
 * specifying a name for the object as well as other information.
 *
 * A copy of a pointer to an ObitUVImagerMF should always be made using the
 * #ObitUVImagerMFRef function which updates the reference count in the object.
 * Then whenever freeing an ObitUVImagerMF or changing a pointer, the function
 * #ObitUVImagerMFUnref will decrement the reference count and destroy the object
 * when the reference count hits 0.
 * There is no explicit destructor.
 *
 * \section ObitUVImagerMFparameters Control Parameters
 *
 * The imaging control parameters are passed through the info object 
 * on the uv data, these control both the output image files and the 
 * processing parameters.
 * Output images:
 * \li "Type"  OBIT_int (1,1,1)   Underlying file type, 1=FITS, 2=AIPS
 * \li "Name"  OBIT_string (?,1,1) Name of image, used as AIPS name or to derive FITS filename
 * \li "Class" OBIT_string (?,1,1) Root of class, used as AIPS class or to derive FITS filename
 * \li "Seq"   OBIT_int (1,1,1) Sequence number
 * \li "Disk"  OBIT_int (1,1,1) Disk number for underlying files
 * \li "HalfStoke"   OBIT_boo (1,1,1)   If true, RR/LL are passed in uvwork [def F]
 *                                      else I
 *
 * UVData selection/calibration/editing control
 * \li "MaxBaseline" OBIT_float scalar = maximum baseline length in wavelengths.
 *              Default = 1.0e15.  Output data not flagged by this criteria.
 * \li "MinBaseline" OBIT_float scalar = minimum baseline length in wavelengths.
 *              Default = 1.0e15.Output data not flagged by this criteria.
 * \li "doCalSelect" OBIT_bool (1,1,1) Select/calibrate/edit data?
 * \li "Stokes" OBIT_string (4,1,1) Selected output Stokes parameters:
 *              "    "=> no translation,"I   ","V   ","Q   ", "U   ", 
 *              "IQU ", "IQUV",  "IV  ", "RR  ", "LL  ", "RL  ", "LR  ", 
 *              "HALF" = RR,LL, "FULL"=RR,LL,RL,LR. [default "    "]
 *               In the above 'F' can substitute for "formal" 'I' (both RR+LL).
 * \li "BChan" OBIT_int (1,1,1) First spectral channel selected. [def all]
 * \li "EChan" OBIT_int (1,1,1) Highest spectral channel selected. [def all]
 * \li "BIF"   OBIT_int (1,1,1) First "IF" selected. [def all]
 * \li "EIF"   OBIT_int (1,1,1) Highest "IF" selected. [def all]
 * \li "doPol"   OBIT_int (1,1,1) >0 -> calibrate polarization.
 * \li "doCalib" OBIT_int (1,1,1) >0 -> calibrate, 2=> also calibrate Weights
 * \li "gainUse" OBIT_int (1,1,1) SN/CL table version number, 0-> use highest
 * \li "flagver" OBIT_int (1,1,1) Flag table version, 0-> use highest, <0-> none
 * \li "BLVer"   OBIT_int (1,1,1) BL table version, 0> use highest, <0-> none
 * \li "BPVer"   OBIT_int (1,1,1) Band pass (BP) table version, 0-> use highest
 * \li "Subarray" OBIT_int (1,1,1) Selected subarray, <=0->all [default all]
 * \li "freqID"   OBIT_int (1,1,1) Selected Frequency ID, <=0->all [default all]
 * \li "timeRange" OBIT_float (2,1,1) Selected timerange in days.
 * \li "UVRange" OBIT_float (2,1,1) Selected UV range in kilowavelengths.
 * \li "Sources" OBIT_string (?,?,1) Source names selected unless any starts with
 *                a '-' in which cse all are deselected (with '-' stripped).
 * \li "Antennas" OBIT_int (?,1,1) a list of selected antenna numbers, if any is negative
 *                 then the absolute values are used and the specified antennas are deselected.
 * \li "corrType" OBIT_int (1,1,1) Correlation type, 0=cross corr only, 1=both, 2=auto only.
 * \li "doBand"  OBIT_int (1,1,1) Band pass application type <0-> none
 *     (1) if = 1 then all the bandpass data for each antenna
 *         will be averaged to form a composite bandpass
 *         spectrum, this will then be used to correct the data.
 *     (2) if = 2 the bandpass spectra nearest in time (in a weighted
 *         sense) to the uv data point will be used to correct the data.
 *     (3) if = 3 the bandpass data will be interpolated in time using
 *         the solution weights to form a composite bandpass spectrum,
 *         this interpolated spectrum will then be used to correct the
 *         data.
 *     (4) if = 4 the bandpass spectra nearest in time (neglecting
 *         weights) to the uv data point will be used to correct the
 *         data.
 *     (5) if = 5 the bandpass data will be interpolated in time ignoring
 *         weights to form a composite bandpass spectrum, this
 *         interpolated spectrum will then be used to correct the data.
 * \li "Smooth"  OBIT_float (3,1,1) specifies the type of spectral smoothing
 *        Smooth(1) = type of smoothing to apply:
 *           0 => no smoothing
 *           1 => Hanning
 *           2 => Gaussian
 *           3 => Boxcar
 *           4 => Sinc (i.e. sin(x)/x)
 *         Smooth(2) = the "diameter" of the function, i.e.
 *           width between first nulls of Hanning triangle
 *           and sinc function, FWHM of Gaussian, width of
 *           Boxcar. Defaults (if < 0.1) are 4, 2, 2 and 3
 *           channels for Smooth(1) = 1 - 4.
 *         Smooth(3) = the diameter over which the convolving
 *           function has value - in channels.
 *           Defaults: 1, 3, 1, 4 times Smooth(2) used when
 *
 * Imaging parameters:
 * \li "do3D" OBIT_bool (1,1,1) 3D image, else 2D? [def TRUE]
 * \li "FOV"  OBIT_float (1,1,1)  Field of view (deg) for Mosaic 
 *                If > 0.0 then a mosaic of images will be added to cover this region.
 *                Note: these are in addition to the NField fields added by 
 *                other parameters
 * \li "doFull"  OBIT_boolean (1,1,1) if TRUE, create full field image to cover FOV [def. FALSE]
 * \li "NField"  OBIT_int (1,1,1) Number of fields defined in input,
 *                if unspecified derive from data and FOV
 * \li "xCells"  OBIT_float (?,1,1) Cell spacing in X (asec) for all images,
 *                if unspecified derive from data
 * \li "yCells"  OBIT_float (?,1,1) Cell spacing in Y (asec) for all images,
 *                if unspecified derive from data
 * \li "nx"      OBIT_int (?,1,1) Minimum number of cells in X for NField images
 *                if unspecified derive from data
 * \li "ny"      OBIT_int (?,1,1) Minimum number of cells in Y for NField images
 *                if unspecified derive from data
 * \li "RAShift" OBIT_float (?,1,1) Right ascension shift (AIPS convention) for each field
 *                if unspecified derive from FOV and data
 * \li "DecShift" OBIT_float (?,1,1) Declination for each field
 *                if unspecified derive from FOV and data
 *
 * Outliers to be added:
 * \li "Catalog"  OBIT_string (?,1,1) =    AIPSVZ format catalog for defining outliers, 
 *                   'None'=don't use [default]
 *                   'Default' = use default catalog.
 *                   Assumed in FITSdata disk 1.
 * \li "OutlierDist" OBIT_float (1,1,1) Maximum distance (deg) from center to include 
 *                    outlier fields from Catalog. [default 1 deg]
 * \li "OutlierFlux" OBIT_float (1,1,1) Minimum estimated flux density include outlier fields
 *                    from Catalog. [default 0.1 Jy ]
 * \li "OutlierSI"   OBIT_float (1,1,1) Spectral index to use to convert catalog flux density 
 *                    to observed frequency.  [default = -0.75]
 * \li "OutlierSize" OBIT_int (?,1,1) Width of outlier field in pixels.  [default 50]
 *
 * Weighting parameters on inUV:
 * \li "nuGrid" OBIT_long (1,1,1) = Number of "U" pixels in weighting grid.
 *               [defaults to "nx"]
 * \li "nvGrid" OBIT_int (1,1,1) Number of "V" pixels in weighting grid.
 * \li "WtBox"  OBIT_int (1,1,1) Size of weighting box in cells [def 1]
 * \li "WtFunc" OBIT_int (1,1,1) Weighting convolution function [def. 1]
 *               1=Pill box, 2=linear, 3=exponential, 4=Gaussian
 *               if positive, function is of radius, negative in u and v.
 * \li "xCells" OBIT_float (1,1,1) Image cell spacing in X in asec.
 * \li "yCells" OBIT_float (1,1,1) Image cell spacing in Y in asec.
 * \li "UVTaper" OBIT_float (1,1,1) UV taper width in kilowavelengths. [def. no taper].
 *               NB: If the taper is applied here is should not also be applied
 *               in the imaging step as the taper will be applied to the
 *               output data.
 * \li "Robust" OBIT_float (1,1,1) Briggs robust parameter. [def. 0.0]
 *               < -7 -> Pure Uniform weight, >7 -> Pure natural weight.
 *               Uses AIPS rather than Briggs definition of Robust.
 * \li "WtPower" OBIT_float (1,1,1) Power to raise weights to.  [def = 1.0]
 *               Note: a power of 0.0 sets all the output weights to 1 as modified
 *               by uniform/Tapering weighting.  Applied in determinng weights 
 *               as well as after.
 */

/*--------------Class definitions-------------------------------------*/
/** ObitUVImagerMF Class structure. */
typedef struct {
#include "ObitUVImagerMFDef.h"   /* this class definition */
} ObitUVImagerMF;

/*----------------- Macroes ---------------------------*/
/** 
 * Macro to unreference (and possibly destroy) an ObitUVImagerMF
 * returns a ObitUVImagerMF*.
 * in = object to unreference
 */
#define ObitUVImagerMFUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitUVImagerMF.
 * returns a ObitUVImagerMF*.
 * in = object to reference
 */
#define ObitUVImagerMFRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitUVImagerMFIsA(in) ObitIsA (in, ObitUVImagerMFGetClass())

/*---------------Public functions---------------------------*/
/** Public: Class initializer. */
void ObitUVImagerMFClassInit (void);

/** Public: Default Constructor. */
ObitUVImagerMF* newObitUVImagerMF (gchar* name);

/** Public: Create UVImagerMF object from description in an ObitInfoList */
void ObitUVImagerMFFromInfo (ObitUVImager *out, gchar *prefix, 
			     ObitInfoList *inList, ObitErr *err);

/** Public: ClassInfo pointer */
gconstpointer ObitUVImagerMFGetClass (void);

/** Public: Copy (deep) constructor. */
ObitUVImager* ObitUVImagerMFCopy  (ObitUVImager *inn, ObitUVImager *outt, ObitErr *err);

/** Public: Copy structure. */
void ObitUVImagerMFClone (ObitUVImager *inn, ObitUVImager *outn, ObitErr *err);

/** Public: Create/initialize ObitUVImagerMF structures */
ObitUVImagerMF* ObitUVImagerMFCreate (gchar* name, olong order, ofloat maxFBW, 
				      ofloat alpha, odouble alphaRefF, 
				      ObitUV *uvdata,  ObitErr *err);

/** Public: Create/initialize ObitUVImagerMF structures given mosaic */
ObitUVImagerMF* ObitUVImagerMFCreate2 (gchar* name, olong order, ObitUV *uvdata, 
				       ObitImageMosaicMF *mosaic, ObitErr *err);
/** Public: Add second polarization */
void ObitUVImagerMFAddPol2 (ObitUVImagerMF *in, ObitUV *uvdata2, ObitErr *err);

/** Public: Weight data */
 void ObitUVImagerMFWeight (ObitUVImager *in, ObitErr *err);

/** Public: Shift image for 2D */
void ObitUVImagerMFShifty (ObitUVImager *in, olong *field, gboolean doall, ObitErr *err);

/** Public: Extract information about underlying structures to ObitInfoList */
void ObitUVImagerMFGetInfo (ObitUVImager *inn, gchar *prefix, ObitInfoList *outList, 
			    ObitErr *err);

/** Public: Get number of parallel images */
olong ObitUVImagerMFGetNumPar (ObitUVImager *inn, gboolean doBeam, ObitErr *err);
/** Public: return secondary (UPol) ImageMosaic member */
ObitImageMosaic* ObitUVImagerMFGetMosaic2 (ObitUVImagerMF *in, ObitErr *err);
/** Public: Form secondary (UPol) Image */
void ObitUVImagerMFImage2 (ObitUVImagerMF *in, olong *field, gboolean doWeight, 
			gboolean doBeam, gboolean doFlatten, ObitErr *err);
/*----------- ClassInfo Structure -----------------------------------*/
/**
 * ClassInfo Structure.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitUVImagerMFClassDef.h"
} ObitUVImagerMFClassInfo; 

#endif /* OBITFUVIMAGERMF_H */ 
