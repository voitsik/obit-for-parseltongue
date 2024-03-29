/* $Id$            */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2002-2023                                          */
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
#ifndef OBIT_H 
#define OBIT_H 
#include <glib.h>
#include <string.h>
// #include "memwatch.h"  /* For debugging memory problems */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ObitTypes.h"
#include "ObitErr.h"
#include "ObitInfoList.h"
/** 
 * \file Obit.h
 * Base class of Obit library.
 *
 * This class is the virtual base class of most Obit classes.
 * Class hierarchies are generally noted in the names of modules, i.e.
 * Obit is the base class from which (almost) all others are derived.
 * Obit class derivation is by means of nested include files; each class has an
 * include file for the data members and for the class function pointers.
 * These include files include the corresponding includes of their parent class.
 * The functional members are defined in ObitClassDef.h 
 * and the data members in ObitDef.h to allow recursive definition of derived classes. 
 *
 * \section ObitUsage Usage
 * No instances should be created of this class. 
 */
/*-------- Obit: Merx mollis mortibus nuper ------------------*/
/*-------------- enumerations -------------------------------------*/
/*--------------Class definitions-------------------------------------*/
/** 
 * Obit object recognition string.  
 * This is to be the first gint32 of each object.
 * The value in ascii is "Obit"
 */
#define OBIT_ID 0x4f626974

/** Obit base class definition.
 *
 * This is the base class for Obit objects.
 */
typedef struct {
#include "ObitDef.h"    /* actual CLASS definition */
} Obit;

/*---------------Public functions---------------------------*/
/** Public: Constructor. */
Obit* newObit (gchar* name);
/* define type for ClassInfo structure */
typedef gpointer (*newObitFP)(gchar* name);

/** Public: Class initializer. */
void ObitClassInit (void);
typedef void (*ObitClassInitFP) (void);

/** Public: ClassInfo pointer */
gconstpointer ObitGetClass (void);
typedef gconstpointer (*ObitGetClassFP)(void);

/** Public: Copy (deep) constructor. */
Obit* ObitCopy  (Obit *in, Obit *out, ObitErr *err);
typedef gpointer (*ObitCopyFP)   (gpointer in, gpointer out, 
				  ObitErr *err);

/** Public: Copy (shallow) constructor. */
Obit* ObitClone (Obit *in, Obit *out);
typedef gpointer (*ObitCloneFP)  (Obit* in, Obit* out);

/** Public: Ref pointer, increment reference count, return pointer. */
gpointer ObitRef (gpointer in);
typedef  gpointer (*ObitRefFP)   (gpointer in);

/** Public: Unref pointer, decrement reference count and destroy if 0. */
gpointer ObitUnref (gpointer in);
typedef gpointer (*ObitUnrefFP) (gpointer *in);

/** Public: returns TRUE is object is a member of myClassInfo or a 
    derived class. */
gboolean ObitIsA (gpointer in, gconstpointer type);
typedef gboolean (*ObitIsAFP) (gpointer in, gconstpointer class);

/**
 * Returns Obit magic blanking float value
 * This is adopted from AIPS and correcponds to the string 'INDE'
 * \return float magic value
 */
static inline ofloat ObitMagicF (void)
{
  static union FBLANKequiv {
    gchar string[4];
    ofloat fblank;
  } FBLANK;
  FBLANK.string[0] = 'I'; 
  FBLANK.string[1] = 'N'; 
  FBLANK.string[2] = 'D'; 
  FBLANK.string[3] = 'E'; 
  
  return FBLANK.fblank;
} /* end ObitMagicF */

/** Public: trim trailing blanks from string */
void ObitTrimTrail (gchar *str);

/** Public: trim trailing blanks from string - no embedded strings allowed */
void ObitTrimTrailNoBlank (gchar *str);

/** Public: compare strings */
gboolean ObitStrCmp (gchar *str1, gchar *str2, olong maxlen);

/** Public: return today's date as  yyyy-mm-dd*/
gchar* ObitToday (void);

/** 
 * Public: Is GPU support compiled? 
 * If FALSE and GPU is requested, turn it off and give warning
 * \param myInput  ObitInfoList with parameters.
 * \param err ObitErr for any warning.
 * \return TRUE or FALSE depending on HAVE_GPU
 */
static inline gboolean ObitHaveGPU (ObitInfoList *myInput, ObitErr *err) {
#if HAVE_GPU==1  /*  GPU? */
  return TRUE;   /* all good */
#else
  /* See if GPU asked for */
  ObitInfoType type;
  gint32 dim[MAXINFOELEMDIM]= {1,1,1,1,1};
  gboolean doGPU=FALSE, doGPUGrid=FALSE;
  ObitInfoListGetTest(myInput, "doGPU",     &type, dim, &doGPU);
  ObitInfoListGetTest(myInput, "doGPUGrid", &type, dim, &doGPUGrid);
  if (!(doGPU || doGPUGrid)) return FALSE;
  /* Turn off and warn*/
  dim[0] = 1; type = OBIT_bool;
  doGPU=FALSE, doGPUGrid=FALSE;
  ObitInfoListAlwaysPut (myInput, "doGPU",     type, dim, &doGPU);
  ObitInfoListAlwaysPut (myInput, "doGPUGrid", type, dim, &doGPUGrid);
  Obit_log_error(err, OBIT_InfoWarn, "Turned off doGPU, not compiled in.");
  return FALSE;
  #endif
} /* end ObitHaveGPU  */

/** Function pointers for private functions */
typedef void (*ObitInitFP)      (gpointer in);
typedef void (*ObitClearFP)     (gpointer in);

/*----------------------- Class Info ---------------------------------*/
/** Public: Set Class function pointers. */
void ObitClassInfoDefFn (gpointer inClass);
typedef void (*ObitClassInfoDefFnFP) (gpointer inClass);

/**
 * ClassInfo Structure.
 * Contains class name, a pointer to any base class
 * (NULL if none) and function pointers.
 */
typedef struct  {
#include "ObitClassDef.h" /* actual definitions */
} ObitClassInfo; 

/** Public: returns TRUE is object is type or a derived class. */
gboolean ObitInfoIsA (ObitClassInfo* in, ObitClassInfo* type);
typedef gboolean (*ObitInfoIsAFP) (ObitClassInfo* in, ObitClassInfo* type);


#endif /* OBIT_H */ 
