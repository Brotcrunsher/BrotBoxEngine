#include "BBE/SimpleFile.h"
#include "BBE/SimpleThread.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <memory>
#include <cstring>

#ifdef __linux__
#include <dbus/dbus.h>
#include <unistd.h>
#include <limits.h>
#include <cstdlib>
#endif

static std::mutex backupPathMutex;
static bbe::String backupPath;
static std::mutex lastIoMutex;
static bbe::TimePoint lastIo;
static size_t totalIoCalls = 0;

static void updateIoStats()
{
	std::lock_guard _(lastIoMutex);
	lastIo = bbe::TimePoint();
	totalIoCalls++;
}

void bbe::simpleFile::backup::setBackupPath(const bbe::String &path)
{
	std::lock_guard lg(backupPathMutex);
	if (path.isEmpty())
	{
		backupPath = "";
	}
	else
	{
		if (path.endsWith("/")) backupPath = path;
		else backupPath = path + "/";

		bbe::simpleFile::createDirectory(backupPath);
	}
}

bool bbe::simpleFile::backup::isBackupPathSet()
{
	std::lock_guard lg(backupPathMutex);
	return !backupPath.isEmpty();
}

bbe::String bbe::simpleFile::backup::backupFullPath(const bbe::String &path)
{
	std::lock_guard lg(backupPathMutex);
	return backupPath + path;
}

void bbe::simpleFile::backup::writeBinaryToFile(const bbe::String &filePath, const bbe::ByteBuffer &buffer)
{
	bbe::simpleFile::writeBinaryToFile(filePath, buffer);
	if (isBackupPathSet())
	{
		bbe::simpleFile::writeBinaryToFile(backupFullPath(filePath), buffer);
	}
}

void bbe::simpleFile::backup::createDirectory(const bbe::String &path)
{
	bbe::simpleFile::createDirectory(path);
	if (isBackupPathSet())
	{
		bbe::simpleFile::createDirectory(backupFullPath(path));
	}
}

void bbe::simpleFile::backup::appendBinaryToFile(const bbe::String &filePath, const bbe::ByteBuffer &buffer)
{
	bbe::simpleFile::appendBinaryToFile(filePath, buffer);
	if (isBackupPathSet())
	{
		bbe::simpleFile::appendBinaryToFile(backupFullPath(filePath), buffer);
	}
}

bbe::ByteBuffer bbe::simpleFile::readBinaryFile(const bbe::String &filepath)
{
	updateIoStats();
	if (std::filesystem::is_directory(filepath.getRaw()))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}

	std::ifstream file(filepath.getRaw(), std::ios::binary | std::ios::ate);

	if (file)
	{
		size_t fileSize = (size_t)file.tellg();
		if (fileSize == (size_t)-1)
		{
			throw std::runtime_error("Couldn't determin file size.");
		}
		bbe::List<unsigned char> fileBuffer;
		fileBuffer.resizeCapacityAndLength(fileSize);
		file.seekg(0);
		file.read((char *)fileBuffer.getRaw(), fileSize);
		file.close();
		return bbe::ByteBuffer(std::move(fileBuffer));
	}
	else
	{
		throw std::runtime_error("Failed to open file!");
	}
}

bool bbe::simpleFile::readBinaryFileIfChanged(const bbe::String &filepath, bbe::ByteBuffer &outContents, std::filesystem::file_time_type &inOutPreviousModify)
{
	updateIoStats();
	if (std::filesystem::is_directory(filepath.getRaw()))
	{
		bbe::Crash(bbe::Error::IllegalArgument);
	}

	std::filesystem::file_time_type currentModifyTime;
	try
	{
		currentModifyTime = std::filesystem::last_write_time(filepath.getRaw());
	}
	catch (std::filesystem::filesystem_error &)
	{
		// Probably the file is still in the write process, so we just report that nothing changed.
		return false;
	}
	if (currentModifyTime <= inOutPreviousModify && inOutPreviousModify != std::filesystem::file_time_type()) return false;

	try
	{
		outContents = readBinaryFile(filepath);
	}
	catch (std::runtime_error &)
	{
		// Probably the file is still in the write process, so we just report that nothing changed.
		return false;
	}
	inOutPreviousModify = currentModifyTime;

	return true;
}

