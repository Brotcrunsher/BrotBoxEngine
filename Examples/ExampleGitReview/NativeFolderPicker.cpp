#include "NativeFolderPicker.h"

#include <array>
#include <cstdio>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <shobjidl.h>
#include <combaseapi.h>
#endif

namespace gitReview
{
	namespace
	{
		std::string trimNewlines(std::string s)
		{
			while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
				s.pop_back();
			return s;
		}

#if !defined(_WIN32)
		std::optional<std::string> runCapture(const char *cmd)
		{
			FILE *p = popen(cmd, "r");
			if (!p)
				return std::nullopt;
			std::string out;
			std::array<char, 4096> buf{};
			while (fgets(buf.data(), static_cast<int>(buf.size()), p))
				out += buf.data();
			const int st = pclose(p);
			if (st != 0)
				return std::nullopt;
			return trimNewlines(std::move(out));
		}
#endif
	}

	std::optional<std::string> pickFolderInteractive(const char *title)
	{
		(void)title;
#ifdef _WIN32
		IFileOpenDialog *pFileOpen = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen));
		if (FAILED(hr) || !pFileOpen)
			return std::nullopt;

		DWORD dwOptions = 0;
		if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions)))
			pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

		pFileOpen->SetTitle(L"Select Git repository folder");

		hr = pFileOpen->Show(nullptr);
		if (FAILED(hr))
		{
			pFileOpen->Release();
			return std::nullopt;
		}

		IShellItem *pItem = nullptr;
		hr = pFileOpen->GetResult(&pItem);
		pFileOpen->Release();
		if (FAILED(hr) || !pItem)
			return std::nullopt;

		PWSTR pszPath = nullptr;
		hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
		pItem->Release();
		if (FAILED(hr) || !pszPath)
			return std::nullopt;

		char narrow[MAX_PATH * 4] = {};
		WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, narrow, sizeof(narrow) - 1, nullptr, nullptr);
		CoTaskMemFree(pszPath);
		return std::string(narrow);
#elif defined(__APPLE__)
		std::string esc = "Select Git repository";
		const std::string cmd =
			"osascript -e 'POSIX path of (choose folder with prompt \"" + esc + "\")'";
		return runCapture(cmd.c_str());
#else
		(void)title;
		if (auto z = runCapture("zenity --file-selection --directory --title='Select Git repository' 2>/dev/null"))
			return z;
		if (auto k = runCapture("kdialog --getexistingdirectory . --title 'Select Git repository' 2>/dev/null"))
			return k;
		return std::nullopt;
#endif
	}
}
