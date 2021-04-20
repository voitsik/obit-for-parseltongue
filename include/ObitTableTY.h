/* $Id$   */
/* DO NOT EDIT - file generated by ObitTables.pl                      */
/*--------------------------------------------------------------------*/
/*;  Copyright (C)  2009                                              */
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
#ifndef OBITTABLETY_H 
#define OBITTABLETY_H 

#include "Obit.h"
#include "ObitErr.h"
#include "ObitTable.h"
#include "ObitData.h"

/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitTableTY.h
 * ObitTableTY class definition.
 *
 * This class is derived from the #ObitTable class.
 *
 * This class contains tabular data and allows access.
 * "AIPS TY" contains Antenna and system temperature information.
 * An ObitTableTY is the front end to a persistent disk resident structure.
 * Both FITS and AIPS cataloged data are supported.
 * This class is derived from the ObitTable class. 
 *
 * This class contains tabular data and allows access.
 * "AIPS TY" table
 * An ObitTableTY is the front end to a persistent disk resident structure.
 * Both FITS (as Tables) and AIPS cataloged data are supported.
 *
 * \section TableDataStorage Table data storage
 * In memory tables are stored in a fashion similar to how they are 
 * stored on disk - in large blocks in memory rather than structures.
 * Due to the word alignment requirements of some machines, they are 
 * stored by order of the decreasing element size: 
 * double, float long, int, short, char rather than the logical order.
 * The details of the storage in the buffer are kept in the 
 * #ObitTableTYDesc.
 *
 * In addition to the normal tabular data, a table will have a "_status"
 * column to indicate the status of each row.
 * The status value is read from and written to (some modification) AIPS 
 * tables but are not written to externally generated FITS tables which
 * don't have these colummns.  It will be written to Obit generated tables
 * which will have these columns.
 * Status values:
 * \li status = 0 => normal
 * \li status = 1 => row has been modified (or created) and needs to be
 *                   written.
 * \li status = -1 => row has been marked invalid.
 *
 * \section ObitTableTYSpecification Specifying desired data transfer parameters
 * The desired data transfers are specified in the member ObitInfoList.
 * There are separate sets of parameters used to specify the FITS or AIPS 
 * data files.
 * In the following an ObitInfoList entry is defined by 
 * the name in double quotes, the data type code as an #ObitInfoType enum 
 * and the dimensions of the array (? => depends on application).
 * To specify whether the underlying data files are FITS or AIPS
 * \li "FileType" OBIT_int (1,1,1) OBIT_IO_FITS or OBIT_IO_AIPS 
 * which are values of an #ObitIOType enum defined in ObitIO.h.
 *
 * The following apply to both types of files:
 * \li "nRowPIO", OBIT_int, Max. Number of visibilities per 
 *     "Read" or "Write" operation.  Default = 1.
 *
 * \subsection TableFITS FITS files
 * This implementation uses cfitsio which allows using, in addition to 
 * regular FITS images, gzip compressed files, pipes, shared memory 
 * and a number of other input forms.
 * The convenience Macro #ObitTableTYSetFITS simplifies specifying the 
 * desired data.
 * Binary tables are used for storing visibility data in FITS.
 * For accessing FITS files the following entries in the ObitInfoList 
 * are used:
 * \li "FileName" OBIT_string (?,1,1) FITS file name.
 * \li "TabName"  OBIT_string (?,1,1) Table name (e.g. "AIPS CC").
 * \li "Ver"      OBIT_int    (1,1,1) Table version number
 *
 * subsection ObitTableTYAIPS AIPS files
 * The ObitAIPS class must be initialized before accessing AIPS files; 
 * this uses #ObitAIPSClassInit.
 * For accessing AIPS files the following entries in the ObitInfoList 
 * are used:
 * \li "Disk" OBIT_int (1,1,1) AIPS "disk" number.
 * \li "User" OBIT_int (1,1,1) user number.
 * \li "CNO"  OBIT_int (1,1,1) AIPS catalog slot number.
 * \li "TableType" OBIT_string (2,1,1) AIPS Table type
 * \li "Ver"  OBIT_int    (1,1,1) AIPS table version number.
 *
 * \section ObitTableTYaccess Creators and Destructors
 * An ObitTableTY can be created using newObitTableTYValue which attaches the 
 * table to an ObitData for the object.  
 * If the output ObitTableTY has previously been specified, including file information,
 * then ObitTableTYCopy will copy the disk resident as well as the memory 
 * resident information.
 *
 * A copy of a pointer to an ObitTableTY should always be made using the
 * ObitTableTYRef function which updates the reference count in the object.
 * Then whenever freeing an ObitTableTY or changing a pointer, the function
 * ObitTableTYUnref will decrement the reference count and destroy the object
 * when the reference count hits 0.
 *
 * \section ObitTableTYUsage I/O
 * Visibility data is available after an input object is "Opened"
 * and "Read".
 * I/O optionally uses a buffer attached to the ObitTableTY or some external
 * location.
 * To Write an ObitTableTY, create it, open it, and write.
 * The object should be closed to ensure all data is flushed to disk.
 * Deletion of an ObitTableTY after its final unreferencing will automatically
 * close it.
 */

