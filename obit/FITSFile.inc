/* $Id$   */  
/*--------------------------------------------------------------------*/
/* Swig module description for FITS file utilities                    */
/*                                                                    */
/*;  Copyright (C) 2007-2019                                          */
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
/*;  Correspondence this software should be addressed as follows:     */
/*;         Internet email: bcotton@nrao.edu.                         */
/*;         Postal address: William Cotton                            */
/*;                         National Radio Astronomy Observatory      */
/*;                         520 Edgemont Road                         */
/*;                         Charlottesville, VA 22903-2475 USA        */
/*--------------------------------------------------------------------*/

%{
#include "ObitFileFITS.h"
#include "ObitFITS.h"
%}


%inline %{
extern long FITSFileExist(long disk, const char *filename, ObitErr *err)
{
  gchar *fullname=NULL, *fullname2=NULL;
  gboolean exist;
  long lexist=0;

  // Full file name
  fullname = ObitFITSFilename ((olong)disk, filename, err);
  if (fullname==NULL) return lexist;

  exist = ObitFileFITSExist (fullname, err);

  // If this didn't work try with ".gz"
  if (!exist) {
    fullname2 = g_strconcat (fullname, ".gz", NULL);
    exist = ObitFileFITSExist (fullname2, err);
  }
  if (exist) lexist = 1;
  else lexist = 0;

  if (fullname)  g_free(fullname);
  if (fullname2) g_free(fullname2);
  return lexist;
} /* end FITSFileExist */

extern long FITSAddDir(const char *dir, ObitErr *err)
{
    return (long)ObitFITSAddDir(dir, err);
} /* end FITSAddDirname */

extern void FITSSetDir(const char *dir, long disk, ObitErr *err)
{
    ObitFITSSetDir(dir, (olong)disk, err);
} /* end FITSSetDirname */

extern long FITSGetNumDisk(ObitErr *err)
{
    return (long)ObitFITSGetNumDisk(err);
} /* end FITSGetNumDisk */

extern char* FITSGetDirname(long disk, ObitErr *err)
{
    return (char*)ObitFITSDirname((olong)disk, err);
} /* end FITSSetDirname */
%}
