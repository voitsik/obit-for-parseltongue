""" Python Obit SkyModel class

This class contains a sky model and can Fourier transform it
and subtract from or divide into a UV data..
Both FITS and AIPS cataloged files are supported.

SkyModel Members with python interfaces:
List      - used to pass instructions to processing 
Mosaic    - ImageMosaic
"""
# $Id$
#-----------------------------------------------------------------------
#  Copyright (C) 2004-2019
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
#-----------------------------------------------------------------------

# Obit SkyModel
from __future__ import absolute_import
from __future__ import print_function
import Obit, _Obit, OErr, ImageMosaic, InfoList, UV

# Python shadow class to ObitSkyModel class

# class name in C
myClass = "ObitSkyModel"
 
#
class SkyModel(Obit.SkyModel):
    """ Python Obit SkyModel class
    
    This class contains a sky model and can Fourier transform it
    and subtract from or divide into a UV data..
    Both FITS and AIPS cataloged files are supported.
    
    SkyModel Members with python interfaces:
    List      - used to pass instructions to processing 
    Mosaic    - ImageMosaic
    """
    def __init__(self, name) :
        super(SkyModel, self).__init__()
        Obit.CreateSkyModel(self.this, name)
        self.myClass = myClass
    def __del__(self, DeleteSkyModel=_Obit.DeleteSkyModel):
        if _Obit!=None:
            DeleteSkyModel(self.this)
    def __setattr__(self,name,value):
        if name == "me" :
            # Out with the old
            if self.this!=None:
                Obit.SkyModelUnref(Obit.SkyModel_Get_me(self.this))
            # In with the new
            Obit.SkyModel_Set_me(self.this,value)
            return
        if name=="Mosaic":
            PSetMosaic(self,value)
            return
        self.__dict__[name] = value
    def __getattr__(self,name):
        if not isinstance(self, SkyModel):
            return "Bogus dude",str(self.__class__)
        if name == "me" : 
            return Obit.SkyModel_Get_me(self.this)
        # Virtual members
        if name=="List":
            return PGetList(self)
        if name=="Mosaic":
            return PGetMosaic(self)
        raise AttributeError(str(name))
    def __repr__(self):
        if not isinstance(self, SkyModel):
            return "Bogus dude",str(self.__class__)
        return "<C SkyModel instance> " + Obit.SkyModelGetName(self.me)
    def cast(self, toClass):
        """ Casts object pointer to specified class
        
        self     = object whose cast pointer is desired
        toClass  = Class string to cast to
        """
        ################################################################
         # by type first, the base class
        try:
            if Obit.SkyModelIsA(self.me)!=0:
                # no need to recast
                return self
        except:
            pass
        # Try 0ther
        try:
            recast = self.Cast(toClass)
            return recast
        except:
                pass
        # If it gets here bail
        raise TypeError("Unknown type to cast")
    # end cast
    
    # Input structure for subtraction
    cUVSubInput={'structure':['UVSub',[('InData',   'Input UV data'),
                                       ('SkyModel', 'Sky model'),
                                       ('OutData',  'Output uv data'),
                                       ('doCalSelect','Select/calibrate/edit data?'),
                                       ('REPLACE',  'Replace data with model?'),
                                       ('Stokes',   'Stokes parameter, blank-> unchanged from input'),
                                       ('CCVer',    'CC table versions to use [def all 0 => highest]'),
                                       ('BComp',    'Start CC to use per table, 1-rel [def 1 ]'),
                                       ('EComp',    'Highest CC to use per table, 1-rel [def to end]'),
                                       ('BChan',    'First spectral channel selected. [def all]'),
                                       ('EChan',    'Highest spectral channel selected. [def all]'),
                                       ('BIF',      'First IF selected. [def all]'),
                                       ('EIF',      'Highest IF selected. [def all]'),
                                       ('doPol',    '>0 -> calibrate polarization.'),
                                       ('doCalib',  '>0 -> calibrate, 2=> also calibrate Weights'),
                                       ('gainUse',  'SN/CL table version number, 0-> use highest'),
                                       ('flagVer',  'Flag table version, 0-> use highest, <0-> none'),
                                       ('BLVer',    'BL table version, 0> use highest, <0-> none'),
                                       ('BPVer',    'Band pass (BP) table version, 0-> use highest'),
                                       ('Subarray', 'Selected subarray, <=0->all [default all]'),
                                       ('freqID',   'Selected Frequency ID, <=0->all [default all]'),
                                       ('timeRange','Selected timerange in days.  0s -> all'),
                                       ('UVRange',  'Selected UV range in wavelengths. 0s -> all'),
                                       ('Sources',  'Source names selected unless any starts with'),
                                       ('Antennas', 'A list of selected antenna numbers, if any is negative'),
                                       ('corrType', 'Correlation type, 0=cross corr only, 1=both, 2=auto only.'),
                                       ('doBand',   'Band pass application type <0-> none'),
                                       ('Smooth',   'Specifies the type of spectral smoothing'),
                                       ('do3D',     'If 3D imaging wanted. [def false]'),
                                       ('Factor',   'model multiplications factor (-1=>add) [def 1]'),
                                       ('PBCor',    'If TRUE make relative primary beam corrections. [def false]'),
                                       ('antSize',  'Diameter of antennas for PBCor,.[def 25.0]'),
                                       ('minFlux',  'Minimum flux density model or pixel [def -1.0e20]'),
                                       ('Type',     'Model type (ObitSkyModelType) [def OBIT_SkyModel_Comps]'),
                                       ('Mode',     'Model mode (ObitSkyModelMode) [def OBIT_SkyModel_Fastest]'),
                                       ('MODPTFLX', 'Point model flux in Jy, [def 0.0]'),
                                       ('MODPTXOF', 'Point model x offset in deg  [def 0.0]'),
                                       ('MODPTYOF', 'Point model y offset in deg  [def 0.0]'),
                                       ('MODPTYPM', 'Point other parameters  [def all 0.0]')]],
                 # defaults
                 'InData':None,
                 'SkyModel':None,
                 'OutData':None,
                 'doCalSelect':True,
                 'REPLACE':False,
                 'Stokes':'FULL',
                 'CCVer':None,
                 'BComp':None,
                 'EComp':None,
                 'BChan':0,
                 'EChan':0,
                 'BIF':0,
                 'EIF':0,
                 'doPol':False,
                 'doCalib':0,
                 'gainUse':0,
                 'flagVer':-1,
                 'BLVer':-1,
                 'BPVer':-1,
                 'Subarray':0,
                 'freqID':0,
                 'timeRange':[0.0,10.0],
                 'UVRange':[0.0,0.0],
                 'Sources':[""],
                 'Antennas': [0],
                 'corrType':0,
                 'doBand':-1,
                 'Smooth':[0.0, 0.0, 0.0],
                 'do3D':False,
                 'Factor':1.0,
                 'PBCor':False,
                 'antSize':25.0,
                 'minFlux':-1.0e20,
                 'Type':0,
                 'Mode':0,
                 'MODPTFLX':0.0,
                 'MODPTXOF':0.0,
                 'MODPTYOF':0.0,
                 'MODPTYPM':[0.0,0.0,0.0,0.0]}
    
    def SubUV (self, err, input=cUVSubInput):
        """ Fourier transform Sky model and subtract from uv data
        
        A SkyModel is Fourier transformed and subtracted from a uv data
        self    = Python SkyModel object
        err     = Python Obit Error/message stack
        input   = input parameter dictionary
        Use SkyModel.input(input) to review contents of input structure 
        """

        # Save self on inputs
        input["SkyModel"] = self;

        PSubUV(err, input=input)
        # end SubUV
    
    # Input structure for division
    cUVDivInput={'structure':['UVDiv',[('InData',   'Input UV data'),
                                       ('SkyModel', 'Sky model'),
                                       ('OutData',  'Output uv data'),
                                       ('doCalSelect','Select/calibrate/edit data?'),
                                       ('REPLACE',  'Replace data with model?'),
                                       ('Stokes',   'Stokes parameter, blank-> unchanged from input'),
                                       ('CCVer',    'CC table versions to use [def all 0 => highest]'),
                                       ('BComp',    'Start CC to use per table, 1-rel [def 1 ]'),
                                       ('EComp',    'Highest CC to use per table, 1-rel [def to end]'),
                                       ('BChan',    'First spectral channel selected. [def all]'),
                                       ('EChan',    'Highest spectral channel selected. [def all]'),
                                       ('BIF',      'First IF selected. [def all]'),
                                       ('EIF',      'Highest IF selected. [def all]'),
                                       ('doPol',    '>0 -> calibrate polarization.'),
                                       ('doCalib',  '>0 -> calibrate, 2=> also calibrate Weights'),
                                       ('gainUse',  'SN/CL table version number, 0-> use highest'),
                                       ('flagVer',  'Flag table version, 0-> use highest, <0-> none'),
                                       ('BLVer',    'BL table version, 0> use highest, <0-> none'),
                                       ('BPVer',    'Band pass (BP) table version, 0-> use highest'),
                                       ('Subarray', 'Selected subarray, <=0->all [default all]'),
                                       ('freqID',   'Selected Frequency ID, <=0->all [default all]'),
                                       ('timeRange','Selected timerange in days.  0s -> all'),
                                       ('UVRange',  'Selected UV range in wavelengths. 0s -> all'),
                                       ('Sources',  'Source names selected unless any starts with'),
                                       ('Antennas', 'A list of selected antenna numbers, if any is negative'),
                                       ('corrType', 'Correlation type, 0=cross corr only, 1=both, 2=auto only.'),
                                       ('doBand',   'Band pass application type <0-> none'),
                                       ('Smooth',   'Specifies the type of spectral smoothing'),
                                       ('do3D',     'If 3D imaging wanted. [def false]'),
                                       ('Factor',   'model multiplications factor (-1=>add) [def 1]'),
                                       ('PBCor',    'If TRUE make relative primary beam corrections. [def false]'),
                                       ('antSize',  'Diameter of antennas for PBCor,.[def 25.0]'),
                                       ('minFlux',  'Minimum flux density model or pixel [def -1.0e20]'),
                                       ('Type',     'Model type (ObitSkyModelType) [def OBIT_SkyModel_Comps]'),
                                       ('Mode',     'Model mode (ObitSkyModelMode) [def OBIT_SkyModel_Fastest]'),
                                       ('MODPTFLX', 'Point model flux in Jy, [def 0.0]'),
                                       ('MODPTXOF', 'Point model x offset in deg  [def 0.0]'),
                                       ('MODPTYOF', 'Point model y offset in deg  [def 0.0]'),
                                       ('MODPTYPM', 'Point other parameters  [def all 0.0]')]],
                 # defaults
                 'InData':None,
                 'SkyModel':None,
                 'OutData':None,
                 'doCalSelect':True,
                 'REPLACE':False,
                 'Stokes':'FULL',
                 'CCVer':None,
                 'BComp':None,
                 'EComp':None,
                 'BChan':0,
                 'EChan':0,
                 'BIF':0,
                 'EIF':0,
                 'doPol':False,
                 'doCalib':0,
                 'gainUse':0,
                 'flagVer':-1,
                 'BLVer':-1,
                 'BPVer':-1,
                 'Subarray':0,
                 'freqID':0,
                 'timeRange':[0.0,10.0],
                 'UVRange':[0.0,0.0],
                 'Sources':[""],
                 'Antennas': [0],
                 'corrType':0,
                 'doBand':-1,
                 'Smooth':[0.0, 0.0, 0.0],
                 'do3D':False,
                 'Factor':1.0,
                 'PBCor':False,
                 'antSize':25.0,
                 'minFlux':-1.0e20,
                 'Type':0,
                 'Mode':0,
                 'MODPTFLX':0.0,
                 'MODPTXOF':0.0,
                 'MODPTYOF':0.0,
                 'MODPTYPM':[0.0,0.0,0.0,0.0]}
    
    def DivUV (self, err, input=cUVDivInput):
        """ Fourier transform Sky model and divide into uv data
        
        A SkyModel is Fourier transformed and divided into a uv data
        self    = Python SkyModel object
        err     = Python Obit Error/message stack
        input   = input parameter dictionary
        Use SkyModel.input(input) to review contents of input structure 
        """

        # Save self on inputs
        input["SkyModel"] = self;

        PDivUV(err, input=input)
        # end DivUV
    
    # end class SkyModel