bbe::List<float> bbe::simpleFile::readFloatArrFromFile(const bbe::String &filePath)
{
	bbe::ByteBuffer bb = readBinaryFile(filePath);
	auto bbs = bb.getSpan();
	bbe::List<float> retVal;
	while (bbs.hasMore())
	{
		float val = 0.f;
		bbs.read(val);
		retVal.add(val);
	}
	return retVal;
}

void bbe::simpleFile::writeFloatArrToFile(const bbe::String &filePath, const float *arr, size_t size)
{
	updateIoStats();
	std::ofstream file(filePath.getRaw(), std::ios_base::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file!");
	}
	const char *carr = (const char *)arr;
	std::copy(carr, carr + (sizeof(float) * size), std::ostreambuf_iterator<char>(file));
	file.close();
}

void bbe::simpleFile::writeFloatArrToFile(const bbe::String &filePath, const bbe::List<float> &data)
{
	writeFloatArrToFile(filePath, data.getRaw(), data.getLength());
}

void bbe::simpleFile::writeStringToFile(const bbe::String &filePath, const bbe::String &stringToWrite)
{
	updateIoStats();
	std::ofstream file(filePath.getRaw());
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file!");
	}
	file << stringToWrite.getRaw();
	file.close();
}

void bbe::simpleFile::writeBinaryToFile(const bbe::String &filePath, const bbe::ByteBuffer &buffer)
{
	updateIoStats();
	std::ofstream file(filePath.getRaw(), std::ios_base::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file!");
	}
	std::copy(buffer.getRaw(), buffer.getRaw() + buffer.getLength(), std::ostreambuf_iterator<char>(file));
	file.close();
}

void bbe::simpleFile::appendStringToFile(const bbe::String &filePath, const bbe::String &stringToAppend)
{
	updateIoStats();
	std::ofstream file(filePath.getRaw(), std::ofstream::app);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file!");
	}
	file << stringToAppend.getRaw();
	file.close();
}

void bbe::simpleFile::appendBinaryToFile(const bbe::String &filePath, const bbe::ByteBuffer &buffer)
{
	updateIoStats();
	std::ofstream file(filePath.getRaw(), std::ios::binary | std::ofstream::app);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file!");
	}
	std::copy(buffer.getRaw(), buffer.getRaw() + buffer.getLength(), std::ostreambuf_iterator<char>(file));
	file.close();
}

bool bbe::simpleFile::doesFileExist(const bbe::String &filePath)
{
	updateIoStats();
	std::ifstream f(filePath.getRaw());
	return (bool)f;
}

void bbe::simpleFile::createDirectory(const bbe::String &path)
{
	updateIoStats();
	if (!std::filesystem::is_directory(path.getRaw()) || !std::filesystem::exists(path.getRaw()))
	{
		std::filesystem::create_directories(path.getRaw());
	}
}

bool bbe::simpleFile::deleteFile(const bbe::String &path)
{
	updateIoStats();
	return std::filesystem::remove(path.getRaw());
}

bbe::String bbe::simpleFile::readFile(const bbe::String &filePath)
{
	updateIoStats();
	std::ifstream f(filePath.getRaw());
	if (!f.is_open())
	{
		throw std::runtime_error("Could not open file!");
	}
	std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	return bbe::String(str.data());
}

bbe::List<bbe::String> bbe::simpleFile::readLines(const bbe::String &filePath)
{
	updateIoStats();
	std::ifstream file(filePath.getRaw());
	std::string line;
	bbe::List<bbe::String> retVal;
	while (std::getline(file, line))
	{
		retVal.add(line.data());
	}

	return retVal;
}

std::optional<bbe::TimePoint> bbe::simpleFile::getLastModifyTime(const bbe::String &filePath)
{
	updateIoStats();
	std::error_code ec;
	const auto mod = std::filesystem::last_write_time(filePath.getRaw(), ec);
	if (ec)
	{
		return std::nullopt;
	}
	// Madness :)
	const std::time_t t =
		std::chrono::system_clock::to_time_t(
			std::chrono::time_point_cast<std::chrono::system_clock::duration>(
				mod - std::chrono::file_clock::now() + std::chrono::system_clock::now()));
	return bbe::TimePoint(t);
}

bbe::TimePoint bbe::simpleFile::getLastIo()
{
	std::lock_guard _(lastIoMutex);
	return lastIo;
}

size_t bbe::simpleFile::getTotalIoCalls()
{
	std::lock_guard _(lastIoMutex);
	return totalIoCalls;
}

