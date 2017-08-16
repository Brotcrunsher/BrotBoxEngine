#pragma once

#include "../BBE/ExceptionHelper.h"

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

	CREATE_EXCEPTION(IllegalStateException);

	CREATE_EXCEPTION(AllocatorOutOfMemoryException);
	CREATE_EXCEPTION(AllocatorOutOfHandlesException);
	CREATE_EXCEPTION(NullptrDeallocationException);
	CREATE_EXCEPTION(MalformedPointerException);

	CREATE_EXCEPTION(KeyAlreadyUsedException);

	CREATE_EXCEPTION(IllegalArgumentException);
	CREATE_EXCEPTION(ContainerEmptyException);

	CREATE_EXCEPTION(IllegalIndexException);

	CREATE_EXCEPTION(NotImplementedException);

	CREATE_EXCEPTION(NoTransformsLeftException);
}
