/*==========================================================================

  Copyright (c) 2014 Uditha L. Jayarathne, ujayarat@robarts.ca

  Use, modification and redistribution of the software, in source or
  binary forms, are permitted provided that the following terms and
  conditions are met:

  1) Redistribution of the source code, in verbatim or modified
  form, must retain the above copyright notice, this license,
  the following disclaimer, and any notices that refer to this
  license and/or the following disclaimer.  

  2) Redistribution in binary form must include the above copyright
  notice, a copy of this license and the following disclaimer
  in the documentation or with other materials provided with the
  distribution.

  3) Modified copies of the source code must be clearly marked as such,
  and must not be misrepresented as verbatim copies of the source code.

  THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE SOFTWARE "AS IS"
  WITHOUT EXPRESSED OR IMPLIED WARRANTY INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE.  IN NO EVENT SHALL ANY COPYRIGHT HOLDER OR OTHER PARTY WHO MAY
  MODIFY AND/OR REDISTRIBUTE THE SOFTWARE UNDER THE TERMS OF THIS LICENSE
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, LOSS OF DATA OR DATA BECOMING INACCURATE
  OR LOSS OF PROFIT OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF
  THE USE OR INABILITY TO USE THE SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGES.
  =========================================================================*/

#include "video_source_wrapper.h"

VideoSourceWrapper::VideoSourceWrapper()
{
	_MIL_port = 12; // MIL Port.
	_vtkMILSource   = vtkMILVideoSource::New();
}

VideoSourceWrapper::~VideoSourceWrapper()
{
	_vtkMILSource->ReleaseSystemResources();
}

IplImage* VideoSourceWrapper::get_frame()
{
	_vtkMILSource->Grab();

	vtkImageData* imgData = _vtkMILSource->GetOutput();
	imgData->SetScalarTypeToChar();
	imgData->Update();
		
	int dims[3];
	imgData->GetDimensions(dims);

	char* dstPtr = _frame->imageData;
	char* charPtr = (char*)(imgData->GetScalarPointer(0, 0, 0));
	memcpy(dstPtr, charPtr, dims[0]*dims[1]*3);
	//cvCvtColor(_frame, _frame, CV_RGB2BGR);

	return _frame;
}
void VideoSourceWrapper::init_MIL_source(long sysID, long appID)
{
	_vtkMILSource->SetMILSystemNumber( 0 );
	_vtkMILSource->SetMILAppID(appID);
	_vtkMILSource->SetMILSysID(sysID);
	_vtkMILSource->SetMILSystemTypeToMorphisQxT();
	_vtkMILSource->SetMILDigitizerNumber( _MIL_port );
	_vtkMILSource->SetVideoInput( VTK_MIL_COMPOSITE);
	_vtkMILSource->SetVideoFormatToNTSC();
	_vtkMILSource->SetOutputFormatToRGB();
	_vtkMILSource->SetFrameSize( 640, 480, 1);
	_vtkMILSource->SetDataOrigin(0, 0, 0);
	//_vtkMILSource->SetFrameRate( 20 );
	//_vtkMILSource->SetOpacity(1);
	_vtkMILSource->Print(std::cerr);
	_vtkMILSource->Initialize();
	_vtkMILSource->Record();

	// Initialize class variables
	_vtkMILSource->Grab();

	vtkImageData* imgData = _vtkMILSource->GetOutput();
	imgData->SetScalarTypeToChar();
	imgData->Update();

	imgData->GetDimensions(_dims);

	_frame = cvCreateImage(cvSize(_dims[0], _dims[1]), 8, 3);
}

void VideoSourceWrapper::set_MIL_sysID(long id)
{
	_vtkMILSource->SetMILSysID(id);
}

void VideoSourceWrapper::set_MIL_appID(long id)
{
	_vtkMILSource->SetMILAppID(id);
}

long VideoSourceWrapper::get_MIL_sysID()
{
	return _vtkMILSource->GetMILSysID();
}

long VideoSourceWrapper::get_MIL_appID()
{
	return _vtkMILSource->GetMILAppID();
}

void VideoSourceWrapper::set_MIL_port(int port)
{
	_MIL_port = port;
}