#ifndef __EMSCRIPTEN__
void bbe::simpleFile::forEachFile(const bbe::String &filePath, const std::function<void(const bbe::String &)> &func)
{
	updateIoStats();
	for (const auto &f : std::filesystem::directory_iterator(filePath.getRaw()))
	{
		func(f.path().c_str());
	}
}
#endif

#ifdef __linux__
static std::string getLinuxAbsoluteEnv(const char *name)
{
	const char *value = std::getenv(name);
	if (value == nullptr || value[0] != '/')
	{
		return "";
	}
	return value;
}

static std::string getLinuxConfigHome()
{
	std::string configHome = getLinuxAbsoluteEnv("XDG_CONFIG_HOME");
	if (!configHome.empty())
	{
		return configHome;
	}

	std::string home = getLinuxAbsoluteEnv("HOME");
	if (home.empty())
	{
		throw std::runtime_error("Could not determine HOME directory.");
	}
	return home + "/.config";
}

static std::string escapeDesktopEntryString(const char *input)
{
	std::string escaped;
	for (const char *c = input; *c != '\0'; ++c)
	{
		if (*c == '\\')
		{
			escaped += "\\\\";
		}
		else if (*c == '\n')
		{
			escaped += "\\n";
		}
		else
		{
			escaped += *c;
		}
	}
	return escaped;
}

static bool needsDesktopExecQuoting(const std::string &s)
{
	for (char c : s)
	{
		switch (c)
		{
		case ' ':
		case '\t':
		case '\n':
		case '"':
		case '\'':
		case '>':
		case '<':
		case '~':
		case '|':
		case '&':
		case ';':
		case '$':
		case '*':
		case '?':
		case '=':
		case '#':
		case '(':
		case ')':
		case '`':
			return true;
		default:
			break;
		}
	}
	return false;
}

static std::string quoteDesktopExecArg(const char *input)
{
	std::string raw = input;
	if (!needsDesktopExecQuoting(raw))
	{
		return raw;
	}

	std::string escaped = "\"";
	for (char c : raw)
	{
		if (c == '"')
		{
			escaped += "\\\"";
		}
		else if (c == '`')
		{
			escaped += "\\`";
		}
		else if (c == '$')
		{
			escaped += "\\$";
		}
		else if (c == '\\')
		{
			escaped += "\\\\\\\\";
		}
		else
		{
			escaped += c;
		}
	}
	escaped += "\"";
	return escaped;
}

struct ScopedDBusError
{
	DBusError error;

	ScopedDBusError()
	{
		dbus_error_init(&error);
	}

	~ScopedDBusError()
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
	}

	DBusError *get()
	{
		return &error;
	}
};

struct DBusConnectionDeleter
{
	void operator()(DBusConnection *connection) const
	{
		if (connection)
		{
			dbus_connection_close(connection);
			dbus_connection_unref(connection);
		}
	}
};

struct DBusMessageDeleter
{
	void operator()(DBusMessage *message) const
	{
		if (message)
		{
			dbus_message_unref(message);
		}
	}
};

using ScopedDBusConnection = std::unique_ptr<DBusConnection, DBusConnectionDeleter>;
using ScopedDBusMessage = std::unique_ptr<DBusMessage, DBusMessageDeleter>;

static ScopedDBusConnection getPrivateSessionBus()
{
	ScopedDBusError error;
	DBusConnection *connection = dbus_bus_get_private(DBUS_BUS_SESSION, error.get());
	if (connection == nullptr)
	{
		return ScopedDBusConnection(nullptr);
	}
	dbus_connection_set_exit_on_disconnect(connection, FALSE);
	return ScopedDBusConnection(connection);
}

static bool linuxPortalIsAvailable(DBusConnection *connection)
{
	ScopedDBusError error;
	const dbus_bool_t hasPortal = dbus_bus_name_has_owner(connection, "org.freedesktop.portal.Desktop", error.get());
	return hasPortal == TRUE && !dbus_error_is_set(error.get());
}

static std::string makePortalHandleToken(const char *prefix)
{
	static std::atomic_uint64_t counter = 0;
	return std::string(prefix) + "_" + std::to_string(::getpid()) + "_" + std::to_string(++counter);
}

