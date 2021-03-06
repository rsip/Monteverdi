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
#include "otbProjectionModel.h"
#include <ogr_spatialref.h>
#include "otbMapProjections.h"
#include <string>

#include "vnl/vnl_random.h"

#include "otbImageToGenericRSOutputParameters.h"

namespace otb
{

/**
 * Constructor
 */
ProjectionModel::ProjectionModel()
{
  m_OutputChanged = false;
  m_TransformChanged = false;
  m_TempTransformChanged = false;
  m_UseDEM = false;
  m_EstimateInputRPCModel = false;

  m_AvgElevation = 0.;

  m_Resampler = ResampleFilterType::New();

  m_Transform = TransformType::New();
  m_InverseTransform = TransformType::New();
}

/**
 * Destructor
 */
ProjectionModel::~ProjectionModel()
{
}

///////////////////////////////////////////////////////////////
/**     CASE : input transform is not allowed */
/////////////// /////////////// /////////////// ///////////////

/**
 * Update the input Projection Ref
 */
void ProjectionModel::UpdateInputUTMTransform(int zone, bool north)
{
  typedef UtmInverseProjection UtmProjectionType;
  UtmProjectionType::Pointer utmProjection = UtmProjectionType::New();

  std::string hem = "N";
  if (!north) hem = "S";

  utmProjection->SetHemisphere(hem[0]);
  utmProjection->SetZone(zone);
  std::string utmRef = utmProjection->GetWkt();

  // Report projection ref (not done by the resample filter)
  itk::MetaDataDictionary& dict = m_InputImage->GetMetaDataDictionary();
  itk::EncapsulateMetaData<std::string>(dict, MetaDataKey::ProjectionRefKey, utmRef);

  // Up To date the transform
  m_Transform->SetInputProjectionRef(utmRef);
  m_Transform->SetOutputProjectionRef(m_OutputProjectionRef);
  m_Transform->InstanciateTransform();

  m_TempTransformChanged = true;
  this->NotifyAll();
  m_TempTransformChanged = false;
}

/**
 * Update the input Projection LambertII Projection Ref
 */
void ProjectionModel::InitializeInputLambertIITransform()
{
  typedef Lambert2EtenduForwardProjection Lambert2Type;
  Lambert2Type::Pointer lambert2Projection = Lambert2Type::New();
  std::string lambertRef = lambert2Projection->GetWkt();

  // Report projection ref (not done by the resample filter)
  itk::MetaDataDictionary& dict = m_InputImage->GetMetaDataDictionary();
  itk::EncapsulateMetaData<std::string>(dict, MetaDataKey::ProjectionRefKey, lambertRef);

  // Update the transform
  m_Transform->SetInputProjectionRef(lambertRef);
  m_Transform->SetOutputProjectionRef(m_OutputProjectionRef);
  m_Transform->InstanciateTransform();

  m_TempTransformChanged = true;
  this->NotifyAll();
  m_TempTransformChanged = false;
}

/**
 * Initialize the Transmercator Projection and Transform
 */
/*void ProjectionModel::UpdateInputTMTransform(double scale, double falseEasting, double falseNorthing)
{
  typedef otb::TransMercatorInverseProjection TransMercatorProjectionType;
  TransMercatorProjectionType::Pointer transMercatorProjection = TransMercatorProjectionType::New();
  transMercatorProjection->SetParameters(falseEasting, falseNorthing, scale);
  std::string tmRef = transMercatorProjection->GetWkt();

  // Report projection ref (not done by the resample filter)
  itk::MetaDataDictionary& dict = m_InputImage->GetMetaDataDictionary();
  itk::EncapsulateMetaData<std::string>(dict, MetaDataKey::ProjectionRefKey, tmRef);

  // Up To date the transform
  m_Transform->SetInputProjectionRef(tmRef);
  m_Transform->SetOutputProjectionRef(m_OutputProjectionRef);
  m_Transform->InstanciateTransform();

  m_TempTransformChanged = true;
  this->NotifyAll();
  m_TempTransformChanged = false;
}*/

void ProjectionModel::UpdateInputEPSGTransform(int rsidNumber)
{

  std::string epsgRef = otb::GeoInformationConversion::ToWKT(rsidNumber);

  // Report projection ref (not done by the resample filter)
  itk::MetaDataDictionary& dict = m_InputImage->GetMetaDataDictionary();
  itk::EncapsulateMetaData<std::string>(dict, MetaDataKey::ProjectionRefKey, epsgRef);

  // Up To date the transform
  m_Transform->SetInputProjectionRef(epsgRef);
  m_Transform->SetOutputProjectionRef(m_OutputProjectionRef);
  m_Transform->InstanciateTransform();

  m_TempTransformChanged = true;
  this->NotifyAll();
  m_TempTransformChanged = false;

}

//////////////////////////////////////////////////////////////////
/**     END CASE : input transform is not accessible */
//////////////////////////////////////////////////////////////////

/**
 *
 */
void ProjectionModel::UpdateUTMTransform(int zone, bool hemisphere)
{
  typedef UtmInverseProjection UtmProjectionType;
  UtmProjectionType::Pointer utmProjection = UtmProjectionType::New();

  std::string hem = "N";
  if (!hemisphere) hem = "S";

  utmProjection->SetHemisphere(hem[0]);
  utmProjection->SetZone(zone);
  m_OutputProjectionRef = utmProjection->GetWkt();

  // Build the Generic RS transform
  m_Transform->SetInputProjectionRef(m_InputImage->GetProjectionRef());
  m_Transform->SetInputDictionary(m_InputImage->GetMetaDataDictionary());

  m_Transform->SetOutputProjectionRef(m_OutputProjectionRef);
  m_Transform->InstanciateTransform();

  // Get the transform
  m_Transform->GetInverse(m_InverseTransform);

  // Update the output image parameters
  this->UpdateOutputParameters();

  //Notify the view that the transform has changed
  m_TransformChanged = true;
  this->NotifyAll();
  m_TransformChanged = false;
}

void ProjectionModel::UpdateEPSGTransform(int rsidNumber)
{

  m_OutputProjectionRef = otb::GeoInformationConversion::ToWKT(rsidNumber);

  // Build the Generic RS transform
  m_Transform->SetInputProjectionRef(m_InputImage->GetProjectionRef());
  m_Transform->SetInputDictionary(m_InputImage->GetMetaDataDictionary());

  m_Transform->SetOutputProjectionRef(m_OutputProjectionRef);
  m_Transform->InstanciateTransform();

  // Get the inverse
  m_Transform->GetInverse(m_InverseTransform);

  this->UpdateOutputParameters();

  m_TransformChanged = true;
  this->NotifyAll();
  m_TransformChanged = false;

}

/**
 * Initialize the Remote Sensing Transform
 */
void ProjectionModel::InitializeLambertIITransform()
{
  typedef Lambert2EtenduForwardProjection Lambert2Type;
  Lambert2Type::Pointer lambert2Projection = Lambert2Type::New();
  m_OutputProjectionRef = lambert2Projection->GetWkt();

  // Build the Generic RS transform
  m_Transform->SetInputProjectionRef(m_InputImage->GetProjectionRef());
  m_Transform->SetInputDictionary(m_InputImage->GetMetaDataDictionary());

  m_Transform->SetOutputProjectionRef(m_OutputProjectionRef);
  m_Transform->InstanciateTransform();

  // Get the inverse
  m_Transform->GetInverse(m_InverseTransform);

  this->UpdateOutputParameters();

  m_TransformChanged = true;
  this->NotifyAll();
  m_TransformChanged = false;
}

/**
 * Initialize the Transmercator Projection and Transform
 */
/*void ProjectionModel::UpdateTMTransform(double scale, double falseEasting, double falseNorthing)
{
  typedef otb::TransMercatorInverseProjection TransMercatorProjectionType;
  TransMercatorProjectionType::Pointer transMercatorProjection = TransMercatorProjectionType::New();
  transMercatorProjection->SetParameters(falseEasting, falseNorthing, scale);
  m_OutputProjectionRef = transMercatorProjection->GetWkt();

  // Build the Generic RS transform
  m_Transform->SetInputProjectionRef(m_InputImage->GetProjectionRef());
  m_Transform->SetInputDictionary(m_InputImage->GetMetaDataDictionary());

  m_Transform->SetOutputProjectionRef(m_OutputProjectionRef);
  m_Transform->InstanciateTransform();

  // Instanciate the inverse RS transform
  m_Transform->GetInverse(m_InverseTransform);

  this->UpdateOutputParameters();

  m_TransformChanged = true;
  this->NotifyAll();
  m_TransformChanged = false;
}*/

/**
 *
 */
void ProjectionModel::UpdateWGS84Transform()
{
  // Build the Output Projection Ref
  // WGS84
  char * wgs84Ref;
  OGRSpatialReference oSRS;
  oSRS.SetWellKnownGeogCS("WGS84");
  oSRS.exportToWkt(&wgs84Ref);
  m_OutputProjectionRef = wgs84Ref;

  // Build the Generic RS transform
  m_Transform->SetInputProjectionRef(m_InputImage->GetProjectionRef());
  m_Transform->SetInputDictionary(m_InputImage->GetMetaDataDictionary());

  m_Transform->SetOutputProjectionRef(m_OutputProjectionRef);
  m_Transform->InstanciateTransform();

  // Get the transform
  m_Transform->GetInverse(m_InverseTransform);

  // Update the output image parameters
  this->UpdateOutputParameters();

  //Notify the view that the transform has changed
  m_TransformChanged = true;
  this->NotifyAll();
  m_TransformChanged = false;
}

/**
 * Compute the new Origin, Spacing & Size of the output
 */
void ProjectionModel::UpdateOutputParameters()
{
  // Ccompute the output parameters stuff
  typedef otb::ImageToGenericRSOutputParameters<InputImageType> OutputParamEstimatorType;
  OutputParamEstimatorType::Pointer estimator = OutputParamEstimatorType::New();

  estimator->SetInput(m_InputImage);
  estimator->SetOutputProjectionRef(m_OutputProjectionRef);
  estimator->Compute();

  // Keep the max absolute spacing value
  double maxAbsSpVal = std::max( std::abs(estimator->GetOutputSpacing()[0]),
                                 std::abs(estimator->GetOutputSpacing()[1]));

  // Edit the output image parmaters
  m_OutputOrigin = estimator->GetOutputOrigin();
  // Force the spacing to be samein X and Y
  m_OutputSpacing[0] = estimator->GetOutputSpacing()[0] > 0 ? maxAbsSpVal : -maxAbsSpVal;
  m_OutputSpacing[1] = estimator->GetOutputSpacing()[1] > 0 ? maxAbsSpVal : -maxAbsSpVal;
  m_OutputSize = estimator->GetOutputSize();

  // Keep a copy of the origin of the whole projected image
  m_WholeOutputOrigin = estimator->GetOutputOrigin();
  m_WholeOutputSpacing = estimator->GetOutputSpacing();
  // Force the spacing to be samein X and Y
  m_WholeOutputSpacing[0] = estimator->GetOutputSpacing()[0] > 0 ? maxAbsSpVal : -maxAbsSpVal;
  m_WholeOutputSpacing[1] = estimator->GetOutputSpacing()[1] > 0 ? maxAbsSpVal : -maxAbsSpVal;
  m_WholeOutputSize = estimator->GetOutputSize();

}

/**
 *
 */
void ProjectionModel::ProjectRegion(unsigned int sizeX, unsigned int sizeY, double spacingX, double spacingY,
                                    double originX, double originY)
{
  // Edit the size
  m_OutputSize[0] = sizeX;
  m_OutputSize[1] = sizeY;

  // Edit the spacing
  m_OutputSpacing[0] = spacingX;
  m_OutputSpacing[1] = spacingY;

  // Edit the origin in the cartographic projection
  OutputPointType geoPoint;
  geoPoint[0] = originX;
  geoPoint[1] = originY;
  m_OutputOrigin = m_Transform->GetTransform()->GetSecondTransform()->TransformPoint(geoPoint);
}

/**
 *
 */
void ProjectionModel::ReprojectImage()
{
  // Fill the resampler parameters
  m_Resampler->SetInput(m_InputImage);
  m_Resampler->SetOutputSize(m_OutputSize);
  m_Resampler->SetOutputSpacing(m_OutputSpacing);
  m_Resampler->SetOutputOrigin(m_OutputOrigin);
  m_Resampler->SetOutputProjectionRef(m_Transform->GetOutputProjectionRef());

  // Estimate Input RPC Model
  if (m_EstimateInputRPCModel)
    {
    m_Resampler->EstimateInputRpcModelOn();
    m_Resampler->SetInputRpcGridSize(20);
    }

  // Use the DEM
  // if (m_UseDEM)
  //   {
  //   m_Resampler->SetDEMDirectory(m_Transform->GetDEMDirectory());
  //   }

  // Default padding value
  InputImageType::PixelType defaultValue;
  //itk::PixelBuilder<InputImageType::PixelType>::Zero(defaultValue, m_InputImage->GetNumberOfComponentsPerPixel());
  itk::NumericTraits<InputImageType::PixelType>::SetLength(defaultValue, m_InputImage->GetNumberOfComponentsPerPixel());
  m_Resampler->SetEdgePaddingValue(defaultValue);

  m_Output = m_Resampler->GetOutput();
  m_OutputChanged = true;
  this->NotifyAll();
  m_OutputChanged = false;
}

/**
 *
 */
void ProjectionModel::SetDEMPath(std::string dem)
{
  otb::DEMHandler::Instance()->SetDefaultHeightAboveEllipsoid(m_AvgElevation);

  if (m_UseDEM)
    {
    // Update the dem directory
    otb::DEMHandler::Instance()->OpenDEMDirectory(dem);
    }

  m_Transform->InstanciateTransform();
  m_Transform->GetInverse(m_InverseTransform);
}

/**
 *
 */
void ProjectionModel::NotifyListener(ListenerBase * listener)
{
  listener->Notify();
}

} // End namespace
