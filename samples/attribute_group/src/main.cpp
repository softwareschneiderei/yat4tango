//----------------------------------------------------------------------------
// Copyright (c) 2004-2015 Synchrotron SOLEIL
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Lesser Public License v3
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/lgpl.html
//----------------------------------------------------------------------------
#include <iostream>
#include <yat4tango/LogHelper.h>
#include <yat4tango/ExceptionHelper.h>
#include <yat4tango/AttributeGroup.h>     

//-----------------------------------------------------------------------------
// MAIN
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{ 
  std::cout << "AttributeGroupTester..." << std::endl;

  const std::string t1_dev_name("tango/test/1");
  const std::string t2_dev_name("tango/test/2");
  const std::string t3_dev_name("tango/test/3");
  
  const std::string no_dev_name("not/defined/dev");
  
  std::string a00(t1_dev_name + "/short_scalar");
  std::string a01(t1_dev_name + "/long_scalar");
  std::string a02(t1_dev_name + "/throw_exception");
  std::string a03(t1_dev_name + "/no_value");
  
  std::string a04(t1_dev_name + "/long_spectrum");
  std::string a05(t1_dev_name + "/double_image");
  
  std::string a06(no_dev_name + "/no_value");
  
  std::string a07(t1_dev_name + "/ampli");
  std::string a08(t1_dev_name + "/double_scalar");
  std::string a09(t2_dev_name + "/double_scalar");
  std::string a10(t2_dev_name + "/ampli");

  enum 
  {
    A00 = 0,
    A01,
    A02,
    A03,
    A04,
    A05,
    A06,
    A07,
    A08,
    A09,
    A10
  };
  
  yat4tango::AttributeGroup ag1;
  
  try
  {
    ag1.register_attribute(a00);
    ag1.register_attribute(a01);
    
    std::vector<std::string> v;
    v.push_back(a02);
    v.push_back(a03);
    ag1.register_attributes(v);
  }
  catch (Tango::DevFailed& df)
  {
    std::cout << "AttributeGroupTester::unexpected tango error!" << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cout << "AttributeGroupTester::unknown error" << std::endl;
    return -1;
  }
  
  try
  {
    ag1.register_attribute(a04);
  }
  catch (Tango::DevFailed& df)
  {
    std::cout << "AttributeGroupTester::got expected error for attribute "  
              << a04
              << std::endl;
  }
  catch (...)
  {
    std::cout << "AttributeGroupTester::unknown error" << std::endl;
    return -1;
  }
  
  try
  {
    ag1.register_attribute(a05);
  }
  catch (Tango::DevFailed& df)
  {
    std::cout << "AttributeGroupTester::got expected error for attribute " 
              << a05
              << std::endl;
  }
  catch (...)
  {
    std::cout << "unknown error" << std::endl;
    return -1;
  }
  
  const std::vector<std::string>& al = ag1.attributes_list();
  std::cout << "AttributeGroupTester::currently registered attributes..." << std::endl; 
  for (size_t i = 0; i < al.size(); i++)
  {
    std::cout << "\t- " << al[i] << std::endl; 
  }

  try
  {
    yat4tango::AttributeGroupReplyList agrs = ag1.read();
    
    if ( agrs.has_failed() )
    {
      std::cout << "AttributeGroupTester::at least one attribute failed during AttributeGroup::read (as expected)" 
                << std::endl;
    }
    
    for (size_t r = 0; r < agrs.size(); r++)
    {
      std::cout << "AttributeGroupTester::analysing reply from "
                << agrs[r].dev_name() 
                << "/"
                << agrs[r].attr_name() 
                << std::endl
                << "\t- has-failed..."
                << agrs[r].has_failed() 
                << std::endl;
      switch (r)
      {
        case A00:
        {
          Tango::DevShort ds;
          if ( ! (agrs[r] >> ds) )
          {
            std::cout << "\t- value........"
                      << "NAN (value extraction failed)"
                      << std::endl;
          }
          else
          {
            std::cout << "\t- value........"
                      << ds
                      << std::endl;
          }
        }
        break;
        case A01:
        {
          Tango::DevLong dl;
          if ( ! (agrs[r] >> dl) )
          {
            std::cout << "\t- value........"
                      << "NAN (value extraction failed)"
                      << std::endl;
          }
          else
          {
            std::cout << "\t- value........"
                      << dl
                      << std::endl;
          }
        }
        break;
        case A02:
        {
          Tango::DevDouble dd;
          if ( ! (agrs[r] >> dd) )
          {
            std::cout << "\t- value........"
                      << "NAN (value extraction failed)"
                      << std::endl;
          }
          else
          {
            std::cout << "\t- value........"
                      << dd
                      << std::endl;
          }
        }
        break;
        case A03:
        {
          Tango::DevDouble dd;
          if ( ! (agrs[r] >> dd) )
          {
            std::cout << "\t- value........"
                      << "NAN (value extraction failed)"
                      << std::endl;
          }
          else
          {
            std::cout << "\t- value........"
                      << dd
                      << std::endl;
          }
        }
        break;
      }
    }
  }
  catch (Tango::DevFailed& df)
  {
    std::cout << "AttributeGroupTester::unexpected Tango error from AttributeGroup::read" 
              << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cout << "AttributeGroupTester::unknown error from AttributeGroup::read" 
              << std::endl;
    return -1;
  }
  
/*
  try
  {
    std::vector<Tango::DevDouble> v;
    v.push_back(0.);
    v.push_back(1.);
    v.push_back(2.);
    v.push_back(3.);
    ag1.write(v);
  }
  catch (Tango::DevFailed& df)
  {
    std::cout << "AttributeGroupTester::got expected Tango error from AttributeGroup::write" 
              << std::endl;
  }
  catch (...)
  {
    std::cout << "AttributeGroupTester::unknown error from AttributeGroup::write" 
              << std::endl;
    return -1;
  }
*/

  yat4tango::AttributeGroup ag2;
  
  try
  {
    ag2.register_attribute(a07);
    ag2.register_attribute(a09);
    ag2.register_attribute(a08);
    ag2.register_attribute(a10); 
  }
  catch (Tango::DevFailed& df)
  {
    std::cout << "AttributeGroupTester::unexpected tango error!" << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cout << "AttributeGroupTester::unknown error" << std::endl;
    return -1;
  }

  try
  {
    yat4tango::AttributeGroupReplyList agrs = ag2.read();
    
    if ( agrs.has_failed() )
    {
      std::cout << "AttributeGroupTester::at least one attribute failed during AttributeGroup::read (unexpected error)" 
                << std::endl;
    }
    
    for (size_t r = 0; r < agrs.size(); r++)
    {
      std::cout << "AttributeGroupTester::analysing reply from "
                << agrs[r].dev_name() 
                << "/"
                << agrs[r].attr_name() 
                << std::endl
                << "\t- has-failed..."
                << agrs[r].has_failed() 
                << std::endl;
      Tango::DevDouble dd;
      if ( ! (agrs[r] >> dd) )
      {
        std::cout << "\t- value........"
                  << "NAN (value extraction failed)"
                  << std::endl;
      }
      else
      {
        std::cout << "\t- value........"
                  << dd
                  << std::endl;
      }
    }
  }
  catch (Tango::DevFailed& df)
  {
    std::cout << "AttributeGroupTester::unexpected Tango error from AttributeGroup::read" 
              << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cout << "AttributeGroupTester::unknown error from AttributeGroup::read" 
              << std::endl;
    return -1;
  }

  try
  {
    std::vector<Tango::DevDouble> v;
    v.push_back(1.);
    v.push_back(3.);
    v.push_back(2.);
    v.push_back(4.);
    ag2.write(v);
  }
  catch (Tango::DevFailed& df)
  {
    std::cout << "AttributeGroupTester::got expected Tango error from AttributeGroup::write" 
              << std::endl;
  }
  catch (...)
  {
    std::cout << "AttributeGroupTester::unknown error from AttributeGroup::write" 
              << std::endl;
    return -1;
  }

  std::cout << "AttributeGroupTester::test passed!" << std::endl;
  
  return 0;
}
