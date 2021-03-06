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

#include "otbWriterMVCModule.h"

#include "otbVectorImage.h"
#include "otbImageFileReader.h"

int otbWriterMVCModuleTest(int argc, char* argv[])
{
  otb::WriterMVCModule::Pointer specificModule = otb::WriterMVCModule::New();
  otb::Module::Pointer          module = specificModule.GetPointer();

  std::cout << "Module: " << module << std::endl;

  // Put in the tests
  const char * infname = argv[1];

  typedef otb::WriterMVCModule::FloatingVectorImageType ImageType;
  typedef otb::ImageFileReader<ImageType>               ReaderType;

  // Don't ask erase confirmation
  specificModule->SetCheckFileExistance(false);

  //reader
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(infname);
  reader->GenerateOutputInformation();

  otb::DataObjectWrapper wrapperIn = otb::DataObjectWrapper::Create(reader->GetOutput());
  std::cout << "Input wrapper: " << wrapperIn << std::endl;

  module->AddInputByKey("InputDataSet", wrapperIn);

  module->Start();

  Fl::lock();
  Fl::check();

  // Simulate file chooser and ok callback
  specificModule->GetView()->vFilePath->value(argv[2]);
  Fl::check();

  specificModule->GetView()->guiOK->do_callback();

  

  Fl::run();

  return EXIT_SUCCESS;

}