static std::string makePortalExpectedRequestPath(DBusConnection *connection, const std::string &token)
{
	const char *uniqueName = dbus_bus_get_unique_name(connection);
	if (uniqueName == nullptr || uniqueName[0] == '\0')
	{
		return "";
	}

	std::string sender = uniqueName;
	if (!sender.empty() && sender[0] == ':')
	{
		sender.erase(sender.begin());
	}
	for (char &c : sender)
	{
		if (c == '.')
		{
			c = '_';
		}
	}
	return "/org/freedesktop/portal/desktop/request/" + sender + "/" + token;
}

static bool addPortalResponseMatch(DBusConnection *connection, const std::string &requestPath)
{
	if (requestPath.empty())
	{
		return true;
	}

	const std::string matchRule =
		"type='signal',interface='org.freedesktop.portal.Request',member='Response',path='" + requestPath + "'";
	ScopedDBusError error;
	dbus_bus_add_match(connection, matchRule.c_str(), error.get());
	dbus_connection_flush(connection);
	return !dbus_error_is_set(error.get());
}

static void removePortalResponseMatch(DBusConnection *connection, const std::string &requestPath)
{
	if (requestPath.empty())
	{
		return;
	}

	const std::string matchRule =
		"type='signal',interface='org.freedesktop.portal.Request',member='Response',path='" + requestPath + "'";
	ScopedDBusError error;
	dbus_bus_remove_match(connection, matchRule.c_str(), error.get());
	dbus_connection_flush(connection);
}

static bool appendStringOption(DBusMessageIter *optionsIter, const char *key, const char *value)
{
	DBusMessageIter entryIter;
	DBusMessageIter variantIter;
	if (!dbus_message_iter_open_container(optionsIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter)) return false;
	if (!dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key)) return false;
	if (!dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "s", &variantIter)) return false;
	if (!dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value)) return false;
	if (!dbus_message_iter_close_container(&entryIter, &variantIter)) return false;
	return dbus_message_iter_close_container(optionsIter, &entryIter) == TRUE;
}

static bool appendBoolOption(DBusMessageIter *optionsIter, const char *key, bool value)
{
	DBusMessageIter entryIter;
	DBusMessageIter variantIter;
	const dbus_bool_t boolValue = value ? TRUE : FALSE;
	if (!dbus_message_iter_open_container(optionsIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter)) return false;
	if (!dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key)) return false;
	if (!dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "b", &variantIter)) return false;
	if (!dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_BOOLEAN, &boolValue)) return false;
	if (!dbus_message_iter_close_container(&entryIter, &variantIter)) return false;
	return dbus_message_iter_close_container(optionsIter, &entryIter) == TRUE;
}

static bool appendByteArrayOption(DBusMessageIter *optionsIter, const char *key, const std::string &value)
{
	DBusMessageIter entryIter;
	DBusMessageIter variantIter;
	DBusMessageIter arrayIter;

	if (!dbus_message_iter_open_container(optionsIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter)) return false;
	if (!dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key)) return false;
	if (!dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "ay", &variantIter)) return false;
	if (!dbus_message_iter_open_container(&variantIter, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &arrayIter)) return false;

	std::vector<unsigned char> bytes(value.begin(), value.end());
	bytes.push_back('\0');
	if (!bytes.empty())
	{
		const unsigned char *data = bytes.data();
		if (!dbus_message_iter_append_fixed_array(&arrayIter, DBUS_TYPE_BYTE, &data, static_cast<int>(bytes.size()))) return false;
	}

	if (!dbus_message_iter_close_container(&variantIter, &arrayIter)) return false;
	if (!dbus_message_iter_close_container(&entryIter, &variantIter)) return false;
	return dbus_message_iter_close_container(optionsIter, &entryIter) == TRUE;
}

static int hexDigitToInt(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + c - 'a';
	if (c >= 'A' && c <= 'F') return 10 + c - 'A';
	return -1;
}

static bbe::String decodeFileUri(const char *uri)
{
	constexpr const char *prefix = "file://";
	if (std::strncmp(uri, prefix, std::strlen(prefix)) != 0)
	{
		return "";
	}

	std::string path = uri + std::strlen(prefix);
	if (path.rfind("localhost/", 0) == 0)
	{
		path.erase(0, std::strlen("localhost"));
	}
	else if (!path.empty() && path[0] != '/')
	{
		return "";
	}

	std::string decoded;
	for (size_t i = 0; i < path.size(); ++i)
	{
		if (path[i] == '%' && i + 2 < path.size())
		{
			const int high = hexDigitToInt(path[i + 1]);
			const int low = hexDigitToInt(path[i + 2]);
			if (high >= 0 && low >= 0)
			{
				decoded += static_cast<char>((high << 4) | low);
				i += 2;
				continue;
			}
		}
		decoded += path[i];
	}

	return bbe::String(decoded.c_str());
}

