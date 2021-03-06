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
#include "otbOpticalCalibrationModule.h"

#include "itksys/SystemTools.hxx"
#include "itkMetaDataDictionary.h"
#include "otbOpticalImageMetadataInterfaceFactory.h"
#include "otbOpticalImageMetadataInterface.h"
#include "FLU/Flu_File_Chooser.h"
#include "otbMsgReporter.h"

namespace otb
{
/** Constructor */
OpticalCalibrationModule::OpticalCalibrationModule()
{
  // This module needs pipeline locking
  this->NeedsPipelineLockingOn();

  m_LastPath = "";
  m_CanUpdateParameters = true;
  m_HelpTextBuffer = new Fl_Text_Buffer();

  // Add a new input
  this->AddInputDescriptor<ImageType>("InputImage", "Image to apply OpticalCalibration on");
}

/** Destructor */
OpticalCalibrationModule::~OpticalCalibrationModule()
{
  delete m_HelpTextBuffer;
}

/** PrintSelf method */
void OpticalCalibrationModule::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  // Call superclass implementation
  Superclass::PrintSelf(os, indent);
}

/** The custom run command */
void OpticalCalibrationModule::Run()
{
  // Until window closing, module will be busy
  this->BusyOn();

  // Here is the body of the module.
  // When the Run() method is called, necessary inputs have been
  // passed to the module.

  // First step is to retrieve the inputs

  // To handle an input with multiple supported type :
  m_InputImage = this->GetInputData<ImageType>("InputImage");

  // One of this pointer will be NULL:
  if (m_InputImage.IsNotNull())
    {
    m_InputImage->UpdateOutputInformation();
    // Build the GUI
    this->Build();
    }
  else
    {
    this->BusyOff();
    itkExceptionMacro(<< "Input image is NULL.");
    }

  if (this->CheckMetadata())
    {
    m_LastPath = this->GetInputDataDescription<ImageType>("InputImage", 0);

    this->Init();
    this->InitHelper();
    this->UpdateCoefSetup();
    // open the GUI
    this->Show();
    }
  else
    {
    this->BusyOff();
    return;
    }
}

bool
OpticalCalibrationModule
::CheckMetadata()
{
  // This line is copied from the method that calls this method to be able to use it through the test.
  if (m_InputImage.IsNull())
    {
    MsgReporter::GetInstance()->SendError("No input image detected.");
    return false;
    }

  itk::MetaDataDictionary             dict = m_InputImage->GetMetaDataDictionary();
  OpticalImageMetadataInterface::Pointer lImageMetadataInterface = OpticalImageMetadataInterfaceFactory::CreateIMI(dict);

  std::string sensorID = lImageMetadataInterface->GetSensorID();

  // // Test the sensor : only QB, IKONOS and Spot are supported
  // if (sensorID.find("QB02") == std::string::npos &&
  //     sensorID.find("Spot") == std::string::npos &&
  //     sensorID.find("WV02") == std::string::npos &&
  //     sensorID.find("Formosat") == std::string::npos &&
  //     sensorID.find("IKONOS-2") == std::string::npos)
  //   {
  //   MsgReporter::GetInstance()->SendError("Invalid input image. Only IKONOS-2, Spot4-5, QuickBird, WorldView2 and Formosat2 are supported.");
  //   return false;
  //   }

  // Test if needed data are available.
  try
    {
    // ImageToLuminance
    lImageMetadataInterface->GetPhysicalGain();
    lImageMetadataInterface->GetPhysicalBias();

    // LuminanceToReflectance
    lImageMetadataInterface->GetDay();
    lImageMetadataInterface->GetMonth();

    lImageMetadataInterface->GetSolarIrradiance();
    lImageMetadataInterface->GetSunElevation();

    //ReflectanceToSurfaceReflectance
    lImageMetadataInterface->GetSpectralSensitivity();

    }
  catch (itk::ExceptionObject& err)
    {
    std::ostringstream oss;
    oss.str("");
    oss << "Invalid input image medadata. The parsing returns the following error:\n";
    oss << err.GetDescription();
    MsgReporter::GetInstance()->SendError(oss.str().c_str());
    return false;
    }

  return true;

}

