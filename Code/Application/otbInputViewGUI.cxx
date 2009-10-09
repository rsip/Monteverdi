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
#ifndef __otbInputViewGUI_cxx
#define __otbInputViewGUI_cxx

#include "otbInputViewGUI.h"

#include <FL/fl_ask.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Browser.H>
#include "base/ossimFilename.h"
#include "base/ossimDirectory.h"
#include "otbMacro.h"
#include "itkExceptionObject.h"



namespace otb
{

InputViewGUI
::InputViewGUI() : m_Model(), m_Controller(), m_ModuleInstanceId(""), m_InputChoiceMap()
{
}


void
InputViewGUI
::BuildInputInterface()
{

   if( m_Model.IsNotNull() && m_Controller != NULL)
   {
     wInputWindow->size_range(wInputWindow->w(), wInputWindow->h(), wInputWindow->w(), 0, 0, 0);
     gScrollInput->type(Fl_Scroll::VERTICAL_ALWAYS);

    // deactivate Ok button
    //TODO bOk->deactivate();

    // to count the number of Fl_Input_Choice to display
    unsigned int cpt =0;
    // if there are several Fl_Input_Choice to draw
    unsigned int height = 60;
    unsigned int base = height/2;
    unsigned int i =0;
    InputDataDescriptorMapType lInputDataMap = m_Model->GetModuleInputsByInstanceId(m_ModuleInstanceId);
    InputDataDescriptorMapType::const_iterator it_in;

     itk::OStringStream oss;

    // loop on the requiered input data of the module : m_ModuleInstanceId
    for (it_in = lInputDataMap.begin();it_in != lInputDataMap.end();it_in++)
    {
      /** Build the Fl_Choice **/
      Fl_Choice *inputChoice;
      // create Input Widgets considering the needed inputs   
      inputChoice = new Fl_Choice( 85,base + cpt* height, 400, 25);//, it_in->second.GetDataDescription().c_str() );
      inputChoice->copy_label(it_in->second.GetDataDescription().c_str());
      inputChoice->box(FL_PLASTIC_DOWN_BOX);
      inputChoice->align(FL_ALIGN_TOP);
      InputChoiceDescriptor::Pointer inputChoiceDesc = InputChoiceDescriptor::New();
      inputChoiceDesc->m_FlChoice = inputChoice;

      // we check if there are convenient outputs in the modules
      std::vector<std::string> moduleInstances = m_Model->GetAvailableModuleInstanceIds();
      for(i=0;i<moduleInstances.size();i++)
      {
        // look after all the outputs of each instance of module
        OutputDataDescriptorMapType lOutputDataMap = m_Model->GetModuleOutputsByInstanceId(moduleInstances[i]);
        OutputDataDescriptorMapType::const_iterator it_out;
        for(it_out=lOutputDataMap.begin();it_out!=lOutputDataMap.end();it_out++)
        {
          // if the type is ok, we can add the label in the Fl_Input_Choice
          if(it_in->second.IsTypeCompatible(it_out->second.GetDataType()))
          {
           oss.str("");
           oss<<moduleInstances[i];
           oss<<" : ";
           oss<<it_out->second.GetDataKey();
            inputChoice->add(oss.str().c_str());

            /** Build the inputChoiceDescriptor */
            inputChoiceDesc->m_ChoiceVector.push_back(StringPairType(moduleInstances[i],it_out->first));
          }
        }
      }

      gScrollInput->add(inputChoice);

      /** Build the Fl_Check_Button **/
      if(it_in->second.IsOptional())
      {
        this->BuildCheckBox(cpt,height,inputChoiceDesc);
        inputChoiceDesc->SetOptional(true);
      }
      /** Build the List **/
      if(it_in->second.IsMultiple())
      {
        this->BuildList(cpt,height,inputChoiceDesc);
        inputChoiceDesc->SetMultiple(true);
        cpt+= 2;
      }

      /** Add the inputChoiceDescriptor into the inputChoiceMap */
      m_InputChoiceMap[it_in->first] = inputChoiceDesc;
      cpt++;

    }
    gLabel->value(m_ModuleInstanceId.c_str());
  }

}

void
InputViewGUI
::BuildCheckBox(int cpt,int height,InputChoiceDescriptor* inputChoiceDesc)
{
  Fl_Check_Button *checkButton = new Fl_Check_Button( 60,height/2+cpt* height, 25, 25);
  gScrollInput->add(checkButton);
  inputChoiceDesc->m_FlChoice->deactivate();
  checkButton->callback((Fl_Callback *)InputViewGUI::ActivateInputChoice,(void *)inputChoiceDesc);
}

void
InputViewGUI
::ActivateInputChoice(Fl_Widget * w, void * v)
{
  InputChoiceDescriptor* inputChoiceDesc = (InputChoiceDescriptor *)v;
  if(inputChoiceDesc->m_FlChoice->active())
    {
    inputChoiceDesc->m_FlChoice->deactivate();
    if(inputChoiceDesc->IsMultiple())
      inputChoiceDesc->m_FlBrowser->deactivate();
    }
    else
    {
    inputChoiceDesc->m_FlChoice->activate();
    if(inputChoiceDesc->IsMultiple())
      inputChoiceDesc->m_FlBrowser->activate();
    }
}

void
InputViewGUI
::BuildList(int cpt,int height,InputChoiceDescriptor* inputChoiceDesc)
{
  Fl_Browser *browser = new Fl_Browser( 85,height+cpt* height, 400, 110);
  browser->box(FL_PLASTIC_DOWN_BOX);
  browser->type(2);
  //browser->selection_color(FL_BLUE);
  browser->selection_color(inputChoiceDesc->m_FlChoice->selection_color());

  gScrollInput->add(browser);

  Fl_Button *plusButton = new Fl_Button( 490+15, height/2+cpt* height+2, 20, 20, "+");
  plusButton->box(FL_PLASTIC_ROUND_DOWN_BOX);
  plusButton->color((Fl_Color)55);
  plusButton->labelfont(1);
  plusButton->labelsize(17);
  plusButton->labelcolor((Fl_Color)186);
  gScrollInput->add(plusButton);
  plusButton->callback((Fl_Callback *)InputViewGUI::AddInputToList,(void *)inputChoiceDesc);

  Fl_Button *minusButton = new Fl_Button( 490+15, height+cpt* height + 37, 20, 20, "-");
  minusButton->box(FL_PLASTIC_ROUND_DOWN_BOX);
  minusButton->color((Fl_Color)55);
  minusButton->labelfont(1);
  minusButton->labelsize(17);
  minusButton->labelcolor((Fl_Color)186);
  gScrollInput->add(minusButton);
  minusButton->callback((Fl_Callback *)InputViewGUI::RemoveInputFromList,(void *)inputChoiceDesc);

  Fl_Button *clearButton = new Fl_Button( 490, height+cpt* height+85, 50, 25, "Clear");
  gScrollInput->add(clearButton);
  clearButton->box(FL_PLASTIC_DOWN_BOX);
  clearButton->color((Fl_Color)55);
  clearButton->labelfont(1);
  clearButton->labelsize(12);
  clearButton->labelcolor((Fl_Color)186);
  clearButton->callback((Fl_Callback *)InputViewGUI::ClearList,(void *)inputChoiceDesc);

  // Save the browser
  inputChoiceDesc->m_FlBrowser = browser;

  if(inputChoiceDesc->IsOptional())
    inputChoiceDesc->m_FlBrowser->deactivate();
}


// **        Callbacks         ** //

void 
InputViewGUI
::Ok()
{
  unsigned int i;

  if(m_ModuleInstanceId != gLabel->value())
    { 
      int res = m_Controller->ChangeInstanceId(m_ModuleInstanceId, gLabel->value());
      if(res != 0)
       return;
      m_ModuleInstanceId = gLabel->value();
    }

  // Connect 
  for(InputChoiceDescriptorMapType::const_iterator mIt = m_InputChoiceMap.begin(); mIt!=m_InputChoiceMap.end();++mIt)
  {
    // Multiple data
    if(mIt->second->IsMultiple())
    {
      if( !mIt->second->IsOptional() ||
          (mIt->second->IsOptional() && mIt->second->m_FlChoice->active() ) )
      {
        for(i=0;i<mIt->second->m_Indexes.size();i++)
        {
          int ind = mIt->second->m_Indexes[i];
          if(ind >= 0)
          {
            StringPairType spair = mIt->second->m_ChoiceVector[ind];
            m_Controller->AddModuleConnection(spair.first,spair.second,m_ModuleInstanceId,mIt->first);
          }
        }
      }
    }
    else // Single data
    {
      // mandatory OR optional & active
      if( !mIt->second->IsOptional() ||
          (mIt->second->IsOptional() && mIt->second->m_FlChoice->active() ) )
      {
        if(mIt->second->HasSelected())
        {
          StringPairType spair = mIt->second->GetSelected();
          m_Controller->AddModuleConnection(spair.first,spair.second,m_ModuleInstanceId,mIt->first);
        }
      }
    }
  }

  // Start()
  m_Controller->StartModuleByInstanceId(m_ModuleInstanceId);
  wInputWindow->hide();

}

void 
InputViewGUI
::Cancel()
{
  wInputWindow->hide();
}

void 
InputViewGUI
::Show()
{
  wInputWindow->show();
}

void 
InputViewGUI
::AddInputToList(Fl_Widget * w, void * v)
{
  InputChoiceDescriptor* inputChoiceDesc = (InputChoiceDescriptor *)v;
  int choiceVal = inputChoiceDesc->m_FlChoice->value();
  if(choiceVal >= 0)
    {
      inputChoiceDesc->m_FlBrowser->add(inputChoiceDesc->m_FlChoice->text(choiceVal));
      inputChoiceDesc->m_FlChoice->redraw();
      inputChoiceDesc->m_FlBrowser->redraw();
    }

    inputChoiceDesc->m_Indexes.push_back(choiceVal);
std::cout<< "choiceVal :" <<choiceVal<<std::endl;
}

void 
InputViewGUI
::RemoveInputFromList(Fl_Widget * w, void * v)
{
  InputChoiceDescriptor* inputChoiceDesc = (InputChoiceDescriptor *)v;

  int choiceVal = inputChoiceDesc->m_FlBrowser->value();
  inputChoiceDesc->m_FlBrowser->remove(choiceVal);

  if( choiceVal <= inputChoiceDesc->m_FlBrowser->size() )
    {
      inputChoiceDesc->m_FlBrowser->value(choiceVal);
    }
  else
    {
      inputChoiceDesc->m_FlBrowser->value(choiceVal-1);
    }

  inputChoiceDesc->m_FlBrowser->redraw();

  // Erase the target index
  inputChoiceDesc->m_Indexes.erase(inputChoiceDesc->m_Indexes.begin()+choiceVal-1);

}

void 
InputViewGUI
::ClearList(Fl_Widget * w, void * v)
{
  InputChoiceDescriptor* inputChoiceDesc = (InputChoiceDescriptor *)v;
  inputChoiceDesc->m_FlBrowser->clear();
  inputChoiceDesc->m_FlBrowser->redraw();

  // Cheat the indexe is set to -1 
  inputChoiceDesc->m_Indexes.clear();
}

} // end namespace otb

#endif
