/* $Id$   */
/* DO NOT EDIT - file generated by ObitTables.pl                      */
/*--------------------------------------------------------------------*/
/*;  Copyright (C)  2013                                              */
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
#ifndef OBITTABLEIDI_FREQUENCY_H 
#define OBITTABLEIDI_FREQUENCY_H 

#include "Obit.h"
#include "ObitErr.h"
#include "ObitTable.h"
#include "ObitData.h"

/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitTableIDI_FREQUENCY.h
 * ObitTableIDI_FREQUENCY class definition.
 *
 * This class is derived from the #ObitTable class.
 *
 * This class contains tabular data and allows access.
 * This table is part of the IDI uv data format.
 * "IDI\_FREQUENCY" contains frequency related information for ``Bands'' in uv
 * data.
 * A ``Band'' is a construct that allows sets of arbitrarily spaced frequencies.
 * An ObitTableIDI\_FREQUENCY is the front end to a persistent disk resident structure.
 * Only FITS cataloged data are supported.
 * This class is derived from the ObitTable class.
 *
 * This class contains tabular data and allows access.
 * "IDI_FREQUENCY" table
 * An ObitTableIDI_FREQUENCY is the front end to a persistent disk resident structure.
 * Only FITS (as Tables) are supported.
 *
 * \section TableDataStorage Table data storage
 * In memory tables are stored in a fashion similar to how they are 
 * stored on disk - in large blocks in memory rather than structures.
 * Due to the word alignment requirements of some machines, they are 
 * stored by order of the decreasing element size: 
 * double, float long, int, short, char rather than the logical order.
 * The details of the storage in the buffer are kept in the 
 * #ObitTableIDI_FREQUENCYDesc.
 *
 * In addition to the normal tabular data, a table will have a "_status"
 * column to indicate the status of each row.
 *
 * \section ObitTableIDI_FREQUENCYSpecification Specifying desired data transfer parameters
 * The desired data transfers are specified in the member ObitInfoList.
 * In the following an ObitInfoList entry is defined by 
 * the name in double quotes, the data type code as an #ObitInfoType enum 
 * and the dimensions of the array (? => depends on application).
 *
 * The following apply to both types of files:
 * \li "nRowPIO", OBIT_int, Max. Number of visibilities per 
 *     "Read" or "Write" operation.  Default = 1.
 *
 * \subsection TableFITS FITS files
 * This implementation uses cfitsio which allows using, in addition to 
 * regular FITS images, gzip compressed files, pipes, shared memory 
 * and a number of other input forms.
 * The convenience Macro #ObitTableIDI_FREQUENCYSetFITS simplifies specifying the 
 * desired data.
 * Binary tables are used for storing visibility data in FITS.
 * For accessing FITS files the following entries in the ObitInfoList 
 * are used:
 * \li "FileName" OBIT_string (?,1,1) FITS file name.
 * \li "TabName"  OBIT_string (?,1,1) Table name (e.g. "AIPS CC").
 * \li "Ver"      OBIT_int    (1,1,1) Table version number
 *
 *
 * \section ObitTableIDI_FREQUENCYaccess Creators and Destructors
 * An ObitTableIDI_FREQUENCY can be created using newObitTableIDI_FREQUENCYValue which attaches the 
 * table to an ObitData for the object.  
 * If the output ObitTableIDI_FREQUENCY has previously been specified, including file information,
 * then ObitTableIDI_FREQUENCYCopy will copy the disk resident as well as the memory 
 * resident information.
 *
 * A copy of a pointer to an ObitTableIDI_FREQUENCY should always be made using the
 * ObitTableIDI_FREQUENCYRef function which updates the reference count in the object.
 * Then whenever freeing an ObitTableIDI_FREQUENCY or changing a pointer, the function
 * ObitTableIDI_FREQUENCYUnref will decrement the reference count and destroy the object
 * when the reference count hits 0.
 *
 * \section ObitTableIDI_FREQUENCYUsage I/O
 * Visibility data is available after an input object is "Opened"
 * and "Read".
 * I/O optionally uses a buffer attached to the ObitTableIDI_FREQUENCY or some external
 * location.
 * To Write an ObitTableIDI_FREQUENCY, create it, open it, and write.
 * The object should be closed to ensure all data is flushed to disk.
 * Deletion of an ObitTableIDI_FREQUENCY after its final unreferencing will automatically
 * close it.
 */

/*--------------Class definitions-------------------------------------*/

/** Number of characters for Table keyword */
 #define MAXKEYCHARTABLEIDI_FREQUENCY 24

/** ObitTableIDI_FREQUENCY Class structure. */
typedef struct {
#include "ObitTableIDI_FREQUENCYDef.h"   /* this class definition */
} ObitTableIDI_FREQUENCY;

/** ObitTableIDI_FREQUENCYRow Class structure. */
typedef struct {
#include "ObitTableIDI_FREQUENCYRowDef.h"   /* this class row definition */
} ObitTableIDI_FREQUENCYRow;

