/* $Id$  */
/*--------------------------------------------------------------------*/
/*;  Copyright (C) 2005-2020                                          */
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

#include <math.h>
#include "ObitUVDesc.h"
#include "ObitTableCLUtil.h"
#include "ObitTableNX.h"

/*----------------Obit: Merx mollis mortibus nuper ------------------*/
/**
 * \file ObitTableCLUtil.c
 * ObitTableCL class utility function definitions.
 */

/*----------------------Public functions---------------------------*/


/**
 * Copies CL tables from inUV to outUV with selection in inUV
 * If calibration is selected on inUV, no tables are copied
 * \param inUV     Input UV to copy from
 * \param outUV    Output UV to copy to
 * \param *err     ObitErr error stack.
 * \return I/O Code  OBIT_IO_OK = OK.
 */
ObitIOCode ObitTableCLSelect (ObitUV *inUV, ObitUV *outUV, ObitErr *err)
{
  ObitIOCode retCode = OBIT_IO_SpecErr;
  ObitTableCL    *inTab=NULL, *outTab=NULL;
  ObitTableCLRow *inRow=NULL, *outRow=NULL;
  ObitInfoType type;
  olong itemp, iif, oif, i;
  olong highCLver, iCLver, inCLRow, outCLRow;
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  union ObitInfoListEquiv InfoReal; 
  oint numPol, numIF, numTerm;
  gboolean wanted;
  gchar *CLType = "AIPS CL";
  gchar *routine = "ObitTableCLSelect";

  /* error checks */
  g_assert (ObitErrIsA(err));
  if (err->error) return retCode;
  g_assert (ObitUVIsA(inUV));
  g_assert (ObitUVIsA(outUV));

  /* Calibration selected? */
  type = OBIT_float; InfoReal.flt = 0.0;
  ObitInfoListGetTest(inUV->info, "doCalib", &type, (gint32*)dim, &InfoReal);
  if (type==OBIT_float) itemp = InfoReal.flt + 0.5;
  else itemp = InfoReal.itg;
  if (itemp>0) return  OBIT_IO_OK;  /* Yep, don't copy */

  /* Fully instantiate UV files */
  ObitUVFullInstantiate (inUV, TRUE, err);
  if (err->error )Obit_traceback_val (err, routine, inUV->name, retCode);
  ObitUVFullInstantiate (outUV, FALSE, err);
  if (err->error )Obit_traceback_val (err, routine, outUV->name, retCode);

  /* How many CL tables  */
  highCLver = ObitTableListGetHigh (inUV->tableList, CLType);

  /* Are there any? */
  if (highCLver <= 0) return OBIT_IO_OK;

  /* Loop over CL tables */
  for (iCLver=1; iCLver<=highCLver; iCLver++) {

    /* Get input table */
    numPol = 0;
    numIF  = 0;
    numTerm = 0;
    inTab = 
      newObitTableCLValue (inUV->name, (ObitData*)inUV, &iCLver, OBIT_IO_ReadOnly, 
			   numPol, numIF, numTerm, err);
    if (err->error) Obit_traceback_val (err, routine, inTab->name, retCode);
    /* Find it */
    if (inTab==NULL) continue;  /* No keep looping */

    /* Open input table */
    retCode = ObitTableCLOpen (inTab, OBIT_IO_ReadOnly, err);
    if ((retCode != OBIT_IO_OK) || (err->error))
      Obit_traceback_val (err, routine, inTab->name, retCode);

    /* Delete any old output table */
    retCode = ObitDataZapTable ((ObitData*)outUV, CLType, iCLver, err);
    if (err->error) Obit_traceback_val (err, routine, outUV->name, retCode);
 
    /* Create output table */
    numPol = MIN (2, inUV->mySel->numberPoln);
    numIF  = inUV->mySel->numberIF;
    numTerm = inTab->numTerm;
    outTab = 
      newObitTableCLValue (outUV->name, (ObitData*)outUV, &iCLver, OBIT_IO_WriteOnly, 
			   numPol, numIF, numTerm, err);
    if (err->error) Obit_traceback_val (err, routine, outUV->name, retCode);
    /* Create it? */
    Obit_retval_if_fail((outTab!=NULL), err, retCode,
			"%s: Could not create CL table %d for %s", 
			routine, iCLver, outTab->name);
 
  /* Open output table */
  retCode = ObitTableCLOpen (outTab, OBIT_IO_WriteOnly, err);
  if ((retCode != OBIT_IO_OK) || (err->error))
    Obit_traceback_val (err, routine, inTab->name, retCode);
  
   /* Update header info */
    outTab->revision = inTab->revision;
    outTab->numAnt   = inTab->numAnt;
    outTab->mGMod    = inTab->mGMod;

    /* Set rows */
    inRow  = newObitTableCLRow (inTab);
    outRow = newObitTableCLRow (outTab);
    ObitTableCLSetRow (outTab, outRow, err);
    if (err->error) Obit_traceback_val (err, routine, outTab->name, retCode);

    /* Loop over table copying selected data */
    outCLRow = -1;
    for (inCLRow=1; inCLRow<=inTab->myDesc->nrow; inCLRow++) {
      retCode = ObitTableCLReadRow (inTab, inCLRow, inRow, err);
      if ((retCode != OBIT_IO_OK) || (err->error))
	Obit_traceback_val (err, routine, inUV->name, retCode);
      if (inRow->status==-1) continue;
  
      /* Want this one? */
      wanted = ((inRow->Time >= inUV->mySel->timeRange[0]) && 
		(inRow->Time <= inUV->mySel->timeRange[1]));
      wanted = wanted && ((inUV->mySel->FreqID <= 0) ||
			  (inRow->FreqID==inUV->mySel->FreqID));
      wanted = wanted && ((inUV->mySel->SubA <= 0) ||
			  (inRow->SubA==inUV->mySel->SubA));
      wanted = wanted && ObitUVSelWantAnt(inUV->mySel, inRow->antNo);
      wanted = wanted && ObitUVSelWantSour(inUV->mySel, inRow->SourID);
      if (!wanted) continue;

      /* Copy selected data */
      outRow->Time     = inRow->Time;
      outRow->TimeI    = inRow->TimeI;
      outRow->SourID   = inRow->SourID;
      outRow->antNo    = inRow->antNo;
      outRow->SubA     = inRow->SubA;
      outRow->FreqID   = inRow->FreqID;
      outRow->IFR      = inRow->IFR;
      outRow->atmos    = inRow->atmos;
      outRow->Datmos   = inRow->Datmos;
      for (i=0; i<inTab->numTerm; i++) 
	outRow->GeoDelay[i] = inRow->GeoDelay[i];
      outRow->MBDelay1  = inRow->MBDelay1;
      outRow->clock1    = inRow->clock1;
      outRow->Dclock1   = inRow->Dclock1;
      outRow->dispers1  = inRow->dispers1;
      outRow->Ddispers1 = inRow->Ddispers1;
      oif = 0;
      for (iif=inUV->mySel->startIF-1; 
	   iif<inUV->mySel->startIF+inUV->mySel->numberIF-1;
	   iif++) {
	     outRow-> DopplerOff[oif] = inRow-> DopplerOff[iif];
	     outRow->Real1[oif]   = inRow->Real1[iif];
	     outRow->Imag1[oif]   = inRow->Imag1[iif];
	     outRow->Delay1[oif]  = inRow->Delay1[iif];
	     outRow->Rate1[oif]   = inRow->Rate1[iif];
	     outRow->Weight1[oif] = inRow->Weight1[iif];
	     outRow->RefAnt1[oif] = inRow->RefAnt1[iif];
	     oif++;
	   }
      if (numPol>1) {
	outRow->MBDelay2  = inRow->MBDelay2;
	outRow->clock2    = inRow->clock2;
	outRow->Dclock2   = inRow->Dclock2;
	outRow->dispers2  = inRow->dispers2;
	outRow->Ddispers2 = inRow->Ddispers2;
	oif = 0;
	for (iif=inUV->mySel->startIF-1; 
	     iif<inUV->mySel->startIF+inUV->mySel->numberIF-1;
	     iif++) {
	  outRow->Real2[oif]   = inRow->Real2[iif];
	  outRow->Imag2[oif]   = inRow->Imag2[iif];
	  outRow->Delay2[oif]  = inRow->Delay2[iif];
	  outRow->Rate2[oif]   = inRow->Rate2[iif];
	  outRow->Weight2[oif] = inRow->Weight2[iif];
	  outRow->RefAnt2[oif] = inRow->RefAnt2[iif];
	  oif++;
	}
      } /* End second poln */

      retCode = ObitTableCLWriteRow (outTab, outCLRow, outRow, err);
      if ((retCode != OBIT_IO_OK) || (err->error))
	Obit_traceback_val (err, routine, inUV->name, retCode);
    } /* end loop over rows */
    
    /* Close tables */
    retCode = ObitTableCLClose (inTab, err);
    if ((retCode != OBIT_IO_OK) || (err->error))
      Obit_traceback_val (err, routine, inTab->name, retCode);
    retCode = ObitTableCLClose (outTab, err);
    if ((retCode != OBIT_IO_OK) || (err->error))
      Obit_traceback_val (err, routine, outTab->name, retCode);
 
    /* release table objects */
    inTab  = ObitTableCLUnref(inTab);
    outTab = ObitTableCLUnref(outTab);

    /* release row objects */
    inRow  = ObitTableCLRowUnref(inRow);
    outRow = ObitTableCLRowUnref(outRow);
  } /* end loop over tables */

  return retCode;
} /* end ObitTableCLSelect */

