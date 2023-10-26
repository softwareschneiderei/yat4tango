//----------------------------------------------------------------------------
// Copyright (c) 2004-2016 The Tango Community
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// YAT4Tango LIBRARY
//----------------------------------------------------------------------------
//
// Copyright (c) 2004-2016 The Tango Community
//
// The YAT4Tango library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU General Public License as published 
// by the Free Software Foundation; either version 2 of the License, or (at 
// your option) any later version.
//
// The YAT4Tango library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// See COPYING file for license details  
//
// Contact:
//      Nicolas Leclercq
//      Synchrotron SOLEIL
//------------------------------------------------------------------------------
/*!
 * \authors See AUTHORS file
 */

// ============================================================================
// DEPENDENCIES
// ============================================================================
#include <yat4tango/ExceptionHelper.h>

namespace yat4tango
{
  YATDevFailed::YATDevFailed(const yat::Exception& ex)
  {
    const yat::Exception::ErrorList& yat_errors = ex.errors;
    errors.length( yat_errors.size() );
    for (unsigned long i = 0; i < errors.length(); i++)
    {
      errors[i].reason = CORBA::string_dup( yat_errors[i].reason.c_str() );
      errors[i].desc   = CORBA::string_dup( yat_errors[i].desc.c_str()   );
      errors[i].origin = CORBA::string_dup( yat_errors[i].origin.c_str() );
      switch (yat_errors[i].severity)
      {
        case yat::WARN:
          errors[i].severity = Tango::WARN;
          break;
        case yat::PANIC:
          errors[i].severity = Tango::PANIC;
          break;
        case yat::ERR:
        default:
          errors[i].severity = Tango::ERR;
          break;
      }
    }
  }


  TangoYATException::TangoYATException( const Tango::DevFailed& df )
  {
    const Tango::DevErrorList& tango_err_list = df.errors;
    for (unsigned long i = 0; i < tango_err_list.length(); i++) 
    {
      Tango::ErrSeverity df_sev = df.errors[i].severity;
      yat::ErrorSeverity yat_sev = (df_sev == Tango::WARN ? yat::WARN
                                    : (df_sev == Tango::ERR ? yat::ERR
                                       : (df_sev == Tango::PANIC ? yat::PANIC
                                          : yat::ERR)));
      
      this->push_error( df.errors[i].reason,
                        df.errors[i].desc,
                        df.errors[i].origin,
                        -1,
                        yat_sev);
    }
  }

  //------------------------------------------------------------------------
  void throw_yat_exception()
  {
      try
      {
          throw;
      }
      catch (const yat::Exception&)
      {
          throw;
      }
      catch (const Tango::DevFailed& df)
      {
          THROW_TANGO_TO_YAT_EXCEPTION(df);
      }
      catch (const std::exception& std_err)
      {
          throw yat::Exception("STD_ERROR", std_err.what(), "??");
      }
      catch (...)
      {
          throw yat::Exception("UNKNOWN_ERROR", "unknown error caught", "??");
      }
  }

  //------------------------------------------------------------------------
  void throw_yat_exception(const std::string& reason, const std::string& desc,
                           const std::string& origin)
  {
      try
      {
          throw_yat_exception();
      }
      catch (yat::Exception& err)
      {
          err.push_error(reason, desc, origin);
          throw err;
      }
  }

  //------------------------------------------------------------------------
  void throw_devfailed_exception()
  {
      try
      {
          throw;
      }
      catch (const Tango::DevFailed&)
      {
          throw;
      }
      catch (const yat::Exception& yat_err)
      {
          THROW_YAT_TO_TANGO_EXCEPTION(yat_err);
      }
      catch (const std::exception& std_err)
      {
          Tango::Except::throw_exception("STD_ERROR", std_err.what(), "??");
      }
      catch (...)
      {
          Tango::Except::throw_exception("UNKNOWN_ERROR", "unknown error caught", "??");
      }
  }

  //------------------------------------------------------------------------
  void throw_devfailed_exception(const std::string& reason, const std::string& desc,
                                 const std::string& origin)
  {
      try
      {
          throw_devfailed_exception();
      }
      catch (Tango::DevFailed& err)
      {
          Tango::Except::re_throw_exception(err, reason, desc, origin);
      }
  }

}
