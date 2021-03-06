/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
See OTBCopyright.txt for details.


    This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE,  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbObjectCountingModel_h
#define __otbObjectCountingModel_h

#include "otbTypeManager.h"
#include "otbMVCModel.h"
#include "otbListenerBase.h"
#include "otbEventsSender.h"
#include "otbVectorImage.h"
#include "otbImage.h"
#include "otbImageFileReader.h"
#include "otbStreamingShrinkImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkVector.h"
#include "itkListSample.h"
//#include "itkListSampleToHistogramGenerator.h"
#include "itkSampleToHistogramFilter.h"
#include "itkDenseFrequencyContainer2.h"
#include "itkHistogram.h"
#include "otbMultiChannelExtractROI.h"
#include "otbPolygon.h"
#include "otbObjectList.h"
#include "otbSVMImageClassificationFilter.h"
#include "itkFixedArray.h"

#include "otbSVMSampleListModelEstimator.h"
#include "otbSpectralAngleDistanceImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include "otbMeanShiftSegmentationFilter.h"
#include "otbLabelToBoundaryImageFilter.h"
#include "itkChangeLabelImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "otbPersistentVectorizationImageFilter.h"
#include "otbSVMKernels.h"
#include "itkListSample.h"

namespace otb
{
typedef enum
  {
  NO_IMAGE,
  HAS_IMAGE,
  EXAMPLES_SELECTED,
  POLYGONS_DETECTED,
  HAS_REFERENCEPIXEL
  }
ObjectCountingApplicationStatesType;

typedef enum
  {
  EXTRACT,
  FULL_IMAGE
  }
ImageToWorkType;

template <class TInput1, class TInput2, class TOutput>
class ClassifBoundaryFunctor
{
public:
  ClassifBoundaryFunctor() {}
  ~ClassifBoundaryFunctor() {}
  inline TOutput operator ()(const TInput1& classif, const TInput2& bound)
  {
    TOutput val;
    if (classif == 1 && bound == 1) val = 0;
    else val = static_cast<TOutput>(classif);

    return val;
  }
};

/** \class ObjectCountingModel
 *
 *
 *
 */

class ITK_ABI_EXPORT ObjectCountingModel
  : public EventsSender<std::string>, public itk::Object
{

public:
  /** Standard class typedefs */
  typedef ObjectCountingModel           Self;
  typedef MVCModel<ListenerBase>        Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard type macro */
  itkTypeMacro(ObjectCountingModel, MVCModel);

  /** Get the unique instanc1e of the model */
  static Pointer GetInstance();

  /** Enum def*/
  typedef ObjectCountingApplicationStatesType StatesType;
  typedef ImageToWorkType                     WhichImageType;

  /** Input image typedefs*/
  typedef TypeManager::Floating_Point_Precision   PixelType;
  typedef TypeManager::Label_Short_Precision      LabelPixelType;
  typedef int                                     IntLabelPixelType;
  typedef TypeManager::Labeled_Short_Image        LabeledImageType;
  typedef Image<IntLabelPixelType, 2>             IntLabeledImageType;
  typedef TypeManager::Floating_Point_Image       SingleImageType;
  typedef TypeManager::Floating_Point_VectorImage ImageType;

  typedef ImageType::Pointer         ImagePointerType;
  typedef ImageType::SizeType        SizeType;
  typedef ImageType::PixelType       InputPixelType;
  typedef ImageType::IndexType       IndexType;
  typedef ImageType::RegionType      RegionType;
  typedef ImageFileReader<ImageType> ReaderType;
  typedef ReaderType::Pointer        ReaderPointerType;

  //typedef ImageView<VisualizationModelType>            ImageViewType;

  typedef InverseCosSAMKernelFunctor                              KernelType;
  typedef StreamingShrinkImageFilter<ImageType, ImageType>        ShrinkFilterType;
  typedef ShrinkFilterType::Pointer                               ShrinkFilterPointerType;
  typedef itk::ImageRegionConstIterator<ImageType>                IteratorType;
  typedef itk::Vector<itk::NumericTraits<PixelType>::RealType, 1> MeasurementVectorType;
  typedef itk::Statistics::ListSample<MeasurementVectorType>      ListSampleType;
  typedef float                                                   HistogramMeasurementType;
  /*typedef itk::Statistics::ListSampleToHistogramGenerator<
      ListSampleType,
      HistogramMeasurementType,
      itk::Statistics::DenseFrequencyContainer, 1>                   HistogramGeneratorType; */

  typedef itk::Statistics::Histogram<
      HistogramMeasurementType,
      itk::Statistics::DenseFrequencyContainer2>                  HistogramType;
  typedef HistogramType::SizeType                                 HistogramSizeType;

  typedef itk::Statistics::SampleToHistogramFilter<
      ListSampleType,
      HistogramType>                                              HistogramGeneratorType;

  //typedef HistogramGeneratorType::HistogramType        HistogramType;
  typedef HistogramType::Pointer                       HistogramPointerType;
  typedef MultiChannelExtractROI<PixelType, PixelType> ExtractROIFilterType;
  typedef otb::Polygon<double>                         PolygonType;
  typedef PolygonType::Pointer                         PolygonPointerType;
  typedef PolygonType::ContinuousIndexType             PolygonIndexType;
  typedef ObjectList<PolygonType>                      PolygonListType;
  typedef PolygonListType::Pointer                     PolygonListPointerType;

  typedef SVMImageClassificationFilter<ImageType, IntLabeledImageType>           ClassifierType;
  typedef ClassifierType::Pointer                                                ClassifierPointerType;
  typedef ClassifierType::ModelType                                              ModelType;
  typedef ImageType::PixelType                                                   SampleType;
  typedef itk::Statistics::ListSample<SampleType>                                SampleListType;
  typedef itk::FixedArray<IntLabelPixelType, 1>                                  TrainingSampleType;
  typedef itk::Statistics::ListSample<TrainingSampleType>                        TrainingListSampleType;
  typedef SVMSampleListModelEstimator<SampleListType, TrainingListSampleType>    EstimatorType;
  typedef EstimatorType::Pointer                                                 EstimatorPointerType;
  typedef SpectralAngleDistanceImageFilter<ImageType, SingleImageType>           SpectralAngleType;
  typedef SpectralAngleType::Pointer                                             SpectralAnglePointerType;
  typedef itk::BinaryThresholdImageFilter<SingleImageType, LabeledImageType>     ThresholdFilterType;
  typedef ThresholdFilterType::Pointer                                           ThresholdFilterPointerType;
  typedef itk::IntensityWindowingImageFilter<SingleImageType, SingleImageType>   RescaleFilterType;
  typedef RescaleFilterType::Pointer                                             RescaleFilterPointerType;
  typedef TypeManager::Labeled_Int_Image                                         MSLabeledImageType;
  typedef MeanShiftSegmentationFilter<ImageType, MSLabeledImageType, ImageType>  MeanShiftFilterType;
  typedef MSLabeledImageType::PixelType                                          MSLabeledPixelType;
  typedef MeanShiftFilterType::Pointer                                           MeanShiftFilterPointerType;
  typedef MeanShiftSmoothingImageFilter<ImageType, ImageType>                    MeanShiftSmoothFilterType;
  typedef LabelToBoundaryImageFilter<MSLabeledImageType,MSLabeledImageType>      BoundaryExtractorType;
  typedef itk::ChangeLabelImageFilter<IntLabeledImageType, LabeledImageType>     ChangeLabelImageFilterType;
  typedef ChangeLabelImageFilterType::Pointer                                    ChangeLabelImageFilterPointerType;
  typedef ClassifBoundaryFunctor<LabelPixelType, LabelPixelType, LabelPixelType> ClassifBoundaryFunctorType;
  typedef itk::BinaryFunctorImageFilter<LabeledImageType,
      MSLabeledImageType,
      LabeledImageType,
      ClassifBoundaryFunctorType>             ClassifBoundaryFilterType;
  typedef ClassifBoundaryFilterType::Pointer                                     ClassifBoundaryFilterPointerType;
  typedef itk::ConnectedComponentImageFilter<LabeledImageType, LabeledImageType> ConnectedFilterType;
  typedef ConnectedFilterType::Pointer                                           ConnectedFilterPointerType;
  typedef itk::RelabelComponentImageFilter<LabeledImageType, LabeledImageType>   RelabelFilterType;
  typedef RelabelFilterType::Pointer                                             RelabelFilterPointerType;
  typedef PersistentVectorizationImageFilter<LabeledImageType, PolygonType>      PersistentVectorizationFilterType;
  typedef PersistentVectorizationFilterType::Pointer
  PersistentVectorizationFilterPointerType;

  /** Filters initialization */
  void InitFilters();
  void InitInputFilters();
  /** Load an Image */
  void OpenImage();
  /** Inpout Image ql dynamic*/
  PixelType GetMax(unsigned int i) const;
  PixelType GetMin(unsigned int i) const;
  /** Constrains current region to largest possible region */
  bool ConstrainsCurrentRegion();
  /** Run image extraction on current region */
  void RunImageExtraction();
  /** Run the algorithm*/
  void RunChain(const char * cfname);
  /** Run over the full image */
  void SavePolygon(const char * cfname);
  /** Polygon manipulation */
  void NewPolygon(const IndexType& index);
  void AddPointToCurrentPolygon(const IndexType& index);
  void ErasePolygon(const IndexType& index);
  void GenerateInputExampleSampleList();
  /** Extract Input Classification*/
  void Classification();
  /** Model computation*/
  void Learning();
  /** Run chain over the extract */
  void RunOverExtract();
  /** Compute Spectral Angle over extract */
  void ComputeSpectralAngle();
  /** Compute Reference pixel from polygons list*/
  void ComputeReferencePixel();
  /** Compute mean shift images */
  void ComputeMeanShift();
  /** Generate output from MS and classif */
  void FuseData();
  /** Transform output chain in raster image + polygon list*/
  void PrepareOutput();
  /** Get Number of objects*/
  unsigned int GetNumberOfObjects();
  /** Check input polygons validity */
  void CheckInputPolygonsValidity();

  itkSetMacro(ShrinkFactor, unsigned int);
  itkGetMacro(ShrinkFactor, unsigned int);
  itkGetObjectMacro(InputImage, ImageType);
  itkGetObjectMacro(ExtractedImage, ImageType);
  itkGetObjectMacro(Quicklook, ImageType);
  itkGetObjectMacro(Shrinker, ShrinkFilterType);
  itkGetMacro(State, StatesType);
  itkSetMacro(CurrentRegion, RegionType);
  itkGetMacro(CurrentRegion, RegionType);
  itkSetMacro(QuicklookSize, SizeType);
  itkGetMacro(QuicklookSize, SizeType);
  itkSetMacro(WhichImage, WhichImageType);
  itkGetMacro(WhichImage, WhichImageType);

  itkGetObjectMacro(InputPolyList, PolygonListType);
  itkGetObjectMacro(OutputPolyList, PolygonListType);
  itkGetObjectMacro(ExtractPolyList, PolygonListType);
  //itkGetObjectMacro(MeanShiftFilter, MeanShiftFilterType);
  itkGetObjectMacro(RelabelFilter, RelabelFilterType);

  itkSetMacro(InputPolygonListIndex, unsigned int);
  itkGetMacro(InputPolygonListIndex, unsigned int);
  itkGetMacro(ErasedPolygonIndex, int);
  itkGetObjectMacro(InputSampleList, SampleListType);
  itkGetMacro(ReferencePixel, InputPixelType);
  itkSetMacro(UseSVM, bool);
  itkSetMacro(ThresholdValue, PixelType);
  itkGetMacro(ThresholdValue, PixelType);
  itkGetObjectMacro(ExtractOutputImage, LabeledImageType);
  itkSetMacro(OutputImageName, std::string);
  itkSetMacro(OutputVectorFileName, std::string);
  itkSetMacro(UseSmoothing, bool);

  // Mean Shift filter parameters
  itkSetMacro(SpatialRadius, unsigned int);
  itkSetMacro(RangeRadius, unsigned int);
  itkSetMacro(Scale, unsigned int);
  itkSetMacro(MinRegionSize, unsigned int);

  itkSetMacro(NuParameter, double);
  itkGetMacro(NuParameter, double);

  LabeledImageType::Pointer GetOutputLabeledImage()
  {
    return m_PersistentVectorizationFilter->GetOutput();
  }

  ImagePointerType GetInputImage() const
  {
    return m_InputImage;
  }

  void SetInputImage(ImagePointerType image);

  /** This is protected for the singleton. Use GetInstance() instead. */
  itkNewMacro(Self);

  void Quit();
protected:

  /** Constructor */
  ObjectCountingModel();
  /** Destructor */
  virtual ~ObjectCountingModel();

  void GenerateQuicklook();
  void GenerateHistogram();

private:
  ObjectCountingModel(const Self&); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  /** Notify a given listener of changes */
  virtual void Notify(ListenerBase * listener);

  /** Singleton instance */
  static Pointer Instance;

  /** Notify a given listener of changes */
  virtual void NotifyListener(ListenerBase * listener);

  /** Application state */
  StatesType m_State;
  /** Which image to compute*/
  WhichImageType m_WhichImage;
  /** Input Image */
  ImagePointerType m_InputImage;
  /** Extracted Input Image */
  ImagePointerType m_ExtractedImage;
  /** Reader type*/
  ReaderPointerType m_Reader;
  /** Shrink for input*/
  ShrinkFilterPointerType m_Shrinker;
  /** Shrink factor*/
  unsigned int m_ShrinkFactor;
  /** Histogram list */
  std::vector<HistogramPointerType> m_Histogram;
  /** percentage used for normalization */
  double m_ClampThreshold;
  /** Input QL*/
  ImagePointerType m_Quicklook;
  /** Size of the quicklook to generate */
  SizeType m_QuicklookSize;
  /** Region to work on */
  RegionType m_CurrentRegion;
  /** Input polygon list*/
  PolygonListPointerType m_InputPolyList;
  /** Output polygon list*/
  PolygonListPointerType m_OutputPolyList;
  /** current polygon index*/
  unsigned int m_InputPolygonListIndex;
  int          m_ErasedPolygonIndex;

  /** Reference Pixel for spectral angle */
  InputPixelType m_ReferencePixel;
  /** use svm or spectral angle */
  bool m_UseSVM;
  /** Input Polygon example pixel */
  SampleListType::Pointer m_InputSampleList;
  /** SVM Classifier*/
  ClassifierPointerType m_Classifier;
  /** SVM Model */
  ModelType::Pointer m_Model;
  /** Change classif label (-1->0 et 1 -> 1) */
  ChangeLabelImageFilterPointerType m_ChangeLabelFilter;
  /** Spectral Angle Filter */
  SpectralAnglePointerType m_SpectralAngleFilter;
  /** Thresholder type*/
  ThresholdFilterPointerType m_Thresholder;
  /** Rescaler type */
  RescaleFilterPointerType m_Rescaler;
  /** Threshold value */
  PixelType m_ThresholdValue;
  /** Classif output storage */
  LabeledImageType::Pointer m_ClassifOutputImage;
  /** MeanShift filter */
  MeanShiftFilterPointerType m_MeanShiftFilter;
  /** Mean Shift Spatial Radius value */
  unsigned int m_SpatialRadius;
  /** Mean Shift Range Radius value */
  unsigned int m_RangeRadius;
  /** Mean Shift Minimum Size Region value */
  unsigned int m_MinRegionSize;
  /** Boundary extractor */
  BoundaryExtractorType::Pointer m_BoundaryExtractor;
  /** Smoothing filter */
  MeanShiftSmoothFilterType::Pointer m_MSSmoothingFilter;

  /** Store output extract chain */
  LabeledImageType::Pointer m_ExtractOutputImage;
  /**Store extract output polygon */
  PolygonListPointerType m_ExtractPolyList;
  /** Output Image Name */
  std::string m_OutputImageName;
  /** Output vector file name */
  std::string m_OutputVectorFileName;
  /** Conected filter */
  ConnectedFilterPointerType m_ConnectedFilter;
  /** Relabel filter */
  RelabelFilterPointerType m_RelabelFilter;
  /** Vectorization filter type */
  PersistentVectorizationFilterPointerType m_PersistentVectorizationFilter;
  /** Boundary and classif image fusion filter */
  ClassifBoundaryFilterPointerType m_ClassifBoundaryFilter;
  /** Use smoothing*/
  bool m_UseSmoothing;
  /** Compute RefPix over smoothing image*/
  bool m_RefPixelSmoothedImage;
  /** Nu parameter for SVM model estimation */
  double m_NuParameter;
  /** Kernel for the SVMs */
  KernelType* m_Kernel;
};
}
#endif
