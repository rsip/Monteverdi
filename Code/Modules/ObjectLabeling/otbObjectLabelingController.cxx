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
#ifndef __otbObjectLabelingController_cxx
#define __otbObjectLabelingController_cxx

#include "otbObjectLabelingController.h"
#include "otbMsgReporter.h"
#include <FL/fl_ask.H>

namespace otb
{

ObjectLabelingController::ObjectLabelingController() : m_View(), m_WidgetsController(),
                                                                    m_ResizingHandler(), m_ChangeScaleHandler(), m_ChangeRegionHandler(),
                                                 m_ChangeScaledRegionHandler(), m_PixelDescriptionHandler(), m_MouseClickHandler()
{
  // Build the widgets controller
  m_WidgetsController         = WidgetsControllerType::New();
  
  // Build the action handlers
  m_ResizingHandler           = ResizingHandlerType::New();
  m_ChangeScaleHandler        = ChangeScaleHandlerType::New();
  m_ChangeRegionHandler       = ChangeRegionHandlerType::New();
  m_ChangeScaledRegionHandler = ChangeScaledRegionHandlerType::New();
  m_PixelDescriptionHandler   = PixelDescriptionActionHandlerType::New();
  m_MouseClickHandler         = MouseClickActionHandlerType::New();

  // Set up mouse click handler
  m_MouseClickHandler->SetMouseButton(3);
  m_MouseClickHandler->ActiveOnScrollWidgetOff();

  // Add the action handlers to the widgets controller
  m_WidgetsController->AddActionHandler(m_ResizingHandler);
  m_WidgetsController->AddActionHandler(m_ChangeScaleHandler);
  m_WidgetsController->AddActionHandler(m_ChangeRegionHandler);
  m_WidgetsController->AddActionHandler(m_ChangeScaledRegionHandler);
  m_WidgetsController->AddActionHandler(m_PixelDescriptionHandler);
  m_WidgetsController->AddActionHandler(m_MouseClickHandler);
}

ObjectLabelingController::~ObjectLabelingController()
{}


void ObjectLabelingController::SetView(ObjectLabelingView * view)
{
  m_View = view;
  m_ResizingHandler->SetView(m_View->GetImageView());
  m_ChangeScaleHandler->SetView(m_View->GetImageView());
  m_ChangeRegionHandler->SetView(m_View->GetImageView());
  m_ChangeScaledRegionHandler->SetView(m_View->GetImageView());
  m_PixelDescriptionHandler->SetView(m_View->GetImageView());
  m_MouseClickHandler->SetView(m_View->GetImageView());
}


void ObjectLabelingController::OpenImage(VectorImageType* vimage, LabeledImageType* limage)
{
  try
    {
    m_Model->OpenImage(vimage, limage);
    }
  catch (itk::ExceptionObject & err)
    {
    std::string desc = err.GetDescription();
    if (desc.find("End point not with +/-1 line from line") != std::string::npos)
      {
      std::ostringstream oss;
      oss << "Invalid label image, too many labels";
      MsgReporter::GetInstance()->SendError(oss.str());
      }
    else
      {
      std::ostringstream oss;
      oss << "Invalid input image(s).";
      oss << "The following exception was caught: ";
      oss << err.GetDescription();
      MsgReporter::GetInstance()->SendError(oss.str());
      }
    m_View->wMainWindow->hide();
    }
}

void ObjectLabelingController::ClassSelected(unsigned int classIndex)
{
  try
    {
    m_Model->SelectClass(classIndex);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::ClearSelectedClass()
{
  try
    {
    m_Model->ClearSelectedClass();
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::AddClass()
{
  try
    {
    m_Model->AddClass();
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}
void ObjectLabelingController::RemoveClass(unsigned int classIndex)
{
  try
    {
    m_Model->RemoveClass(classIndex);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}
void ObjectLabelingController::ClearClasses()
{
  try
    {
    m_Model->ClearClasses();
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}
void ObjectLabelingController::ChangeClassColor(unsigned int classIndex, const ColorType & color)
{
  try
    {
    m_Model->SetClassColor(color, classIndex);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}
void ObjectLabelingController::ChangeClassName(unsigned int classIndex, const char * name)
{
  try
    {
    m_Model->SetClassName(name, classIndex);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}
void ObjectLabelingController::ChangeClassLabel(unsigned int classIndex, const LabelType & label)
{
  try
    {
    m_Model->SetClassLabel(label, classIndex);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}
void ObjectLabelingController::RemoveObject(unsigned int objectIndex)
{
  if(m_Model->HasSelectedClass())
    {
    try
      {
      m_Model->RemoveSampleFromClass(m_Model->GetClass(m_Model->GetSelectedClass()).m_Samples[objectIndex], m_Model->GetSelectedClass());
      }
    catch(itk::ExceptionObject & err)
      {
      MsgReporter::GetInstance()->SendError(err.GetDescription());
      }
    }
}

void ObjectLabelingController::SelectObject(unsigned int objectIndex)
{
  if(m_Model->HasSelectedClass())
    {
    try
      {
      m_Model->FocusOnSample(m_Model->GetClass(m_Model->GetSelectedClass()).m_Samples[objectIndex]);
      m_Model->SelectSample(m_Model->GetClass(m_Model->GetSelectedClass()).m_Samples[objectIndex]);
      

      }
    catch(itk::ExceptionObject & err)
      {
      MsgReporter::GetInstance()->SendError(err.GetDescription());
      }
    }
}

void ObjectLabelingController::ClearObjects()
{
  if(m_Model->HasSelectedClass())
    {
    try
      {
      m_Model->ClearSamplesFromClass(m_Model->GetSelectedClass());
      }
    catch(itk::ExceptionObject & err)
      {
      MsgReporter::GetInstance()->SendError(err.GetDescription());
      }
    }

}

void ObjectLabelingController::SaveSamplesToXMLFile(const char * fname)
{
  try
    {
    m_Model->SaveSamplesToXMLFile(fname);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::LoadSamplesFromXMLFile(const char * fname)
{
  try
    {
    m_Model->LoadSamplesFromXMLFile(fname);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::SaveClassificationParametersToXMLFile(const char * fname)
{
  try
    {
    m_Model->SaveClassificationParametersToXMLFile(fname);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}


void ObjectLabelingController::SaveClassification()
{
  try
    {
    m_Model->SaveClassification();
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::SampleMargin()
{
  try
    {
    m_Model->SampleMargin();
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}


void ObjectLabelingController::ChangeKernelType(int kernel)
{
  try
    {
    m_Model->GetSVMEstimator()->SetKernelType(kernel);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::ChangeNumberOfCrossValidationFolders(unsigned int nb)
{
  try
    {
    m_Model->GetSVMEstimator()->SetNumberOfCrossValidationFolders(nb);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::ChangeParametersOptimisation(bool value)
{
  try
    {
    m_Model->GetSVMEstimator()->SetParametersOptimization(value);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::ChangeNumberOfCoarseSteps(unsigned int nb)
{
  try
    {
    m_Model->GetSVMEstimator()->SetCoarseOptimizationNumberOfSteps(nb);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::ChangeNumberOfFineSteps(unsigned int nb)
{
  try
    {
    m_Model->GetSVMEstimator()->SetFineOptimizationNumberOfSteps(nb);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::ChangeNumberOfMarginSamples(unsigned int nb)
{
  try
    {
    m_Model->GetMarginSampler()->SetNumberOfCandidates(nb);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::ChangeMarginColor(const ColorType & color){}

void ObjectLabelingController::FocusOnMarginSample(unsigned int sample)
{
  if(sample>0 && sample <= m_Model->GetMarginSamples().size())
    {
    try
      {
      m_Model->FocusOnSample(m_Model->GetMarginSamples()[sample-1]);
      m_Model->SelectSample(m_Model->GetMarginSamples()[sample-1]);
      }
    catch(itk::ExceptionObject & err)
      {
      MsgReporter::GetInstance()->SendError(err.GetDescription());
      }
    }
}

void ObjectLabelingController::ClearMarginSamples()
{
  try
    {
    m_Model->ClearMarginSamples();
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::Classify()
{
  try
    {
    m_Model->Classify();
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}
void ObjectLabelingController::ClearClassification()
{
  try
    {
    m_Model->ClearClassification();
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::ChangeFeatureState(const std::string & fname, bool state)
{
  try
    {
    m_Model->ChangeFeatureState(fname, state);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::ChangeClassificationOpacity(double value)
{
  try
    {
    m_Model->ChangeClassificationOpacity(value);
    }
  catch(itk::ExceptionObject & err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void ObjectLabelingController::UpdateViewerDisplay()
{
  if(!m_Model->GetVectorImage())
    {
      return;
    }
  
  std::vector<unsigned int> channels;
  if (m_View->rViewerSetupColorMode->value())
    {
      channels.push_back(atoi(m_View->iRChannelChoice->value())-1);
      channels.push_back(atoi(m_View->iGChannelChoice->value())-1);
      channels.push_back(atoi(m_View->iBChannelChoice->value())-1);
    }
  else if (m_View->rViewerSetupGrayscaleMode->value())
    {
      channels.push_back(atoi(m_View->iGrayscaleChannelChoice->value())-1);
    }

  m_Model->UpdateViewerDisplay(channels);
}


} // end namespace otb

#endif
