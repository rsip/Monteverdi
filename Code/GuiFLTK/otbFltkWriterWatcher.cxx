/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.

  Some parts of this code are derived from ITK. See ITKCopyright.txt
  for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "otbFltkWriterWatcher.h"

namespace otb
{

FltkWriterWatcher
::FltkWriterWatcher(itk::ProcessObject* process,
                    int x, int y, int w, int h,
                    const char *comment)
  : WriterWatcherBase(process, comment),
    m_CurrentFilterProgress(0),
    m_CurrentWriterProgress(0)
{
  this->BuildGUI(x, y, w, h, comment);
}

FltkWriterWatcher
::FltkWriterWatcher(itk::ProcessObject* process,
                    itk::ProcessObject* source,
                    int x, int y, int w, int h,
                    const char *comment)
  : WriterWatcherBase(process, source, comment),
    m_CurrentFilterProgress(0),
    m_CurrentWriterProgress(0)
{
  this->BuildGUI(x, y, w, h, comment);
}

void FltkWriterWatcher
::BuildGUI(int x, int y, int w, int h, const char * /*comment*/)
{
  m_Window = new Fl_Window(x, y, w + 10, h + 15);
  m_Window->label(m_Comment.c_str());
  m_Window->begin();
  m_FilterProgress = new Fl_Progress(5, 5, w, h / 2);
  m_FilterProgress->selection_color(FL_DARK_BLUE);
  m_FilterProgress->minimum(0);
  m_FilterProgress->maximum(1);
  m_FilterProgress->label("current tile");
  m_FilterProgress->align(FL_ALIGN_INSIDE);

  m_WriterProgress = new Fl_Progress(5, h / 2 + 10, w, h / 2);
  m_WriterProgress->selection_color(FL_RED);
  m_WriterProgress->minimum(0);
  m_WriterProgress->maximum(1);
  m_WriterProgress->label("writing");
  m_WriterProgress->align(FL_ALIGN_INSIDE);
  m_Window->end();

  m_FilterProgress->show();
  m_WriterProgress->show();
  Fl::check();
}

FltkWriterWatcher
::~FltkWriterWatcher()
{
  m_Window->remove(m_WriterProgress);
  m_Window->remove(m_FilterProgress);
  delete m_WriterProgress;
  delete m_FilterProgress;
  delete m_Window;
}


void
FltkWriterWatcher
::StartFilter()
{
  m_Window->show();
  m_FilterProgress->show();
  m_WriterProgress->show();
  Fl::check();
}

void
FltkWriterWatcher
::ShowFilterProgress()
{
  if (m_SourceProcess)
    {
    double progress = m_SourceProcess->GetProgress();

    // Update only at each 0.5 percent
    if (progress - m_CurrentFilterProgress > 0.005)
      {
      m_FilterProgress->value(progress);
      m_CurrentFilterProgress = progress;
      Fl::check();
      }
    }
}

void
FltkWriterWatcher
::EndFilter()
{
}

void
FltkWriterWatcher
::StartWriter()
{
  m_Window->show();
  m_FilterProgress->show();
  m_WriterProgress->show();
  Fl::check();
}

void
FltkWriterWatcher
::ShowWriterProgress()
{
  if (m_Process)
    {
    double progress = m_Process->GetProgress();

    // Update only at each 0.5 percent
    if (progress - m_CurrentWriterProgress > 0.005)
      {
      m_WriterProgress->value(progress);
      m_CurrentWriterProgress = progress;
      Fl::check();
      }
    }
}

void
FltkWriterWatcher
::EndWriter()
{
  m_Window->hide();
}

} // end namespace otb
