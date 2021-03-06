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

#include "otbSpeckleFilteringController.h"
#include "otbMsgReporter.h"

namespace otb
{

SpeckleFilteringController
::SpeckleFilteringController()
{

/** NewVisu */
// Build a "visu"controller
}

SpeckleFilteringController
::~SpeckleFilteringController()
{

}

void
SpeckleFilteringController
::ProcessLeeFilter(unsigned int radius)
{
  try
    {
    m_Model->LeeFiltering(radius);
    }
  catch (itk::ExceptionObject& err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void
SpeckleFilteringController
::ProcessFrostFilter(unsigned int radius, double DeRamp)
{
  try
    {
    m_Model->FrostFiltering(radius, DeRamp);
    }
  catch (itk::ExceptionObject& err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

} // end namespace otb
