@ECHO off

::------------------------------------------------------------------------
:: ======== YAT_X86 ======== 
::------------------------------------------------------------------------
set YAT_ROOT_X86=
set YAT_INC_X86=%YAT_ROOT_X86%\include
set YAT_LIB_X86=%YAT_ROOT_X86%\lib\static
set YAT_SHARED_LIB_X86=%YAT_ROOT_X86%\lib\shared
set YAT_LIB_LIST_X86=yat.lib
set YAT_DBG_LIB_LIST_X86=yatd.lib

::------------------------------------------------------------------------
:: ======== YAT_X64 ======== 
::------------------------------------------------------------------------
set YAT_ROOT_X64C:\dev\tango_812_x64_vc11
set YAT_INC_X64=%YAT_ROOT_X64%\include
set YAT_LIB_X64=%YAT_ROOT_X64%\lib\static
set YAT_SHARED_LIB_X64=%YAT_ROOT_X64%\lib\shared
set YAT_LIB_LIST_X64=yat.lib
set YAT_DBG_LIB_LIST_X64=yatd.lib

::------------------------------------------------------------------------
:: ======== LOG4TANGO_X86 ======== 
::------------------------------------------------------------------------
SET LOG4TANGO_ROOT_X86=
SET LOG4TANGO_INC_X86=%LOG4TANGO_ROOT_X86%\include
SET LOG4TANGO_LIB_X86=%LOG4TANGO_ROOT_X86%\lib\shared
SET LOG4TANGO_BIN_X86=%LOG4TANGO_ROOT_X86%\bin
SET LOG4TANGO_LIB_LIST_X86=log4tango.lib
SET LOG4TANGO_DBG_LIB_LIST_X86=log4tangod.lib
SET PATH=%LOG4TANGO_BIN_X86%;%PATH%

::------------------------------------------------------------------------
:: ======== LOG4TANGO_X64 ======== 
::------------------------------------------------------------------------
SET LOG4TANGO_ROOT_X64=C:\dev\tango_812_x64_vc11
SET LOG4TANGO_INC_X64=%LOG4TANGO_ROOT_X64%\include
SET LOG4TANGO_LIB_X64=%LOG4TANGO_ROOT_X64%\lib\shared
SET LOG4TANGO_BIN_X64=%LOG4TANGO_ROOT_X64%\bin
SET LOG4TANGO_LIB_LIST_X64=log4tango.lib
SET LOG4TANGO_DBG_LIB_LIST_X64=log4tangod.lib
SET PATH=%LOG4TANGO_BIN_X64%;%PATH%

::------------------------------------------------------------------------
:: ======== TANGO_X86 ======== 
::------------------------------------------------------------------------
SET TANGO_ROOT_X86=
SET TANGO_INC_X86=%TANGO_ROOT_X86%\include
SET TANGO_LIB_X86=%TANGO_ROOT_X86%\lib\shared
SET TANGO_BIN_X86=%TANGO_ROOT_X86%\bin
SET TANGO_LIB_LIST_X86=tango.lib
SET TANGO_DBG_LIB_LIST_X86=tangod.lib
SET PATH=%TANGO_BIN_X86%;%PATH%

::------------------------------------------------------------------------
:: ======== TANGO_X64 ======== 
::------------------------------------------------------------------------
SET TANGO_ROOT_X64=C:\dev\tango_812_x64_vc11
SET TANGO_INC_X64=%TANGO_ROOT_X64%\include
SET TANGO_LIB_X64=%TANGO_ROOT_X64%\lib\shared
SET TANGO_BIN_X64=%TANGO_ROOT_X64%\bin
SET TANGO_LIB_LIST_X64=tango.lib
SET TANGO_DBG_LIB_LIST_X64=tangod.lib
SET PATH=%TANGO_BIN_X64%;%PATH%

::------------------------------------------------------------------------
:: ======== OMNIORB_X86 ======== 
::------------------------------------------------------------------------
SET OMNIORB_ROOT_X86=
SET OMNIORB_INC_X86=%OMNIORB_ROOT_X86%\include
SET OMNIORB_LIB_X86=%OMNIORB_ROOT_X86%\lib\shared
SET OMNIORB_BIN_X86=%OMNIORB_ROOT_X86%\bin
SET OMNIORB_LIB_LIST_X86=omnithread34_rt.lib;omniORB416_rt.lib;COS416_rt.lib;omniDynamic416_rt.lib
SET OMNIORB_DBG_LIB_LIST_X86=omnithread34_rtd.lib;omniORB416_rtd.lib;COS416_rtd.lib;omniDynamic416_rtd.lib
SET PATH=%OMNIORB_BIN_X86%;%PATH%

::------------------------------------------------------------------------
:: ======== OMNIORB_X64 ======== 
::------------------------------------------------------------------------
SET OMNIORB_ROOT_X64=C:\dev\tango_812_x64_vc11
SET OMNIORB_INC_X64=%OMNIORB_ROOT_X64%\include
SET OMNIORB_LIB_X64=%OMNIORB_ROOT_X64%\lib\shared
SET OMNIORB_BIN_X64=%OMNIORB_ROOT_X64%\bin
SET OMNIORB_LIB_LIST_X64=omnithread34_rt.lib;omniORB416_rt.lib;COS416_rt.lib;omniDynamic416_rt.lib
SET OMNIORB_DBG_LIB_LIST_X64=omnithread34_rtd.lib;omniORB416_rtd.lib;COS416_rtd.lib;omniDynamic416_rtd.lib
SET PATH=%OMNIORB_BIN_X64%;%PATH%

::------------------------------------------------------------------------
:: ======== ZMQ_X86 ======== 
::------------------------------------------------------------------------
SET ZMQ_ROOT_X86=
SET ZMQ_INC_X86=%ZMQ_ROOT_X86%\include
SET ZMQ_LIB_X86=%ZMQ_ROOT_X86%\lib\shared
SET ZMQ_BIN__X86=%ZMQ_ROOT_X86%\bin
SET ZMQ_LIB_LIST_X86=zmq.lib
SET ZMQ_DBG_LIB_LIST_X86=zmqd.lib
SET PATH=%ZMQ_BIN_X86%;%PATH%

::------------------------------------------------------------------------
:: ======== ZMQ_X64 ======== 
::------------------------------------------------------------------------
SET ZMQ_ROOT_X64=C:\dev\tango_812_x64_vc11
SET ZMQ_INC_X64=%ZMQ_ROOT_X64%\include
SET ZMQ_LIB_X64=%ZMQ_ROOT_X64%\lib\shared
SET ZMQ_BIN_X64=%ZMQ_ROOT_X64%\bin
SET ZMQ_LIB_LIST_X64=zmq.lib
SET ZMQ_DBG_LIB_LIST_X64=zmqd.lib
::SET PATH=%ZMQ_BIN_X64%;%PATH%

START /B .\YAT4tango.sln


