"""Python Obit FITS file directory utilities."""
# $Id$
# -----------------------------------------------------------------------
#  Copyright (C) 2006,2019
#  Associated Universities, Inc. Washington DC, USA.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, write to the Free
#  Software Foundation, Inc., 675 Massachusetts Ave, Cambridge,
#  MA 02139, USA.
#
#  Correspondence concerning this software should be addressed as follows:
#         Internet email: bcotton@nrao.edu.
#         Postal address: William Cotton
#                         National Radio Astronomy Observatory
#                         520 Edgemont Road
#                         Charlottesville, VA 22903-2475 USA
# -----------------------------------------------------------------------

# Python interface to FITS directory utilities
import os
import pydoc
import string

from . import Obit, OErr

global FITSdisks, nFITS
FITSdisks = []
nFITS = 0

# ObitTalk Stuff
try:
    import FITS

    # Get FITS disks Info from ObitTalk
    nFITS = len(FITS.disks) - 1
    for i in range(1, nFITS + 1):
        FITSdisks.append(FITS.disks[i].dirname)
except Exception:
    # Get list of FITS disks from os
    for dsk in ["FITS", "FITS01", "FITS02", "FITS03", "FITS04", "FITS05", "FITS06"]:
        dir = os.getenv(dsk)
        if dir:
            FITSdisks.append(dir)
            nFITS = len(FITSdisks)
    del dsk, dir


def PListDir(disk, dir=None):
    """
    List files in FITS directory.

    * disk = FITS disk number, <=0 -> current directory
    * dir  = relative or abs. path of directory, def. = cwd
      Only used if disk == 0
    """
    ################################################################
    if disk > 0:
        flist = os.listdir(FITSdisks[disk - 1])
        dirlist = (
            "FITS Directory listing for disk "
            + str(disk)
            + ":"
            + FITSdisks[disk - 1]
            + "\n"
        )
    else:  # current directory
        if dir is None:
            dir = "./"
        flist = os.listdir(dir)
        dirlist = "File listing for directory " + dir + "\n"

    for i, f in enumerate(flist):
        dirlist = dirlist + "{:>6} {}\n".format(i, f)

    # User pager
    pydoc.ttypager(dirlist)
    # end PListDir


def PAddDir(newDir, err, URL=None):
    """
    Add a new FITS directory.

    returns FITS disk number

    * newDir   = new directory path
    * err      = Python Obit Error/message stack
    * URL      = URL if on a remote host (Only if using OTObit/ParselTongue)
    """
    ################################################################
    global FITSdisks, nFITS
    # Checks
    if not OErr.OErrIsA(err):
        raise TypeError("err MUST be an OErr")
    #
    retDisk = Obit.FITSAddDir(newDir, err.me)
    FITSdisks.append(newDir)
    nFITS = len(FITSdisks)
    # print "DEBUG nFITS",nFITS
    if err.isErr:
        OErr.printErrMsg(err, "Error adding FITS directory")
        # Update ObitTalk stuff
    try:
        FITS.FITS.disks.append(FITS.FITSDisk(URL, retDisk, newDir))
    except Exception:
        pass

    return retDisk
    # end PAddDir


def PSetDir(newDir, disk, err, URL=None):
    """
    Replace FITS directory.

    returns FITS disk number

    * newDir   = new directory path
    * err      = Python Obit Error/message stack
    * URL      = URL if on a remote host (Only if using OTObit/ParselTongue)
    """
    ################################################################
    global FITSdisks, nFITS
    # Checks
    if not OErr.OErrIsA(err):
        raise TypeError("err MUST be an OErr")
    #
    retDisk = Obit.FITSSetDir(newDir, disk, err.me)
    FITSdisks[disk] = newDir
    nFITS = len(FITSdisks)
    # print "DEBUG nFITS",nFITS
    if err.isErr:
        OErr.printErrMsg(err, "Error replacinging FITS directory")
        # Update ObitTalk stuff
    try:
        FITS.FITS.disks[disk] = FITS.FITSDisk(URL, disk, newDir)
    except Exception:
        pass
    # end PSetDir


def PGetDir(disk, dir=None):
    """
    Return list of files in FITS directory.

    * disk = FITS disk number <=0 -> current directory
    * dir  = relative or abs. path of directory, def. = cwd
      Only used if disk == 0
    """
    ################################################################
    if disk > 0:
        flist = os.listdir(FITSdisks[disk - 1])
    else:
        if dir is None:
            dir = "./"
        flist = os.listdir(dir)
    return flist
    # end PGetDir


def PExist(file, disk, err):
    """
    Test if FITS file exists.

    return True if exists, else False

    * file     = FITS file name
    * disk     = FITS disk number
    """
    ################################################################
    exist = Obit.FITSFileExist(disk, file, err.me)
    return exist != 0
    # end PExist