static bool extractFirstUriFromResponse(DBusMessage *responseMessage, bbe::String &outPath)
{
	DBusMessageIter iter;
	if (!dbus_message_iter_init(responseMessage, &iter)) return false;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UINT32) return false;
	dbus_uint32_t responseCode = 2;
	dbus_message_iter_get_basic(&iter, &responseCode);
	if (responseCode != 0) return false;

	if (!dbus_message_iter_next(&iter)) return false;
	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) return false;

	DBusMessageIter dictIter;
	dbus_message_iter_recurse(&iter, &dictIter);
	while (dbus_message_iter_get_arg_type(&dictIter) == DBUS_TYPE_DICT_ENTRY)
	{
		DBusMessageIter entryIter;
		dbus_message_iter_recurse(&dictIter, &entryIter);
		if (dbus_message_iter_get_arg_type(&entryIter) == DBUS_TYPE_STRING)
		{
			const char *key = nullptr;
			dbus_message_iter_get_basic(&entryIter, &key);
			if (std::strcmp(key, "uris") == 0 && dbus_message_iter_next(&entryIter) && dbus_message_iter_get_arg_type(&entryIter) == DBUS_TYPE_VARIANT)
			{
				DBusMessageIter variantIter;
				dbus_message_iter_recurse(&entryIter, &variantIter);
				if (dbus_message_iter_get_arg_type(&variantIter) == DBUS_TYPE_ARRAY)
				{
					DBusMessageIter uriIter;
					dbus_message_iter_recurse(&variantIter, &uriIter);
					if (dbus_message_iter_get_arg_type(&uriIter) == DBUS_TYPE_STRING)
					{
						const char *uri = nullptr;
						dbus_message_iter_get_basic(&uriIter, &uri);
						outPath = decodeFileUri(uri);
						return !outPath.isEmpty();
					}
				}
			}
		}
		dbus_message_iter_next(&dictIter);
	}

	return false;
}

static bool waitForPortalResponse(DBusConnection *connection, const std::string &requestPath, bbe::String &outPath)
{
	while (true)
	{
		if (!dbus_connection_read_write(connection, -1))
		{
			return false;
		}

		ScopedDBusMessage message(dbus_connection_pop_message(connection));
		if (!message)
		{
			continue;
		}

		const char *path = dbus_message_get_path(message.get());
		if (path == nullptr || requestPath != path)
		{
			continue;
		}

		if (dbus_message_is_signal(message.get(), "org.freedesktop.portal.Request", "Response"))
		{
			return extractFirstUriFromResponse(message.get(), outPath);
		}
	}
}

