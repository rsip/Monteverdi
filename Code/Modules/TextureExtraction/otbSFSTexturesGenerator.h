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
#ifndef __otbSFSTexturesGenerator_h
#define __otbSFSTexturesGenerator_h

#include "otbFeatureTex.h"
#include "otbTextureExtractionModel.h"
#include "otbSFSTexturesImageFilter.h"

namespace otb
{
/** \class SFSTexturesGenerator
 *
 *
 *
 */
class SFSTexturesGenerator
{
public:
  typedef TextureExtractionModel            ModelType;
  typedef ModelType::Pointer                ModelPointerType;
  typedef ModelType::FilterType             FilterType;
  typedef ModelType::PixelType              PixelType;
  typedef ModelType::SingleImageType        SingleImageType;
  typedef ModelType::SingleImagePointerType SingleImagePointerType;
  typedef FeatureInfoTex::FeatureType       FeatureType;
  typedef FeatureInfoBase::FeatureType      FeatureBaseType;

  /***************************/
  /** Filter type declaration*/
  /***************************/

  typedef SFSTexturesImageFilter<SingleImageType, SingleImageType> SFSTexturesFilterType;

  /************/
  /** Methods */
  /************/

  template <class TFilterTypeMethod> typename TFilterTypeMethod::Pointer GenericAddRAndNIRFilter(ModelPointerType model,
                                                                                                 FeatureBaseType type,
                                                                                                 unsigned int redId,
                                                                                                 unsigned int nirId,
                                                                                                 std::string msg);

  // AddSFSTexturesFilter
  void AddSFSTexturesFilter(ModelPointerType pModel,
                            FeatureBaseType pType,
                            double pSpectralThr,
                            unsigned int pSpatialThr,
                            unsigned int pNbDirection,
                            double pAlpha,
                            unsigned int pRatioMaxConsNb)
  {

    for (unsigned int i = 0; i < pModel->GetInputImageList()->Size(); i++)
      {
      SFSTexturesFilterType::Pointer filter = SFSTexturesFilterType::New();
      filter->SetSpatialThreshold(pSpatialThr);
      filter->SetSpectralThreshold(pSpectralThr);
      filter->SetRatioMaxConsiderationNumber(pRatioMaxConsNb);
      filter->SetAlpha(pAlpha);
      filter->SetNumberOfDirections(pNbDirection);
      filter->SetInput(pModel->GetInputImageList()->GetNthElement(i));
      filter->InitFeatureStatus(false);
      std::ostringstream oss;

      switch (pType)
        {
        case FeatureInfoTex::SFS_LEN:
          {
          oss << "SFS Length: ";
          filter->SetFeatureStatus(SFSTexturesFilterType::LENGTH, true); //LENGTH, SFSTexturesFilterType::WIDTH, PSI, WMEAN, RATIO, SD
          break;
          }
        case FeatureInfoTex::SFS_WID:
          {
          oss << "SFS Width: ";
          filter->SetFeatureStatus(SFSTexturesFilterType::WIDTH, true);
          break;
          }
        case FeatureInfoTex::SFS_PSI:
          {
          oss << "SFS PSI: ";
          filter->SetFeatureStatus(SFSTexturesFilterType::PSI, true);
          break;
          }
        case FeatureInfoTex::SFS_WME:
          {
          oss << "SFS W-mean: ";
          filter->SetFeatureStatus(SFSTexturesFilterType::WMEAN, true);
          break;
          }
        case FeatureInfoTex::SFS_RAT:
          {
          oss << "SFS Ratio: ";
          filter->SetFeatureStatus(SFSTexturesFilterType::RATIO, true);
          break;
          }
        case FeatureInfoTex::SFS_SD:
          {
          oss << "SFS SD: ";
          filter->SetFeatureStatus(SFSTexturesFilterType::SD, true);
          break;
          }
        default:
          {
          }
        }

      oss << pSpatialThr << "," << pSpectralThr << "," << pNbDirection << "," << pAlpha << "," << pRatioMaxConsNb;
      std::string mess = oss.str();
      pModel->AddFeatureFilter(filter, pType, i, 0, mess);
      }
  }

  // GenerateSFSTextureOutputImage
  SingleImagePointerType GenerateSFSTextureOutputImage(ModelPointerType pModel,
                                                       FeatureBaseType pType,
                                                       unsigned int pInputListId)
  {
    SingleImagePointerType         image =  SingleImageType::New();
    SFSTexturesFilterType::Pointer filter =
      dynamic_cast<SFSTexturesFilterType*>(static_cast<FilterType *>(pModel->GetFilterList()->GetNthElement(
                                                                       pInputListId)));
    switch (pType)
      {
      case FeatureInfoTex::SFS_LEN:
        {
        image = filter->GetLengthOutput();
        break;
        }
      case FeatureInfoTex::SFS_WID:
        {
        image = filter->GetWidthOutput();
        break;
        }
      case FeatureInfoTex::SFS_PSI:
        {
        image = filter->GetPSIOutput();
        break;
        }
      case FeatureInfoTex::SFS_WME:
        {
        image = filter->GetWMeanOutput();
        break;
        }
      case FeatureInfoTex::SFS_RAT:
        {
        image = filter->GetRatioOutput();
        break;
        }
      case FeatureInfoTex::SFS_SD:
        {
        image = filter->GetSDOutput();
        break;
        }
      default:
        {
        }
      }
    return image;
  }

protected:

private:

};

}
#endif