/**
 * Create dummy CL table table (applying will not modify data)
 * \param inUV     Input UV data. Control parameters:
 * \li "solInt"    OBIT_float (1,1,1) Solution interval in sec [def 10 sec].
 * \param outUV    UV with which the output  Table is to be associated
 * \param ver      CL table version
 * \param err      Error stack, returns if not empty.
 * \return Pointer to the newly created ObitTableCL object which is 
 *                 associated with outUV.
 */
ObitTableCL* ObitTableCLGetDummy (ObitUV *inUV, ObitUV *outUV, olong ver, 
				  ObitErr *err)
{
#define MAXANT    300    /* Maximum number of antennas */
  ObitTableCL *outCal=NULL;
  ObitTableCLRow *row=NULL;
  ObitUVDesc *desc=NULL;
  ObitIOAccess access;
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  ObitInfoType type;
  ofloat *rec, solInt, t0, sumTime;
  ofloat lastTime=-1.0, lastSource=-1.0, lastFQID=-1.0, curSource=1.0, curFQID=0.0;
  olong iRow, i, ia, maxant, highVer;
  olong  nTime, SubA=-1, ant1, ant2, lastSubA=-1;
  oint numPol, numIF, numTerm, numAnt;
  gboolean doCalSelect, doFirst=TRUE, someData=FALSE, gotAnt[MAXANT];
  ObitIOCode retCode;
  gchar *tname;
  gchar *routine = "ObitTableCLGetDummy";
 
   /* error checks */
  if (err->error) return outCal;
  g_assert (ObitUVIsA(inUV));
  desc = inUV->myDesc;

  /* Calibration/selection wanted? */ 
  doCalSelect = FALSE;
  ObitInfoListGetTest(inUV->info, "doCalSelect", &type, dim, &doCalSelect);
  if (doCalSelect) access = OBIT_IO_ReadCal;
  else access = OBIT_IO_ReadWrite;

  /* open UV data to fully instantiate if not already open */
  if ((inUV->myStatus==OBIT_Inactive) || (inUV->myStatus==OBIT_Defined)) {
    retCode = ObitUVOpen (inUV, access, err);
    if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);
  }
  t0 = -1.0e20;

  /* Delete output table if extant */
  highVer = ObitTableListGetHigh (inUV->tableList, "AIPS CL");
  if (highVer>=ver) {
    ObitDataZapTable ((ObitData*)outUV, "AIPS CL", ver, err);
    if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);
  }

  /* Create output */
  if (desc->jlocs>=0)  numPol = MIN (2, desc->inaxes[desc->jlocs]);
  else                 numPol = 1;
  if (desc->jlocif>=0) numIF = desc->inaxes[desc->jlocif];
  else                 numIF = 1;
  numTerm= 1;
  numAnt = inUV->myDesc->numAnt[0];/* actually highest antenna number */
  tname  = g_strconcat ("Calibration for: ",inUV->name, NULL);
  outCal = newObitTableCLValue(tname, (ObitData*)outUV, &ver, OBIT_IO_WriteOnly,  
			       numPol, numIF, numTerm, err);
  g_free (tname);
  if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);

  /* Get parameters for calibration */
  /* "Solution interval" default 10 sec */
  solInt = 10.0;
  ObitInfoListGetTest(inUV->info, "solInt", &type, dim, (gpointer*)&solInt);
  solInt /= 86400.0;  /* to days */

  /* Open table */
  if ((ObitTableCLOpen (outCal, OBIT_IO_WriteOnly, err) 
       != OBIT_IO_OK) || (err->error))  { /* error test */
    Obit_log_error(err, OBIT_Error, "%s: ERROR opening input CL table", routine);
    return outCal;
  }

  /* Create Row */
  row = newObitTableCLRow (outCal);

  /* Attach row to output buffer */
  ObitTableCLSetRow (outCal, row, err);
  if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);

  /* Set header values */
  outCal->numAnt    = numAnt;  /* Max. antenna number */

 /* Initialize */
  row->Time   = 0.0;
  row->TimeI  = 0.0;
  row->SourID = 0;
  row->antNo  = 0;
  row->SubA   = 0;
  row->FreqID = 0;
  row->GeoDelay[0]= 0.0;
  row->IFR       = 0.0;
  row->atmos     = 0.0;
  row->Datmos    = 0.0;
  row->MBDelay1  = 0.0;
  row->clock1    = 0.0;
  row->Dclock1   = 0.0;
  row->dispers1  = 0.0;
  row->Ddispers1 = 0.0;
  /* IF dependent things */
  for (i=0; i<numIF; i++) {
    row->Real1[i]   = 1.0;
    row->Imag1[i]   = 0.0;
    row->Rate1[i]   = 0.0;
    row->Delay1[i]  = 0.0;
    row->Weight1[i] = 1.0;
    row->RefAnt1[i] = 0;
  }
  /* Multiple ppolarizations */
  if (numPol>1) {
    row->clock2    = 0.0;
    row->Dclock2   = 0.0;
    row->dispers2  = 0.0;
    row->Ddispers2 = 0.0;
    /* IF dependent things */
    for (i=0; i<numIF; i++) {
      row->Real2[i]   = 1.0;
      row->Imag2[i]   = 0.0;
      row->Rate2[i]   = 0.0;
      row->Delay2[i]  = 0.0;
      row->Weight2[i] = 1.0;
      row->RefAnt2[i] = 0;
    }
  } /* end two poln */
  /* List of antennas found */
  for (i=0; i<MAXANT; i++) gotAnt[i] = FALSE;
  maxant = 0;

  /* loop looking at data data */
  retCode = OBIT_IO_OK;
  sumTime = 0.0;
  nTime   = 0;
  doFirst = TRUE;
  while (retCode == OBIT_IO_OK) {
    
    /* read buffer */
    if (doCalSelect) retCode = ObitUVReadSelect (inUV, NULL, err);
    else retCode = ObitUVRead (inUV, NULL, err);
    /* EOF is OK */
    if (retCode==OBIT_IO_EOF) ObitErrClear(err);
    if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);
    if (retCode==OBIT_IO_EOF) break; /* done? */
    
    /* Record pointer */
    rec = inUV->buffer;
    
    /* First time */
    if (t0<-1.0e10) {
      t0         = rec[inUV->myDesc->iloct];
      if (inUV->myDesc->ilocsu>=0) lastSource = rec[inUV->myDesc->ilocsu];
      if (inUV->myDesc->ilocfq>=0) lastFQID   = rec[inUV->myDesc->ilocfq];
      lastTime   = rec[inUV->myDesc->iloct];
      ObitUVDescGetAnts(inUV->myDesc, rec, &ant1, &ant2, &lastSubA);
    }
    
    /* Loop over buffer */
    for (i=0; i<inUV->myDesc->numVisBuff; i++) {

      /* Accumulation or scan finished? If so, write "calibration".*/
      if (inUV->myDesc->ilocsu>=0) curSource = rec[inUV->myDesc->ilocsu];
      if (inUV->myDesc->ilocfq>=0) curFQID   = rec[inUV->myDesc->ilocfq];
      if ((rec[inUV->myDesc->iloct] > (t0+solInt)) || 
	  (curSource != lastSource) ||  
	  (curFQID != lastFQID)) {
	
	/* Not first time - assume first descriptive parameter never blanked */
	if (nTime>0) {
	  /* if new scan write end of last scan and this time */
	  if ((curSource != lastSource) ||  
	      (curFQID != lastFQID)) {
	    /* Need first entry for scan? */
	    if (doFirst) {
	      doFirst = FALSE;
	      row->Time  = t0;
	      row->TimeI = 0.0;
	      row->SourID = (oint)(lastSource+0.5);
	      row->FreqID = (oint)(lastFQID+0.5);
	      row->SubA   = lastSubA;
	      /* Loop over antennas found */
	      for (ia=1; ia<=maxant; ia++) {
		if (!gotAnt[ia]) continue;
		iRow = -1;
		row->antNo = ia;
		if ((ObitTableCLWriteRow (outCal, iRow, row, err)
		     != OBIT_IO_OK) || (err->error>0)) { 
		  Obit_log_error(err, OBIT_Error, "%s: ERROR writing CL Table file", routine);
		  return outCal;
		}
	      }
	    } else { /* Not first scan */
	      /* values for end of previous scan */
	      row->Time   = lastTime; 
	      row->TimeI  = 0.0;
	      row->SourID = (oint)(lastSource+0.5);
	      row->FreqID = (oint)(lastFQID+0.5);
	      row->SubA   = lastSubA;
	      /* Loop over antennas found */
	      for (ia=1; ia<=maxant; ia++) {
		if (!gotAnt[ia]) continue;
		iRow = -1;
		row->antNo = ia;
		if ((ObitTableCLWriteRow (outCal, iRow, row, err)
		     != OBIT_IO_OK) || (err->error>0)) { 
		  Obit_log_error(err, OBIT_Error, "%s: ERROR writing CL Table file", routine);
		  return outCal;
		}
	      }
	      /* Values for start of next scan */
	      row->Time   = rec[inUV->myDesc->iloct]; 
	      row->TimeI  = 0.0;
	      if (inUV->myDesc->ilocsu>=0) row->SourID = (oint)(rec[inUV->myDesc->ilocsu]+0.5);
	      row->SubA   = SubA;
	    } /* end write beginning of scan value */
	  } else {  /* in middle of scan - use average time */
	    /* Set descriptive info on Row */
	    row->Time  = sumTime/nTime;  /* time */
	    row->TimeI = MAX (0.0, (2.0 * (row->Time - t0)));
	    if (inUV->myDesc->ilocsu>=0) row->SourID = (oint)(rec[inUV->myDesc->ilocsu]+0.5);
	    row->SubA   = SubA;
	  }
      
	  /* Write Cal table */
	  /* Loop over antennas found */
	  row->SubA   = lastSubA;
	  for (ia=1; ia<=maxant; ia++) {
	    if (!gotAnt[ia]) continue;
	    iRow = -1;
	    row->antNo = ia;
	    if ((ObitTableCLWriteRow (outCal, iRow, row, err)
		 != OBIT_IO_OK) || (err->error>0)) { 
	      Obit_log_error(err, OBIT_Error, "%s: ERROR writing CL Table file", routine);
	      return outCal;
	    }
	  }
	  /* initialize accumulators */
	  t0         = rec[inUV->myDesc->iloct];
	  lastSource = curSource;
	  lastFQID   = curFQID;
	  sumTime    = 0.0;
	  nTime      = 0;
	  lastSubA   = -1;
 
	  /* Clear list of antennas found */
	  for (ia=0; ia<maxant; ia++) gotAnt[ia] = FALSE;
	  maxant = 0;

	} /* end of write entry if there is data */
      } /* end write entry */
      
      /* accumulate statistics
	 Antennas etc. */
      ObitUVDescGetAnts(inUV->myDesc, rec, &ant1, &ant2, &SubA);
      if(lastSubA<=0) lastSubA = SubA;
      gotAnt[ant1] = TRUE;
      gotAnt[ant2] = TRUE;
      maxant = MAX (maxant, ant2);
      sumTime += rec[inUV->myDesc->iloct];
      lastTime = rec[inUV->myDesc->iloct];
      nTime++; /* how many data points */
      rec += inUV->myDesc->lrec; /* Data record pointer */
      someData = TRUE;
      
    } /* end loop over buffer load */
  } /* end loop reading/gridding data */
  
  /* Finish up any data in accumulator */
  if (nTime>0) {
    /* Set descriptive info on Row */
    row->Time   = sumTime/nTime;
    row->TimeI  = lastTime - t0;

    /* Write Cal table */
    /* Loop over antennas found */
    for (ia=1; ia<=maxant; ia++) {
      if (!gotAnt[ia]) continue;
      iRow = -1;
      row->antNo = ia;
      if ((ObitTableCLWriteRow (outCal, iRow, row, err)
	   != OBIT_IO_OK) || (err->error>0)) { 
	Obit_log_error(err, OBIT_Error, "%s: ERROR writing CL Table file", routine);
	return outCal;
      }
    }
  } /* End final cal */

  /* Close cal table */
  if ((ObitTableCLClose (outCal, err) 
       != OBIT_IO_OK) || (err->error>0)) { /* error test */
    Obit_log_error(err, OBIT_Error, "%s: ERROR closing CL Table", routine);
    return outCal;
  }
  
  /* Close data */
  retCode = ObitUVClose (inUV, err);
  if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);

  /* Give warning if no data selected */
  if (!someData) Obit_log_error(err, OBIT_InfoWarn, 
				"%s: Warning: NO data selected", routine);

  /* Cleanup */
  row = ObitTableCLRowUnref(row);

  return outCal;
} /* end ObitTableCLGetDummy */
/**
 * Create dummy CL table table from NX table (applying will not modify data)
 * \param inUV     Input UV data. Control parameters:
 * \li "solInt"    OBIT_float (1,1,1) Solution interval in sec [def 10 sec].
 * \param outUV    UV with which the output  Table is to be associated
 * \param ver      CL table version
 * \param err      Error stack, returns if not empty.
 * \return Pointer to the newly created ObitTableCL object which is 
 *                 associated with outUV.
 */
