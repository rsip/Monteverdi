/*=========================================================================

Program:   ORFEO Toolbox
Language:  C++
Date:      $Date$
Version:   $Revision$


Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
See OTBCopyright.txt for details.


This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbAsynchronousProcess_H
#define __otbAsynchronousProcess_H


#include "otbAsynchronousProcessBase.h"
#include "itkMultiThreader.h"

namespace otb
{
namespace Process
{

  class AsynchronousProcess : public AsynchronousProcessBase
{
 public:
  AsynchronousProcess( std::string file );
  virtual ~AsynchronousProcess(){};
  
 protected: 
  virtual void Run(void * t); 
  
 private:
  std::string m_FileName;
  //ThreaderType::Pointer m_Threader;
 
};

} // namespace Process
} // namespace otb

#endif