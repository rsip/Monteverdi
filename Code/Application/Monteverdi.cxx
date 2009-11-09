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

#include "ConfigureMonteverdi.h"

#include "otbMonteverdiModel.h"
#include "otbMonteverdiViewGUI.h"
#include "otbMonteverdiController.h"
#include "otbSplashScreen.h"
#include "otbI18n.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_PNG_Image.H>

#include <string>
#include <ctime>
#include <iostream>




// There are function prototype conflits under cygwin between standard w32 API
// and standard C ones
#ifndef CALLBACK
#if defined(_WINDOWS) || defined(__CYGWIN__)
#define CALLBACK __stdcall
#else
#define CALLBACK
#endif
#endif

#include "otbReaderModule.h"
#include "otbSpeckleFilteringModule.h"
#include "otbFeatureExtractionModule.h"
#include "otbOrthorectificationModule.h"
#include "otbMeanShiftModule.h"
#include "otbWriterModule.h"
#include "otbWriterMVCModule.h"
#include "otbSupervisedClassificationModule.h"
#include "otbMeanShiftModule.h"
#include "otbPanSharpeningModule.h"
#include "otbViewerModule.h"
#include "otbCachingModule.h"
#include "otbSarIntensityModule.h"
#include "otbHomologousPointExtractionModule.h"
#include "otbExtractROIModule.h"
#include "otbConcatenateModule.h"
#include "otbProjectionModule.h"
#include "otbSuperimpositionModule.h"
#include "otbAlgebraModule.h"
#include "otbKMeansModule.h"
#include "otbChangeDetectionModule.h"
#include "otbGCPToSensorModelModule.h"
#include "otbThresholdModule.h"

int main(int argc, char* argv[])
{
  //Internationalization
  otbI18nMacro();

  // Splash Screen
  typedef otb::SplashScreen::Pointer SplashScreenPointerType;

 // SplashScreenPointerType splash = otb::SplashScreen::New();
 // splash->Build();
 // splash->Show();


  // Application
  typedef otb::MonteverdiModel       ModelType;
  typedef otb::MonteverdiController  ControllerType;
  typedef otb::MonteverdiViewGUI     ViewType;

  // Create the MVC
  ModelType::Pointer model = otb::MonteverdiModel::GetInstance();
  ViewType::Pointer view = ViewType::New();
  ControllerType::Pointer controller = ControllerType::New();
  controller->SetView(view);
  view->SetMonteverdiController(controller);

  // Register modules
  model->RegisterModule<otb::ReaderModule>("Reader", otbGetTextMacro("File/Open dataset"));
  model->RegisterModule<otb::WriterModule> ("Writer", otbGetTextMacro("File/Save dataset"));
  model->RegisterModule<otb::ExtractROIModule>("ExtractROI", otbGetTextMacro("File/Extract ROI from dataset"));
  model->RegisterModule<otb::WriterMVCModule> ("Specific writer for X image", otbGetTextMacro("File/Save dataset (advanced)"));
  model->RegisterModule<otb::SpeckleFilteringModule>("Speckle", otbGetTextMacro("SAR/Despeckle image"));
  model->RegisterModule<otb::SarIntensityModule>("SarIntensity", otbGetTextMacro("SAR/Compute intensity and log-intensity"));

  model->RegisterModule<otb::FeatureExtractionModule>("FeatureExtraction", otbGetTextMacro("Filtering/Feature extraction"));
  model->RegisterModule<otb::SupervisedClassificationModule>("SupervisedClassification", otbGetTextMacro("Learning/SVM classification"));
  model->RegisterModule<otb::KMeansModule>("KMeans", otbGetTextMacro("Learning/KMeans clustering"));
  model->RegisterModule<otb::OrthorectificationModule>("Orthorectification", otbGetTextMacro("Geometry/Orthorectification"));
  model->RegisterModule<otb::ProjectionModule>("Projection", otbGetTextMacro("Geometry/Reproject image"));
  model->RegisterModule<otb::SuperimpositionModule>("Superimposition", otbGetTextMacro("Geometry/Superimpose two images"));
  model->RegisterModule<otb::HomologousPointExtractionModule>("HomologousPoints", otbGetTextMacro("Geometry/Homologous points extraction"));
  model->RegisterModule<otb::GCPToSensorModelModule>("GCPToSensorModel", otbGetTextMacro("Geometry/GCP To Sensor Model"));

  model->RegisterModule<otb::MeanShiftModule> ("MeanShift", otbGetTextMacro("Filtering/Mean shift clustering"));
  model->RegisterModule<otb::PanSharpeningModule> ("PanSharpening", otbGetTextMacro("Filtering/Pan-sharpen an image"));
  model->RegisterModule<otb::ViewerModule>("Viewer", otbGetTextMacro("Visualization/Viewer"));
  model->RegisterModule<otb::CachingModule>("zCaching", otbGetTextMacro("File/Cache dataset"));
  model->RegisterModule<otb::ConcatenateModule>("Concatenate", otbGetTextMacro("File/Concatenate images"));

  model->RegisterModule<otb::AlgebraModule>("Algebra", otbGetTextMacro("Filtering/Band Math"));
  model->RegisterModule<otb::ChangeDetectionModule>("ChangeDetection", otbGetTextMacro("Filtering/Change Detection"));
  model->RegisterModule<otb::ThresholdModule>("Threshold", otbGetTextMacro("Filtering/Threshold"));
  
  // Launch Monteverdi
  view->InitWidgets();
  view->Show();

  Fl::lock();
  return Fl::run();
}
