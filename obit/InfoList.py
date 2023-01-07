"""Python Obit multidimensional array of float class.

ObitInfoList Linked list of labeled items class.
This facility allows storing arrays of values of the same (native)
data type and retrieving them by name or order number in the list.
This container class is similar to a Python dictionary and is used
extensively in Obit to pass control information.
Most Obit objects contain an InfoList member.

Members:
Dict      - Python dictionary with contents of InfoList
"""
# $Id$
# -----------------------------------------------------------------------
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
# -----------------------------------------------------------------------

# Python shadow class to ObitInfoList class

from . import Obit, OErr, _Obit

# Make sure we have a long type in python3
long = int


class InfoList(Obit.InfoList):
    """
    Python Obit InfoList class.

    ObitInfoList Linked list of labeled items class.
    This facility allows storing arrays of values of the same (native)
    data type and retrieving them by name or order number in the list.
    This container class is similar to a Python dictionary and is used
    extensively in Obit to pass control information.
    Most Obit objects contain an InfoList member.
    """

    def __init__(self):
        super().__init__()
        Obit.CreateInfoList(self.this)

    def __del__(self, DeleteInfoList=_Obit.DeleteInfoList):
        if _Obit is not None:
            DeleteInfoList(self.this)

    def __setattr__(self, name, value):
        if name == "me":
            if value is None:
                raise TypeError("None given for InfoList object")
            Obit.InfoList_Set_me(self.this, value)
            return
        if name == "Dict":
            return PSetDict(self, value)
        self.__dict__[name] = value

    def __getattr__(self, name):
        if name == "me":
            return Obit.InfoList_Get_me(self.this)
        if name == "Dict":
            return PGetDict(self)
        raise AttributeError(name)

    def __repr__(self):
        return "<C InfoList instance>"

    def set(self, name, value, ttype=None):
        """Save a value in an InfoList.

        Set an entry in an InfoList, possibly redefining its type and dimension.

        * self  = input Python InfoList
        * name  = name of desired entry
        * value = value to save, either a scalar integer, float, boolean or string
          or a 1D array of one of these types
          Type and dimensionality determined from value unless ttype is set
        * ttype = data type, "double", "long", None=>type of value
        """
        itype = type(value)
        dim = [1, 1, 1, 1, 1]
        # Local value of the correct type
        tvalue = value
        if ttype:
            if itype == list:
                if ttype == "double":
                    tvalue = []
                    for x in value:
                        tvalue.append(float(x))
                if ttype == "long":
                    tvalue = []
                    for x in value:
                        tvalue.append(long(x))
            else:
                if ttype == "double":
                    tvalue = float(value)
                if ttype == "long":
                    tvalue = long(value)
        itype = type(tvalue)
        # List
        if itype == list:
            itype = type(tvalue[0])  # Type of array element
            dim[0] = long(len(tvalue))
            dim[1] = 1
            # float as double
            if itype == float:
                if ttype == "double":
                    Obit.InfoListAlwaysPutDouble(self.me, name, dim, tvalue)
                else:
                    Obit.InfoListAlwaysPutFloat(self.me, name, dim, tvalue)
                return
                # int/long
            elif itype in (int,):
                if ttype == "long":
                    Obit.InfoListAlwaysPutLong(self.me, name, dim, tvalue)
                else:
                    tvalue = []
                    for x in value:
                        tvalue.append(long(x))
                    Obit.InfoListAlwaysPutInt(self.me, name, dim, tvalue)
                return
                # string
            elif itype == str:
                dim[0] = long(len(tvalue[0]))
                for x in tvalue[0:]:
                    dim[0] = long(max(dim[0], len(x)))
                dim[1] = long(len(value))
                if dim[1] > 1:  # array of strings
                    Obit.InfoListAlwaysPutString(self.me, name, dim, tvalue)
                else:  # single
                    Obit.InfoListAlwaysPutSString(self.me, name, dim, tvalue)
                return
                # boolean
            elif itype == bool:
                Obit.InfoListAlwaysPutBoolean(self.me, name, dim, tvalue)
                PAlwaysPutBoolean(self, name, dim, value)
                return
            # List
            if itype == list:
                dim[1] = long(len(tvalue))
                dim[0] = long(len(tvalue[0]))
                temp = []
                for x in tvalue:
                    for y in x:
                        if ttype == "long":
                            temp.append(int(y))
                        else:
                            temp.append(y)
                jtype = type(tvalue[0][0])  # Type of array of arrays
                # float as double
                if jtype == float:
                    if ttype == "double":
                        Obit.InfoListAlwaysPutDouble(self.me, name, dim, temp)
                    else:
                        Obit.InfoListAlwaysPutFloat(self.me, name, dim, temp)
                    return
                # int/long
                elif jtype in (int,):
                    if ttype == "long":
                        Obit.InfoListAlwaysPutLong(self.me, name, dim, temp)
                    else:
                        ttemp = []
                        for x in temp:
                            tvalue.append(long(x))
                        Obit.InfoListAlwaysPutInt(self.me, name, dim, ttemp)
                    return
                # string
                elif jtype == str:
                    dim[2] = long(len(tvalue[0][0]))
                    for x in value[0][0:]:
                        dim[2] = long(max(dim[2], len(x)))
                    if dim[2] > 1:  # array of strings
                        Obit.InfoListAlwaysPutString(self.me, name, dim, temp)
                    else:  # single
                        Obit.InfoListAlwaysPutSString(self.me, name, dim, temp)
                    return
                # boolean
                elif jtype == bool:
                    Obit.InfoListAlwaysPutBoolean(self.me, name, dim, temp)
                    PAlwaysPutBoolean(self, name, dim, value)
                    return
                else:
                    raise TypeError("Unknown type " + str(jtype))

            else:
                raise TypeError("Unknown type " + str(itype))

        # Scalars
        # float
        elif itype == float:
            if ttype == "double":
                Obit.InfoListAlwaysPutDouble(self.me, name, dim, [tvalue])
            else:
                Obit.InfoListAlwaysPutFloat(self.me, name, dim, [tvalue])
            return
            # int / long
        elif itype in (int,):
            if ttype == "long":
                Obit.InfoListAlwaysPutLong(self.me, name, dim, [long(tvalue)])
            else:
                Obit.InfoListAlwaysPutInt(self.me, name, dim, [long(tvalue)])
            return
            # string
        elif itype == str:
            dim[0] = long(len(value))
            Obit.InfoListAlwaysPutSString(self.me, name, dim, [tvalue])
            return
            # boolean
        elif itype == bool:
            Obit.InfoListAlwaysPutBoolean(self.me, name, dim, [long(tvalue)])
            return
        else:
            raise TypeError(f"Unsupported type {itype}")

    # end set

    def get(self, name):
        """
        Retrieve a value from an InfoList.

        Returns python list containing data:

        =====  ===================
        index  description
        =====  ===================
        0      return code, 0=OK else failed
        1      name
        2      type code: int=2, oint=3, long=4, float=10, double=11, string=14,
               boolean=15
        3      dimension array as list, e.g. [1,1,1,1,1] for scalar
        4      data array
        =====  ===================

        * self     = input Python InfoList
        * name     = name of desired entry
        """
        # Ignore any exception when item not found
        return Obit.InfoListGet(self.me, name)

    # end set

