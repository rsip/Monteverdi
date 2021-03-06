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

#ifndef __otbGCPToSensorModelControllerInterface_h
#define __otbGCPToSensorModelControllerInterface_h

#include "itkObject.h"
#include "itkContinuousIndex.h"
#include "otbGCPToSensorModelModel.h"

namespace otb
{
class ITK_ABI_EXPORT GCPToSensorModelControllerInterface
  : public itk::Object
{
public:
  /** Standard class typedefs */
  typedef GCPToSensorModelControllerInterface Self;
  typedef itk::Object                         Superclass;
  typedef itk::SmartPointer<Self>             Pointer;
  typedef itk::SmartPointer<const Self>       ConstPointer;

  typedef GCPToSensorModelModel  ModelType;
  typedef itk::ContinuousIndex<> ContinuousIndexType;

  /** Standard type macros */
  itkTypeMacro(GCPToSensorModelControllerInterface, Superclass);

  /** Users actions */
  virtual void LinkPixelDescriptors() = 0;
  virtual void AddPoints(float x, float y, float lon, float lat, float elev) = 0;
  virtual void ClearPointList() = 0;
  virtual void DeletePointFromList(unsigned int id) = 0;
  virtual void LeftMouseButtonClicked(ContinuousIndexType index) = 0;
  virtual void FocusOn(ContinuousIndexType id) = 0;
  virtual void SetDEMPath(const std::string& filePath) = 0;
  virtual void ChangeDEM() = 0;
  virtual void ReloadGCPsList() = 0;
  virtual bool OK() = 0;
  virtual void Quit() = 0;

  virtual void SearchPlaceName(double longitude, double latitude) = 0;
  virtual void SearchLonLat(std::string placename) = 0;
  virtual void DisplayMap(std::string placename,
                          double longitude,
                          double latitude,
                          unsigned int depth,
                          long int sizeX,
                          long int sizeY) = 0;
  virtual void ExportGcpsToXmlFile(const char * fname) = 0;
  virtual void ImportGcpsFromXmlFile(const char * fname) = 0;

  virtual ModelType* GetModel() = 0;

protected:
  /** Constructor */
  GCPToSensorModelControllerInterface() {}
  /** Destructor */
  ~GCPToSensorModelControllerInterface() {}

private:
  GCPToSensorModelControllerInterface(const Self&); //purposely not implemented
  void operator =(const Self&); //purposely not implemented
};
} // end namespace otb

#endif
