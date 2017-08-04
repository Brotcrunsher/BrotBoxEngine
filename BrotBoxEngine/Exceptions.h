#pragma once

#include "ExceptionHelper.h"

namespace bbe
{
	CREATE_EXCEPTION(AlreadyCreatedException);
	CREATE_EXCEPTION(AlreadyUploadedException);
	CREATE_EXCEPTION(NotInitializedException);
	CREATE_EXCEPTION(NoCorrectMemoryType);
	CREATE_EXCEPTION(IllegalBufferSize);

	CREATE_EXCEPTION(BufferNoSourceException);
	CREATE_EXCEPTION(BufferIsNotMappedException);
	CREATE_EXCEPTION(BufferAlreadyMappedException);
	CREATE_EXCEPTION(BufferAlreadyUploadedException);
	CREATE_EXCEPTION(BufferTooSmallException);
	
	CREATE_EXCEPTION(NoSuchKeycodeException);

	CREATE_EXCEPTION(SingletonViolationException);
}