# end InfoList Class


# Commonly used, dangerous variables
dim = [1, 1, 1, 1, 1]


def PCopy(inList):
    """
    Copy list.

    return copy of input InfoList

    * inList    = input Python InfoList
    """
    ################################################################
    out = InfoList()
    out.me = Obit.InfoListCopy(inList.me)
    return out


# end PCopy


def PCopyData(inList, outList):
    """
    Copy all entries from inList to outList.

    * inList    = input Python InfoList
    * outList   = output Python InfoList, previously exists
    """
    ################################################################
    Obit.InfoListCopyData(inList.me, outList.me)
    # end PCopyData


def PRemove(inList, name):
    """
    Remove item name from list.

    * inList    = input Python InfoList
    * name      = name of desired entry
    """
    ################################################################
    Obit.InfoListRemove(inList.me, name)
    # end PRemove


def PItemResize(inList, name, type, dim):
    """
    Change the dimension and/or data type of an entry.

    Create an entry in list if it doesn't previously exist,
    any existing data will be lost.

    * inList   = input Python InfoList

    * name     = name of desired entry
    * type     = data type of object
               * int=2, oint=3, long=4, float=10, double=11, string=14, boolean=15
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
    """
    ################################################################
    Obit.InfoListItemResize(inList, name, type, dim)
    # end PItemResize


