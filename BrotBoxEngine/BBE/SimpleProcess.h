
#include "../BBE/Game.h"

#ifdef WIN32
namespace bbe
{
	namespace simpleProcess
	{
		bool isRunAsAdmin();
		bool relaunchAsAdmin(bbe::Game* game);
		bool ensureAdmin(bbe::Game* game);
		bool drawElevationButton(bbe::Game* game);
	}
}
#endif
