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

#include "otbGCPToSensorModelController.h"

#include "otbMsgReporter.h"
//#include <FL/fl_ask.H>


namespace otb
{

GCPToSensorModelController
::GCPToSensorModelController(): m_View(), m_Model(), m_WidgetController(), m_MapWidgetController(), m_MouseClickedController(),
                                          m_ResizingHandler(), m_ChangeRegionHandler(), m_ChangeScaledRegionHandler(),
                                          m_ChangeScaleHandler(), m_LeftMouseClickedHandler(), m_PixelActionHandler(),
                                          m_PixelModel(), m_PixelView(), m_MapResizingHandler(), m_MapScrollZoomHandler(),
                                          m_MapMouseMapActionHandler(), m_GuessActionHandler()
{
  // Build the widgets controller
  m_WidgetController          = WidgetControllerType::New();
  m_MapWidgetController       = WidgetControllerType::New();
  m_MouseClickedController    = MouseClickedController::New();

  // Build the action handlers
  m_ResizingHandler           = ResizingHandlerType::New();
  m_ChangeRegionHandler       = ChangeRegionHandlerType::New();
  m_ChangeScaledRegionHandler = ChangeScaledRegionHandlerType::New();
  m_ChangeScaleHandler        = ChangeScaleHandlerType::New();
  m_LeftMouseClickedHandler   = MouseClickedHandlertype::New();
  m_PixelActionHandler        = PixelDescriptionActionHandlerType::New();
  m_PixelModel                = PixelDescriptionModelType::New();
  m_PixelView                 = PixelDescriptionViewType::New();
  
  m_MapResizingHandler        = ResizingHandlerType::New();
  m_MapScrollZoomHandler      = ScrollZoomHandlerType::New();
  m_MapMouseMapActionHandler  = MouseMapActionHandlerType::New();
  m_GuessActionHandler        = GuessActionHandlerType::New();

  // Link pixel clicked model (controller in relity...)
  m_MouseClickedController->SetMouseButton(1);
  m_MouseClickedController->SetImageViewId(0);
  m_MouseClickedController->SetController(this);
  m_LeftMouseClickedHandler->SetModel(m_MouseClickedController);
  m_LeftMouseClickedHandler->SetActiveOnFullWidget(false);
  m_LeftMouseClickedHandler->SetActiveOnScrollWidget(false);

  // Add the action handlers to the widgets controller
  m_WidgetController->AddActionHandler(m_ResizingHandler);
  m_WidgetController->AddActionHandler(m_ChangeRegionHandler);
  m_WidgetController->AddActionHandler(m_ChangeScaledRegionHandler);
  m_WidgetController->AddActionHandler(m_ChangeScaleHandler);
  m_WidgetController->AddActionHandler(m_LeftMouseClickedHandler);
  m_WidgetController->AddActionHandler(m_GuessActionHandler);
  
  m_MapWidgetController->AddActionHandler(m_MapResizingHandler);
  m_MapWidgetController->AddActionHandler(m_MapScrollZoomHandler);
  m_MapWidgetController->AddActionHandler(m_MapMouseMapActionHandler);
}


GCPToSensorModelController
::~GCPToSensorModelController()
{
  MsgReporter::GetInstance()->Hide();
}

void
GCPToSensorModelController
::SetModel(ModelType* model)
{
  m_Model = model;
  // Register the model to the action handlers
  m_ResizingHandler->SetModel(m_Model->GetVisualizationModel());
  m_ChangeRegionHandler->SetModel(m_Model->GetVisualizationModel());
  m_ChangeScaledRegionHandler->SetModel(m_Model->GetVisualizationModel());
  m_ChangeScaleHandler->SetModel(m_Model->GetVisualizationModel());
  
  m_MapResizingHandler->SetModel(m_Model->GetMapVisualizationModel());
  m_MapScrollZoomHandler->SetModel(m_Model);
  m_MapMouseMapActionHandler->SetModel(m_Model);
  m_GuessActionHandler->SetModel(m_Model);
}

void
GCPToSensorModelController
::SetView(GCPToSensorModelView * view)
{
  m_View = view;
  m_ResizingHandler->SetView(m_View->GetImageView());
  m_ChangeRegionHandler->SetView(m_View->GetImageView());
  m_ChangeScaledRegionHandler->SetView(m_View->GetImageView());
  m_ChangeScaleHandler->SetView(m_View->GetImageView());
  m_LeftMouseClickedHandler->SetView(m_View->GetImageView());
  
  m_MapResizingHandler->SetView(m_View->GetMapView());
  m_MapScrollZoomHandler->SetView(m_View->GetMapView());
  m_MapMouseMapActionHandler->SetView(m_View->GetMapView());
  m_GuessActionHandler->SetView(m_View->GetImageView());
}

void
GCPToSensorModelController
::LinkPixelDescriptors()
{
  //  image pixel descriptor
  m_WidgetController->AddActionHandler(m_PixelActionHandler);
  m_PixelModel->SetLayers(m_Model->GetVisualizationModel()->GetLayers());
  m_PixelView->SetModel(m_PixelModel);
  m_PixelActionHandler->SetModel(m_PixelModel);
  m_PixelActionHandler->SetView(m_View->GetImageView());
  m_View->gPixelValue->add(m_PixelView->GetPixelDescriptionWidget());
  m_PixelView->GetPixelDescriptionWidget()->show();
  m_PixelView->GetPixelDescriptionWidget()->resize(m_View->gPixelValue->x(),m_View->gPixelValue->y(), m_View->gPixelValue->w(), m_View->gPixelValue->h());
}


void
GCPToSensorModelController
::AddPoints( float x, float y, float lon, float lat, float elev )
{
  // Try to add GCP
  try
  {
    m_Model->AddGCP(x, y, lon, lat, elev);
  }
  catch (itk::ExceptionObject & err)
  {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    return;
  }
}

void
GCPToSensorModelController
::ClearPointList()
{
  m_Model->ClearGCPsContainer();
}

void
GCPToSensorModelController
::DeletePointFromList( unsigned int id )
{
  m_Model->RemovePointFromGCPsContainer( id );
}


void
GCPToSensorModelController
::LeftMouseButtonClicked( ContinuousIndexType index )
{
  IndexType id;
  id[0] = static_cast<long>(index[0]);
  id[1] = static_cast<long>(index[1]);
  
  m_View->ChangePointValue(id);
}


void
GCPToSensorModelController
::FocusOn(ContinuousIndexType index)
{
  IndexType id;
  id[0] = static_cast<long>(index[0]);
  id[1] = static_cast<long>(index[1]);

  m_ChangeRegionHandler->GetModel()->SetExtractRegionCenter(id);
  m_ChangeRegionHandler->GetModel()->Update();

#ifdef OTB_USE_CURL  
  // Focus on map
  m_Model->CenterMapOnSelectedPoint(id[0], id[1], 16);
#endif
}


void
GCPToSensorModelController
::UpdatePointList()
{
  std::cout << "Not implemented yet!" << std::endl;

/*  // redraw inputlist (height or not) + compute new transform + update stats
  IndexesListType indexesList = m_Model->GetIndexesList();
  if( m_Model->GetProjectionType() == ModelType::RPC )
    m_Model->GenerateUsedElevation();
  
  if( m_Model->GetUsedElevation().size() != indexesList.size() )
    {
      MsgReporter::GetInstance()->SendError("Lost in the wind... sorry start from scratch");
      return;
    }

    // add point to the input point list
  double height = 0.;
  for(unsigned int i=0; i<indexesList.size(); i++)
    {
      if( m_Model->GetProjectionType() == ModelType::RPC )
        height = m_Model->GetUsedElevation(i);
      m_View->AddPointsToList( indexesList[i].first, indexesList[i].second, height );
    }

    // compute transform
  try
    {
      m_Model->ComputeTransform();
    }
  catch (itk::ExceptionObject & err)
    {
      MsgReporter::GetInstance()->SendError(err.GetDescription());
      return;
    }*/
}


void
GCPToSensorModelController
::UpdateStats()
{
  std::cout << "GCPToSensorModelController::UpdateStats() TODO." << std::endl;

/*  itk::OStringStream oss;
  std::vector<double> values;
  ModelType::Continuous3DIndexListType outTranformedPoint = m_Model->TransformPoints();
  IndexesListType                      indexesList        = m_Model->GetIndexesList();
  ContinuousIndexType idFix, idOut;
  Continuous3DIndexType idOut3D, idTrans3D;
 
  for(unsigned int i=0; i<indexesList.size(); i++)
    {
      idFix = indexesList[i].first;
      idOut = indexesList[i].second;
      idTrans3D = outTranformedPoint[i];
      idOut3D[0] = idOut[0];
      idOut3D[1] = idOut[1];
      if( m_Model->GetProjectionType() == ModelType::RPC)
       idOut3D[2] = m_Model->GetUsedElevation()[i];
      else
       {
         idOut3D[2] = 0.;
         idTrans3D[2] = 0.;
       }

      oss.str("");
      oss<<idFix<<" -> ";
      values.push_back( idOut3D.EuclideanDistanceTo(idTrans3D) );
      oss<<idTrans3D<<", error: "<<vcl_sqrt(values[values.size()-1]);
      m_View->tError->add( oss.str().c_str() );
    }

  double sum = 0;
  for(unsigned int i=0; i<values.size(); i++)
    {
      sum += values[i];
    }
  sum = vcl_sqrt(sum);
  sum = sum / static_cast<double>(values.size());

  oss.str("");
  oss<<sum;
  m_View->tMeanError->value(oss.str().c_str());

  oss.str("");
  oss<<m_Model->GetGroundError();
  m_View->tGroundError->value(oss.str().c_str());*/
}

void
GCPToSensorModelController
::SetDEMPath( const std::string & filePath )
{
  try
  {
    m_Model->SetDEMPath( filePath );
  }
  catch (itk::ExceptionObject & err)
  {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    return;
  }
}

void
GCPToSensorModelController
::ChangeDEM()
{
  // If Mean 
  if( static_cast<bool>(m_View->cMean->value()) )
  {
    m_Model->SetMeanElevation( static_cast<double>(m_View->vMeanElev->value()) );
    m_Model->SetElevMgt(ModelType::MEAN);
  }
  // If DEM
  else if( static_cast<bool>(m_View->cDEM->value()) )
  {
    if( m_Model->GetDEMPath() == "")
    {
      MsgReporter::GetInstance()->SendError("No DEM directory path selected.");
      return;
    }
    m_Model->SetElevMgt(ModelType::DEM);
  }
  // If GCP
  else if( static_cast<bool>(m_View->cElev->value()) )
  {
    m_Model->SetElevMgt(ModelType::GCP);
  }
}

void
GCPToSensorModelController
::ReloadGCPsList()
{
  try
  {
    m_Model->LoadGCP();
  }
  catch (itk::ExceptionObject & err)
  {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    return;
  }
}

void
GCPToSensorModelController
::OK()
{
  try
  {
    m_Model->OK();
  }
  catch (itk::ExceptionObject & err)
  {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    return;
  }
}
  
void
GCPToSensorModelController
::SearchPlaceName(double latitude, double longitude)
{
  try
  {
    m_Model->SearchPlaceName(latitude, longitude);
  }
  catch (itk::ExceptionObject & err)
  {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    return;
  }
}

void
GCPToSensorModelController
::SearchLatLong(std::string placename)
{
  try
  {
    m_Model->SearchLatLong(placename);
  }
  catch (itk::ExceptionObject & err)
  {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    return;
  }
}

void
GCPToSensorModelController
::DisplayMap(std::string placename, double latitude, double longitude, unsigned int depth, long int sizeX, long int sizeY)
{
  try
  {
    m_Model->DisplayMap(placename, latitude, longitude, depth, sizeX, sizeY);
  }
  catch (itk::ExceptionObject & err)
  {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    return;
  }
}

void
GCPToSensorModelController
::ExportGcpsToXmlFile(const char * fname)
{
  try
  {
    m_Model->ExportGCPsToXmlFile(fname);
  }
  catch (itk::ExceptionObject & err)
  {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    return;
  }
}

void
GCPToSensorModelController
::ImportGcpsFromXmlFile(const char * fname)
{
  try
  {
    m_Model->ImportGCPsFromXmlFile(fname);
  }
  catch (itk::ExceptionObject & err)
  {
    MsgReporter::GetInstance()->SendError(err.GetDescription());
    return;
  }
}

} // end namespace otb