void
OpticalCalibrationModule
::Init()
{
  Fl_Text_Buffer *buff = new Fl_Text_Buffer();
  tdAtmoParam->buffer(buff);
  tdAtmoParam->redraw();

  // Instanciate filters
  m_ImageToLuminanceFilter                = ImageToLuminanceImageFilterType::New();
  m_LuminanceToReflectanceFilter          = LuminanceToReflectanceImageFilterType::New();
  m_ReflectanceToSurfaceReflectanceFilter = ReflectanceToSurfaceReflectanceImageFilterType::New();
  m_ParamAtmo                             = ReflectanceToSurfaceReflectanceImageFilterType::AtmoCorrectionParametersType::New();
  m_ParamAcqui                            = ReflectanceToSurfaceReflectanceImageFilterType::AcquiCorrectionParametersType::New();
  m_DifferenceFilter                      = DifferenceImageFilterType::New();
  // Link filters
  m_ImageToLuminanceFilter->SetInput(m_InputImage);
  m_LuminanceToReflectanceFilter->SetInput(m_ImageToLuminanceFilter->GetOutput());
  m_ReflectanceToSurfaceReflectanceFilter->SetInput(m_LuminanceToReflectanceFilter->GetOutput());
  m_DifferenceFilter->SetValidInput(m_LuminanceToReflectanceFilter->GetOutput());
  m_DifferenceFilter->SetTestInput(m_ReflectanceToSurfaceReflectanceFilter->GetOutput());

  // Init some acquisition parameters
  itk::MetaDataDictionary dict = m_InputImage->GetMetaDataDictionary();
  OpticalImageMetadataInterface::Pointer lImageMetadataInterface =
    OpticalImageMetadataInterfaceFactory::CreateIMI(dict);
  // Set spectral sensitivity
  if (lImageMetadataInterface->GetSpectralSensitivity()->Capacity() > 0)
    {
    m_ParamAcqui->SetWavelengthSpectralBand(lImageMetadataInterface->GetSpectralSensitivity());
    }
  // try to set the angles
  try
    {
    m_ParamAcqui->SetSolarZenithalAngle(90. - lImageMetadataInterface->GetSunElevation());
    m_ParamAcqui->SetSolarAzimutalAngle(lImageMetadataInterface->GetSunAzimuth());
    m_ParamAcqui->SetViewingZenithalAngle(90. - lImageMetadataInterface->GetSatElevation());
    m_ParamAcqui->SetViewingAzimutalAngle(lImageMetadataInterface->GetSatAzimuth());
    }
  catch (itk::ExceptionObject& err)
    {
    // silent catch, set angles to defaults
    m_ParamAcqui->SetSolarZenithalAngle(0.0);
    m_ParamAcqui->SetSolarAzimutalAngle(0.0);
    m_ParamAcqui->SetViewingZenithalAngle(0.0);
    m_ParamAcqui->SetViewingAzimutalAngle(0.0);
    }
  // try to set the date
  try
    {
    m_ParamAcqui->SetDay(lImageMetadataInterface->GetDay());
    m_ParamAcqui->SetMonth(lImageMetadataInterface->GetMonth());
    }
  catch (itk::ExceptionObject& err)
    {
    // silent catch, set date to default
    m_ParamAcqui->SetDay(0);
    m_ParamAcqui->SetMonth(0);
    }
  m_ReflectanceToSurfaceReflectanceFilter->SetAcquiCorrectionParameters(m_ParamAcqui);
  m_ReflectanceToSurfaceReflectanceFilter->SetAtmoCorrectionParameters(m_ParamAtmo);

  // Init aerosol model
  guiAerosolModel->value(0);
  // Init Band selection
  guiBandSelection->value(0);
  bProgress->minimum(0);
  bProgress->maximum(1);
  bProgress->value(1);
  bProgress->hide();

  this->UpdateCorrectionParameters();
  this->UpdateParametersCallback();
}

void
OpticalCalibrationModule
::UpdateInformation()
{
  std::ostringstream oss;
  oss.str("");
  oss << "Optical calibration module: ";
  std::string myFile(this->GetInputDataDescription<ImageType>("InputImage", 0));
  oss << itksys::SystemTools::GetFilenameName(myFile);
  oss << " (" << m_InputImage->GetNumberOfComponentsPerPixel();
  if (m_InputImage->GetNumberOfComponentsPerPixel() != 1) oss << " bands , ";
  else oss << " band , ";

  oss << m_InputImage->GetLargestPossibleRegion().GetSize() << ")";
  wMainWindow->copy_label(oss.str().c_str());
  m_LastPath = itksys::SystemTools::GetFilenamePath(myFile);
}

