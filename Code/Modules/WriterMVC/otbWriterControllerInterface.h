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
#ifndef __otbWriterControllerInterface_h
#define __otbWriterControllerInterface_h

#include "otbWriterModel.h"
#include "itkProcessObject.h"
#include "otbImageWidgetController.h"

namespace otb
{
class ITK_EXPORT  WriterControllerInterface
: public itk::ProcessObject
{
public:
  /** Standard class typedefs */
  typedef WriterControllerInterface Self;
  typedef itk::ProcessObject                   Superclass;
  typedef itk::SmartPointer<Self>              Pointer;
  typedef itk::SmartPointer<const Self>        ConstPointer;

  typedef WriterModel               ModelType;
  




  /** Standard type macros */
  itkTypeMacro(WriterControllerInterface,Superclass);

  /** Users actions */
  virtual void OpenInputImage(const char * filename) =0;
//   virtual void SaveOutput() =0;
//   virtual void InitInput() =0;
//   virtual void ClearFeatures() =0;
// //   virtual void CreateFeature(FeatureType featureType) =0;
//   virtual void ClearSelectedChannels() =0;
//   virtual void SetOutputFileName( const char * name ) =0;
//   virtual void AddInputChannels(std::vector<unsigned int> list) =0;
//   virtual void ChangeFilterStatus(int id)=0;
//   virtual void ViewedRegionChanged()=0;
//   virtual void AddToOutputListOrder(int id)=0;
//   virtual void RemoveFromOutputListOrder(int id)=0;
//   virtual void ExchangeOutputListOrder( int direction )=0;
  


protected:
  /** Constructor */
  WriterControllerInterface()
  {
    m_Model = ModelType::GetInstance();
  }
  /** Destructor */
  ~WriterControllerInterface() {};

  /** The instance of the model (for commodity) */
  ModelType * m_Model;

private:
  WriterControllerInterface(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};
} // end namespace otb

#endif