def PIsA(inList):
    """
    Tell if input really is InfoList.

    Returns True or False

    * inList    = input Python InfoList
    """
    ################################################################
    # Checks
    if not isinstance(inList, InfoList):
        return False

    return Obit.InfoListIsA(inList.me)
    # end PIsA


def PPutInt(inList, name, dim, data, err):
    """
    Add an long entry, error if conflict.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
                 MUST have 5 entries
    * data     = data as a 1-D array of integers
    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not isinstance(data[0], int):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be list of ints")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    # make sure data is int
    ldata = [int(d) for d in data]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x
    if prod < len(data):
        raise ValueError("more data than defined in dim")

    if prod > len(data):
        raise ValueError("less data than defined in dim")

    Obit.InfoListPutInt(inList.me, name, ldim, ldata, err.me)

    if err.isErr:
        OErr.printErrMsg(err, "Error adding entry")
    # end PPutInt


def PAlwaysPutInt(inList, name, dim, data):
    """
    Add an int entry, changing type/dim of entry if needed.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
                 MUST have 5 entries
    * data     = data as a 1-D array of integers
    """
    ################################################################
    # Checks
    if not isinstance(data[0], int):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be list of ints")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    # make sure data is int
    ldata = [int(d) for d in data]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x

    if prod < len(data):
        raise ValueError("more data than defined in dim")

    if prod > len(data):
        raise ValueError("less data than defined in dim")

    Obit.InfoListAlwaysPutInt(inList.me, name, ldim, ldata)
    # end PAlwaysPutInt


def PPutLong(inList, name, dim, data, err):
    """
    Add an long entry, error if conflict.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
               MUST have 5 entries
    * data     = data as a 1-D array of integers
    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not isinstance(data[0], int):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be list of ints")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    # make sure data is int
    ldata = [int(d) for d in data]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x

    if prod < len(data):
        raise ValueError("more data than defined in dim")

    if prod > len(data):
        raise ValueError("less data than defined in dim")

    Obit.InfoListPutLong(inList.me, name, ldim, ldata, err.me)

    if err.isErr:
        OErr.printErrMsg(err, "Error adding entry")
    # end PPutLong


def PAlwaysPutLong(inList, name, dim, data):
    """
    Add an long entry, changing type/dim of entry if needed.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
                 MUST have 5 entries
    * data     = data as a 1-D array of integers
    """
    ################################################################
    # Checks
    if not isinstance(data[0], int):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be list of ints")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    # make sure data is int
    ldata = [int(d) for d in data]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x

    if prod < len(data):
        raise ValueError("more data than defined in dim")

    if prod > len(data):
        raise ValueError("less data than defined in dim")

    Obit.InfoListAlwaysPutLong(inList.me, name, ldim, ldata)
    # end PAlwaysPutLong


def PPutFloat(inList, name, dim, data, err):
    """
    Add an float entry, error if conflict.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
                 MUST have 5 entries
    * data     = data as a 1-D array of float
    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not isinstance(data[0], float):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be list of floats")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x

    if prod < len(data):
        # print("data, size", prod, data)
        raise ValueError("more data than defined in dim")

    if prod > len(data):
        raise ValueError("less data than defined in dim")

    Obit.InfoListPutFloat(inList.me, name, ldim, data, err.me)

    if err.isErr:
        OErr.printErrMsg(err, "Error adding entry")
    # end PPutFloat


def PAlwaysPutFloat(inList, name, dim, data):
    """
    Add an float entry, changing type/dim of entry if needed.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
               MUST have 5 entries
    * data     = data as a 1-D array of float
    """
    ################################################################
    # Checks
    if not isinstance(data[0], float):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be list of floats")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x
    if prod < len(data):
        raise ValueError("more data than defined in dim")
    if prod > len(data):
        raise ValueError("less data than defined in dim")
    Obit.InfoListAlwaysPutFloat(inList.me, name, ldim, data)
    # end PAlwaysPutFloat


def PPutDouble(inList, name, dim, data, err):
    """
    Add an double entry, error if conflict.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
               MUST have 5 entries
    * data     = data as a 1-D array of double
    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not isinstance(data[0], float):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be list of floats")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x

    if prod < len(data):
        raise ValueError("more data than defined in dim")

    if prod > len(data):
        raise ValueError("less data than defined in dim")

    Obit.InfoListPutDouble(inList.me, name, ldim, data, err.me)

    if err.isErr:
        OErr.printErrMsg(err, "Error adding entry")
    # end  PPutDouble