void
OpticalCalibrationModule
::InitHelper()
{
  tHelper->buffer(m_HelpTextBuffer);

  tHelper->insert("This file contains the spectral sensitivity of the sensor. \n");
  tHelper->insert("It should respect the following a design:\n");
  tHelper->insert("Format is MinSpectralValue MaxSpectralValue UserStep\n");
  tHelper->insert("the list of coefficients for each band.\n\n");

  tHelper->insert("Here is an imaginary example:\n");
  tHelper->insert("0.475 0.655 0.0026\n");
  tHelper->insert("0.000\n");
  tHelper->insert("...\n");
  tHelper->insert("1.000\n");
  tHelper->insert("...\n");
  tHelper->insert("0.000\n");
  tHelper->insert("0.695 0.755 0.0026\n");
  tHelper->insert("0.000\n");
  tHelper->insert("...\n");
  tHelper->insert("1.000\n");
  tHelper->insert("...\n");
  tHelper->insert("0.000\n");

  tHelper->insert("\n");
  tHelper->insert("Please note that:\n");
  tHelper->insert("- The spectral unit is 1e-6m.\n");
  tHelper->insert("- If the field UserStep is not specified, a default\n");
  tHelper->insert("  value of 0, 0025.1e-6m is considered.\n");
  tHelper->insert("- Order have an importance. You have to respect the band image order.\n");
  tHelper->insert("- If no spectral sensitivity file is specified, the module will use metadata\n");
  tHelper->insert("  to compute them.\n");

}

void
OpticalCalibrationModule
::UpdateCoefSetup()
{
  unsigned int       lNbComponent = m_InputImage->GetNumberOfComponentsPerPixel();
  std::ostringstream oss;
  oss.str("");

  guiBandSelection->clear();

  for (unsigned int i = 0; i < lNbComponent; ++i)
    {
    oss.str("");
    oss << i;
    guiBandSelection->add(oss.str().c_str());
    }

  tdAtmoParam->activate();

  m_ParamAtmo = m_ReflectanceToSurfaceReflectanceFilter->GetAtmoCorrectionParameters(); //chris
  guiOzoneAmount->value(m_ParamAtmo->GetOzoneAmount());
  guiAtmoPressure->value(m_ParamAtmo->GetAtmosphericPressure());
  guiAerosolModel->value(m_ParamAtmo->GetAerosolModel());
  guiWater->value(m_ParamAtmo->GetWaterVaporAmount());
  guiAeroTh->value(m_ParamAtmo->GetAerosolOptical());
  guiAerosolModel->redraw();
  guiOzoneAmount->redraw();
  guiAtmoPressure->redraw();
  guiWater->redraw();
  guiAeroTh->redraw();
}

void
OpticalCalibrationModule
::UpdateRadiativeTermsCallback()
{
  unsigned int ch = static_cast<unsigned int>(atoi(guiBandSelection->value()));

  AtmosphericRadiativeTerms::Pointer terms = m_ReflectanceToSurfaceReflectanceFilter->GetAtmosphericRadiativeTerms();

  gIntrinsicRef->value(terms->GetIntrinsicAtmosphericReflectance(ch));
  guiAlbedo->value(terms->GetSphericalAlbedo(ch));
  guiGasT->value(terms->GetTotalGaseousTransmission(ch));
  guiDT->value(terms->GetDownwardTransmittance(ch));
  guiUT->value(terms->GetUpwardTransmittance(ch));
  guiUDiffT->value(terms->GetUpwardDiffuseTransmittance(ch));
  guiUDirT->value(terms->GetUpwardDirectTransmittance(ch));
  guiUDTR->value(terms->GetUpwardDiffuseTransmittanceForRayleigh(ch));
  guiUDTA->value(terms->GetUpwardDiffuseTransmittanceForAerosol(ch));

  gIntrinsicRef->redraw();
  guiAlbedo->redraw();
  guiGasT->redraw();
  guiDT->redraw();
  guiUT->redraw();
  guiUDiffT->redraw();
  guiUDirT->redraw();
  guiUDTR->redraw();
  guiUDTA->redraw();
  gRadTerms->redraw();
}

