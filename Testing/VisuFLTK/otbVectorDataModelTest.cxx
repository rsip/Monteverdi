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
#include "otbImageLayerRenderingModel.h"
#include "otbVectorImage.h"
#include "itkRGBPixel.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbImageLayerGenerator.h"
#include "otbImageLayer.h"
#include "otbImageView.h"
#include <FL/Fl.H>
#include "otbImageWidgetController.h"
#include "otbWidgetResizingActionHandler.h"
#include "otbChangeScaledExtractRegionActionHandler.h"
#include "otbChangeExtractRegionActionHandler.h"
#include "otbChangeScaleActionHandler.h"
#include "otbPixelDescriptionModel.h"
#include "otbPixelDescriptionActionHandler.h"
#include "otbPixelDescriptionView.h"

// Vector data includes
#include "otbVectorData.h"
#include "otbVectorDataFileReader.h"
#include "otbVectorDataProjectionFilter.h"
#include "otbVectorDataGlComponent.h"
#include "otbVectorDataModel.h"
#include "otbVectorDataActionHandler.h"

int otbVectorDataModelTest(int argc, char * argv[])
{
  // params
  const char *       infname  = argv[1];
  const unsigned int scrollSize = atoi(argv[2]);
  const unsigned int fullSize = atoi(argv[3]);
  const unsigned int zoomSize = atoi(argv[4]);
  const double       run      = atoi(argv[5]);

  // typedefs
  typedef int                                            PixelType;
  typedef itk::RGBPixel<unsigned char>                   RGBPixelType;
  typedef otb::Image<RGBPixelType, 2>                    OutputImageType;
  typedef otb::VectorImage<PixelType, 2>                 ImageType;
  typedef otb::ImageLayer<ImageType, OutputImageType>    LayerType;
  typedef otb::ImageFileReader<ImageType>                ReaderType;
  typedef otb::ImageLayerGenerator<LayerType>            LayerGeneratorType;
  typedef otb::ImageLayerRenderingModel<OutputImageType> ModelType;
  typedef otb::ImageView<ModelType>                      ViewType;
  typedef otb::ImageWidgetController                     ControllerType;
  typedef otb::WidgetResizingActionHandler
  <ModelType, ViewType>                                  ResizingHandlerType;
  typedef otb::ChangeScaledExtractRegionActionHandler
  <ModelType, ViewType>                                  ChangeScaledRegionHandlerType;
  typedef otb::ChangeExtractRegionActionHandler
  <ModelType, ViewType>                                  ChangeRegionHandlerType;
  typedef otb::ChangeScaleActionHandler
  <ModelType, ViewType>                                  ChangeScaleHandlerType;
  typedef otb::PixelDescriptionModel<OutputImageType> PixelDescriptionModelType;
  typedef otb::PixelDescriptionActionHandler
  <PixelDescriptionModelType, ViewType>                  PixelDescriptionActionHandlerType;
  typedef otb::PixelDescriptionView
  <PixelDescriptionModelType>                            PixelDescriptionViewType;
  typedef otb::VectorDataActionHandler
  <otb::VectorDataModel, ViewType>                      VectorDataActionHandlerType;
  // VectorData
  typedef otb::VectorDataModel::VectorDataType VectorDataType;
  typedef otb::VectorDataModel::PointType      PointType;
  typedef otb::VectorDataModel::LineType       LineType;
  typedef otb::VectorDataModel::PolygonType    PolygonType;
  typedef otb::VectorDataModel::VertexType     VertexType;

  typedef otb::VectorDataFileReader<VectorDataType> VectorDataFileReaderType;
  typedef otb::VectorDataProjectionFilter<VectorDataType,
      VectorDataType>
  VectorDataProjectionFilterType;
  typedef otb::VectorDataGlComponent<VectorDataType> VectorDataGlComponentType;

  // Instantiation
  ModelType::Pointer                 model      = ModelType::New();
  PixelDescriptionModelType::Pointer pixelModel = PixelDescriptionModelType::New();
  pixelModel->SetLayers(model->GetLayers());

  // Reading input image
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(infname);
  reader->GenerateOutputInformation();

  // Generate the layer
  LayerGeneratorType::Pointer generator = LayerGeneratorType::New();
  generator->SetImage(reader->GetOutput());
  generator->GenerateLayer();

  // Add the layer to the model
  model->AddLayer(generator->GetLayer());

  // Build a view
  ViewType::Pointer view = ViewType::New();
  view->SetModel(model);

  // Build the VectorDataModel
  otb::VectorDataModel::Pointer vdModel = otb::VectorDataModel::New();
  vdModel->RegisterListener(view);

  VectorDataGlComponentType::Pointer vgl = VectorDataGlComponentType::New();
  vgl->SetVectorData(vdModel->GetVectorData());

  view->GetScrollWidget()->AddGlComponent(vgl);
  view->GetFullWidget()->AddGlComponent(vgl);
  view->GetZoomWidget()->AddGlComponent(vgl);

  // Build a controller
  ControllerType::Pointer controller = ControllerType::New();
  view->SetController(controller);

  // Add the resizing handler
  ResizingHandlerType::Pointer resizingHandler = ResizingHandlerType::New();
  resizingHandler->SetModel(model);
  resizingHandler->SetView(view);
  controller->AddActionHandler(resizingHandler);

  // Add the change scaled region handler
  ChangeScaledRegionHandlerType::Pointer changeScaledHandler = ChangeScaledRegionHandlerType::New();
  changeScaledHandler->SetModel(model);
  changeScaledHandler->SetView(view);
  changeScaledHandler->SetMouseButton(2);
  controller->AddActionHandler(changeScaledHandler);

  // Add the change extract region handler
  ChangeRegionHandlerType::Pointer changeHandler = ChangeRegionHandlerType::New();
  changeHandler->SetModel(model);
  changeHandler->SetView(view);
  changeHandler->SetMouseButton(2);
  controller->AddActionHandler(changeHandler);

  // Add the change scaled handler
  ChangeScaleHandlerType::Pointer changeScaleHandler = ChangeScaleHandlerType::New();
  changeScaleHandler->SetModel(model);
  changeScaleHandler->SetView(view);
  controller->AddActionHandler(changeScaleHandler);

  // Add the pixel description action handler
  PixelDescriptionActionHandlerType::Pointer pixelActionHandler = PixelDescriptionActionHandlerType::New();
  pixelActionHandler->SetView(view);
  pixelActionHandler->SetModel(pixelModel);
  controller->AddActionHandler(pixelActionHandler);

  // Add the VectorData action handler
  VectorDataActionHandlerType::Pointer vdHandler = VectorDataActionHandlerType::New();
  vdHandler->SetView(view);
  vdHandler->SetModel(vdModel);
  controller->AddActionHandler(vdHandler);

  // Exercise the model

  // polygons are recorded with a 0.5 shift...
  PointType newOrigin;
  newOrigin[0] = reader->GetOutput()->GetOrigin()[0] + 0.5;
  newOrigin[1] = reader->GetOutput()->GetOrigin()[1] + 0.5;
  vdModel->SetOrigin(newOrigin);
  vdModel->SetSpacing(reader->GetOutput()->GetSpacing());

  std::cout << "Origin : "          << vdModel->GetOrigin() << std::endl;
  std::cout << "Spacing : "         << vdModel->GetSpacing() << std::endl;
  std::cout << "CurrentNodeType : " << vdModel->GetCurrentNodeType() << std::endl;

  // Add a polygon
  VertexType point;
  point[0] = 2;
  point[1] = 2;
  vdModel->AddPointToGeometry(point);
  point[0] = 30;
  point[1] = 7;
  vdModel->AddPointToGeometry(point);
  point[0] = 40;
  point[1] = 40;
  vdModel->AddPointToGeometry(point);
  point[0] = 20;
  point[1] = 30;
  vdModel->AddPointToGeometry(point);

  // Add another polygon
  vdModel->EndGeometry();
  point[0] = 100;
  point[1] = 100;
  vdModel->AddPointToGeometry(point);
  point[0] = 130;
  point[1] = 107;
  vdModel->AddPointToGeometry(point);
  point[0] = 140;
  point[1] = 140;
  vdModel->AddPointToGeometry(point);
  point[0] = 120;
  point[1] = 130;
  vdModel->AddPointToGeometry(point);

  // Add a point
  vdModel->EndGeometry();
  vdModel->SetCurrentNodeType(otb::FEATURE_POINT);
  point[0] = 50;
  point[1] = 50;
  vdModel->AddPointToGeometry(point);

  // Add another point withour ending geometry
  vdModel->EndGeometry();
  point[0] = 10;
  point[1] = 100;
  vdModel->AddPointToGeometry(point);

  // Add a line (without ending previous geometry)
  vdModel->SetCurrentNodeType(otb::FEATURE_LINE);
  point[0] = 100;
  point[1] = 10;
  vdModel->AddPointToGeometry(point);
  point[0] = 90;
  point[1] = 20;
  vdModel->AddPointToGeometry(point);
  point[0] = 110;
  point[1] = 30;
  vdModel->AddPointToGeometry(point);
  vdModel->EndGeometry();

  vdModel->DeleteGeometry();
  vdModel->SetSelectedGeometry(1);
  vdModel->DeleteGeometry();

  // Build a pixel description view
  PixelDescriptionViewType::Pointer pixelView = PixelDescriptionViewType::New();
  pixelView->SetModel(pixelModel);

  Fl_Window pixelWindow(fullSize, 50);
  if (fullSize > 0)
    {
    pixelWindow.add(pixelView->GetPixelDescriptionWidget());
    pixelWindow.resizable(pixelView->GetPixelDescriptionWidget());
    pixelWindow.show();
    pixelView->GetPixelDescriptionWidget()->show();
    pixelView->GetPixelDescriptionWidget()->resize(0, 0, fullSize, 50);
    }

  Fl_Window scrollWindow(scrollSize, scrollSize);
  if (scrollSize > 0)
    {
    scrollWindow.add(view->GetScrollWidget());
    scrollWindow.resizable(view->GetScrollWidget());
    scrollWindow.show();
    view->GetScrollWidget()->show();
    view->GetScrollWidget()->resize(0, 0, scrollSize, scrollSize);
    }

  Fl_Window fullWindow(fullSize, fullSize);
  if (fullSize > 0)
    {
    fullWindow.add(view->GetFullWidget());
    fullWindow.resizable(view->GetFullWidget());
    fullWindow.show();
    view->GetFullWidget()->show();
    view->GetFullWidget()->resize(0, 0, fullSize, fullSize);
    }

  Fl_Window zoomWindow(zoomSize, zoomSize);
  if (zoomSize > 0)
    {
    zoomWindow.add(view->GetZoomWidget());
    zoomWindow.resizable(view->GetZoomWidget());
    zoomWindow.show();
    view->GetZoomWidget()->show();
    view->GetZoomWidget()->resize(0, 0, zoomSize, zoomSize);
    }

  if (run)
    {
    Fl::run();
    }
  else
    {
    Fl::check();
    }

  zoomWindow.remove(view->GetZoomWidget());
  scrollWindow.remove(view->GetScrollWidget());
  fullWindow.remove(view->GetFullWidget());
  pixelWindow.remove(pixelView->GetPixelDescriptionWidget());

  return EXIT_SUCCESS;
}