/*--------------Class definitions-------------------------------------*/

/** Number of characters for Table keyword */
 #define MAXKEYCHARTABLETY 24

/** ObitTableTY Class structure. */
typedef struct {
#include "ObitTableTYDef.h"   /* this class definition */
} ObitTableTY;

/** ObitTableTYRow Class structure. */
typedef struct {
#include "ObitTableTYRowDef.h"   /* this class row definition */
} ObitTableTYRow;

/*----------------- Macroes ---------------------------*/
/** 
 * Macro to unreference (and possibly destroy) an ObitTableTY
 * returns an ObitTableTY*.
 * in = object to unreference
 */
#define ObitTableTYUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableTY.
 * returns an ObitTableTY*.
 * in = object to reference
 */
#define ObitTableTYRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableTYIsA(in) ObitIsA (in, ObitTableTYGetClass())

/** 
 * Macro to unreference (and possibly destroy) an ObitTableTYRow
 * returns an ObitTableTYRow*.
 * in = object to unreference
 */
#define ObitTableTYRowUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableTYRow.
 * returns an ObitTableTYRow*.
 * in = object to reference
 */
#define ObitTableTYRowRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableTYRowIsA(in) ObitIsA (in, ObitTableTYRowGetClass())

/*---------------Public functions---------------------------*/
/*----------------Table Row Functions ----------------------*/
/** Public: Row Class initializer. */
void ObitTableTYRowClassInit (void);

/** Public: Constructor. */
ObitTableTYRow* newObitTableTYRow (ObitTableTY *table);

/** Public: ClassInfo pointer */
gconstpointer ObitTableTYRowGetClass (void);

/*------------------Table Functions ------------------------*/
/** Public: Class initializer. */
void ObitTableTYClassInit (void);

/** Public: Constructor. */
ObitTableTY* newObitTableTY (gchar* name);

/** Public: Constructor from values. */
ObitTableTY* 
newObitTableTYValue (gchar* name, ObitData *file, olong *ver,
  		     ObitIOAccess access,
                     oint numIF, oint numPol,
		     ObitErr *err);

/** Public: Class initializer. */
void ObitTableTYClassInit (void);

/** Public: ClassInfo pointer */
gconstpointer ObitTableTYGetClass (void);

/** Public: Copy (deep) constructor. */
ObitTableTY* ObitTableTYCopy  (ObitTableTY *in, ObitTableTY *out, 
			   ObitErr *err);

/** Public: Copy (shallow) constructor. */
ObitTableTY* ObitTableTYClone (ObitTableTY *in, ObitTableTY *out);

/** Public: Convert an ObitTable to an ObitTableTY */
ObitTableTY* ObitTableTYConvert  (ObitTable *in);

/** Public: Create ObitIO structures and open file */
ObitIOCode ObitTableTYOpen (ObitTableTY *in, ObitIOAccess access, 
			  ObitErr *err);

/** Public: Read a table row */
ObitIOCode 
ObitTableTYReadRow  (ObitTableTY *in, olong iTYRow, ObitTableTYRow *row,
		     ObitErr *err);

/** Public: Init a table row for write */
void 
ObitTableTYSetRow  (ObitTableTY *in, ObitTableTYRow *row,
		     ObitErr *err);

/** Public: Write a table row */
ObitIOCode 
ObitTableTYWriteRow  (ObitTableTY *in, olong iTYRow, ObitTableTYRow *row,
		     ObitErr *err);

/** Public: Close file and become inactive */
ObitIOCode ObitTableTYClose (ObitTableTY *in, ObitErr *err);

/*----------- ClassInfo Structure -----------------------------------*/
/**
 * ClassInfo Structure.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableTYClassDef.h"
} ObitTableTYClassInfo; 

/**
 * ClassInfo Structure For TableTYRow.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableTYRowClassDef.h"
} ObitTableTYRowClassInfo; 
#endif /* OBITTABLETY_H */ 
