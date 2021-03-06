#ifndef __otbBasicModuleView_h
#define __otbBasicModuleView_h

#include "otbMeanShiftModuleViewGroup.h"

#include "otbEventsListener.h"
#include "otbMeanShiftModuleModel.h"
#include "otbMeanShiftModuleControllerInterface.h"
#include "itkSimpleFastMutexLock.h"

#include "otbImageView.h"

namespace otb
{
/** \class MeanShiftModuleView
 *
 */
class ITK_ABI_EXPORT MeanShiftModuleView
  : public EventsListener<std::string>, public MeanShiftModuleViewGroup, public itk::Object
{
public:
  /** Standard class typedefs */
  typedef MeanShiftModuleView           Self;
  typedef MeanShiftModuleViewGroup      Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standards macros */
  itkNewMacro(Self);
  itkTypeMacro(MeanShiftModuleView, Object);

  typedef MeanShiftModuleModel::VisualizationModelType VisualizationModelType;

  typedef MeanShiftModuleModel::RGBPixelType PixelType;
  typedef ImageView<VisualizationModelType>  ImageViewType;

  /** Event from the model */
  virtual void Notify(const std::string& event);

  /** Fluid call backs*/
  virtual void Exit();
  virtual void RunSegmentation();
  virtual void SwitchClusters();
  virtual void SwitchBoundaries();
  virtual void SetSpatialRadius(unsigned int sr);
  virtual void SetSpectralRadius(double sr);
  virtual void SetMinRegionSize(unsigned int mr);
  virtual void SetOpacity(double op);
  virtual void ViewerSetupOkCallback();

  void Show();
  void Hide();
  void UpdateViewerSetup();

  /** Set the controller */
  itkSetObjectMacro(Controller, MeanShiftModuleControllerInterface);
  itkGetObjectMacro(Controller, MeanShiftModuleControllerInterface);

  /** Set the widgets controller */
  itkSetObjectMacro(WidgetsController, ImageWidgetController);

  /** Build the interface */
  virtual void Build();

  /** Get a pointer to the view parts for the controller */
  itkGetObjectMacro(ImageView, ImageViewType);

  void SetModel(MeanShiftModuleModel* model);

protected:
  /** Constructor */
  MeanShiftModuleView();

  /** Destructor */
  virtual ~MeanShiftModuleView();

  /** Refresh the image widgets */
  void RefreshInterface();
  void RefreshVisualization();

private:
  MeanShiftModuleView(const Self&); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  /** Pointer to the controller */
  MeanShiftModuleControllerInterface::Pointer m_Controller;

  /** Pointer to the widgets controller */
  ImageWidgetController::Pointer m_WidgetsController;

  /** Pointer to the model */
  MeanShiftModuleModel::Pointer m_Model;

  /** Image view */
  ImageViewType::Pointer m_ImageView;

  itk::SimpleFastMutexLock m_Mutex;

};
} //end namespace otb

#endif