ObitTableCL* ObitTableCLGetDummyNX (ObitUV *inUV, ObitUV *outUV, olong ver, 
				    ObitErr *err)
{
#define MAXANT    300    /* Maximum number of antennas */
  ObitTableCL *outCal=NULL;
  ObitTableCLRow *row=NULL;
  ObitTableNX  *NXTable=NULL;
  ObitTableNXRow *NXRow=NULL;
  ObitUVDesc *desc=NULL;
  ObitIOAccess access;
  gint32 dim[MAXINFOELEMDIM] = {1,1,1,1,1};
  ObitInfoType type;
  ofloat solInt, t1, delta;
  olong iRow, oRow, nTime, i, ia, iT, highVer;
  oint numPol, numIF, numTerm, numAnt;
  gboolean doCalSelect;
  ObitIOCode iretCode;
  gchar *tname;
  gchar *routine = "ObitTableCLGetDummyNX";
 
   /* error checks */
  if (err->error) return outCal;
  g_assert (ObitUVIsA(inUV));
  desc = inUV->myDesc;

  /* Calibration/selection wanted? */ 
  doCalSelect = FALSE;
  ObitInfoListGetTest(inUV->info, "doCalSelect", &type, dim, &doCalSelect);
  if (doCalSelect) access = OBIT_IO_ReadCal;
  else access = OBIT_IO_ReadWrite;

  /* open UV data to fully instantiate if not already open */
  if ((inUV->myStatus==OBIT_Inactive) || (inUV->myStatus==OBIT_Defined)) {
    ObitUVOpen (inUV, access, err);
    if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);
  }

  /* Delete output table if extant */
  highVer = ObitTableListGetHigh (inUV->tableList, "AIPS CL");
  if (highVer>=ver) {
    ObitDataZapTable ((ObitData*)outUV, "AIPS CL", ver, err);
    if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);
  }

  /* Create output */
  if (desc->jlocs>=0)  numPol = MIN (2, desc->inaxes[desc->jlocs]);
  else                 numPol = 1;
  if (desc->jlocif>=0) numIF = desc->inaxes[desc->jlocif];
  else                 numIF = 1;
  numTerm= 1;
  numAnt = inUV->myDesc->numAnt[0];/* actually highest antenna number */
  tname  = g_strconcat ("Calibration for: ",inUV->name, NULL);
  outCal = newObitTableCLValue(tname, (ObitData*)outUV, &ver, OBIT_IO_WriteOnly,  
			       numPol, numIF, numTerm, err);
  g_free (tname);
  if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);

  /* Get parameters for calibration */
  /* "Solution interval" default 10 sec */
  solInt = 10.0;
  ObitInfoListGetTest(inUV->info, "solInt", &type, dim, (gpointer*)&solInt);
  solInt /= 86400.0;  /* to days */

  /* Open NX Table */
  ver = 1;
  NXTable = newObitTableNXValue (inUV->name, (ObitData*)inUV, &ver, OBIT_IO_ReadOnly, err);
  /* Should be there */
  Obit_retval_if_fail((NXTable!=NULL), err, outCal,
		      "%s: iNdeX table does not exist - use UV.PUtilIndex to create", routine);
  /* Open Index table  */
  iretCode = ObitTableNXOpen (NXTable, OBIT_IO_ReadOnly, err);
  if ((iretCode!=OBIT_IO_OK) || (err->error)) /* add traceback,return */
    Obit_traceback_val (err, routine, inUV->name, outCal);
  /* Create row structure */
  NXRow = newObitTableNXRow(NXTable);

  /* Open CL table */
  if ((ObitTableCLOpen (outCal, OBIT_IO_WriteOnly, err) 
       != OBIT_IO_OK) || (err->error))  { /* error test */
    Obit_log_error(err, OBIT_Error, "%s: ERROR opening input CL table", routine);
    return outCal;
  }

  /* Create CL Row */
  row = newObitTableCLRow (outCal);

  /* Attach row to output buffer */
  ObitTableCLSetRow (outCal, row, err);
  if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);

  /* Set header values */
  outCal->numAnt    = numAnt;  /* Max. antenna number */

 /* Initialize */
  row->Time   = 0.0;
  row->TimeI  = 0.0;
  row->SourID = 0;
  row->antNo  = 0;
  row->SubA   = 0;
  row->FreqID = 0;
  row->GeoDelay[0]= 0.0;
  row->IFR       = 0.0;
  row->atmos     = 0.0;
  row->Datmos    = 0.0;
  row->MBDelay1  = 0.0;
  row->clock1    = 0.0;
  row->Dclock1   = 0.0;
  row->dispers1  = 0.0;
  row->Ddispers1 = 0.0;
  /* IF dependent things */
  for (i=0; i<numIF; i++) {
    row->Real1[i]   = 1.0;
    row->Imag1[i]   = 0.0;
    row->Rate1[i]   = 0.0;
    row->Delay1[i]  = 0.0;
    row->Weight1[i] = 1.0;
    row->RefAnt1[i] = 0;
  }
  /* Multiple polarizations */
  if (numPol>1) {
    row->clock2    = 0.0;
    row->Dclock2   = 0.0;
    row->dispers2  = 0.0;
    row->Ddispers2 = 0.0;
    /* IF dependent things */
    for (i=0; i<numIF; i++) {
      row->Real2[i]   = 1.0;
      row->Imag2[i]   = 0.0;
      row->Rate2[i]   = 0.0;
      row->Delay2[i]  = 0.0;
      row->Weight2[i] = 1.0;
      row->RefAnt2[i] = 0;
    }
  } /* end two poln */

  /* loop over NX Table  */
  for (iRow=1; iRow<=NXTable->myDesc->nrow; iRow++) {
    iretCode = ObitTableNXReadRow (NXTable, iRow, NXRow, err);
    if (err->error) Obit_traceback_val (err, routine, inUV->name, outCal);

    /* Timerange */
    t1 = NXRow->Time-0.5*NXRow->TimeI;

    /* Scan info */
    row->SourID = NXRow->SourID;  /* Source */
    row->SubA   = NXRow->SubA;    /* Subarray */
    row->FreqID = NXRow->FreqID;  /* Frequency group ID */

    /* Divvy up scan */
    nTime = MAX (1, (olong)(0.5+(NXRow->TimeI/solInt)));
    delta = NXRow->TimeI/nTime;
    for (iT=0; iT<nTime; iT++) {
      row->Time   = t1 + iT*delta;
      row->TimeI  = delta;
      /* Loop over antennas writing entries */
      for (ia=1; ia<=numAnt; ia++) {
	oRow = -1;
	row->antNo = ia;
	if ((ObitTableCLWriteRow (outCal, oRow, row, err)
	     != OBIT_IO_OK) || (err->error>0)) { 
	  Obit_log_error(err, OBIT_Error, "%s: ERROR writing CL Table file", routine);
	  return outCal;
	}
      }
    } /* end loop over times */
    /* One at end */
      row->Time   = t1 + iT*delta;
       /* Loop over antennas writing entries */
      for (ia=1; ia<=numAnt; ia++) {
	oRow = -1;
	row->antNo = ia;
	if ((ObitTableCLWriteRow (outCal, oRow, row, err)
	     != OBIT_IO_OK) || (err->error>0)) { 
	  Obit_log_error(err, OBIT_Error, "%s: ERROR writing CL Table file", routine);
	  return outCal;
	}
      }
  } /* end loop over NX Table */


  /* Close NX table */
  if ((ObitTableNXClose (NXTable, err) 
       != OBIT_IO_OK) || (err->error>0)) { /* error test */
    Obit_log_error(err, OBIT_Error, "%s: ERROR closing NX Table", routine);
    return outCal;
  }
  
  /* Close cal table */
  if ((ObitTableCLClose (outCal, err) 
       != OBIT_IO_OK) || (err->error>0)) { /* error test */
    Obit_log_error(err, OBIT_Error, "%s: ERROR closing CL Table", routine);
    return outCal;
  }
  
  /* Cleanup */
  row   = ObitTableCLRowUnref(row);
  NXRow = ObitTableNXRowUnref(NXRow);
  NXTable = ObitTableNXUnref(NXTable);

  return outCal;
} /* end ObitTableCLGetDummyNX */