def PAlwaysPutDouble(inList, name, dim, data):
    """
    Add an integer entry, changing type/dim of entry if needed.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
                 MUST have 5 entries
    * data     = data as a 1-D array of double
    """
    ################################################################
    # Checks
    if not isinstance(data[0], float):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be list of floats")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x

    if prod < len(data):
        raise ValueError("more data than defined in dim")

    if prod > len(data):
        raise ValueError("less data than defined in dim")

    Obit.InfoListAlwaysPutDouble(inList.me, name, ldim, data)
    # end PAlwaysPutDouble


def PPutBoolean(inList, name, dim, data, err):
    """
    Add an boolean entry (1,0), error if conflict.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
                 MUST have 5 entries
    * data     = data as a 1-D array of boolean (1,0)
    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not isinstance(data[0], bool):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be list of bools")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    # make sure data is int
    ldata = [int(d) for d in data]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x

    if prod < len(data):
        raise ValueError("more data than defined in dim")

    if prod > len(data):
        raise ValueError("less data than defined in dim")

    Obit.InfoListPutBoolean(inList.me, name, ldim, ldata, err.me)

    if err.isErr:
        OErr.printErrMsg(err, "Error adding entry")
    # end PPutBoolean


def PAlwaysPutBoolean(inList, name, dim, data):
    """
    Add an boolean entry, changing type/dim of entry if needed.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
                 MUST have 5 entries
    * data     = data as a 1-D array of boolean (1,0)
    """
    ################################################################
    # Checks
    if not isinstance(data[0], bool):
        # print("class is", data[0].__class__)
        raise TypeError("data[0] MUST be bool")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    # make sure data is int
    ldata = [int(d) for d in data]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x

    if prod < len(data):
        raise ValueError("more data than defined in dim")

    if prod > len(data):
        raise ValueError("less data than defined in dim")

    Obit.InfoListAlwaysPutBoolean(inList.me, name, ldim, ldata)
    # end PAlwaysPutBoolean


def PPutString(inList, name, dim, data, err):
    """
    Add an string entry, error if conflict.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
               MUST have 5 entries
    * data     = data as a 1-D array of strings (rectangular char array)
    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not isinstance(data[0], str):
        # print("class is", data[0].__class__)
        raise TypeError("data MUST be a string")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x
    if prod < len(data):
        raise ValueError("more data than defined in dim")

    # Call depends on whether single string, 1 or 2 D arrays
    if dim[1] > 1:  # array of strings
        Obit.InfoListPutString(inList.me, name, ldim, data, err.me)
    else:  # single
        Obit.InfoListPutSString(inList.me, name, ldim, data, err.me)

    if err.isErr:
        OErr.printErrMsg(err, "Error adding entry")
    # end  PPutString


def PAlwaysPutString(inList, name, dim, data):
    """
    Add an String entry, changing type/dim of entry if needed.

    * inList   = input Python InfoList
    * name     = name of desired entry
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
                 MUST have 5 entries
    * data     = data as a 1-D array of (rectangular char array)
    """
    ################################################################
    # Checks
    if not isinstance(data[0], str):
        # print("class is", data[0].__class__)
        raise TypeError("type of data[0] MUST be a string")

    if len(dim) < 5:
        raise ValueError("dim has fewer then 5 entries")

    # make sure dim is int
    ldim = [int(d) for d in dim]

    prod = 1
    for x in dim:
        if x > 0:
            prod = prod * x
    if prod < len(data):
        raise ValueError("more data than defined in dim")

    # Call depends on whether single string, 1 or 2 D arrays
    if dim[1] > 1:  # array of strings
        Obit.InfoListAlwaysPutString(inList.me, name, ldim, data)
    else:  # single
        Obit.InfoListAlwaysPutSString(inList.me, name, ldim, data)
    # end PAlwaysPutString