# Allow external access to inputs
UVSubInput = SkyModel.cUVSubInput
UVDivInput = SkyModel.cUVDivInput

# Commonly used, dangerous variables
dim=[1,1,1,1,1]
blc=[1,1,1,1,1,1,1]
trc=[0,0,0,0,0,0,0]
err=OErr.OErr()

def input(inputDict):
    """ Print the contents of an input Dictionary

    inputDict = Python Dictionary containing the parameters for a routine
    There should be a member of the dictionary ('structure') with a value
    being a list containing:
    1) The name for which the input is intended (string)
    2) a list of tuples consisting of (parameter name, doc string)
       with an entry for each parameter in the dictionary.
       The display of the the inputs dictionary will be in the order of
       the tuples and display the doc string after the value.
       An example:
       Soln2CalInput={'structure':['Soln2Cal',[('InData','Input OTF'),
                                               ('soln','input soln table version'),
                                               ('oldCal','input cal table version, -1=none'),
                                               ('newCal','output cal table')]],
                      'InData':None, 'soln':0, 'oldCal':-1, 'newCal':0}
    """
    ################################################################
    structure = inputDict['structure']  # Structure information
    print('Inputs for ',structure[0])
    for k,v in structure[1]:
        print('  ',k,' = ',inputDict[k],' : ',v)
        
    # end input