static bool linuxPortalFileChooser(const char *methodName, const char *title, bbe::String &inOutPath, const bbe::String &defaultExtension)
{
	ScopedDBusConnection connection = getPrivateSessionBus();
	if (!connection || !linuxPortalIsAvailable(connection.get()))
	{
		return false;
	}

	const std::string handleToken = makePortalHandleToken("bbe");
	const std::string expectedPath = makePortalExpectedRequestPath(connection.get(), handleToken);
	if (!addPortalResponseMatch(connection.get(), expectedPath))
	{
		return false;
	}

	ScopedDBusMessage message(dbus_message_new_method_call(
		"org.freedesktop.portal.Desktop",
		"/org/freedesktop/portal/desktop",
		"org.freedesktop.portal.FileChooser",
		methodName));
	if (!message)
	{
		removePortalResponseMatch(connection.get(), expectedPath);
		return false;
	}

	DBusMessageIter iter;
	dbus_message_iter_init_append(message.get(), &iter);
	const char *parentWindow = "";
	if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &parentWindow))
	{
		removePortalResponseMatch(connection.get(), expectedPath);
		return false;
	}
	if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &title))
	{
		removePortalResponseMatch(connection.get(), expectedPath);
		return false;
	}

	DBusMessageIter optionsIter;
	if (!dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &optionsIter))
	{
		removePortalResponseMatch(connection.get(), expectedPath);
		return false;
	}

	const bool isSaveDialog = std::strcmp(methodName, "SaveFile") == 0;
	bool optionsOk = appendStringOption(&optionsIter, "handle_token", handleToken.c_str()) && appendBoolOption(&optionsIter, "modal", true);

	if (isSaveDialog)
	{
		std::filesystem::path currentPath;
		if (!inOutPath.isEmpty())
		{
			currentPath = std::filesystem::path(inOutPath.getRaw());
			if (currentPath.has_parent_path())
			{
				optionsOk = optionsOk && appendByteArrayOption(&optionsIter, "current_folder", currentPath.parent_path().string());
			}
			if (std::filesystem::exists(currentPath))
			{
				optionsOk = optionsOk && appendByteArrayOption(&optionsIter, "current_file", currentPath.string());
			}
		}

		std::string currentName = currentPath.filename().string();
		if (currentName.empty())
		{
			currentName = "untitled";
			if (!defaultExtension.isEmpty())
			{
				currentName += ".";
				currentName += defaultExtension.getRaw();
			}
		}
		optionsOk = optionsOk && appendStringOption(&optionsIter, "current_name", currentName.c_str());
	}
	else if (!inOutPath.isEmpty())
	{
		const std::filesystem::path currentPath(inOutPath.getRaw());
		const std::filesystem::path folder = std::filesystem::is_directory(currentPath) ? currentPath : currentPath.parent_path();
		if (!folder.empty())
		{
			optionsOk = optionsOk && appendByteArrayOption(&optionsIter, "current_folder", folder.string());
		}
	}

	if (!dbus_message_iter_close_container(&iter, &optionsIter) || !optionsOk)
	{
		removePortalResponseMatch(connection.get(), expectedPath);
		return false;
	}

	ScopedDBusError error;
	ScopedDBusMessage reply(dbus_connection_send_with_reply_and_block(connection.get(), message.get(), -1, error.get()));
	if (!reply || dbus_error_is_set(error.get()))
	{
		removePortalResponseMatch(connection.get(), expectedPath);
		return false;
	}

	const char *returnedHandle = nullptr;
	if (!dbus_message_get_args(reply.get(), error.get(), DBUS_TYPE_OBJECT_PATH, &returnedHandle, DBUS_TYPE_INVALID) || returnedHandle == nullptr)
	{
		removePortalResponseMatch(connection.get(), expectedPath);
		return false;
	}

	const std::string actualPath = returnedHandle;
	if (actualPath != expectedPath && !addPortalResponseMatch(connection.get(), actualPath))
	{
		removePortalResponseMatch(connection.get(), expectedPath);
		return false;
	}

	bbe::String selectedPath;
	const bool success = waitForPortalResponse(connection.get(), actualPath.empty() ? expectedPath : actualPath, selectedPath);
	removePortalResponseMatch(connection.get(), expectedPath);
	if (actualPath != expectedPath)
	{
		removePortalResponseMatch(connection.get(), actualPath);
	}

	if (!success)
	{
		return false;
	}

	inOutPath = selectedPath;
	return true;
}
#endif

#ifdef WIN32
#define NOMINMAX
#include "windows.h"
#include "winnls.h"
#include "shobjidl.h"
#include "objbase.h"
#include "objidl.h"
#include "shlguid.h"
#include <vector>

bbe::String bbe::simpleFile::getUserName()
{
	updateIoStats();
	TCHAR buffer[256];
	DWORD bufferSize = sizeof(buffer) / sizeof(TCHAR);
	if (GetUserName(buffer, &bufferSize))
	{
		return bbe::String(buffer);
	}
	else
	{
		throw std::runtime_error("Error getting user name.");
	}
}

bbe::String bbe::simpleFile::getAutoStartDirectory()
{
	return "C:/Users/" + getUserName() + "/AppData/Roaming/Microsoft/Windows/Start Menu/Programs/Startup/";
}

bbe::String bbe::simpleFile::getExecutablePath()
{
	updateIoStats();
	TCHAR buffer[1024] = {};
	GetModuleFileName(NULL, buffer, sizeof(buffer) / sizeof(TCHAR));
	return bbe::String(buffer);
}

bbe::String bbe::simpleFile::getWorkingDirectory()
{
	updateIoStats();
	TCHAR buffer[1024] = {};
	GetCurrentDirectory(sizeof(buffer) / sizeof(TCHAR), buffer);
	return bbe::String(buffer);
}

