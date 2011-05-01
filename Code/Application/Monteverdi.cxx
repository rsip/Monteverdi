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
#include <string>
#include <ctime>
#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_PNG_Image.H>

#include "itksys/SystemTools.hxx"

#include "ConfigureMonteverdi.h"

#include "otbMonteverdiModel.h"
#include "otbMonteverdiViewGUI.h"
#include "otbMonteverdiController.h"
#include "otbSplashScreen.h"
#include "otbI18n.h"

// There are function prototype conflits under cygwin between standard w32 API
// and standard C ones
#ifndef CALLBACK
#if defined(_WINDOWS) || defined(__CYGWIN__)
#define CALLBACK __stdcall
#else
#define CALLBACK
#endif
#endif

#include "otbMsgReporter.h"

#include "otbReaderModule.h"
#include "otbSpeckleFilteringModule.h"
#include "otbFeatureExtractionModule.h"
//#include "otbOrthorectificationModule.h"
#include "otbMeanShiftModule.h"
#include "otbWriterModule.h"
#include "otbWriterMVCModule.h"
#include "otbSupervisedClassificationModule.h"
#include "otbSupervisedClassificationModule2.h"
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
#include "otbKMeansModule.h"
#include "otbChangeDetectionModule.h"
#include "otbGCPToSensorModelModule.h"
#include "otbThresholdModule.h"
#include "otbOpticalCalibrationModule.h"
#include "otbSarCalibrationModule.h"
#include "otbObjectCountingModule.h"
#include "otbCommandLineArgumentParser.h"
#include "otbTileExportModule.h"
#include "otbObjectLabelingModule.h"
#include "otbFineCorrelationModule.h"
#include "otbVectorizationModule.h"
#include "otbSpectrumModule.h"
#include "otbBandMathModule.h"
#include "otbPolarimetricSynthesisModule.h"
#include "otbDEMToImageGeneratorModule.h"
#include "otbColorMappingModule.h"
#include "otbImageStatisticsModule.h"
#include "otbBayesianFusionModule.h"
#include "otbSARPolarimetrySinclairModule.h"
#include "otbSARPolarimetryMuellerModule.h"
#include "otbSARPolarimetryReciprocalModule.h"
#include "otbSARPolarimetryAnalysisModule.h"

#ifdef OTB_USE_MAPNIK
#include "otbRasterizationModule.h"
#endif

#ifdef OTB_USE_CURL
#include "otbTileMapImportModule.h"
#endif


