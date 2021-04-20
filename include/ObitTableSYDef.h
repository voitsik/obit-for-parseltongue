/* $Id:  $   */
/* DO NOT EDIT - file generated by ObitTables.pl                      */
/*--------------------------------------------------------------------*/
/*;  Copyright (C)  2017                                              */
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
/*  Define the basic components of the ObitTableSY  structure          */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableSYDef.h
 * ObitTableSY structure members for derived classes.
 */
#include "ObitTableDef.h"  /* Parent class definitions */
/** The number of IFs */
oint  nIF;
/** The number of polarizations. */
oint  nPol;
/** The number of antennas in table (max. ant. number in file) */
oint  nAnt;
/** Revision number of the table definition. */
oint  revision;
/** Column offset for The center time. in table record */
olong  TimeOff;
/** Physical column number for The center time. in table record */
olong  TimeCol;
/** Column offset for Time interval of record in table record */
olong  TimeIOff;
/** Physical column number for Time interval of record in table record */
olong  TimeICol;
/** Column offset for [OPTIONAL] Calibration type 0=NOISE_TUBE, 1=SOLAR_FILTER, 2=Other in table record */
olong  CalTypeOff;
/** Physical column number for [OPTIONAL] Calibration type 0=NOISE_TUBE, 1=SOLAR_FILTER, 2=Other in table record */
olong  CalTypeCol;
/** Column offset for Source ID number in table record */
olong  SourIDOff;
/** Physical column number for Source ID number in table record */
olong  SourIDCol;
/** Column offset for Antenna number in table record */
olong  antennaNoOff;
/** Physical column number for Antenna number in table record */
olong  antennaNoCol;
/** Column offset for Subarray number in table record */
olong  SubAOff;
/** Physical column number for Subarray number in table record */
olong  SubACol;
/** Column offset for Frequency id of scan in table record */
olong  FreqIDOff;
/** Physical column number for Frequency id of scan in table record */
olong  FreqIDCol;
/** Column offset for (P_on-P_off)*G  Poln # NPOL ), invalid data fblanked in table record */
olong  PwrDif1Off;
/** Physical column number for (P_on-P_off)*G  Poln # NPOL ), invalid data fblanked in table record */
olong  PwrDif1Col;
/** Column offset for (P_on+P_off)*G  Poln # NPOL ), invalid data fblanked in table record */
olong  PwrSum1Off;
/** Physical column number for (P_on+P_off)*G  Poln # NPOL ), invalid data fblanked in table record */
olong  PwrSum1Col;
/** Column offset for Post switched power gain Poln # NPOL), invalid data fblanked in table record */
olong  Gain1Off;
/** Physical column number for Post switched power gain Poln # NPOL), invalid data fblanked in table record */
olong  Gain1Col;
/** Column offset for (P_on-P_off)*G  Poln # NPOL ), invalid data fblanked in table record */
olong  PwrDif2Off;
/** Physical column number for (P_on-P_off)*G  Poln # NPOL ), invalid data fblanked in table record */
olong  PwrDif2Col;
/** Column offset for (P_on+P_off)*G  Poln # NPOL ), invalid data fblanked in table record */
olong  PwrSum2Off;
/** Physical column number for (P_on+P_off)*G  Poln # NPOL ), invalid data fblanked in table record */
olong  PwrSum2Col;
/** Column offset for Post switched power gain Poln # NPOL), invalid data fblanked in table record */
olong  Gain2Off;
/** Physical column number for Post switched power gain Poln # NPOL), invalid data fblanked in table record */
olong  Gain2Col;