/*----------------- Macroes ---------------------------*/
/** 
 * Macro to unreference (and possibly destroy) an ObitTableIDI_FREQUENCY
 * returns an ObitTableIDI_FREQUENCY*.
 * in = object to unreference
 */
#define ObitTableIDI_FREQUENCYUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableIDI_FREQUENCY.
 * returns an ObitTableIDI_FREQUENCY*.
 * in = object to reference
 */
#define ObitTableIDI_FREQUENCYRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableIDI_FREQUENCYIsA(in) ObitIsA (in, ObitTableIDI_FREQUENCYGetClass())

/** 
 * Macro to unreference (and possibly destroy) an ObitTableIDI_FREQUENCYRow
 * returns an ObitTableIDI_FREQUENCYRow*.
 * in = object to unreference
 */
#define ObitTableIDI_FREQUENCYRowUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableIDI_FREQUENCYRow.
 * returns an ObitTableIDI_FREQUENCYRow*.
 * in = object to reference
 */
#define ObitTableIDI_FREQUENCYRowRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableIDI_FREQUENCYRowIsA(in) ObitIsA (in, ObitTableIDI_FREQUENCYRowGetClass())

/*---------------Public functions---------------------------*/
/*----------------Table Row Functions ----------------------*/
/** Public: Row Class initializer. */
void ObitTableIDI_FREQUENCYRowClassInit (void);

/** Public: Constructor. */
ObitTableIDI_FREQUENCYRow* newObitTableIDI_FREQUENCYRow (ObitTableIDI_FREQUENCY *table);

/** Public: ClassInfo pointer */
gconstpointer ObitTableIDI_FREQUENCYRowGetClass (void);

/*------------------Table Functions ------------------------*/
/** Public: Class initializer. */
void ObitTableIDI_FREQUENCYClassInit (void);

/** Public: Constructor. */
ObitTableIDI_FREQUENCY* newObitTableIDI_FREQUENCY (gchar* name);

/** Public: Constructor from values. */
ObitTableIDI_FREQUENCY* 
newObitTableIDI_FREQUENCYValue (gchar* name, ObitData *file, olong *ver,
  		     ObitIOAccess access,
                     oint no_band,
		     ObitErr *err);

/** Public: Class initializer. */
void ObitTableIDI_FREQUENCYClassInit (void);

/** Public: ClassInfo pointer */
gconstpointer ObitTableIDI_FREQUENCYGetClass (void);

/** Public: Copy (deep) constructor. */
ObitTableIDI_FREQUENCY* ObitTableIDI_FREQUENCYCopy  (ObitTableIDI_FREQUENCY *in, ObitTableIDI_FREQUENCY *out, 
			   ObitErr *err);

/** Public: Copy (shallow) constructor. */
ObitTableIDI_FREQUENCY* ObitTableIDI_FREQUENCYClone (ObitTableIDI_FREQUENCY *in, ObitTableIDI_FREQUENCY *out);

/** Public: Convert an ObitTable to an ObitTableIDI_FREQUENCY */
ObitTableIDI_FREQUENCY* ObitTableIDI_FREQUENCYConvert  (ObitTable *in);

/** Public: Create ObitIO structures and open file */
ObitIOCode ObitTableIDI_FREQUENCYOpen (ObitTableIDI_FREQUENCY *in, ObitIOAccess access, 
			  ObitErr *err);

/** Public: Read a table row */
ObitIOCode 
ObitTableIDI_FREQUENCYReadRow  (ObitTableIDI_FREQUENCY *in, olong iIDI_FREQUENCYRow, ObitTableIDI_FREQUENCYRow *row,
		     ObitErr *err);

/** Public: Init a table row for write */
void 
ObitTableIDI_FREQUENCYSetRow  (ObitTableIDI_FREQUENCY *in, ObitTableIDI_FREQUENCYRow *row,
		     ObitErr *err);

/** Public: Write a table row */
ObitIOCode 
ObitTableIDI_FREQUENCYWriteRow  (ObitTableIDI_FREQUENCY *in, olong iIDI_FREQUENCYRow, ObitTableIDI_FREQUENCYRow *row,
		     ObitErr *err);

/** Public: Close file and become inactive */
ObitIOCode ObitTableIDI_FREQUENCYClose (ObitTableIDI_FREQUENCY *in, ObitErr *err);

/*----------- ClassInfo Structure -----------------------------------*/
/**
 * ClassInfo Structure.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableIDI_FREQUENCYClassDef.h"
} ObitTableIDI_FREQUENCYClassInfo; 

/**
 * ClassInfo Structure For TableIDI_FREQUENCYRow.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableIDI_FREQUENCYRowClassDef.h"
} ObitTableIDI_FREQUENCYRowClassInfo; 
#endif /* OBITTABLEIDI_FREQUENCY_H */ 
