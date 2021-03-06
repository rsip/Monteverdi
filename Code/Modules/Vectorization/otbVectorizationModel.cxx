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
#include "otbVectorizationModel.h"
#include "itkPreOrderTreeIterator.h"

#include "otbVectorDataIntoImageProjectionFilter.h"
#include "otbMsgReporter.h"

namespace otb
{

VectorizationModel::
VectorizationModel(): m_DEMPath(""), m_UseDEM(false),
                      m_ExtractRegionUpdated(false),
                      m_ActualLayerNumber(0)
{
  // Visualization
  m_VisualizationModel  = VisualizationModelType::New();
  m_ImageGenerator      = LayerGeneratorType::New();

  // Input & Output
  m_InputImage = VectorImageType::New();

  // VectorData model
  m_VectorDataModel = VectorDataModelType::New();
  m_VectorDataModel->RegisterListener(this);

  // Output changed flag
  m_OutputChanged = false;

  // Extract Filter
  m_ExtractImageFilter = ExtractImageFilterType::New();

  // Selected Polygon on right click in automatic mode
  m_SelectedPolygon     = PolygonType::New();
  m_SelectedPolygonNode = DataNodeType::New();

  // Build the automatic vectordata vector for each polygon selected
  m_SelectedVectorData           = VectorDataType::New();
  DataNodeType::Pointer root     = m_SelectedVectorData->GetDataTree()->GetRoot()->Get();
  DataNodeType::Pointer document = DataNodeType::New();
  DataNodeType::Pointer folder   = DataNodeType::New();

  document->SetNodeType(otb::DOCUMENT);
  folder->SetNodeType(otb::FOLDER);

  m_SelectedVectorData->GetDataTree()->Add(document, root);
  m_SelectedVectorData->GetDataTree()->Add(folder, document);
  m_SelectedVectorData->GetDataTree()->Add(m_SelectedPolygonNode, folder);
}

VectorizationModel
::~VectorizationModel()
{}

void VectorizationModel::
Notify(ListenerBase * listener)
{
  listener->Notify();
}

void
VectorizationModel
::SetImage(VectorImagePointerType image)
{
  image->UpdateOutputInformation();
  m_InputImage = image;

  // Generate the layer
  m_ImageGenerator->SetImage(image);
  m_ImageGenerator->GenerateQuicklookOn();
  m_ImageGenerator->GenerateLayer();

  m_ImageGenerator->GetLayer()->SetName("InputImage");

  // Clear previous layers
  m_VisualizationModel->ClearLayers();

  // Add the layer to the models
  m_VisualizationModel->AddLayer(m_ImageGenerator->GetLayer());
  m_VisualizationModel->Update();

//  m_VectorDataModel->SetOrigin(m_InputImage->GetOrigin());
//  m_VectorDataModel->SetSpacing(m_InputImage->GetSpacing());
}

void VectorizationModel
::AddVectorData(VectorDataPointerType vData)
{
  if(m_InputImage.IsNull())
    {
      itkExceptionMacro("Invalid input image.");
    }

  typedef otb::VectorDataIntoImageProjectionFilter<VectorDataType, VectorImageType>
    VectorDataReprojectionType;

  VectorDataReprojectionType::Pointer reproj = VectorDataReprojectionType::New();
  reproj->SetInputImage(m_InputImage);
  reproj->SetInputVectorData(vData.GetPointer());
  // We want to transform into image index coordinates
  reproj->SetUseOutputSpacingAndOriginFromImage(true);
  if (m_UseDEM==true)
    {

    typedef otb::DEMHandler DEMHandlerType;
    DEMHandlerType::Pointer DEMTest = DEMHandlerType::Instance();

    if (!m_DEMPath.empty() && DEMTest->IsValidDEMDirectory(m_DEMPath.c_str()))
      {
      DEMTest->OpenDEMDirectory(m_DEMPath);
      }
    else
      {
      std::ostringstream oss;
      oss<<"Invalid DEM directory "<<m_DEMPath<<"."<< std::endl
         <<"DEM will not be used, errors may occur for VectorData and image superposition";

      MsgReporter::GetInstance()->SendError(oss.str());
      }
    }
  reproj->Update();

  VectorDataType::Pointer vdata = reproj->GetOutput();
  if (vdata->Size() <= 1000)
    {
    m_VectorDataModel->AddVectorData(vdata);
    }
  else
    {
    itkExceptionMacro("The Input VectorData contains to many features to be loaded");
    }
}

void VectorizationModel
::RemoveDataNode(DataNodeType * node)
{
  // Look-up node in the graph
  itk::PreOrderTreeIterator<VectorDataType::DataTreeType>
  it(m_VectorDataModel->GetVectorData()->GetDataTree());
  it.GoToBegin();

  while (!it.IsAtEnd() && it.Get() != node)
    {
    ++it;
    }
  // If node is found, remove it
  if (!it.IsAtEnd())
    {
    it.Remove();
     this->NotifyAll();
    }
}

void VectorizationModel
::SetDataNodeFieldAsInt(DataNodeType * node, const std::string& name, int value)
{
  node->SetFieldAsInt(name, value);
  this->NotifyAll();
}
void VectorizationModel
::SetDataNodeFieldAsFloat(DataNodeType * node, const std::string& name, float value)
{
  this->NotifyAll();
}
void VectorizationModel
::SetDataNodeFieldAsString(DataNodeType* node, const std::string& name, const std::string& value)
{
  otbMsgDevMacro( "Setting field " << name << " with value " << value << " to node " << node );
  node->SetFieldAsString(name, value);
  this->NotifyAll();
}
void VectorizationModel
::RemoveFieldFromDataNode(DataNodeType * node, const std::string& name)
{

  // TODO: Not implemented yet, requires new methods in DataNode class.
}
void VectorizationModel
::RemovePointFromDataNode(DataNodeType * node,
                          const unsigned long& index,
                          bool interiorRing,
                          const unsigned int& interiorRingIndex)
{
  switch (node->GetNodeType())
    {
    case FEATURE_POINT:
      {
      // If the geometry is a point, remove it
      this->RemoveDataNode(node);
      break;
      }
    case FEATURE_LINE:
      {
      if (node->GetLine()->GetVertexList()->Size() < 3)
        {
        this->RemoveDataNode(node);
        }
      else
        {
        // Since PolylineParametricPath does not provide read-write access to the vertex list, nor
        // a method to remove a given vertex, we must use a const_cast here.
        DataNodeType::LineType::VertexListType * pointContainer
          = const_cast<DataNodeType::LineType::VertexListType *>(node->GetLine()->GetVertexList());
        pointContainer->DeleteIndex(index);
        }
      break;
      }
    case FEATURE_POLYGON:
      {
      if (!interiorRing)
        {
        if (node->GetPolygonExteriorRing()->GetVertexList()->Size() < 4)
          {
          this->RemoveDataNode(node);
          }
        else
          {
          // Since PolylineParametricPath does not provide read-write
          // access to the vertex list, nor a method to remove a given
          // vertex, we must use a const_cast here.
          DataNodeType::PolygonType::VertexListType * pointContainer
            = const_cast<DataNodeType::PolygonType::VertexListType *>(
              node->GetPolygonExteriorRing()->GetVertexList());

          pointContainer->DeleteIndex(index);
          }
        }
      else
        {
        if (interiorRingIndex < node->GetPolygonInteriorRings()->Size())
          {
          // Since PolylineParametricPath does not provide read-write
          // access to the vertex list, nor a method to remove a given
          // vertex, we must use a const_cast here.
          DataNodeType::PolygonType::VertexListType * pointContainer
            = const_cast<DataNodeType::PolygonType::VertexListType *>(
              node->GetPolygonInteriorRings()->GetNthElement(interiorRingIndex)->GetVertexList());

          pointContainer->DeleteIndex(index);
          }
        }
      }
    default:
      {
      // Not supported yet
      break;
      }
    }
  this->NotifyAll();
}

void VectorizationModel
::UpdatePointFromDataNode(DataNodeType * node,
                          const unsigned long& index,
                          const PointType& point,
                          bool interiorRing,
                          const unsigned int& interiorRingIndex)
{
  // Cast PointType to VertexType
  VertexType vertex;
  vertex[0] = point[0];
  vertex[1] = point[1];

  switch (node->GetNodeType())
    {
    case FEATURE_POINT:
      {
      // If the geometry is a point, remove it
      node->SetPoint(point);
      break;
      }
    case FEATURE_LINE:
      {
      if (index < node->GetLine()->GetVertexList()->Size())
        {
        // Since PolylineParametricPath does not provide read-write
        // access to the vertex list, nor a method to set a given
        // vertex, we must use a const_cast here.
        DataNodeType::LineType::VertexListType * pointContainer
          = const_cast<DataNodeType::LineType::VertexListType *>(
            node->GetLine()->GetVertexList());

        pointContainer->SetElement(index, vertex);
        }
      break;
      }
    case FEATURE_POLYGON:
      {
      if (!interiorRing)
        {
        if (index < node->GetPolygonExteriorRing()->GetVertexList()->Size())
          {
          // Since PolylineParametricPath does not provide read-write
          // access to the vertex list, nor a method to set a given
          // vertex, we must use a const_cast here.
          DataNodeType::PolygonType::VertexListType * pointContainer
            = const_cast<DataNodeType::PolygonType::VertexListType *>(
              node->GetPolygonExteriorRing()->GetVertexList());

          pointContainer->SetElement(index, vertex);
          }
        }
      else
        {
        if (interiorRingIndex < node->GetPolygonInteriorRings()->Size()
            && index < node->GetPolygonInteriorRings()->GetNthElement(interiorRingIndex)->GetVertexList()->Size())
          {
          // Since PolylineParametricPath does not provide read-write
          // access to the vertex list, nor a method to set a given
          // vertex, we must use a const_cast here.
          DataNodeType::PolygonType::VertexListType * pointContainer
            = const_cast<DataNodeType::PolygonType::VertexListType *>(
              node->GetPolygonInteriorRings()->GetNthElement(interiorRingIndex)->GetVertexList());

          pointContainer->SetElement(index, vertex);
          }
        }
      }
    default:
      {
      // Not supported yet
      break;
      }
    }
  this->NotifyAll();
}


void
VectorizationModel
::OK()
{
  typedef otb::VectorDataProjectionFilter
    <VectorDataType, VectorDataType>                    ProjectionFilterType;
  ProjectionFilterType::Pointer vectorDataProjection = ProjectionFilterType::New();
  vectorDataProjection->SetInput(m_VectorDataModel->GetVectorData());

  PointType lNewOrigin;
  lNewOrigin[0] = m_InputImage->GetOrigin()[0];
  lNewOrigin[1] = m_InputImage->GetOrigin()[1];

  vectorDataProjection->SetInputOrigin(lNewOrigin);
  vectorDataProjection->SetInputSpacing(m_InputImage->GetSpacing());

  std::string projectionRef;
  itk::ExposeMetaData<std::string>(m_InputImage->GetMetaDataDictionary(),
                                   MetaDataKey::ProjectionRefKey, projectionRef );
  vectorDataProjection->SetInputProjectionRef(projectionRef);
  vectorDataProjection->SetInputKeywordList(m_InputImage->GetImageKeywordlist());
  /*if(m_UseDEM==true)
    {
    if (!m_DEMPath.empty())
      {
      vectorDataProjection->SetDEMDirectory(m_DEMDirectory);
      }
    else
      {
      itkExceptionMacro("Invalid DEM directory: "<<m_DEMPath<<".");
      }
    }*/

  vectorDataProjection->Update();
  m_Output = vectorDataProjection->GetOutput();

  m_OutputChanged = true;
  this->NotifyAll();
  m_OutputChanged = false;
}


void
VectorizationModel
::Notify()
{
  this->NotifyAll();
}

void
VectorizationModel
::FocusOnDataNode(const PointType& point)
{
  IndexType index;
  index[0] = static_cast<IndexType::IndexValueType>(point[0]);
  index[1] = static_cast<IndexType::IndexValueType>(point[1]);

  // Center the view around the defined index
  m_VisualizationModel->SetScaledExtractRegionCenter(index);
  m_VisualizationModel->SetExtractRegionCenter(index);
  m_VisualizationModel->Update();
}

void VectorizationModel
::ExtractRegionOfImage(RegionType ExtRegion)
{
  if (m_LastRegionSelected == ExtRegion)
    {
    m_ExtractRegionUpdated = false;
    }
  else
    {
    m_ExtractImageFilter->SetInput(m_InputImage);
    m_ExtractImageFilter->SetExtractionRegion(ExtRegion);
    m_LastRegionSelected = ExtRegion;
    m_ExtractRegionUpdated = true;
    }
}

/**
 * Add the polygon to the tree browser.
 *
 */
void VectorizationModel
::RightIndexClicked(const IndexType & index, RegionType ExtRegion)
{
  DataNodeType::Pointer polygonNode = DataNodeType::New();
  polygonNode->SetNodeType(otb::FEATURE_POLYGON);
  polygonNode->SetNodeId("FEATURE_POLYGON");
  polygonNode->SetPolygonExteriorRing( m_SelectedPolygon);
  m_VectorDataModel->GetVectorData()->GetDataTree()->Add(
    polygonNode,
    m_VectorDataModel->GetVectorData()->GetDataTree()->GetRoot()->Get());

  this->NotifyAll();
  m_ActualLayerNumber = 0;
}

/**
 * show the different polygon that the point belongs to.
 */
void VectorizationModel
::LeftIndexClicked(const IndexType & index,
                   RegionType ExtRegion)
{
  if(m_LastRegionSelected == ExtRegion)
    {
    LabelObject2PolygonFunctorType Functor;
    LabelType label = m_LabelImageVector[m_ActualLayerNumber]->GetPixel(index);
    m_SelectedPolygon = Functor(m_LabelMapVector[m_ActualLayerNumber]->GetLabelObject(label));
    m_SelectedPolygonNode->SetPolygonExteriorRing(m_SelectedPolygon);

    if(m_ActualLayerNumber == m_LabelImageVector.size()-1)
      m_ActualLayerNumber = 0;
    else
      m_ActualLayerNumber++;
    }
  else
    {
    m_ExtractRegionUpdated = false;
    }

  this->NotifyAll();
}

void
VectorizationModel::GenerateLayers()
{
  if (m_ExtractRegionUpdated)
    {
    // First delete the previsous layers
    this->DeleteLayers();

    //


    // Generate new layer (labeled image) for each algorithm
    m_LabelImageVector.push_back(GenerateMeanshiftClustering(10, 30, 100));
    m_LabelImageVector.push_back(GenerateMeanshiftClustering(3, 50, 150));
    m_LabelImageVector.push_back(GenerateMeanshiftClustering(2, 5, 10));
    m_LabelImageVector.push_back(GenerateWatershedClustering(1, 0.05, 0.0, 2, 15));
    m_LabelImageVector.push_back(GenerateWatershedClustering(2, 0.1, 0.0, 2, 15));
    m_LabelImageVector.push_back(GenerateWatershedClustering(3, 0.05, 0.005, 2, 12));
    m_LabelImageVector.push_back(GenerateWatershedClustering(4, 0.1, 0.01, 2, 15));
    m_LabelImageVector.push_back(GenerateWatershedClustering(5, 0.15, 0.001, 2, 15));
    m_LabelImageVector.push_back(GenerateGaborClustering(20, 0, 0.32, 0.48, 45, 3, 8, 7, 40, 100));
    m_LabelImageVector.push_back(GenerateGaborClustering(20, 0, 0.32, 0.48, 30, 5, 8, 7, 40, 100));
    m_LabelImageVector.push_back(GenerateGaborClustering(20, 0, 0.32, 0.48, 45, 3, 8, 7, 25, 150));
#ifdef USE_FFTWF
    m_LabelImageVector.push_back(GenerateFastFourierTransformLayer(1, 32, 3));
    m_LabelImageVector.push_back(GenerateFastFourierTransformLayer(2, 32, 3));
    m_LabelImageVector.push_back(GenerateFastFourierTransformLayer(3, 32, 3));
    m_LabelImageVector.push_back(GenerateFastFourierTransformLayer(4, 32, 3));
    m_LabelImageVector.push_back(GenerateFastFourierTransformLayer(5, 32, 3));
#endif
    m_LabelImageVector.push_back(GenerateGrowingRegionLayer(1, 256));
    m_LabelImageVector.push_back(GenerateGrowingRegionLayer(2, 256));
    m_LabelImageVector.push_back(GenerateGrowingRegionLayer(3, 256));
    m_LabelImageVector.push_back(GenerateGrowingRegionLayer(4, 256));
    m_LabelImageVector.push_back(GenerateGrowingRegionLayer(5, 256));

    for(unsigned int i=0; i<m_LabelImageVector.size(); i++)
      m_LabelMapVector.push_back(ConvertLabelImageToLabelMap(m_LabelImageVector[i]));
    }
}

void
VectorizationModel
::DeleteLayers()
{
  m_LabelImageVector.clear();
  m_LabelMapVector.clear();
  m_SelectedPolygonNode->SetPolygonExteriorRing(PolygonType::New());
}

/**
  * Standard Deviation of a gabor convoluted image
  */
VectorizationModel::LabeledImagePointerType
VectorizationModel
::GenerateGaborClustering(unsigned int gaborRad, double phi,
                          double a, double b, double firstDir, int nbDir,
                          unsigned int varRad, int spatialRadius, double rangeRadius,
                          int minRegionSize)
{
  // Instanciate the objects
  StdGaborFilterType::Pointer stdFilter = StdGaborFilterType::New();
  MeanShiftVectorImageFilterType::Pointer msImageFilter = MeanShiftVectorImageFilterType::New();

  //
  stdFilter->SetInput(m_ExtractImageFilter->GetOutput());
  stdFilter->SetA(a);
  stdFilter->SetB(b);
  stdFilter->SetVarianceRadius(varRad);
  stdFilter->SetRadius(gaborRad);
  stdFilter->SetNumberOfDirection(nbDir);
  stdFilter->SetInitialDirection(firstDir);
  stdFilter->SetPhi(phi/180.0*CONST_PI);
  
  msImageFilter->SetInput(stdFilter->GetOutput());
  msImageFilter->SetSpatialBandwidth(spatialRadius);
  msImageFilter->SetRangeBandwidth(rangeRadius);
  msImageFilter->SetMinRegionSize(minRegionSize);
  msImageFilter->SetThreshold(0.1);
  msImageFilter->SetMaxIterationNumber(100);
  msImageFilter->Update();

  // Add the specific text for the segmentation method currently used
  // in order to show it in the GUI
  std::ostringstream os;
  os <<"Gabor Clustering. GR : "<<gaborRad<<"; Phi : "<<phi<<"; A : "<<a<<"; B : "
     <<b<<"; FirstDir : "<<firstDir<<"; NbDir : "<<nbDir
     <<"; SR : "<<spatialRadius<<"; RR: "
     <<rangeRadius<<"; MRS : "<<minRegionSize
     <<std::endl;
  m_AlgorithmsNameList.push_back(os.str());

  return msImageFilter->GetLabelOutput();
}

/**
 * Mean shift labeled image
 */
VectorizationModel::LabeledImagePointerType
VectorizationModel
::GenerateMeanshiftClustering(int spatialRadius,
                              double rangeRadius,
                              int minRegionSize)
{
  MeanShiftVectorImageFilterType::Pointer
    msImageFilter = MeanShiftVectorImageFilterType::New();

  msImageFilter->SetInput(m_ExtractImageFilter->GetOutput());
  msImageFilter->SetSpatialBandwidth(spatialRadius);
  msImageFilter->SetRangeBandwidth(rangeRadius);
  msImageFilter->SetMinRegionSize(minRegionSize);
  msImageFilter->SetThreshold(0.1);
  msImageFilter->SetMaxIterationNumber(100);
  msImageFilter->Update();

  // Add the specific text for the segmentation method currently used
  // in order to show it in the GUI
  std::ostringstream os;
  os <<"MeanShift. Spatial Radius : "
     <<spatialRadius<<"; Range Radius : "
     <<rangeRadius<<"; MinRegionSize : "
     <<minRegionSize<<std::endl;
  m_AlgorithmsNameList.push_back(os.str());

  std::cout<<"Model: Meanshift clustering: spatial radius = "
           <<spatialRadius<<", rangeradius = "
           <<rangeRadius<<", minimum region size = "
           <<minRegionSize<<std::endl;

  return msImageFilter->GetLabelOutput();
}


VectorizationModel::LabeledImagePointerType
VectorizationModel
::GenerateGrowingRegionLayer(unsigned int channel, unsigned long numberofhistogramsbins)
{
  VectorImageToImageListFilterType::Pointer           image2List      = VectorImageToImageListFilterType::New();
  OtsuThresholdImageFilterType::Pointer               otsuFilter      = OtsuThresholdImageFilterType::New();
  BinaryImageToLabelMapFilterType::Pointer      binary2LabelMap = BinaryImageToLabelMapFilterType::New();
  IntensityChannelFilterType::Pointer           intensityFilter = IntensityChannelFilterType::New();
  LabelMapToLabelImageType::Pointer             lm2li           = LabelMapToLabelImageType::New();


  if(channel<m_ExtractImageFilter->GetOutput()->GetNumberOfComponentsPerPixel()+1 && channel > 0 )
    {
    image2List->SetInput(m_ExtractImageFilter->GetOutput());
    image2List->UpdateOutputInformation();
    otsuFilter->SetInput(image2List->GetOutput()->GetNthElement(channel-1));
    }
  else
    {
    intensityFilter->SetInput(m_ExtractImageFilter->GetOutput());
    otsuFilter->SetInput(intensityFilter->GetOutput());
    }

  otsuFilter->SetNumberOfHistogramBins(numberofhistogramsbins);
  otsuFilter->SetOutsideValue(0);
  otsuFilter->SetInsideValue(255);

  binary2LabelMap->SetInput(otsuFilter->GetOutput());
  binary2LabelMap->SetInputForegroundValue(255);

  lm2li->SetInput(binary2LabelMap->GetOutput());
  lm2li->Update();

  std::ostringstream os;
  os <<"Region Growing Otsu Filter channel "<<channel
     <<" histogram number of Bins "<< numberofhistogramsbins <<std::endl;
  m_AlgorithmsNameList.push_back(os.str());
  std::cout<<"Region Growing Otsu Filter"<<std::endl;
  return lm2li->GetOutput();
}

VectorizationModel::LabeledImagePointerType
VectorizationModel
::GenerateWatershedClustering(unsigned int channel,
                              double level,
                              double threshold,
                              double conductanceParameter,
                              unsigned int numberOfIterations )
{
  GradientAnisotropicDiffusionFilterType::Pointer diffusionFilter    = GradientAnisotropicDiffusionFilterType::New();
  GradientMagnitudeFilterType::Pointer            gradientMagnitudeFilter = GradientMagnitudeFilterType::New();
  WatershedFilterType::Pointer                    watershedFilter    = WatershedFilterType::New();
  VectorImageToImageListFilterType::Pointer         image2List         = VectorImageToImageListFilterType::New();
  IntensityChannelFilterType::Pointer             intensityFilter    = IntensityChannelFilterType::New();

  if(channel<m_ExtractImageFilter->GetOutput()->GetNumberOfComponentsPerPixel()+1 && channel>0)
    {
    image2List->SetInput(m_ExtractImageFilter->GetOutput());
    image2List->UpdateOutputInformation();
    diffusionFilter->SetInput(image2List->GetOutput()->GetNthElement(channel-1));
    }
  else
    {
    intensityFilter->SetInput(m_ExtractImageFilter->GetOutput());
    diffusionFilter->SetInput(intensityFilter->GetOutput());
    }

  diffusionFilter->SetNumberOfIterations(numberOfIterations);
  diffusionFilter->SetConductanceParameter(conductanceParameter);
  diffusionFilter->SetTimeStep(0.125);

  gradientMagnitudeFilter->SetInput(diffusionFilter->GetOutput());
  watershedFilter->SetInput(gradientMagnitudeFilter->GetOutput());
  watershedFilter->SetLevel(level);
  watershedFilter->SetThreshold(threshold);

  // the watershedFilter filter does not give the choice to choose the output image
  // type, cast it into the labeled image type
  typedef itk::CastImageFilter
    <WatershedFilterType::OutputImageType, LabeledImageType>    CastFilterType;

  CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput(watershedFilter->GetOutput());
  castFilter->Update();

  std::ostringstream os;
  os <<"Watershed. Intensity. level : "<<level<<"; Threshold : "<<threshold
     <<"; Conductance Parameter : "<<conductanceParameter
     <<"; Nb Iterations : "<<numberOfIterations<<std::endl;

  m_AlgorithmsNameList.push_back(os.str());

  std::cout<<"Model : Watershed segmentation. level : "<<level<< " Threshold : "<<threshold<<std::endl;

  return   castFilter->GetOutput();
}


#ifdef USE_FFTWF
VectorizationModel::LabeledImagePointerType
VectorizationModel
::GenerateFastFourierTransformLayer(int channel,
                                    int windowSize,
                                    unsigned short threshold)
{
  VectorImageToImageListFilterType::Pointer  image2List  = VectorImageToImageListFilterType::New();
  SpatialFrequencyImageFilterType::Pointer   sfFilter    = SpatialFrequencyImageFilterType::New();
  ScalarConnectedComponentFilterType::Pointer azimuthToLabelImage = ScalarConnectedComponentFilterType::New();
  IntensityChannelFilterType::Pointer intensityFilter = IntensityChannelFilterType::New();


  // Choose the input following the channel selected
  if( (channel < static_cast<int>(m_ExtractImageFilter->GetOutput()->GetNumberOfComponentsPerPixel()+1) ) && (channel > 0) )
    {
    image2List->SetInput(m_ExtractImageFilter->GetOutput());
    image2List->UpdateOutputInformation();
    sfFilter->SetInput(image2List->GetOutput()->GetNthElement(channel-1));
    }
  else
    {
    intensityFilter->SetInput(m_ExtractImageFilter->GetOutput());
    sfFilter->SetInput(intensityFilter->GetOutput());
    }

  // Do the processing
  sfFilter->SetWindowSize(windowSize);

  azimuthToLabelImage->SetInput(sfFilter->GetOutput());
  azimuthToLabelImage->SetDistanceThreshold(threshold);
  azimuthToLabelImage->Update();

  // Add the stream related to this segmentation algorithm
  std::ostringstream os;
  os <<"FFT. Channel = "<<channel<<"; Window Size : "<<windowSize<<"; Threshold : "<<threshold<<std::endl;
  m_AlgorithmsNameList.push_back(os.str());

  return azimuthToLabelImage->GetOutput();
}
#endif


VectorizationModel::LabelMapPointerType
VectorizationModel
::ConvertLabelImageToLabelMap(LabeledImagePointerType inputImage)
{
  LabelImageToLabelMapFilterType::Pointer
    LI2LM = LabelImageToLabelMapFilterType::New();

  LI2LM->SetBackgroundValue(itk::NumericTraits<LabelType>::max());
  LI2LM->SetInput(inputImage);
  LI2LM->Update();
  return LI2LM->GetOutput();
}

} // namespace otb
