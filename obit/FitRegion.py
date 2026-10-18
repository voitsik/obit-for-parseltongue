"""Python Obit FitModel class.

This class contains info about an image fitting region

FitModel Members with python interfaces:

========= ======================================================
name      An optional name for the object.
corner    bottom left corner in selected region of image (0-rel)
dim       dimension of region
peak      peak in region
peakResid peak in region residual after model subtraction
RMSResid  RMS residual
fluxResid Sum of pixel values in residual
nmodel    Number of models
models    Array of FitModels
========= ======================================================
"""
# $Id$
# -----------------------------------------------------------------------
#  Copyright (C) 2007,2019
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

# Obit FitRegion

from . import FInterpolate, FitModel, Image, Obit, OErr, _Obit

# Python shadow class to ObitFitRegion class

# class name in C
myClass = "ObitFitRegion"


class FitRegion(Obit.FitRegion):
    """
    Python Obit FitRegion class.

    This class contains info about an image fitting region

    FitRegion Members with python interfaces:
    """

    def __init__(
        self,
        name="no_name",
        corner=[0, 0],
        dim=[0, 0],
        peak=0.0,
        peakResid=0.0,
        RMSResid=0.0,
        fluxResid=0.0,
        models=[],
    ):
        super().__init__()
        Obit.CreateFitRegion(
            self.this, name, corner, dim, peak, peakResid, RMSResid, fluxResid
        )
        if len(models) > 0:  # Set models if given
            self.nmodel = len(models)
            self.models = models
        self.myClass = myClass

    def __del__(self, DeleteFitRegion=_Obit.DeleteFitRegion):
        if _Obit is not None:
            DeleteFitRegion(self.this)

    def __setattr__(self, name, value):
        if name == "me":
            # Out with the old
            if self.this is not None:
                Obit.FitRegionUnref(Obit.FitRegion_Get_me(self.this))
            # In with the new
            Obit.FitRegion_Set_me(self.this, value)
            return
        # members
        if name == "corner":
            Obit.FitRegionSetCorner(self.me, value)
            return
        if name == "dim":
            Obit.FitRegionSetDim(self.me, value)
            return
        if name == "peak":
            Obit.FitRegionSetPeak(self.me, value)
            return
        if name == "peakResid":
            Obit.FitRegionSetPeakResid(self.me, value)
            return
        if name == "RMSResid":
            Obit.FitRegionSetRMSResid(self.me, value)
            return
        if name == "fluxResid":
            Obit.FitRegionSetFluxResid(self.me, value)
            return
        if name == "nmodel":
            Obit.FitRegionSetNmodel(self.me, value)
            return
        if name == "models":
            nmodel = Obit.FitRegionGetNmodel(self.me)
            # Resize if necessary
            if nmodel != len(value):
                self.nmodel = len(value)
            # Pass one at a time
            i = 0
            for x in value:
                Obit.FitRegionSetModels(self.me, x.me, i)
                i += 1
            return
        self.__dict__[name] = value

    def __getattr__(self, name):
        if not isinstance(self, FitRegion):
            return
        if name == "me":
            return Obit.FitRegion_Get_me(self.this)
        # members
        if name == "corner":
            return Obit.FitRegionGetCorner(self.me)
        if name == "dim":
            return Obit.FitRegionGetDim(self.me)
        if name == "peak":
            return Obit.FitRegionGetPeak(self.me)
        if name == "peakResid":
            return Obit.FitRegionGetPeakResid(self.me)
        if name == "RMSResid":
            return Obit.FitRegionGetRMSResid(self.me)
        if name == "fluxResid":
            return Obit.FitRegionGetFluxResid(self.me)
        if name == "nmodel":
            return Obit.FitRegionGetNmodel(self.me)
        if name == "models":
            # Make into array of python structures
            out = []
            nmodel = Obit.FitRegionGetNmodel(self.me)
            for i in range(0, nmodel):
                tout = FitModel.FitModel("model")
                tout.me = Obit.FitRegionGetModels(self.me, i)
                out.append(tout)
            return out

    def __repr__(self):
        if not isinstance(self, FitRegion):
            return "Bogus Dude" + str(self.__class__)
        return "<C FitRegion instance> " + Obit.FitRegionGetName(self.me)

    def Print(self, ImDesc, file=None):
        """
        Display human readable contents.

        * self     = object with Model to display
        * ImDesc   = Image Descriptor with Beam, etc.
        * file     = if present, the name of a file into which to write
          the information rather than displaying it on the screen
        """
        ################################################################
        # Start output string
        id = ImDesc.Dict
        modelInfo = "\nModel fit for " + id["object"] + "\n"

        # Loop over models
        corner = self.corner
        for imod in self.models:
            modelInfo += imod.Print(ImDesc, corner)

        # Display or log
        if file:
            fd = open(file, "a")
            fd.write(modelInfo)
            fd.close()
        else:
            # pydoc.ttypager(modelInfo)
            print(modelInfo)
        del modelInfo
        # end Print

    # end class FitRegion


def PIsA(inFitRegion):
    """
    Tells if input really a Python Obit FitRegion

    return True, False
    * inFitRegion   = Python FitRegion object
    """
    ################################################################
    if not isinstance(inFitRegion, FitRegion):
        print("Actually is", inFitRegion.__class__)
        return False
    return Obit.FitRegionIsA(inFitRegion.me) != 0
    # end PIsA
