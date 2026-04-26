#include "PasswordGenerator.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#ifdef __linux__
#include <termios.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#endif

static std::string readPassword(const char *prompt)
{
	std::cout << prompt << std::flush;
#ifdef __linux__
	termios oldt;
	tcgetattr(STDIN_FILENO, &oldt);
	termios newt = oldt;
	newt.c_lflag &= ~static_cast<tcflag_t>(ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif
#ifdef _WIN32
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;
	GetConsoleMode(hStdin, &mode);
	SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);
#endif

	std::string pw;
	std::getline(std::cin, pw);
	std::cout << std::endl;

#ifdef __linux__
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
#ifdef _WIN32
	SetConsoleMode(hStdin, mode);
#endif
	return pw;
}

static std::string readLine(const char *prompt)
{
	std::cout << prompt << std::flush;
	std::string line;
	std::getline(std::cin, line);
	return line;
}

#ifdef __linux__
static bool isExecutableInPath(const char *name)
{
	const char *pathEnv = std::getenv("PATH");
	if (!pathEnv) return false;
	std::string remaining(pathEnv);
	while (!remaining.empty())
	{
		size_t sep = remaining.find(':');
		std::string dir = remaining.substr(0, sep);
		remaining = sep == std::string::npos ? "" : remaining.substr(sep + 1);
		std::string full = dir + "/" + name;
		if (access(full.c_str(), X_OK) == 0) return true;
	}
	return false;
}
#endif

static bool setClipboard(const std::string &text)
{
#ifdef __linux__
	struct ClipboardCmd { const char *binary; const char *shellCmd; };
	const ClipboardCmd cmds[] = {
		{"wl-copy",  "wl-copy"},
		{"xclip",    "xclip -selection clipboard"},
		{"xsel",     "xsel --clipboard --input"},
	};
	for (const auto &cmd : cmds)
	{
		if (!isExecutableInPath(cmd.binary)) continue;
		FILE *pipe = popen(cmd.shellCmd, "w");
		if (!pipe) continue;
		fwrite(text.c_str(), 1, text.size(), pipe);
		if (pclose(pipe) == 0) return true;
	}
	return false;
#elif defined(_WIN32)
	if (!OpenClipboard(nullptr)) return false;
	EmptyClipboard();
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
	if (!hMem) { CloseClipboard(); return false; }
	memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
	GlobalUnlock(hMem);
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
	return true;
#else
	(void)text;
	return false;
#endif
}

static void printUsage(const char *exe)
{
	std::cout << "Usage: " << exe << " [--long-password]" << std::endl;
	std::cout << "  --long-password   Generate 32 character password (default: 16)" << std::endl;
}

int main(int argc, char **argv)
{
	bool longPassword = false;
	for (int i = 1; i < argc; i++)
	{
		const char *a = argv[i];
		if (std::strcmp(a, "--long-password") == 0)
		{
			longPassword = true;
		}
		else if (std::strcmp(a, "--help") == 0 || std::strcmp(a, "-h") == 0)
		{
			printUsage(argv[0]);
			return 0;
		}
		else
		{
			std::cerr << "Unknown argument: " << a << std::endl;
			printUsage(argv[0]);
			return 1;
		}
	}

	std::string masterPw = readPassword("Master Password: ");
	std::string masterPwRepeat = readPassword("Repeat Password: ");

	if (masterPw != masterPwRepeat)
	{
		std::cerr << "Passwords do not match." << std::endl;
		return 1;
	}

	std::string user = readLine("User (optional): ");
	std::string service = readLine("Service: ");

	if (service.empty())
	{
		std::cerr << "Service must not be empty." << std::endl;
		return 1;
	}

	std::cout << "Generating..." << std::flush;
	std::string password = PasswordGenerator::generateServicePassword(masterPw, service, user, longPassword ? 32 : 16);
	std::cout << " Done." << std::endl;

	if (setClipboard(password))
	{
		std::cout << "Password copied to clipboard." << std::endl;
	}
	else
	{
		std::cerr << "Failed to copy to clipboard. None of wl-copy, xclip, or xsel found in PATH." << std::endl;
		std::cerr << "  Arch: pacman -S wl-clipboard   (Wayland) or xclip (X11)" << std::endl;
		std::cerr << "  Debian/Ubuntu: apt install wl-clipboard or xclip" << std::endl;
		std::cerr << "Type \"Yes, print to terminal\" to display the password: " << std::flush;
		std::string confirmation;
		std::getline(std::cin, confirmation);
		if (confirmation == "Yes, print to terminal")
		{
			std::cout << password << std::endl;
		}
	}

	return 0;
}
