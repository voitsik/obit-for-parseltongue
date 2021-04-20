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
/*  Define the basic components of the ObitTableCQ  structure          */
/*  This is intended to be included in a class structure definition   */
/**
 * \file ObitTableCQDef.h
 * ObitTableCQ structure members for derived classes.
 */
#include "ObitTableDef.h"  /* Parent class definitions */
/** Revision number of the table definition */
oint  revision;
/** The number of IFs */
oint  numIF;
/** Column offset for Frequency ID {IFQDCQ in AIPSish in table record */
olong  FrqSelOff;
/** Physical column number for Frequency ID {IFQDCQ in AIPSish in table record */
olong  FrqSelCol;
/** Column offset for Subarray number {ISUBCQ in table record */
olong  SubAOff;
/** Physical column number for Subarray number {ISUBCQ in table record */
olong  SubACol;
/** Column offset for Size of FFT in correlator {NFFTCQ in table record */
olong  FFTSizeOff;
/** Physical column number for Size of FFT in correlator {NFFTCQ in table record */
olong  FFTSizeCol;
/** Column offset for No. of channels in correlator{NCHCQ in table record */
olong  numChanOff;
/** Physical column number for No. of channels in correlator{NCHCQ in table record */
olong  numChanCol;
/** Column offset for Spectral averaging factor{NSAVCQ in table record */
olong  SpecAvgOff;
/** Physical column number for Spectral averaging factor{NSAVCQ in table record */
olong  SpecAvgCol;
/** Column offset for Edge frequency {DFRQCQ in table record */
olong  EdgeFreqOff;
/** Physical column number for Edge frequency {DFRQCQ in table record */
olong  EdgeFreqCol;
/** Column offset for Channel bandwidth {DCBWCQ in table record */
olong  ChanBWOff;
/** Physical column number for Channel bandwidth {DCBWCQ in table record */
olong  ChanBWCol;
/** Column offset for Taper function {LTAPCQ in table record */
olong  TaperFnOff;
/** Physical column number for Taper function {LTAPCQ in table record */
olong  TaperFnCol;
/** Column offset for Oversampling factor {NOVSCQ in table record */
olong  OverSampOff;
/** Physical column number for Oversampling factor {NOVSCQ in table record */
olong  OverSampCol;
/** Column offset for Zero-padding factor {NZPDCQ in table record */
olong  ZeroPadOff;
/** Physical column number for Zero-padding factor {NZPDCQ in table record */
olong  ZeroPadCol;
/** Column offset for Filter type {IFLTCQ in table record */
olong  FilterOff;
/** Physical column number for Filter type {IFLTCQ in table record */
olong  FilterCol;
/** Column offset for Time averaging interval {TAVGCQ in table record */
olong  TimeAvgOff;
/** Physical column number for Time averaging interval {TAVGCQ in table record */
olong  TimeAvgCol;
/** Column offset for Quantization (no. of bits per recorded sample){NBITCQ in table record */
olong  numBitsOff;
/** Physical column number for Quantization (no. of bits per recorded sample){NBITCQ in table record */
olong  numBitsCol;
/** Column offset for FFT overlap factor {IOVLCQ in table record */
olong  FFTOverlapOff;
/** Physical column number for FFT overlap factor {IOVLCQ in table record */
olong  FFTOverlapCol;
