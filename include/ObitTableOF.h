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
#ifndef OBITTABLEOF_H 
#define OBITTABLEOF_H 

#include "Obit.h"
#include "ObitErr.h"
#include "ObitTable.h"
#include "ObitData.h"

/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitTableOF.h
 * ObitTableOF class definition.
 *
 * This class is derived from the #ObitTable class.
 *
 * This class contains tabular data and allows access.
 * "AIPS OF" contains some sort of VLA operational information.
 * An ObitTableOF is the front end to a persistent disk resident structure.
 * This class is derived from the ObitTable class. 
 *
 * This class contains tabular data and allows access.
 * "AIPS OF" table
 * An ObitTableOF is the front end to a persistent disk resident structure.
 * Both FITS (as Tables) and AIPS cataloged data are supported.
 *
 * \section TableDataStorage Table data storage
 * In memory tables are stored in a fashion similar to how they are 
 * stored on disk - in large blocks in memory rather than structures.
 * Due to the word alignment requirements of some machines, they are 
 * stored by order of the decreasing element size: 
 * double, float long, int, short, char rather than the logical order.
 * The details of the storage in the buffer are kept in the 
 * #ObitTableOFDesc.
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
 * \section ObitTableOFSpecification Specifying desired data transfer parameters
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
 * The convenience Macro #ObitTableOFSetFITS simplifies specifying the 
 * desired data.
 * Binary tables are used for storing visibility data in FITS.
 * For accessing FITS files the following entries in the ObitInfoList 
 * are used:
 * \li "FileName" OBIT_string (?,1,1) FITS file name.
 * \li "TabName"  OBIT_string (?,1,1) Table name (e.g. "AIPS CC").
 * \li "Ver"      OBIT_int    (1,1,1) Table version number
 *
 * subsection ObitTableOFAIPS AIPS files
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
 * \section ObitTableOFaccess Creators and Destructors
 * An ObitTableOF can be created using newObitTableOFValue which attaches the 
 * table to an ObitData for the object.  
 * If the output ObitTableOF has previously been specified, including file information,
 * then ObitTableOFCopy will copy the disk resident as well as the memory 
 * resident information.
 *
 * A copy of a pointer to an ObitTableOF should always be made using the
 * ObitTableOFRef function which updates the reference count in the object.
 * Then whenever freeing an ObitTableOF or changing a pointer, the function
 * ObitTableOFUnref will decrement the reference count and destroy the object
 * when the reference count hits 0.
 *
 * \section ObitTableOFUsage I/O
 * Visibility data is available after an input object is "Opened"
 * and "Read".
 * I/O optionally uses a buffer attached to the ObitTableOF or some external
 * location.
 * To Write an ObitTableOF, create it, open it, and write.
 * The object should be closed to ensure all data is flushed to disk.
 * Deletion of an ObitTableOF after its final unreferencing will automatically
 * close it.
 */

/*--------------Class definitions-------------------------------------*/

/** Number of characters for Table keyword */
 #define MAXKEYCHARTABLEOF 24

/** ObitTableOF Class structure. */
typedef struct {
#include "ObitTableOFDef.h"   /* this class definition */
} ObitTableOF;

/** ObitTableOFRow Class structure. */
typedef struct {
#include "ObitTableOFRowDef.h"   /* this class row definition */
} ObitTableOFRow;

/*----------------- Macroes ---------------------------*/
/** 
 * Macro to unreference (and possibly destroy) an ObitTableOF
 * returns an ObitTableOF*.
 * in = object to unreference
 */
#define ObitTableOFUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableOF.
 * returns an ObitTableOF*.
 * in = object to reference
 */
#define ObitTableOFRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableOFIsA(in) ObitIsA (in, ObitTableOFGetClass())

/** 
 * Macro to unreference (and possibly destroy) an ObitTableOFRow
 * returns an ObitTableOFRow*.
 * in = object to unreference
 */
#define ObitTableOFRowUnref(in) ObitUnref (in)

/** 
 * Macro to reference (update reference count) an ObitTableOFRow.
 * returns an ObitTableOFRow*.
 * in = object to reference
 */
#define ObitTableOFRowRef(in) ObitRef (in)

/** 
 * Macro to determine if an object is the member of this or a 
 * derived class.
 * Returns TRUE if a member, else FALSE
 * in = object to reference
 */
#define ObitTableOFRowIsA(in) ObitIsA (in, ObitTableOFRowGetClass())

/*---------------Public functions---------------------------*/
/*----------------Table Row Functions ----------------------*/
/** Public: Row Class initializer. */
void ObitTableOFRowClassInit (void);

/** Public: Constructor. */
ObitTableOFRow* newObitTableOFRow (ObitTableOF *table);

/** Public: ClassInfo pointer */
gconstpointer ObitTableOFRowGetClass (void);

/*------------------Table Functions ------------------------*/
/** Public: Class initializer. */
void ObitTableOFClassInit (void);

/** Public: Constructor. */
ObitTableOF* newObitTableOF (gchar* name);

/** Public: Constructor from values. */
ObitTableOF* 
newObitTableOFValue (gchar* name, ObitData *file, olong *ver,
  		     ObitIOAccess access,
                    
		     ObitErr *err);

/** Public: Class initializer. */
void ObitTableOFClassInit (void);

/** Public: ClassInfo pointer */
gconstpointer ObitTableOFGetClass (void);

/** Public: Copy (deep) constructor. */
ObitTableOF* ObitTableOFCopy  (ObitTableOF *in, ObitTableOF *out, 
			   ObitErr *err);

/** Public: Copy (shallow) constructor. */
ObitTableOF* ObitTableOFClone (ObitTableOF *in, ObitTableOF *out);

/** Public: Convert an ObitTable to an ObitTableOF */
ObitTableOF* ObitTableOFConvert  (ObitTable *in);

/** Public: Create ObitIO structures and open file */
ObitIOCode ObitTableOFOpen (ObitTableOF *in, ObitIOAccess access, 
			  ObitErr *err);

/** Public: Read a table row */
ObitIOCode 
ObitTableOFReadRow  (ObitTableOF *in, olong iOFRow, ObitTableOFRow *row,
		     ObitErr *err);

/** Public: Init a table row for write */
void 
ObitTableOFSetRow  (ObitTableOF *in, ObitTableOFRow *row,
		     ObitErr *err);

/** Public: Write a table row */
ObitIOCode 
ObitTableOFWriteRow  (ObitTableOF *in, olong iOFRow, ObitTableOFRow *row,
		     ObitErr *err);

/** Public: Close file and become inactive */
ObitIOCode ObitTableOFClose (ObitTableOF *in, ObitErr *err);

/*----------- ClassInfo Structure -----------------------------------*/
/**
 * ClassInfo Structure.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableOFClassDef.h"
} ObitTableOFClassInfo; 

/**
 * ClassInfo Structure For TableOFRow.
 * Contains class name, a pointer to any parent class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitTableOFRowClassDef.h"
} ObitTableOFRowClassInfo; 
#endif /* OBITTABLEOF_H */ 