def PGet(inList, name):
    """
    Return python list containing data.

       0 - return code, 0=OK else failed
       1 - name
       2 - type
       3 -  dimension array
       4 - data array

    * inList   = input Python InfoList
    * type     = data type of object
               * int=2, oint=3, long=4, float=10, double=11, string=14, boolean=15
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
    * data     = data as a multi dimensional array of (rectangular char array)
    """
    ################################################################
    # Checks
    if not PIsA(inList):
        # print("Actually ", inList.__class__)
        raise TypeError(f"inList MUST be a Python Obit InfoList, not {type(inList)}")

    retVal = Obit.InfoListGet(inList.me, name)

    if retVal[0] != 0:
        raise KeyError(f"{name} not found in InfoList")

    return retVal
    # end PGet


def PGetDict(inList):
    """
    Return contents as python dict, each element is:

       0 - type
       1 - dimension array
       2 - data array

    * type     = data type of object
               * int=2, oint=3, long=4, float=10, double=11, string=14, boolean=15
    * dim      = dimensionality of array as list, e.g. [1,1,1,1,1] for scalar
    * data     = data as a multi dimensional array of (rectangular char array)
    * inList   = input Python InfoList
    """
    ################################################################
    # Checks
    if not PIsA(inList):
        # print("Actually ", inList.__class__)
        raise TypeError(f"inList MUST be a Python Obit InfoList, not {type(inList)}")

    return Obit.InfoListGetDict(inList.me)
    # end PGetDict


def PSetDict(outList, inDict):
    """
    Add entries in InfoList from elements of a python dict, each element is
    a list with the following elements:

    =====  ===========
    index  description
    =====  ===========
    0      type code: int=2, oint=3, long=4, float=10, double=11, string=14, boolean=15
    1      dim, dimension array, an list of 5 int
    2      data list, a 1-D list all of the data type given by type
    =====  ===========

    * outList  = output Python InfoList
    * inDict   = input dict strictly in form generated by PGetDict
    """
    ################################################################
    # Checks
    if not PIsA(outList):
        # print("Actually ", outList.__class__)
        raise TypeError("outList MUST be a Python Obit InfoList")
    if not isinstance(inDict, dict):
        raise TypeError("inDict MUST be a Python dict")

    # Loop over dict
    for x in inDict:
        data = inDict[x]
        # print("DEBUG:", x, data)

        # Check that entry is an list
        if not isinstance(data, list):
            # print("Bad entry:", data)
            raise TypeError(f"entry for {x} MUST be a list")

        # Check type (data[0])
        if not isinstance(data[0], int):
            raise TypeError(f"type for {x} MUST be a int")

        # Check dim (data[1])
        if not isinstance(data[1], list):
            raise TypeError(f"dim for {x} MUST be a list of 5 int")

        if not isinstance(data[1][0], int):
            raise TypeError(f"dim for {x} MUST be a list of 5 int")

        # data (data[2]) must be as a list
        if not isinstance(data[2], list):
            raise TypeError(f"data for {x} MUST be a list")

        #
        # Switch on type
        if data[0] in (1, 2, 3, 4):  # int (long)
            PAlwaysPutLong(outList, x, data[1], data[2])
        elif data[0] == 10:  # float
            PAlwaysPutFloat(outList, x, data[1], data[2])
        elif data[0] == 11:  # double
            PAlwaysPutDouble(outList, x, data[1], data[2])
        elif data[0] == 14:  # string
            PAlwaysPutString(outList, x, data[1], data[2])
        elif data[0] == 15:  # boolean
            PAlwaysPutBoolean(outList, x, data[1], data[2])
        else:  # Unknown
            raise TypeError(f"Unknown data type code {data[0]} for {x}")
    # end loop over entries
    # end PSetDict


def PIsA(inList):
    """
    Tell if input really a Python Obit InfoList.

    return True, False

    * inList   = Python InfoList object
    """
    ################################################################
    # Checks
    if not isinstance(inList, InfoList):
        return False
    return Obit.InfoListIsA(inList.me) != 0
    # end PIsA


def PUnref(inList):
    """
    Decrement reference count.

    Decrement reference count which will destroy object if it goes to zero
    Python object stays defined.

    * inList   = Python InfoList object
    """
    ################################################################
    if inList is None:
        return
    # Checks
    if not PIsA(inList):
        raise TypeError("inList MUST be a Python Obit InfoList")

    inList.me = Obit.InfoListUnref(inList.me)
    # end PUnref