void
OpticalCalibrationModule
::UpdateParamDisplay()
{
  std::ostringstream oss;
  oss.str("");
  AtmosphericRadiativeTerms::Pointer atmoTerms =  m_ReflectanceToSurfaceReflectanceFilter->GetAtmosphericRadiativeTerms();

  oss << atmoTerms;
  Fl_Text_Buffer *buff = new Fl_Text_Buffer();
  buff->text(oss.str().c_str());
  // Erase the first line (class name + pointer address)
  int start = buff->line_start(0);
  int stop = buff->line_end(0);
  buff->remove(start, stop + 1);
  tdParam->buffer(buff);
  tdParam->redraw();

  std::ostringstream oss2;
  oss2.str("");
  oss2 << m_ParamAtmo;
  Fl_Text_Buffer *buff2 = new Fl_Text_Buffer();
  buff2->text(oss2.str().c_str());
  // Erase the first line (class name + pointer address)
  start = buff2->line_start(0);
  stop = buff2->line_end(0);
  buff2->remove(start, stop + 1);
  tdAtmoParam->buffer(buff2);
  tdAtmoParam->redraw();

  std::string aeroFile = teAeronetFile->value();
  if (aeroFile != "")
    {
    guiWater->value(m_ParamAtmo /*param*/->GetWaterVaporAmount());
    guiAeroTh->value(m_ParamAtmo /*param*/->GetAerosolOptical());
    guiWater->redraw();
    guiAeroTh->redraw();
    }
}

void
OpticalCalibrationModule
::UpdateCorrectionParameters()
{
  AerosolModelType aeroMod = AtmosphericCorrectionParameters::NO_AEROSOL;
  std::string      aeroModStd = guiAerosolModel->value();

  if (aeroModStd == "NO AEROSOL") aeroMod = AtmosphericCorrectionParameters::NO_AEROSOL;
  else if (aeroModStd == "CONTINENTAL") aeroMod = AtmosphericCorrectionParameters::CONTINENTAL;
  else if (aeroModStd == "MARITIME") aeroMod = AtmosphericCorrectionParameters::MARITIME;
  else if (aeroModStd == "URBAN") aeroMod = AtmosphericCorrectionParameters::URBAN;
  else if (aeroModStd == "DESERTIC") aeroMod = AtmosphericCorrectionParameters::DESERTIC;
  else
    {
    itkExceptionMacro(<< "Invalid Aerosol Model Type: " << aeroModStd);
    }

  double ozAmount = guiOzoneAmount->value();
  double atmoPres = guiAtmoPressure->value();
  double aeroTh   = guiAeroTh->value();
  double waterAm  = guiWater->value();

  bool        aeronetFile = false;
  std::string aeroFile = teAeronetFile->value();
  if (aeroFile != m_ParamAtmo->GetAeronetFileName() && aeroFile != "")
    {
    m_ParamAtmo->SetAeronetFileName(aeroFile);
    m_ReflectanceToSurfaceReflectanceFilter->SetAtmoCorrectionParameters(m_ParamAtmo);
    aeronetFile = true;
    }

  itk::MetaDataDictionary             dict = m_InputImage->GetMetaDataDictionary();
  OpticalImageMetadataInterface::Pointer lImageMetadataInterface = OpticalImageMetadataInterfaceFactory::CreateIMI(dict);

  std::string ffvFile = teFFVFile->value();
  if (ffvFile != m_ParamAcqui->GetFilterFunctionValuesFileName() &&  ffvFile != "")
    {
    m_ParamAcqui->SetFilterFunctionValuesFileName(ffvFile);
    }


  try
    {
    m_ParamAtmo->SetAerosolModel(aeroMod);
    m_ParamAtmo->SetOzoneAmount(ozAmount);
    m_ParamAtmo->SetAtmosphericPressure(atmoPres);

    if (!aeronetFile)
      {
      m_ParamAtmo->SetAerosolOptical(aeroTh);
      m_ParamAtmo->SetWaterVaporAmount(waterAm);
      }

    m_ReflectanceToSurfaceReflectanceFilter->SetIsSetAtmosphericRadiativeTerms(false);
    m_ReflectanceToSurfaceReflectanceFilter->SetUseGenerateParameters(true);
    bProgress->show();
    Fl::check();
    m_ReflectanceToSurfaceReflectanceFilter->GenerateParameters();
    m_ReflectanceToSurfaceReflectanceFilter->UpdateOutputInformation();

    bProgress->hide();
    Fl::check();
    m_ReflectanceToSurfaceReflectanceFilter->SetUseGenerateParameters(false);

    this->UpdateParamDisplay();
    }
  catch (itk::ExceptionObject& err)
    {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    }
}

