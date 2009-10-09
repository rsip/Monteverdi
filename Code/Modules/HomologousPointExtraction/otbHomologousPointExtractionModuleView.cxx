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

#include "otbHomologousPointExtractionModuleView.h"

#include "otbMsgReporter.h"

#include <FL/Fl_File_Chooser.H>
#include <FL/fl_draw.H>
//#include <FL/Fl_Color_Chooser.H>

namespace otb
{

HomologousPointExtractionModuleView
::HomologousPointExtractionModuleView(): m_Controller(), m_Model(), m_FirstImageView(), m_SecondImageView(), m_FirstCrossGlComponent(), m_SecondCrossGlComponent()
{
  m_Model = HomologousPointExtractionModuleModel::New();
  m_FirstImageView = ImageViewType::New();
  m_SecondImageView = ImageViewType::New();
  m_FirstCrossGlComponent = CrossGlComponent::New();
  m_SecondCrossGlComponent = CrossGlComponent::New();
  m_ColorList.clear();
}


HomologousPointExtractionModuleView
::~HomologousPointExtractionModuleView()
{
}

void
HomologousPointExtractionModuleView
::SetModel(HomologousPointExtractionModuleModel* model)
{
  m_Model = model;
  m_FirstImageView->SetModel(m_Model->GetVisualizationModel(0));
  m_SecondImageView->SetModel(m_Model->GetVisualizationModel(1));
  m_Model->RegisterListener(this);
}

void
HomologousPointExtractionModuleView
::BuildInterface()
{
  if(!m_Controller)
    {
    itkExceptionMacro(<<"Controller is not set, can not build view.");
    }

  if(!m_FirstWidgetsController || !m_SecondWidgetsController)
    {
    itkExceptionMacro(<<"Widgets controller is not set, can not build view.");
    }
  // Build the fltk code
  //HomologousPointExtractionViewGroup::CreateGUI();
  this->CreateGUI();
  //wMainWindow->size_range(400, 300, wMainWindow->w(), wMainWindow->h(), 0, 0);


  // Register controllers
   m_FirstImageView->SetController(m_FirstWidgetsController);
   m_SecondImageView->SetController(m_SecondWidgetsController);

   // Remove registered visualization components from the interface
    gFull1->add(m_FirstImageView->GetFullWidget());
    gScroll1->add(m_FirstImageView->GetScrollWidget());
    gZoom1->add(m_FirstImageView->GetZoomWidget());
    gFull1->resizable(m_FirstImageView->GetFullWidget());
    gScroll1->resizable(m_FirstImageView->GetScrollWidget());
    gZoom1->resizable(m_FirstImageView->GetZoomWidget());

    m_FirstImageView->GetFullWidget()->resize(gFull1->x(),gFull1->y(),gFull1->w(),gFull1->h());
    m_FirstImageView->GetScrollWidget()->resize(gScroll1->x(),gScroll1->y(),gScroll1->w(),gScroll1->h());
    m_FirstImageView->GetZoomWidget()->resize(gZoom1->x(),gZoom1->y(),gZoom1->w(),gZoom1->h());

  // Remove registered visualization components from the interface
    gFull2->add(m_SecondImageView->GetFullWidget());
    gScroll2->add(m_SecondImageView->GetScrollWidget());
    gZoom2->add(m_SecondImageView->GetZoomWidget());
    gFull2->resizable(m_SecondImageView->GetFullWidget());
    gScroll2->resizable(m_SecondImageView->GetScrollWidget());
    gZoom2->resizable(m_SecondImageView->GetZoomWidget());

    m_SecondImageView->GetFullWidget()->resize(gFull2->x(),gFull2->y(),gFull2->w(),gFull2->h());
    m_SecondImageView->GetScrollWidget()->resize(gScroll2->x(),gScroll2->y(),gScroll2->w(),gScroll2->h());
    m_SecondImageView->GetZoomWidget()->resize(gZoom2->x(),gZoom2->y(),gZoom2->w(),gZoom2->h());

   // Show and refresh the interface
    this->wMainWindow->show();

    m_FirstImageView->GetFullWidget()->AddGlComponent( m_FirstCrossGlComponent );
    m_FirstImageView->GetScrollWidget()->AddGlComponent( m_FirstCrossGlComponent );
    m_FirstImageView->GetZoomWidget()->AddGlComponent( m_FirstCrossGlComponent );
    m_SecondImageView->GetFullWidget()->AddGlComponent( m_SecondCrossGlComponent );
    m_SecondImageView->GetScrollWidget()->AddGlComponent( m_SecondCrossGlComponent );
    m_SecondImageView->GetZoomWidget()->AddGlComponent( m_SecondCrossGlComponent );

    m_FirstImageView->GetFullWidget()->show();
    m_FirstImageView->GetScrollWidget()->show();
    m_FirstImageView->GetZoomWidget()->show();
    m_SecondImageView->GetFullWidget()->show();
    m_SecondImageView->GetScrollWidget()->show();
    m_SecondImageView->GetZoomWidget()->show();
    
     // Link pixel descriptors (not do before because widgets have to be instanciated)
     m_Controller->LinkPixelDescriptors();
}

void
HomologousPointExtractionModuleView
::RedrawWidgets()
{
  m_FirstImageView->GetFullWidget()->redraw();
  m_FirstImageView->GetScrollWidget()->redraw();
  m_FirstImageView->GetZoomWidget()->redraw();

  m_SecondImageView->GetFullWidget()->redraw();
  m_SecondImageView->GetScrollWidget()->redraw();
  m_SecondImageView->GetZoomWidget()->redraw();
}


void
HomologousPointExtractionModuleView
::AddPoints()
{
  int x1 = vX1->value();
  int y1 = vY1->value();
  int x2 = vX2->value();
  int y2 = vY2->value();
  m_Controller->AddPoints( x1, y1, x2, y2 );
}

void
HomologousPointExtractionModuleView
::AddPointsToList( IndexType id1, IndexType id2)
{
 itk::OStringStream oss;
 oss<<id1<<" , "<<id2;
 this->lPointList->add(oss.str().c_str());

 srand((id2[0]+id1[1]+id2[0]+id2[1])*123456);
 ColorType color;
 color[0]=rand()/(RAND_MAX+1.0);
 color[1]=rand()/(RAND_MAX+1.0);
 color[2]=rand()/(RAND_MAX+1.0);
 color[3]=1.0;

 fl_color(static_cast<unsigned char>(255*color[0]),
         static_cast<unsigned char>(255*color[1]),
         static_cast<unsigned char>(255*color[2]));
 m_ColorList.push_back(color);

 lPointList->value(lPointList->size());
 this->UpdateListSelectionColor();

 m_FirstCrossGlComponent->AddIndex( id1 );
 m_FirstCrossGlComponent->ChangeColor( color, m_FirstCrossGlComponent->GetColorList().size()-1 );
 m_SecondCrossGlComponent->AddIndex( id2 );
 m_SecondCrossGlComponent->ChangeColor( color, m_SecondCrossGlComponent->GetColorList().size()-1 );
 this->RedrawWidgets();
}



/**
 *
 */
void
HomologousPointExtractionModuleView
::UpdateListSelectionColor()
{
  //selectedIndex
  unsigned int selectedIndex = lPointList->value()-1;
 
  if( selectedIndex < m_ColorList.size() )
    {
      ColorType curColor = m_ColorList[selectedIndex];
      
      // color To fl_color
      fl_color(static_cast<unsigned char>(255*curColor[0]),
              static_cast<unsigned char>(255*curColor[1]),
              static_cast<unsigned char>(255*curColor[2]));
      
      //Update the List Point Color
      lPointList->selection_color(fl_color());
      lPointList->redraw();
    }
}

void
HomologousPointExtractionModuleView
::ChangePointValue(IndexType index, unsigned int viewId )
{
  if( viewId != 0 && viewId != 1 )
    itkExceptionMacro(<<"Invalid view index "<<viewId<<".");
  
  if( viewId == 0 )
    {
      vX1->value(index[0]);
      vY1->value(index[1]);
    }
  else // viewId==1
    {
      vX2->value(index[0]);
      vY2->value(index[1]);
    }
}

void
HomologousPointExtractionModuleView
::ClearPointList()
{
  m_Controller->ClearPointList();
  lPointList->clear();
  lPointList->redraw();
  m_ColorList.clear();

  m_FirstCrossGlComponent->Clear();
  m_SecondCrossGlComponent->Clear();
  this->RedrawWidgets();

}


void
HomologousPointExtractionModuleView
::DeletePoint()
{
  unsigned int id = lPointList->value();
  if( id != 0 )
    {
      m_Controller->DeletePointFromList(id-1);
      lPointList->remove(id);
      m_ColorList.erase(m_ColorList.begin()+id-1);
      if(id<=static_cast<unsigned int>(lPointList->size()))
       lPointList->value(id);
      else
       lPointList->value(1);
      this->UpdateListSelectionColor();
      m_FirstCrossGlComponent->ClearIndex(id-1);
      m_SecondCrossGlComponent->ClearIndex(id-1);
      this->RedrawWidgets();
    }
}


void
HomologousPointExtractionModuleView
::Notify()
{

}

void
HomologousPointExtractionModuleView
::Quit()
{
  MsgReporter::GetInstance()->Hide();
  wMainWindow->hide();
}



}// end namespace
