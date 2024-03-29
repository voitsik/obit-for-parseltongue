# $Id$
# -----------------------------------------------------------------------
#  Copyright (C) 2004,2019
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

# Python shadow class to ObitErr class
from . import InfoList, Obit, _Obit


class OErr(Obit.OErr):
    """
    Python ObitErr message and error stack.

    This is an error stack class for obtaining tracebacks for error conditions.
    This is also the mechanism for passing informative messages.
    No messages, error or informative, are displayed until the contents
    of the stack are explicitly printed
    """

    def __init__(self):
        super().__init__()
        Obit.CreateOErr(self.this)

    def __del__(self, DeleteOErr=_Obit.DeleteOErr):
        if _Obit is not None:
            DeleteOErr(self.this)

    def __setattr__(self, name, value):
        if name == "me":
            Obit.OErr_Set_me(self.this, value)
            return
        self.__dict__[name] = value

    def __getattr__(self, name):
        if name == "me":
            return Obit.OErr_Get_me(self.this)
        if name == "isErr":
            return PIsErr(self)
        raise AttributeError(name)

    def __repr__(self):
        return "<C OErr instance>"

    def __str__(self):
        messages = ""
        msg = Obit.OErrMsg(self.me)
        while msg:
            messages += "%s\n" % msg
            msg = Obit.OErrMsg(self.me)
            continue
        Obit.ObitErrClear(self.me)
        # Clear stack
        return messages

    def Clear(self):
        """Clear Obit error stack."""
        PClear(self)
        # end Clear

    def IsA(self):
        """
        Tells if input really a Python Obit OErr.

        return True, False
        * self   = Python OErr object
        """
        ################################################################
        # Allow derived types
        return Obit.ObitErrIsA(self.me) != 0
        # end IsA


# Error levels
NoErr = 0
Info = 1
Warn = 2
Traceback = 3
MildError = 4
Error = 5
StrongError = 6
Fatal = 7


def PIsErr(err):
    """
    Tell if an error condition exists.

    Returns True if error condition exists, else False

    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not OErrIsA(err):
        # print("calls itself", err, "\n\n")
        raise TypeError("err MUST be a Python ObitErr")
    return Obit.isError(err.me) != 0
    # end PIsErr


def PInit(err, prtLv=0, taskLog="    "):
    """
    Initialize logging.

    * err      = Python Obit Error/message stack to init
    * prtLv    = Message print level, 0=little, 5=LOTS
    * taskLog  = Name of task log file, if given messages go here
      and NOT to the terminal (visible) output.
    """
    ################################################################
    # Checks
    if not OErrIsA(err):
        raise TypeError("err MUST be a Python ObitErr")
    info = InfoList.InfoList()
    info.set("prtLv", prtLv)
    info.set("taskLog", taskLog)
    Obit.ErrorInit(err.me, info.me)
    # end PInit


def PClear(err):
    """
    Clear Obit error stack.

    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not OErrIsA(err):
        raise TypeError("err MUST be a Python ObitErr")
    Obit.ObitErrClear(err.me)
    # end PClear


def PSet(err):
    """
    Set Obit error flag.

    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not OErrIsA(err):
        raise TypeError("err MUST be a Python ObitErr")
    Obit.SetError(err.me)
    # end PSet


def PLog(err, eCode, message):
    """
    Add message To Obit Error/message stack.

    * err      = Python Obit Error/message stack
    * eCode    = error code defined above:
      NoErr, Info, Warn, Traceback,
      MildError, Error, StrongError, Fatal
    """
    ################################################################
    # Checks
    if not OErrIsA(err):
        raise TypeError("err MUST be a Python ObitErr")
    Obit.LogError(err.me, eCode, message)
    # end PLog


def printErr(err):
    """
    Print Obit error stack.

    * err      = Python Obit Error/message stack
    """
    ################################################################
    # Checks
    if not OErrIsA(err):
        raise TypeError("err MUST be a Python ObitErr")
    Obit.ObitErrLog(err.me)
    # end PrintErr


class ObitError(Exception):
    """Obit related error."""


def printErrMsg(err, message="Error"):
    """
    Print Obit error stack and throws runtime exception on error.

    * err     = Python Obit Error/message stack
    * message = message string for exception
    """
    ################################################################
    # Checks
    if not OErrIsA(err):
        raise TypeError("err MUST be a Python ObitErr")
    ierr = Obit.isError(err.me)
    Obit.ObitErrLog(err.me)
    if ierr:
        # print(message)
        raise ObitError(message)
    # end printErrMsg


def OErrIsA(err):
    """
    Tell if object thinks it's a Python ObitErr.

    return true, false (1,0)

    * err    = input Python ObitErr stack
    """
    ################################################################
    # Checks
    if not isinstance(err, OErr):
        return False
    #
    return Obit.ObitErrIsA(err.me)
    # end OErrIsA


def Bomb():
    """Throw an exception to stop the debugger."""
    ################################################################
    #
    Obit.Bomb()
    # end Bomb