void
OpticalCalibrationModule
::ReloadChannelTermsCallback(bool updateIm)
{
  //this->UpdateRadiativeTerms( updateIm );
  unsigned int ch = static_cast<unsigned int>(atoi(guiBandSelection->value()));
  double       intRef   = gIntrinsicRef->value();
  double       albedo   = guiAlbedo->value();
  double       gasT     = guiGasT->value();
  double       dT       = guiDT->value();
  double       uT       = guiUT->value();
  double       uDiffT   = guiUDiffT->value();
  double       uDirR    = guiUDirT->value();
  double       uDTR     = guiUDTR->value();
  double       uDTA     = guiUDTA->value();

  try
    {
    //m_Model->UpdateRadiativeTerm( ch, intRef, albedo, gasT, dT, uT, uDiffT, uDirR, uDTR, uDTA );
    AtmosphericRadiativeTerms::Pointer atmoTerms =
      m_ReflectanceToSurfaceReflectanceFilter->GetAtmosphericRadiativeTerms();
    atmoTerms->SetIntrinsicAtmosphericReflectance(ch, intRef);
    atmoTerms->SetSphericalAlbedo(ch, albedo);
    atmoTerms->SetTotalGaseousTransmission(ch, gasT);
    atmoTerms->SetDownwardTransmittance(ch, dT);
    atmoTerms->SetUpwardTransmittance(ch, uT);
    atmoTerms->SetUpwardDiffuseTransmittance(ch, uDiffT);
    atmoTerms->SetUpwardDirectTransmittance(ch, uDirR);
    atmoTerms->SetUpwardDiffuseTransmittanceForRayleigh(ch, uDTR);
    atmoTerms->SetUpwardDiffuseTransmittanceForAerosol(ch, uDTA);

    m_CanUpdateParameters = true;

    /** param update is done by the controller, that allows to change each channel without computed is parameters each time*/
    if (updateIm == true)
      {
      m_ReflectanceToSurfaceReflectanceFilter->SetIsSetAtmosphericRadiativeTerms(true);
      bProgress->show();
      Fl::check();
      m_ReflectanceToSurfaceReflectanceFilter->GenerateParameters();
      bProgress->hide();
      Fl::check();

      m_ReflectanceToSurfaceReflectanceFilter->SetIsSetAtmosphericRadiativeTerms(false);
      m_CanUpdateParameters = false;
      }
    }
  catch (itk::ExceptionObject& err)
  {
    MsgReporter::GetInstance()->SendError(
        std::string("The Input Image of the ReflectanceToSurfaceReflectanceFilter is not initialized: ")
    + err.GetDescription() );
  }

  this->UpdateParamDisplay();

  Fl_Text_Buffer *buff = new Fl_Text_Buffer();
  buff->text("");
  tdAtmoParam->buffer(buff);
  tdAtmoParam->redraw();
}

void
OpticalCalibrationModule
::OK()
{
  this->ClearOutputDescriptors();

  // Add outputs
  this->AddOutputDescriptor(m_ImageToLuminanceFilter->GetOutput(), "Luminance image", "Luminance image");

  std::cout << "filter function: " << m_ParamAcqui->GetWavelengthSpectralBand() << std::endl; //chris

  if (bChangeScale->value() == 1)
    {
    m_TOAMultiplier        = MultiplyByScalarImageFilterType::New();
    m_TOCMultiplier        = MultiplyByScalarImageFilterType::New();
    m_DiffTOATOCMultiplier = MultiplyByScalarImageFilterType::New();

    m_TOAMultiplier->SetCoef(1000.);
    m_TOCMultiplier->SetCoef(1000.);
    m_DiffTOATOCMultiplier->SetCoef(1000.);

    m_TOAMultiplier->SetInput(m_LuminanceToReflectanceFilter->GetOutput());
    m_TOCMultiplier->SetInput(m_ReflectanceToSurfaceReflectanceFilter->GetOutput());
    m_DiffTOATOCMultiplier->SetInput(m_DifferenceFilter->GetOutput());

    this->AddOutputDescriptor(m_TOAMultiplier->GetOutput(), "TOA image (*1000)", "TOA image 1000");
    this->AddOutputDescriptor(m_TOCMultiplier->GetOutput(), "TOC image (*1000)", "TOC image 1000");
    this->AddOutputDescriptor(m_DiffTOATOCMultiplier->GetOutput(), "Difference TOA-TOC image (*1000)",
                              "Difference TOA-TOC image 1000");
    }
  else
    {
    this->AddOutputDescriptor(m_LuminanceToReflectanceFilter->GetOutput(), "TOA image", "TOA Image");
    this->AddOutputDescriptor(m_ReflectanceToSurfaceReflectanceFilter->GetOutput(), "TOC image",
                              "TOC Image");
    this->AddOutputDescriptor(m_DifferenceFilter->GetOutput(), "Difference TOA-TOC image",
                              "Difference TOA-TOC image");
    }

  // Send an event to Monteverdi application
  this->NotifyAll(MonteverdiEvent("OutputsUpdated", m_InstanceId));

  // Once module is closed, it is no longer busy
  this->BusyOff();
}