int main(int argc, char* argv[])
{
  //Internationalization
  otbI18nMacro();

  // Parse command line parameters
  typedef otb::CommandLineArgumentParser ParserType;
  ParserType::Pointer parser = ParserType::New();

  parser->AddInputImage(false); //Optional parameter
  parser->AddOption("--InputVectorData", "input vector data file name ", "-ivd", 1, false);
  parser->AddOptionNParams("--ImageList", "image list", "-iml", false);
  parser->SetProgramDescription("Monteverdi launcher");
  //   parser->AddOption("--NoSplashScreen", "Deactivate the splash screen","-NoSplash", 0, false);

  typedef otb::CommandLineArgumentParseResult ParserResultType;
  ParserResultType::Pointer parseResult = ParserResultType::New();

  try
    {
    parser->ParseCommandLine(argc, argv, parseResult);
    }
  catch (itk::ExceptionObject& err)
    {
    std::string descriptionException = err.GetDescription();
    if (descriptionException.find("ParseCommandLine(): Help Parser") != std::string::npos)
      {
      return EXIT_SUCCESS;
      }
    }

  // Splash Screen (deactivated for now)
//  if ( !parseResult->IsOptionPresent("--NoSplashScreen") )
//    {
//      typedef otb::SplashScreen::Pointer SplashScreenPointerType;
//
//      SplashScreenPointerType splash = otb::SplashScreen::New();
//      splash->SetDisplayTime(4.);
//      splash->Build();
//      splash->Show();
//    }

  // Application
  typedef otb::MonteverdiModel      ModelType;
  typedef otb::MonteverdiController ControllerType;
  typedef otb::MonteverdiViewGUI    ViewType;

  // Create the MVC
  ModelType::Pointer      model = otb::MonteverdiModel::GetInstance();
  ViewType::Pointer       view = ViewType::New();
  ControllerType::Pointer controller = ControllerType::New();
  controller->SetView(view);
  view->SetMonteverdiController(controller);

  // Register modules
  /***********  File menu *******************/
  model->RegisterModule<otb::ReaderModule>("Reader", otbGetTextMacro("File/Open dataset"));
  model->RegisterModule<otb::WriterModule> ("Writer", otbGetTextMacro("File/Save dataset"));
  model->RegisterModule<otb::WriterMVCModule> ("Specific writer for X image",
                                               otbGetTextMacro("File/Save dataset (advanced)"));
  model->RegisterModule<otb::CachingModule>("Caching", otbGetTextMacro("File/Cache dataset"));
  model->RegisterModule<otb::ExtractROIModule>("ExtractROI", otbGetTextMacro("File/Extract ROI from dataset"));
  model->RegisterModule<otb::ConcatenateModule>("Concatenate", otbGetTextMacro("File/Concatenate images"));

#ifdef OTB_USE_MAPNIK
  model->RegisterModule<otb::RasterizationModule>("Rasterize", otbGetTextMacro("File/Rasterize vector data"));
#endif

  model->RegisterModule<otb::TileExportModule>("Export To Kmz", otbGetTextMacro("File/Export To Kmz"));
#ifdef OTB_USE_CURL
  model->RegisterModule<otb::TileMapImportModule>("Tile Map Import", otbGetTextMacro("File/Tile Map Import"));
#endif
  model->RegisterModule<otb::ImageStatisticsModule>("Image Statistics", otbGetTextMacro("File/Image Statistics"));

  /***********  Visu menu *******************/
  model->RegisterModule<otb::ViewerModule>("Viewer", otbGetTextMacro("Visualization/Viewer"));
  model->RegisterModule<otb::SpectrumModule>("SpectralViewer", otbGetTextMacro("Visualization/Spectral Viewer"));
  model->RegisterModule<otb::ColorMappingModule>("ColorMapping", otbGetTextMacro("Visualization/Color Mapping"));
  
  /***********  Calibration menu *******************/
  model->RegisterModule<otb::OpticalCalibrationModule>("OpticalCalibration",
                                                       otbGetTextMacro("Calibration/Optical Calibration"));
  model->RegisterModule<otb::SarCalibrationModule>("SarCalibration", otbGetTextMacro("Calibration/SAR Calibration"));
  
  /***********  Filtering menu *******************/
  model->RegisterModule<otb::BandMathModule>("BandMath", otbGetTextMacro("Filtering/BandMath"));
  model->RegisterModule<otb::ThresholdModule>("Threshold", otbGetTextMacro("Filtering/Threshold"));
  model->RegisterModule<otb::PanSharpeningModule> ("PanSharpening", otbGetTextMacro("Filtering/Pansharpening/Simple RCS pan-sharpening"));
  model->RegisterModule<otb::BayesianFusionModule>("BayesianFusion",otbGetTextMacro("Filtering/Pansharpening/Bayesian fusion"));
  model->RegisterModule<otb::MeanShiftModule> ("MeanShift", otbGetTextMacro("Filtering/Mean shift clustering"));
  model->RegisterModule<otb::FeatureExtractionModule>("FeatureExtraction",
                                                      otbGetTextMacro("Filtering/Feature extraction"));
  model->RegisterModule<otb::ChangeDetectionModule>("ChangeDetection", otbGetTextMacro("Filtering/Change detection"));
  model->RegisterModule<otb::FineCorrelationModule>("FineCorrelation", otbGetTextMacro("Filtering/Fine Correlation"));
  model->RegisterModule<otb::VectorizationModule>("Vectorization", otbGetTextMacro("Filtering/Vectorization"));

  /***********  SAR menu *******************/
  model->RegisterModule<otb::SpeckleFilteringModule>("Speckle", otbGetTextMacro("SAR/Despeckle image"));
  model->RegisterModule<otb::SarIntensityModule>("SarIntensity",
                                                 otbGetTextMacro("SAR/Compute intensity and log-intensity"));
  model->RegisterModule<otb::PolarimetricSynthesisModule>("PolarimetricSynthesis",
                                                 otbGetTextMacro("SAR/Polarimetric synthesis"));
  model->RegisterModule<otb::SARPolarimetrySinclairModule>("PolarimetrySinclair",
                                                 otbGetTextMacro("SAR/Polarimetric conversion/Sinclair to"));
   model->RegisterModule<otb::SARPolarimetryMuellerModule>("PolarimetryMueller",
                                                 otbGetTextMacro("SAR/Polarimetric conversion/Mueller to"));
  model->RegisterModule<otb::SARPolarimetryReciprocalModule>("PolarimetryReciprocal",
                                                 otbGetTextMacro("SAR/Polarimetric conversion/Reciprocal... to"));
  model->RegisterModule<otb::SARPolarimetryAnalysisModule>("PolarimetryAnalysis",
                                                 otbGetTextMacro("SAR/Analysis"));

  /***********  Learning menu *******************/
  model->RegisterModule<otb::SupervisedClassificationModule>("SupervisedClassification",
                                                             otbGetTextMacro("Learning/SVM classification"));
  model->RegisterModule<otb::SupervisedClassificationModule2>("SupervisedClassification2",
                                                              otbGetTextMacro(
									      "Learning/SVM classification (EXPERIMENTAL)"));
  model->RegisterModule<otb::KMeansModule>("KMeans", otbGetTextMacro("Learning/KMeans clustering"));
  
  /***********  Geometry menu *******************/
//   model->RegisterModule<otb::OrthorectificationModule>("Orthorectification",
//                                                        otbGetTextMacro("Geometry/Orthorectification"));
  model->RegisterModule<otb::ProjectionModule>("Projection", otbGetTextMacro("Geometry/Reproject image"));
  model->RegisterModule<otb::SuperimpositionModule>("Superimposition",
                                                    otbGetTextMacro("Geometry/Superimpose two images"));
  model->RegisterModule<otb::HomologousPointExtractionModule>("HomologousPoints",
                                                              otbGetTextMacro("Geometry/Homologous points extraction"));
  
  model->RegisterModule<otb::ObjectLabelingModule>("Object Labeling (Experimental)", otbGetTextMacro("Learning/Object Labeling"));
  model->RegisterModule<otb::GCPToSensorModelModule>("GCPToSensorModel",
                                                     otbGetTextMacro("Geometry/GCP to sensor model"));
  model->RegisterModule<otb::DEMToImageGeneratorModule>("DEM To Image Generator",
                                                     otbGetTextMacro("Geometry/DEM To Image Generator"));

  // Launch Monteverdi
  view->InitWidgets();
  view->Show();
  Fl::lock();

  //Test if there is an input image (optional)
  if (parseResult->IsOptionInputImagePresent())
    {
    Fl::check();
    std::vector<std::string> moduleVector;

    // Create an instance of module reader and get the ID
    std::string readerId = model->CreateModuleByKey("Reader");

    // Get the module itself
    otb::Module::Pointer module = model->GetModuleByInstanceId(readerId);

    // Simulate file chooser and ok callback
    // Cyrille cast effect !
    otb::ReaderModule::Pointer readerModule =
      static_cast<otb::ReaderModule::Pointer>(dynamic_cast<otb::ReaderModule *>(module.GetPointer()));
    readerModule->vFilePath->value(parseResult->GetInputImage().c_str());
    readerModule->Analyse();
    readerModule->bOk->do_callback();
    Fl::check();

    // Create an instance of module viewer and get the ID
    std::string viewerId = model->CreateModuleByKey("Viewer");

    // Get the module itself
    otb::Module::Pointer module2 = model->GetModuleByInstanceId(viewerId);

    // Open the viewer and simulate a connection
    otb::ViewerModule::Pointer viewerModule =
      static_cast<otb::ViewerModule::Pointer>(dynamic_cast<otb::ViewerModule *>(module2.GetPointer()));

    typedef otb::Module::InputDataDescriptorMapType InputDataDescriptorMapType;
    InputDataDescriptorMapType                 lInputDataMap = model->GetModuleInputsByInstanceId(viewerId);
    InputDataDescriptorMapType::const_iterator it_in;
    it_in = lInputDataMap.begin();

    std::string viewerInputKey = it_in->first;

    typedef otb::InputViewGUI::InputViewComponentMapType InputViewComponentMapType;
    InputViewComponentMapType inputComponentMap;
    inputComponentMap = view->GetInputViewGUI()->GetInputViewComponentMap();

    inputComponentMap[viewerInputKey]->SelectNthChoice(1);

    Fl::check();

    view->GetInputViewGUI()->bOk->do_callback();
    Fl::check();
    }

  if (parseResult->IsOptionPresent("--InputVectorData"))
    {
    Fl::check();
    std::vector<std::string> moduleVector;

    // Get the ModuleInstanceId
    std::string readerId = model->CreateModuleByKey("Reader");

    // Get the module itself
    otb::Module::Pointer module = model->GetModuleByInstanceId(readerId);

    // Simulate file chooser and ok callback
    otb::ReaderModule::Pointer readerModule =
      static_cast<otb::ReaderModule::Pointer>(dynamic_cast<otb::ReaderModule *>(module.GetPointer()));
    readerModule->vFilePath->value(parseResult->GetParameterString("--InputVectorData").c_str());
    readerModule->Analyse();
    readerModule->bOk->do_callback();
    Fl::check();
    }

  if (parseResult->IsOptionPresent("--ImageList"))
    {
      int numberOfImage = parseResult->GetNumberOfParameters("--ImageList");
      for (int i = 0; i < numberOfImage; i++)
        {
        Fl::check();
        std::vector<std::string> moduleVector;

        std::string filename = parseResult->GetParameterString("--ImageList", i);
        if( itksys::SystemTools::FileExists(filename.c_str()) )
          {
            // Get the ModuleInstanceId
            std::string readerId = model->CreateModuleByKey("Reader");
            
            // Get the module itself
            otb::Module::Pointer module = model->GetModuleByInstanceId(readerId);
            
            // Simulate file chooser and ok callback
            otb::ReaderModule::Pointer readerModule = static_cast<otb::ReaderModule::Pointer>(dynamic_cast<otb::ReaderModule *>(module.GetPointer()));
            readerModule->vFilePath->value(parseResult->GetParameterString("--ImageList", i).c_str());
            readerModule->Analyse();
            readerModule->bOk->do_callback();
            Fl::check();
          }
        else
          {
            itk::OStringStream oss;
            oss << "The file "<< filename <<" does not exist.";
            otb::MsgReporter::GetInstance()->SendError( oss.str().c_str() );
          }
        }
    }

  return Fl::run();
}