void bbe::simpleFile::createLink(const bbe::String &from, const bbe::String &to, const bbe::String &workDir)
{
	updateIoStats();
	// More or less from the microsoft examples, adjusted to use BBE Types and coding style.
	// See: https://learn.microsoft.com/en-us/windows/win32/shell/links

	IShellLink *psl;
	HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);
	if (SUCCEEDED(hres))
	{
		psl->SetPath(to.getRaw());
		if (!workDir.isEmpty()) psl->SetWorkingDirectory(workDir.getRaw());

		IPersistFile *ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
		if (SUCCEEDED(hres))
		{
			WCHAR wsz[1024];
			MultiByteToWideChar(CP_ACP, 0, from.getRaw(), -1, wsz, sizeof(wsz) / sizeof(WCHAR));
			hres = ppf->Save(wsz, TRUE);
			ppf->Release();
		}
		psl->Release();
	}
	if (FAILED(hres))
	{
		throw std::runtime_error("Failed to create link.");
	}
}
void bbe::simpleFile::executeBatchFile(const bbe::String &path)
{
	updateIoStats();

	const std::wstring widePath = path.toStdWString();
	if (widePath.empty())
	{
		throw std::runtime_error("Failed to execute batch file.");
	}

	wchar_t systemDirectory[MAX_PATH] = {};
	if (GetSystemDirectoryW(systemDirectory, MAX_PATH) == 0)
	{
		throw std::runtime_error("Failed to locate cmd.exe.");
	}

	const std::wstring cmdPath = std::wstring(systemDirectory) + L"\\cmd.exe";
	std::wstring commandLine = L"/c call \"";
	commandLine += widePath;
	commandLine += L"\"";
	std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
	mutableCommandLine.push_back(L'\0');

	STARTUPINFOW si = {};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = {};
	if (!CreateProcessW(
			cmdPath.c_str(),
			mutableCommandLine.data(),
			nullptr,
			nullptr,
			FALSE,
			0,
			nullptr,
			nullptr,
			&si,
			&pi))
	{
		throw std::runtime_error("Failed to execute batch file.");
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

bool bbe::simpleFile::showOpenDialog(bbe::String &outPath)
{
	updateIoStats();
	OPENFILENAME ofn;
	char path[1024] = {};

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn))
	{
		outPath = path;
		return true;
	}
	return false;
}
bool bbe::simpleFile::showSaveDialog(bbe::String &outPath, const bbe::String &defaultExtension)
{
	updateIoStats();
	OPENFILENAME ofn;
	char path[1024] = {};

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = defaultExtension.getRaw();
	if (GetSaveFileName(&ofn))
	{
		outPath = path;
		return true;
	}
	return false;
}
#endif

#ifdef __linux__
bbe::String bbe::simpleFile::getAutoStartDirectory()
{
	updateIoStats();
	return bbe::String((getLinuxConfigHome() + "/autostart/").c_str());
}

bbe::String bbe::simpleFile::getExecutablePath()
{
	updateIoStats();
	char buffer[PATH_MAX] = {};
	const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
	if (length < 0)
	{
		throw std::runtime_error("Failed to determine executable path.");
	}
	buffer[length] = '\0';
	return bbe::String(buffer);
}

bbe::String bbe::simpleFile::getWorkingDirectory()
{
	updateIoStats();
	return bbe::String(std::filesystem::current_path().string().c_str());
}

void bbe::simpleFile::createLink(const bbe::String &from, const bbe::String &to, const bbe::String &workDir)
{
	updateIoStats();

	const std::filesystem::path desktopFilePath(from.getRaw());
	if (desktopFilePath.has_parent_path())
	{
		std::filesystem::create_directories(desktopFilePath.parent_path());
	}

	std::ofstream file(from.getRaw());
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to create autostart desktop entry.");
	}

	std::string appName = desktopFilePath.stem().string();
	if (appName.empty())
	{
		appName = "Application";
	}

	file << "[Desktop Entry]\n";
	file << "Type=Application\n";
	file << "Version=1.0\n";
	file << "Name=" << escapeDesktopEntryString(appName.c_str()) << "\n";
	file << "Exec=" << quoteDesktopExecArg(to.getRaw()) << "\n";
	if (!workDir.isEmpty())
	{
		file << "Path=" << escapeDesktopEntryString(workDir.getRaw()) << "\n";
	}
	file << "Terminal=false\n";
	file << "X-GNOME-Autostart-enabled=true\n";
	file << "StartupNotify=false\n";
	file.close();

	std::filesystem::permissions(
		desktopFilePath,
		std::filesystem::perms::owner_read |
			std::filesystem::perms::owner_write |
			std::filesystem::perms::owner_exec |
			std::filesystem::perms::group_read |
			std::filesystem::perms::group_exec |
			std::filesystem::perms::others_read |
			std::filesystem::perms::others_exec,
		std::filesystem::perm_options::replace);
}