def newObit(name, err):
    """ Create and initialize an SkyModel structure

    Create sky model object
    Returns the Python SkyModel object
    name     = name desired for object (labeling purposes)
    err      = Python Obit Error/message stack
    """
    ################################################################
    out = SkyModel (name)
    return out      # seems OK
    # end newObit

def PCopy (inSkyModel, outSkyModel, err):
    """ Make a shallow copy of input object.

    Makes structure the same as inSkyModel, copies pointers
    inSkyModel  = Python SkyModel object to copy
    outSkyModel = Output Python SkyModel object, must be defined
    err         = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not OErr.OErrIsA(err):
        raise TypeError("err MUST be an OErr")
    #
    smi = inSkyModel.cast(myClass)  # cast pointer
    smo = outSkyModel.cast(myClass)  # cast pointer
    Obit.SkyModelCopy (smi.me, smo.me, err.me)
    if err.isErr:
        OErr.printErrMsg(err, "Error copying SkyModel")
    # end PCopy

def PGetList (inSkyModel):
    """ Return the member InfoList

    returns InfoList
    inSkyModel  = Python SkyModel object
    """
    ################################################################
    sm = inSkyModel.cast(myClass)  # cast pointer
    out    = InfoList.InfoList()
    out.me = Obit.SkyModelGetList(sm.me)
    return out
    # end PGetList

def PGetMosaic (inSkyModel):
    """ Return the member mosaic

    returns ImageMosaic
    inSkyModel  = Python SkyModel object
    """
    ################################################################
    #
    out    = ImageMosaic.ImageMosaic("None", 1)
    sm = inSkyModel.cast(myClass)  # cast pointer
    out.me = Obit.SkyModelGetImageMosaic(sm.me)
    return out
    # end PGetMosaic

def PSetMosaic (inSkyModel, mosaic):
    """ Replace an ImageMosaic in the SkyModel

    inSkyModel  = Python SkyModel object
    mosaic      = Python ImageMosaic to attach
    """
    ################################################################
    # Checks
    if not ImageMosaic.PIsA(mosaic):
        raise TypeError("array MUST be a Python Obit ImageMosaic")
    #
    sm = inSkyModel.cast(myClass)  # cast pointer
    Obit.SkyModelSetImageMosaic(sm.me, mosaic.me)
    # end PSetMosaic

def PCreate (name, mosaic):
    """ Create the parameters and underlying structures of a SkyModel.

    name      = Name to be given to object
                Most control parameters are in InfoList member
    mosaic    = Python ImageMosaic to attach
    """
    ################################################################
    # Checks
    if not ImageMosaic.PIsA(mosaic):
        raise TypeError("uvData MUST be a Python Obit UV")
    #
    out = SkyModel("None");
    out.me = Obit.SkyModelCreate(name, mosaic.me)
    return out;
    # end PCreate

# Subtract a SkyModel from a UV data set
def PSubUV (err, input=UVSubInput):
    """ Fourier transform Sky model and subtract from uv data

    A SkyModel is Fourier transformed and subtracted 
    from InData and written to outData.
    If doCalSelect, selection by channel/IF will specify the output
    err     = Python Obit Error/message stack
    input   = input parameter dictionary
    
    Input dictionary entries:
    InData      = Input UV data,
    SkyModel    = Input SkyModel,
    OutData     = Output uv data,
    doCalSelect = Select/calibrate/edit data?
    REPLACE     = Replace data with model?
    Stokes      = Stokes parameter, blank-> unchanged from input),
    CCVer       = CC table versions to use [def all 0 => highest]
    BComp       = Start CC to use per table, 1-rel [def 1 ]
    EComp       = Highest CC to use per table, 1-rel [def to end]
    BChan       = First spectral channel selected. [def all]),
    EChan       = Highest spectral channel selected. [def all]),
    BIF         = First IF selected. [def all]),
    EIF         = Highest IF selected. [def all]),
    doPol       = >0 -> calibrate polarization.),
    doCalib     = >0 -> calibrate, 2=> also calibrate Weights),
    gainUse     = SN/CL table version number, 0-> use highest),
    flagVer     = Flag table version, 0-> use highest, <0-> none),
    BLVer       = BL table version, 0> use highest, <0-> none),
    BPVer       = Band pass (BP) table version, 0-> use highest),
    Subarray    = Selected subarray, <=0->all [default all]),
    freqID      = Selected Frequency ID, <=0->all [default all]),
    timeRange   = Selected timerange in days. [8 floats] 0s -> all),
    UVRange     = Selected UV range in wavelengths. 0s -> all),
    Sources     = Source names selected unless any starts with),
    Antennas    = A list of selected antenna numbers, if any is negative),
    corrType    = Correlation type, 0=cross corr only, 1=both, 2=auto only.),
    doBand      = Band pass application type <0-> none),
    Smooth      = Specifies the type of spectral smoothing [three floats]
    do3D        = If 3D imaging wanted. [def false]
    Factor      = Model multiplication factor (-1=>add) [def 1]
    PBCor       = If TRUE make relative primary beam corrections. [def false]
    antSize     = Diameter of antennas for PBCor,.[def 25.0]
    minFlux     = Minimum flux density model or pixel [def -1.0e20]
    Type        = Model type (ObitSkyModelType) [def OBIT_SkyModel_Comps]
                  0=CC Comps, 1=Image, 2=Model
    Mode        = Model mode (ObitSkyModelMode) [def OBIT_SkyModel_Fastest]
                  0=fastest, 1=DFT, 2=Grid
    MODPTFLX    = Point model flux in Jy, [def 0.0]')
    MODPTXOF    = Point model x offset in deg  [def 0.0]
    MODPTYOF    = Point model y offset in deg  [def 0.0]
    MODPTYPM    = Point other parameters  [def all 0.0]
              Parm[3] = 0; Point - no other parameters
              Parm[3] = 1; Gaussian on sky:
                  [0:2] = major_axis (asec),  minor_axis (asec),  
                  Rotation of major axis (deg) from east towards north
              Parm[3] = 3; Uniform sphere:
                  [0] =  radius (asec)
   """
    ################################################################
    # Get input parameters
    inData      = input["InData"]
    inSkyModel  = input["SkyModel"]
    outData     = input["OutData"]
    # Checks
    if not UV.PIsA(inData):
        raise TypeError("inData MUST be a Python Obit UV")
    if not UV.PIsA(outData):
        raise TypeError("outData MUST be a Python Obit UV")
    #
    dim = [1,1,1,1,1]
    #
    # Set control values on SkyModel, inData
    dim[0] = 1;
    inInfo = PGetList(inSkyModel)    # 
    uvInfo = inData.List    # 
    InfoList.PPutBoolean (uvInfo, "doCalSelect",  dim, [input["doCalSelect"]], err)
    InfoList.PPutBoolean (inInfo, "REPLACE",      dim, [input["REPLACE"]], err)
    # Put channel selection on uvdata or skymodel depending on doCalSelect
    if input["doCalSelect"]:
        InfoList.PPutInt   (uvInfo, "BChan",           dim, [input["BChan"]],       err)
        InfoList.PPutInt   (uvInfo, "EChan",           dim, [input["EChan"]],       err)
        InfoList.PPutInt   (uvInfo, "BIF",             dim, [input["BIF"]],         err)
        InfoList.PPutInt   (uvInfo, "EIF",             dim, [input["EIF"]],         err)
    else:
        InfoList.PPutInt   (inInfo, "BChan",           dim, [input["BChan"]],       err)
        InfoList.PPutInt   (inInfo, "EChan",           dim, [input["EChan"]],       err)
        InfoList.PPutInt   (inInfo, "BIF",             dim, [input["BIF"]],         err)
        InfoList.PPutInt   (inInfo, "EIF",             dim, [input["EIF"]],         err)
    itemp = int(input["doPol"])
    InfoList.PPutInt   (uvInfo, "doPol",           dim, [itemp],                err)
    InfoList.PPutInt   (uvInfo, "doCalib",         dim, [input["doCalib"]],     err)
    InfoList.PPutInt   (uvInfo, "doBand",          dim, [input["doBand"]],      err)
    InfoList.PPutInt   (uvInfo, "gainUse",         dim, [input["gainUse"]],     err)
    InfoList.PPutInt   (uvInfo, "flagVer",         dim, [input["flagVer"]],     err)
    InfoList.PPutInt   (uvInfo, "BLVer",           dim, [input["BLVer"]],       err)
    InfoList.PPutInt   (uvInfo, "BPVer",           dim, [input["BPVer"]],       err)
    InfoList.PPutInt   (uvInfo, "Subarray",        dim, [input["Subarray"]],    err)
    InfoList.PPutInt   (uvInfo, "freqID",          dim, [input["freqID"]],      err)
    InfoList.PPutInt   (uvInfo, "corrType",        dim, [input["corrType"]],    err)
    dim[0] = 4
    InfoList.PAlwaysPutString (inInfo, "Stokes",  dim, [input["Stokes"]])
    #InfoList.PAlwaysPutString (uvInfo, "Stokes",  dim, [input["Stokes"]])
    dim[0] = 2
    InfoList.PPutFloat (uvInfo, "UVRange",        dim, input["UVRange"],       err)
    dim[0] = 3
    InfoList.PPutFloat (uvInfo, "Smooth",         dim, input["Smooth"],        err)
    dim[0] = 2
    InfoList.PPutFloat (uvInfo, "timeRange",      dim, input["timeRange"],     err)
    dim[0] = len(input["Antennas"])
    InfoList.PAlwaysPutInt   (uvInfo, "Antennas",  dim, input["Antennas"])
    dim[0]=16;dim[1] = len(input["Sources"])
    InfoList.PAlwaysPutString  (uvInfo, "Sources", dim, input["Sources"])
    dim[0]=1;dim[1] = 1
    InfoList.PPutBoolean (inInfo, "do3D",          dim, [input["do3D"]], err)
    InfoList.PPutInt    (inInfo, "ModelType",       dim, [input["Type"]],    err)
    InfoList.PPutInt    (inInfo, "Mode",            dim, [input["Mode"]],    err)
    InfoList.PPutFloat (inInfo, "Factor",          dim, [input["Factor"]],    err)
    InfoList.PPutBoolean (inInfo, "PBCor",         dim, [input["PBCor"]], err)
    InfoList.PPutFloat (inInfo, "antSize",         dim, [input["antSize"]],    err)
    InfoList.PPutFloat (inInfo, "minFlux",         dim, [input["minFlux"]],    err)
    InfoList.PPutFloat (inInfo, "MODPTFLX",        dim, [input["MODPTFLX"]],    err)
    InfoList.PPutFloat (inInfo, "MODPTXOF",        dim, [input["MODPTXOF"]],    err)
    InfoList.PPutFloat (inInfo, "MODPTYOF",        dim, [input["MODPTYOF"]],    err)
    dim[0] = len(input["MODPTYPM"])
    InfoList.PAlwaysPutFloat (inInfo, "MODPTYPM",        dim, input["MODPTYPM"])
    if input["CCVer"]!=None:
        dim[0] = len(input["CCVer"])
        InfoList.PAlwaysPutInt  (inInfo, "CCVer", dim, input["CCVer"])
    if input["BComp"]!=None:
        dim[0] = len(input["BComp"])
        InfoList.PAlwaysPutInt  (inInfo, "BComp", dim, input["BComp"])
    if input["EComp"]!=None:
        dim[0] = len(input["EComp"])
        InfoList.PAlwaysPutInt  (inInfo, "EComp", dim, input["EComp"])
    #
    # show any errors 
    #OErr.printErrMsg(err, "UVSub: Error setting parameters")
    #
    # Do operation
    sm = inSkyModel.cast(myClass)  # cast pointer
    Obit.SkyModelSubUV(sm.me, inData.me, outData.me, err.me)
    if err.isErr:
        OErr.printErrMsg(err, "Error subtraction SkyModel from UV data")
    # end PSubUV

# Divide a SkyModel into a UV data set
def PDivUV (err, input=UVDivInput):
    """ Fourier transform Sky model and divide into uv data

    A SkyModel is Fourier transformed and divided into
    InData and written to outData.
    If doCalSelect, selection by channel/IF will specify the output
    err     = Python Obit Error/message stack
    input   = input parameter dictionary
    
    Input dictionary entries:
    InData      = Input UV data,
    SkyModel    = Input SkyModel,
    OutData     = Output uv data,
    doCalSelect = Select/calibrate/edit data?),
    REPLACE     = Replace data with model?
    Stokes      = Stokes parameter, blank-> unchanged from input),
    CCVer       = CC table versions to use [def all 0 => highest]
    BComp       = Start CC to use per table, 1-rel [def 1 ]
    EComp       = Highest CC to use per table, 1-rel [def to end]
    BChan       = First spectral channel selected. [def all]),
    EChan       = Highest spectral channel selected. [def all]),
    BIF         = First IF selected. [def all]),
    EIF         = Highest IF selected. [def all]),
    doPol       = >0 -> calibrate polarization.),
    doCalib     = >0 -> calibrate, 2=> also calibrate Weights),
    gainUse     = SN/CL table version number, 0-> use highest),
    flagVer     = Flag table version, 0-> use highest, <0-> none),
    BLVer       = BL table version, 0> use highest, <0-> none),
    BPVer       = Band pass (BP) table version, 0-> use highest),
    Subarray    = Selected subarray, <=0->all [default all]),
    freqID      = Selected Frequency ID, <=0->all [default all]),
    timeRange   = Selected timerange in days. [8 floats] 0s -> all),
    UVRange     = Selected UV range in wavelengths. 0s -> all),
    Sources     = Source names selected unless any starts with),
    Antennas    = A list of selected antenna numbers, if any is negative),
    corrType    = Correlation type, 0=cross corr only, 1=both, 2=auto only.),
    doBand      = Band pass application type <0-> none),
    Smooth      = Specifies the type of spectral smoothing [three floats]
    do3D        = If 3D imaging wanted. [def false]
    Factor      = Model multiplication factor (-1=>add) [def 1]
    PBCor       = If TRUE make relative primary beam corrections. [def false]
    antSize     = Diameter of antennas for PBCor,.[def 25.0]
    minFlux     = Minimum flux density model or pixel [def -1.0e20]
    Type        = Model type (ObitSkyModelType) [def OBIT_SkyModel_Comps]
                  0=CC Comps, 1=Image, 2=Model
    Mode        = Model mode (ObitSkyModelMode) [def OBIT_SkyModel_Fastest]
                  0=fastest, 1=DFT, 2=Grid
    MODPTFLX    = Point model flux in Jy, [def 0.0]')
    MODPTXOF    = Point model x offset in deg  [def 0.0]
    MODPTYOF    = Point model y offset in deg  [def 0.0]
    MODPTYPM    = Point other parameters  [def all 0.0]
              Parm[3] = 0; Point - no other parameters
              Parm[3] = 1; Gaussian on sky:
                  [0:2] = major_axis (asec),  minor_axis (asec),  
                   Rotation of major axis (deg) from east towards north
              Parm[3] = 3; Uniform sphere:
                  [0] =  radius (asec)
    """
    ################################################################
    # Get input parameters
    inData      = input["InData"]
    inSkyModel  = input["SkyModel"]
    outData     = input["OutData"]
    # Checks
    if not UV.PIsA(inData):
        raise TypeError("inData MUST be a Python Obit UV")
    if not UV.PIsA(outData):
        raise TypeError("outData MUST be a Python Obit UV")
    #
    #
    dim = [1,1,1,1,1]
    #
    # Set control values on SkyModel/inData
    dim[0] = 1;
    inInfo = PGetList(inSkyModel)    #
    uvInfo = inData.List   # 
    InfoList.PPutBoolean (uvInfo, "doCalSelect",  dim, [input["doCalSelect"]], err)
    InfoList.PPutBoolean (inInfo, "REPLACE",      dim, [input["REPLACE"]], err)
    # Put channel selection on uvdata or skymodel depending on doCalSelect
    if input["doCalSelect"]:
        InfoList.PPutInt   (uvInfo, "BChan",           dim, [input["BChan"]],       err)
        InfoList.PPutInt   (uvInfo, "EChan",           dim, [input["EChan"]],       err)
        InfoList.PPutInt   (uvInfo, "BIF",             dim, [input["BIF"]],         err)
        InfoList.PPutInt   (uvInfo, "EIF",             dim, [input["EIF"]],         err)
    else:
        InfoList.PPutInt   (inInfo, "BChan",           dim, [input["BChan"]],       err)
        InfoList.PPutInt   (inInfo, "EChan",           dim, [input["EChan"]],       err)
        InfoList.PPutInt   (inInfo, "BIF",             dim, [input["BIF"]],         err)
        InfoList.PPutInt   (inInfo, "EIF",             dim, [input["EIF"]],         err)
    itemp = int(input["doPol"])
    InfoList.PPutInt   (uvInfo, "doPol",           dim, [itemp],                err)
    InfoList.PPutInt   (uvInfo, "doCalib",         dim, [input["doCalib"]],     err)
    InfoList.PPutInt   (uvInfo, "doBand",          dim, [input["doBand"]],      err)
    InfoList.PPutInt   (uvInfo, "gainUse",         dim, [input["gainUse"]],     err)
    InfoList.PPutInt   (uvInfo, "flagVer",         dim, [input["flagVer"]],     err)
    InfoList.PPutInt   (uvInfo, "BLVer",           dim, [input["BLVer"]],       err)
    InfoList.PPutInt   (uvInfo, "BPVer",           dim, [input["BPVer"]],       err)
    InfoList.PPutInt   (uvInfo, "Subarray",        dim, [input["Subarray"]],    err)
    InfoList.PPutInt   (uvInfo, "freqID",          dim, [input["freqID"]],      err)
    InfoList.PPutInt   (uvInfo, "corrType",        dim, [input["corrType"]],    err)
    dim[0] = 4
    InfoList.PAlwaysPutString (uvInfo, "Stokes",        dim, [input["Stokes"]])
    dim[0]=16;dim[1] = len(input["Sources"])
    InfoList.PAlwaysPutString (uvInfo, "Sources",       dim, input["Sources"])
    dim[0] = 2
    InfoList.PPutFloat (uvInfo, "UVRange",        dim, input["UVRange"],       err)
    dim[0] = 3
    InfoList.PPutFloat (uvInfo, "Smooth",         dim, input["Smooth"],        err)
    dim[0] = 2
    InfoList.PPutFloat (uvInfo, "timeRange",      dim, input["timeRange"],     err)
    dim[0] = len(input["Antennas"])
    InfoList.PAlwaysPutInt   (uvInfo, "Antennas",  dim, input["Antennas"])
    dim[0]=16;dim[1] = len(input["Sources"])
    InfoList.PAlwaysPutString  (uvInfo, "Sources", dim, input["Sources"])
    dim[0] = 1;dim[1] = 1
    InfoList.PPutBoolean (inInfo, "do3D",          dim, [input["do3D"]], err)
    InfoList.PPutInt    (inInfo, "ModelType",       dim, [input["Type"]],    err)
    InfoList.PPutInt    (inInfo, "Mode",            dim, [input["Mode"]],    err)
    InfoList.PPutFloat (inInfo, "Factor",          dim, [input["Factor"]],    err)
    InfoList.PPutBoolean (inInfo, "PBCor",         dim, [input["PBCor"]], err)
    InfoList.PPutFloat (inInfo, "antSize",         dim, [input["antSize"]],    err)
    InfoList.PPutFloat (inInfo, "minFlux",         dim, [input["minFlux"]],    err)
    InfoList.PPutFloat (inInfo, "MODPTFLX",        dim, [input["MODPTFLX"]],    err)
    InfoList.PPutFloat (inInfo, "MODPTXOF",        dim, [input["MODPTXOF"]],    err)
    InfoList.PPutFloat (inInfo, "MODPTYOF",        dim, [input["MODPTYOF"]],    err)
    dim[0] = len(input["MODPTYPM"])
    InfoList.PAlwaysPutFloat (inInfo, "MODPTYPM",        dim, input["MODPTYPM"])
    if input["CCVer"]!=None:
        dim[0] = len(input["CCVer"])
        InfoList.PAlwaysPutInt  (inInfo, "CCVer", dim, input["CCVer"])
    if input["BComp"]!=None:
        dim[0] = len(input["BComp"])
        InfoList.PAlwaysPutInt  (inInfo, "BComp", dim, input["BComp"])
    if input["EComp"]!=None:
        dim[0] = len(input["EComp"])
        InfoList.PAlwaysPutLon (inInfo, "EComp", dim, input["EComp"])
    #
    # show any errors 
    #OErr.printErrMsg(err, "UVDiv: Error setting parameters")
    #
    # Do operation
    sm = inSkyModel.cast(myClass)  # cast pointer
    Obit.SkyModelDivUV(sm.me, inData.me, outData.me, err.me)
    if err.isErr:
        OErr.printErrMsg(err, "Error dividing SkyModel into UV data")
    # end PDivUV

def PCompressCC (inSkyModel, err):
    """ Compress CC tables

    Compresses CC tables on all Images in mosaic
    inSkyModel  = Python SkyModel object to compress
    err         = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not OErr.OErrIsA(err):
        raise TypeError("err MUST be an OErr")
    #
    smi = inSkyModel.cast(myClass)  # cast pointer
    Obit.SkyModelCompressCC (smi.me, err.me)
    if err.isErr:
        OErr.printErrMsg(err, "Error copying SkyModel")
    # end PCompressCC

def PGetName (inSkyModel):
    """ Tells Image object name (label)

    returns name as character string
    inSkyModel  = Python SkyModel object
    """
    ################################################################
    sm = inSkyModel.cast(myClass)  # cast pointer
    return Obit.SkyModelGetName(sm.me)
    # end PGetName

def PIsA (inSkyModel):
    """ Tells if input really a Python Obit SkyModel

    return True, False
    inSkyModel   = Python SkyModel object
    """
    ################################################################
    # Checks - allow inheritence
    if not str(inSkyModel.__class__).startswith("SkyModel"):
        return False
    sm = inSkyModel.cast(myClass)  # cast pointer
    return Obit.SkyModelIsA(sm)!=0
    # end PIsA