void
OpticalCalibrationModule
::OpenAeronetFileCallback()
{
  const char * cfname = flu_file_chooser("Pick an Aeronet file...", "*.*", "");

  if (cfname == NULL)
    {
    otbMsgDebugMacro(<< "Empty file name!");
    return;
    }

  m_LastPath = itksys::SystemTools::GetFilenamePath(cfname);
  teAeronetFile->value(cfname);
  teAeronetFile->redraw();
}

void
OpticalCalibrationModule
::OpenFFVFileCallback()
{
  const char * cfname = flu_file_chooser("Pick an Filter Function Values file...", "*.*", "");

  if (cfname == NULL)
    {
    otbMsgDebugMacro(<< "Empty file name!");
    return;
    }

  m_LastPath = itksys::SystemTools::GetFilenamePath(cfname);
  teFFVFile->value(cfname);
  teFFVFile->redraw();
}

void
OpticalCalibrationModule
::SaveAndQuitCallback()
{
  this->UpdateParametersCallback();

  this->OK();
  this->QuitCallback();
}

void
OpticalCalibrationModule
::UpdateParametersCallback()
{
  // Check if some parameters have changed
  // in tRadTerm tab (through m_CanUpdateParameters)

  if (!m_CanUpdateParameters)
    {
    //  in Correction Parameters tab checking each value
    AerosolModelType aeroMod = AtmosphericCorrectionParameters::NO_AEROSOL;
    std::string      aeroModStd = guiAerosolModel->value();

    if (aeroModStd == "NO AEROSOL") aeroMod = AtmosphericCorrectionParameters::NO_AEROSOL;
    else if (aeroModStd == "CONTINENTAL") aeroMod = AtmosphericCorrectionParameters::CONTINENTAL;
    else if (aeroModStd == "MARITIME") aeroMod = AtmosphericCorrectionParameters::MARITIME;
    else if (aeroModStd == "URBAN") aeroMod = AtmosphericCorrectionParameters::URBAN;
    else if (aeroModStd == "DESERTIC") aeroMod = AtmosphericCorrectionParameters::DESERTIC;
    else
      {
      itkExceptionMacro(<< "Invalid Aerosol Model Type: " << aeroModStd);
      }

    double      ozAmount = guiOzoneAmount->value();
    double      atmoPres = guiAtmoPressure->value();
    double      aeroTh   = guiAeroTh->value();
    double      waterAm  = guiWater->value();
    std::string aeroFile = teAeronetFile->value();
    std::string ffvFile = teFFVFile->value();

    if (static_cast<AerosolModelType>(aeroMod) != m_ParamAtmo->GetAerosolModel()) m_CanUpdateParameters = true;
    else if (ozAmount != m_ParamAtmo->GetOzoneAmount()) m_CanUpdateParameters = true;
    else if (atmoPres != m_ParamAtmo->GetAtmosphericPressure()) m_CanUpdateParameters = true;
    else if (ffvFile !=
             m_ParamAcqui->GetFilterFunctionValuesFileName()) m_CanUpdateParameters = true;
    else if (aeroFile != m_ParamAtmo->GetAeronetFileName()) m_CanUpdateParameters = true;
    else if (aeroTh != m_ParamAtmo->GetAerosolOptical()) m_CanUpdateParameters = true;
    else if (waterAm != m_ParamAtmo->GetWaterVaporAmount()) m_CanUpdateParameters = true;
    }

  if (!m_CanUpdateParameters) return;

  // Don't update if nothing changed
  if (tCorrParam->visible() == 1)
    {
    this->UpdateCorrectionParameters();
    tdAtmoParam->show();
    // update the displayed channel radiative terms value
    this->UpdateRadiativeTermsCallback();
    }
  else if (tRadTerm->visible() == 1) this->ReloadChannelTermsCallback(true);

  m_CanUpdateParameters = false;
}

void
OpticalCalibrationModule
::QuitCallback()
{
  wMainWindow->hide();
  this->BusyOff();
}

} // End namespace otb
