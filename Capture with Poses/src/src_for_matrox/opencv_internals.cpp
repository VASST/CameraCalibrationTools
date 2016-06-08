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

#include "opencv_internals.h"

OpenCVInternals::OpenCVInternals(long sysID, long appID, int port)
{
	_video_source = new VideoSourceWrapper();

	//intrinsics = (CvMat*)cvLoad("");
	//distortion_params = (CvMat*)cvLoad(""); // Load distortion params.

	_video_source->set_MIL_port(port);

	if( sysID == 0 && appID == 0){
		_video_source->init_MIL_source(0, 0);
		_MIL_sysID = _video_source->get_MIL_sysID();
		_MIL_appID = _video_source->get_MIL_appID();
	}
	else{
		_MIL_sysID = sysID;
		_MIL_appID = appID;
		_video_source->init_MIL_source(_MIL_sysID, _MIL_appID);
	}
}

OpenCVInternals::~OpenCVInternals()
{
	delete _video_source;
}

IplImage* OpenCVInternals::grab_frame()
{
	return _video_source->get_frame();
}

long OpenCVInternals::get_sysID()
{
	return _MIL_sysID;
}

long OpenCVInternals::get_appID()
{
	return _MIL_appID;
}