bool bbe::simpleFile::showOpenDialog(bbe::String &outPath)
{
	updateIoStats();
	return linuxPortalFileChooser("OpenFile", "Open File", outPath, "");
}

bool bbe::simpleFile::showSaveDialog(bbe::String &outPath, const bbe::String &defaultExtension)
{
	updateIoStats();
	return linuxPortalFileChooser("SaveFile", "Save File", outPath, defaultExtension);
}
#endif

#ifndef __EMSCRIPTEN__
enum class AsyncJobType
{
	WRITE_BINARY,
	CREATE_DIRECTORY,
	APPEND_BINARY,
};

struct AsyncJob
{
	AsyncJobType type;
	bbe::String path;
	bbe::ByteBuffer buffer;
};

bbe::ConcurrentList<AsyncJob> jobs;
std::atomic_bool ioThreadRunning = false;
std::thread ioThread;
std::condition_variable_any conditional;

static void innerIoThreadMain()
{
	while (jobs.getLength() > 0)
	{
		const AsyncJob job = jobs.popFront();
		if (job.type == AsyncJobType::WRITE_BINARY)
		{
			bbe::simpleFile::backup::writeBinaryToFile(job.path, job.buffer);
		}
		else if (job.type == AsyncJobType::CREATE_DIRECTORY)
		{
			bbe::simpleFile::backup::createDirectory(job.path);
		}
		else if (job.type == AsyncJobType::APPEND_BINARY)
		{
			bbe::simpleFile::backup::appendBinaryToFile(job.path, job.buffer);
		}
		else
		{
			bbe::Crash(bbe::Error::IllegalState);
		}

		if (jobs.getLength() == 0 && ioThreadRunning)
		{
			std::unique_lock ul(jobs);
			conditional.wait(ul, []
							 { return jobs.getLength() > 0 || !ioThreadRunning; });
		}
	}
}

static void ioThreadMain()
{
	BBE_TRY_RELEASE
	{
		innerIoThreadMain();
	}
	BBE_CATCH_RELEASE(IO Thread)
}

static void notifyIOThread()
{
	const bool launchIoThread = !ioThreadRunning.exchange(true);
	if (launchIoThread)
	{
		ioThread = std::thread(ioThreadMain);
		bbe::simpleThread::setName(ioThread, "BBE ioThread");
	}
	conditional.notify_all();
}
#endif

bool bbe::simpleFile::backup::async::hasOpenIO()
{
#ifdef __EMSCRIPTEN__
	return false;
#else
	return jobs.getLength() > 0;
#endif
}

void bbe::simpleFile::backup::async::stopIoThread()
{
#ifndef __EMSCRIPTEN__
	ioThreadRunning = false;
	conditional.notify_all();
	if (ioThread.joinable())
	{
		ioThread.join();
	}
#endif
}

void bbe::simpleFile::backup::async::writeBinaryToFile(const bbe::String &filePath, const bbe::ByteBuffer &buffer)
{
	updateIoStats();
#ifdef __EMSCRIPTEN__
	bbe::simpleFile::backup::writeBinaryToFile(filePath, buffer);
#else
	jobs.add({ AsyncJobType::WRITE_BINARY, filePath, buffer });
	notifyIOThread();
#endif
}

void bbe::simpleFile::backup::async::createDirectory(const bbe::String &path)
{
	updateIoStats();
#ifdef __EMSCRIPTEN__
	bbe::simpleFile::backup::createDirectory(path);
#else
	jobs.add({ AsyncJobType::CREATE_DIRECTORY, path });
	notifyIOThread();
#endif
}

void bbe::simpleFile::backup::async::appendBinaryToFile(const bbe::String &filePath, const bbe::ByteBuffer &buffer)
{
	updateIoStats();
#ifdef __EMSCRIPTEN__
	bbe::simpleFile::backup::appendBinaryToFile(filePath, buffer);
#else
	jobs.add({ AsyncJobType::APPEND_BINARY, filePath, buffer });
	notifyIOThread();
#endif
}
