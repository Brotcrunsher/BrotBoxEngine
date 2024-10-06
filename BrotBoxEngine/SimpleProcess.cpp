#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>

#include "BBE/SimpleProcess.h"
#include "imgui.h"
#include "BBE/ImGuiExtensions.h"

bool bbe::simpleProcess::isRunAsAdmin()
{
	static bool isRunAsAdmin = false;
	static bool cacheSet = false;

	if (!cacheSet)
	{
		cacheSet = true;
		BOOL IsMember = FALSE;
		PSID SidToCheck = nullptr;
		SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_NT_AUTHORITY;

		if (AllocateAndInitializeSid(&IdentifierAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &SidToCheck))
		{
			if (!CheckTokenMembership(nullptr, SidToCheck, &IsMember))
			{
				IsMember = FALSE;
			}
			FreeSid(SidToCheck);
		}
		isRunAsAdmin = IsMember;
	}

	return isRunAsAdmin;
}

bool bbe::simpleProcess::relaunchAsAdmin(bbe::Game* game)
{
	TCHAR lpFilename[MAX_PATH];
	if (GetModuleFileName(nullptr, lpFilename, MAX_PATH))
	{
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		sei.lpVerb = TEXT("runas");
		sei.lpFile = lpFilename;
		sei.hwnd = nullptr;
		sei.nShow = SW_NORMAL;

		if (!ShellExecuteEx(&sei))
		{
			DWORD dwError = GetLastError();
			if (dwError == ERROR_CANCELLED)
			{
				return false;
			}
		}
		else
		{
			game->closeWindow();
			return true;
		}
	}
	return false;
}

bool bbe::simpleProcess::ensureAdmin(bbe::Game* game)
{
	if (isRunAsAdmin()) return true;
	relaunchAsAdmin(game);
	return false;
}

bool bbe::simpleProcess::drawElevationButton(bbe::Game* game)
{
	ImGui::BeginDisabled(isRunAsAdmin());
	bool retVal = false;
	if (ImGui::Button("Elevate"))
	{
		if (relaunchAsAdmin(game))
		{
			retVal = true;
		}
	}
	if (isRunAsAdmin())
	{
		ImGui::bbe::tooltip("Already elevated");
	}
	ImGui::EndDisabled();
	return retVal;
}
#endif
