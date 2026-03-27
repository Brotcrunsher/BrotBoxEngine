#include "BBE/TrayIcon.h"

#include <cstring>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>

static NOTIFYICONDATA notifyIconData = {};
static HWND Hwnd = 0;
static HMENU Hmenu = 0;
static char szClassName[] = "MyWindowClassName";
static const char *myTooltip = nullptr;
static bbe::Game *myGame = nullptr;
static bbe::List<std::function<void()>> popupCallbacks;

#define WM_SYSICON (WM_USER + 1)
#define CALLBACK_OFFSET 1000

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SYSICON:
	{
		if (lParam == WM_RBUTTONDOWN)
		{
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(Hwnd);

			UINT clicked = TrackPopupMenu(Hmenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);

			SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
			if (clicked >= CALLBACK_OFFSET)
			{
				const size_t id = clicked - CALLBACK_OFFSET;
				popupCallbacks[id]();
			}
		}
		else if (lParam == WM_LBUTTONDBLCLK)
		{
			myGame->showWindow();
		}
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void bbe::TrayIcon::init(bbe::Game *game, const char *tooltip, IconHandle icon)
{
	myGame = game;
	myTooltip = tooltip;

	WNDCLASSEX wincl = {};
	wincl.hInstance = GetModuleHandle(0);
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;
	wincl.style = CS_DBLCLKS;
	wincl.cbSize = sizeof(WNDCLASSEX);
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(255, 255, 255)));
	RegisterClassEx(&wincl);

	Hwnd = CreateWindowEx(
		0,
		szClassName,
		szClassName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		544,
		375,
		HWND_DESKTOP,
		NULL,
		GetModuleHandle(0),
		NULL);

	setIcon(icon);
}

void bbe::TrayIcon::addPopupItem(const char *title, std::function<void()> callback)
{
	Hmenu = CreatePopupMenu();
	AppendMenu(Hmenu, MF_STRING, popupCallbacks.getLength() + CALLBACK_OFFSET, title);
	popupCallbacks.add(callback);
}

bool bbe::TrayIcon::isVisible()
{
	// Quite Hacky! Let me explain...
	// Switching a tray icon is an expensive operation, even when
	// the tray icon isn't even visible. So this function tries to
	// find out if the tray icon is visible or not. If not, we can
	// early out of the tray switching function (see
	// setCurrentTrayIcon). The only way to figure this out that I
	// found was asking for the position of the tray icon. When it's
	// not visible, the left attribute of the Rect returns something
	// around 300.
	// However! Getting the position of the tray icon is itself
	// (maybe surprisingly) an expensive opteration. So this function
	// only checks every second. If this second hasn't passed, then
	// it simply returns false. In a worse case this leads to a very
	// small (1 second) delay before the animation starts.
	// Honestly - this is dumb! But oh well. It kinda works.
	static bbe::TimePoint autoFalseUntil;

	if (!autoFalseUntil.hasPassed())
	{
		return false;
	}

	NOTIFYICONIDENTIFIER identifier = {};
	identifier.cbSize = sizeof(NOTIFYICONIDENTIFIER);
	identifier.hWnd = Hwnd;
	identifier.uID = 0;
	memset(&identifier.guidItem, 0, sizeof(identifier.guidItem));
	RECT rect;
	Shell_NotifyIconGetRect(&identifier, &rect);

	bool retVal = rect.left > 500;
	if (!retVal)
		autoFalseUntil = bbe::TimePoint().plusSeconds(1);
	return retVal;
}

void bbe::TrayIcon::setIcon(IconHandle icon)
{
	static bool firstCall = true;
	static IconHandle previousHIcon = nullptr;
	if (previousHIcon == icon)
	{
		return;
	}
	previousHIcon = icon;

	memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));

	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = Hwnd;
	notifyIconData.uID = 0;
	notifyIconData.uFlags = NIF_ICON | (firstCall ? (NIF_MESSAGE | NIF_TIP) : 0);
	notifyIconData.hIcon = icon;
	if (firstCall)
	{
		notifyIconData.uCallbackMessage = WM_SYSICON; // Set up our invented Windows Message
		strcpy_s(notifyIconData.szTip, sizeof(notifyIconData.szTip), myTooltip);
	}

	Shell_NotifyIcon(firstCall ? NIM_ADD : NIM_MODIFY, &notifyIconData);
	firstCall = false;
}

void bbe::TrayIcon::update()
{
	// Nothing to do on Windows. Messages are handled by the normal window procedure.
}

#elif defined(__linux__)

#include <dbus/dbus.h>
#include <string>
#include <unistd.h>
#include <vector>

struct bbe::TrayIcon::LinuxIconHandle
{
	int width = 0;
	int height = 0;
	std::vector<unsigned char> argb32NetworkOrder;
};

namespace
{
	constexpr const char *ITEM_OBJECT_PATH = "/StatusNotifierItem";
	constexpr const char *MENU_OBJECT_PATH = "/StatusNotifierMenu";
	constexpr const char *KDE_WATCHER_SERVICE = "org.kde.StatusNotifierWatcher";
	constexpr const char *KDE_WATCHER_INTERFACE = "org.kde.StatusNotifierWatcher";
	constexpr const char *FREEDESKTOP_WATCHER_SERVICE = "org.freedesktop.StatusNotifierWatcher";
	constexpr const char *FREEDESKTOP_WATCHER_INTERFACE = "org.freedesktop.StatusNotifierWatcher";
	constexpr const char *KDE_ITEM_INTERFACE = "org.kde.StatusNotifierItem";
	constexpr const char *FREEDESKTOP_ITEM_INTERFACE = "org.freedesktop.StatusNotifierItem";
	constexpr const char *DBUS_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";
	constexpr const char *DBUS_INTROSPECTABLE_INTERFACE = "org.freedesktop.DBus.Introspectable";
	constexpr const char *DBUSMENU_INTERFACE = "com.canonical.dbusmenu";

	struct MenuItem
	{
		std::string title;
		std::function<void()> callback;
	};

	DBusConnection *g_connection = nullptr;
	std::string g_serviceName;
	std::string g_tooltip;
	bbe::Game *g_game = nullptr;
	bbe::TrayIcon::IconHandle g_currentIcon = nullptr;
	std::vector<MenuItem> g_menuItems;
	uint32_t g_menuRevision = 1;
	bool g_registered = false;

	const char *getWatcherServiceName()
	{
		if (!g_connection)
		{
			return nullptr;
		}

		DBusError error;
		dbus_error_init(&error);
		const dbus_bool_t hasKdeWatcher = dbus_bus_name_has_owner(g_connection, KDE_WATCHER_SERVICE, &error);
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
			return nullptr;
		}
		if (hasKdeWatcher)
		{
			return KDE_WATCHER_SERVICE;
		}

		const dbus_bool_t hasFreedesktopWatcher = dbus_bus_name_has_owner(g_connection, FREEDESKTOP_WATCHER_SERVICE, &error);
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
			return nullptr;
		}
		if (hasFreedesktopWatcher)
		{
			return FREEDESKTOP_WATCHER_SERVICE;
		}

		return nullptr;
	}

	const char *getWatcherInterfaceName(const char *watcherService)
	{
		if (!watcherService)
		{
			return nullptr;
		}
		if (std::strcmp(watcherService, KDE_WATCHER_SERVICE) == 0)
		{
			return KDE_WATCHER_INTERFACE;
		}
		return FREEDESKTOP_WATCHER_INTERFACE;
	}

	bool isItemInterface(const char *interfaceName)
	{
		return interfaceName &&
			   (std::strcmp(interfaceName, KDE_ITEM_INTERFACE) == 0 || std::strcmp(interfaceName, FREEDESKTOP_ITEM_INTERFACE) == 0);
	}

	void appendStringVariant(DBusMessageIter *dictIter, const char *key, const char *value)
	{
		DBusMessageIter entryIter;
		dbus_message_iter_open_container(dictIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
		dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
		DBusMessageIter variantIter;
		dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "s", &variantIter);
		dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
		dbus_message_iter_close_container(&entryIter, &variantIter);
		dbus_message_iter_close_container(dictIter, &entryIter);
	}

	void appendBoolVariant(DBusMessageIter *dictIter, const char *key, dbus_bool_t value)
	{
		DBusMessageIter entryIter;
		dbus_message_iter_open_container(dictIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
		dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
		DBusMessageIter variantIter;
		dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "b", &variantIter);
		dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_BOOLEAN, &value);
		dbus_message_iter_close_container(&entryIter, &variantIter);
		dbus_message_iter_close_container(dictIter, &entryIter);
	}

	void appendUIntVariant(DBusMessageIter *dictIter, const char *key, dbus_uint32_t value)
	{
		DBusMessageIter entryIter;
		dbus_message_iter_open_container(dictIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
		dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
		DBusMessageIter variantIter;
		dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "u", &variantIter);
		dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_UINT32, &value);
		dbus_message_iter_close_container(&entryIter, &variantIter);
		dbus_message_iter_close_container(dictIter, &entryIter);
	}

	void appendIntVariant(DBusMessageIter *dictIter, const char *key, dbus_int32_t value)
	{
		DBusMessageIter entryIter;
		dbus_message_iter_open_container(dictIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
		dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
		DBusMessageIter variantIter;
		dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "i", &variantIter);
		dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_INT32, &value);
		dbus_message_iter_close_container(&entryIter, &variantIter);
		dbus_message_iter_close_container(dictIter, &entryIter);
	}

	void appendObjectPathVariant(DBusMessageIter *dictIter, const char *key, const char *value)
	{
		DBusMessageIter entryIter;
		dbus_message_iter_open_container(dictIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
		dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
		DBusMessageIter variantIter;
		dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "o", &variantIter);
		dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_OBJECT_PATH, &value);
		dbus_message_iter_close_container(&entryIter, &variantIter);
		dbus_message_iter_close_container(dictIter, &entryIter);
	}

	void appendIconPixmapArray(DBusMessageIter *iter, const bbe::TrayIcon::LinuxIconHandle *icon)
	{
		DBusMessageIter iconArrayIter;
		dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "(iiay)", &iconArrayIter);
		if (icon)
		{
			DBusMessageIter iconStructIter;
			dbus_message_iter_open_container(&iconArrayIter, DBUS_TYPE_STRUCT, nullptr, &iconStructIter);
			dbus_int32_t width = icon->width;
			dbus_int32_t height = icon->height;
			dbus_message_iter_append_basic(&iconStructIter, DBUS_TYPE_INT32, &width);
			dbus_message_iter_append_basic(&iconStructIter, DBUS_TYPE_INT32, &height);
			DBusMessageIter bytesIter;
			dbus_message_iter_open_container(&iconStructIter, DBUS_TYPE_ARRAY, "y", &bytesIter);
			if (!icon->argb32NetworkOrder.empty())
			{
				const unsigned char *bytes = icon->argb32NetworkOrder.data();
				dbus_message_iter_append_fixed_array(
					&bytesIter,
					DBUS_TYPE_BYTE,
					&bytes,
					static_cast<int>(icon->argb32NetworkOrder.size()));
			}
			dbus_message_iter_close_container(&iconStructIter, &bytesIter);
			dbus_message_iter_close_container(&iconArrayIter, &iconStructIter);
		}
		dbus_message_iter_close_container(iter, &iconArrayIter);
	}

	void appendToolTipVariant(DBusMessageIter *dictIter)
	{
		const char *key = "ToolTip";
		DBusMessageIter entryIter;
		dbus_message_iter_open_container(dictIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
		dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
		DBusMessageIter variantIter;
		dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "(sa(iiay)ss)", &variantIter);
		DBusMessageIter structIter;
		dbus_message_iter_open_container(&variantIter, DBUS_TYPE_STRUCT, nullptr, &structIter);

		const char *iconName = "";
		dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &iconName);
		appendIconPixmapArray(&structIter, g_currentIcon);
		const char *title = g_tooltip.c_str();
		const char *description = "";
		dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &title);
		dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &description);

		dbus_message_iter_close_container(&variantIter, &structIter);
		dbus_message_iter_close_container(&entryIter, &variantIter);
		dbus_message_iter_close_container(dictIter, &entryIter);
	}

	void appendItemProperties(DBusMessageIter *dictIter)
	{
		const char *category = "ApplicationStatus";
		const char *id = "BROTBOX";
		const char *status = "Active";
		const char *iconName = "";
		const char *overlayIconName = "";
		const char *attentionIconName = "";
		const char *attentionMovieName = "";
		const char *iconThemePath = "";
		const char *menuPath = MENU_OBJECT_PATH;
		const dbus_bool_t itemIsMenu = FALSE;
		const dbus_uint32_t windowId = 0;

		appendStringVariant(dictIter, "Category", category);
		appendStringVariant(dictIter, "Id", id);
		appendStringVariant(dictIter, "Title", g_tooltip.c_str());
		appendStringVariant(dictIter, "Status", status);
		appendUIntVariant(dictIter, "WindowId", windowId);
		appendStringVariant(dictIter, "IconThemePath", iconThemePath);
		appendStringVariant(dictIter, "IconName", iconName);
		appendStringVariant(dictIter, "OverlayIconName", overlayIconName);
		appendStringVariant(dictIter, "AttentionIconName", attentionIconName);
		appendStringVariant(dictIter, "AttentionMovieName", attentionMovieName);
		appendObjectPathVariant(dictIter, "Menu", menuPath);
		appendBoolVariant(dictIter, "ItemIsMenu", itemIsMenu);

		{
			const char *key = "IconPixmap";
			DBusMessageIter entryIter;
			dbus_message_iter_open_container(dictIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
			dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "a(iiay)", &variantIter);
			appendIconPixmapArray(&variantIter, g_currentIcon);
			dbus_message_iter_close_container(&entryIter, &variantIter);
			dbus_message_iter_close_container(dictIter, &entryIter);
		}

		{
			const char *key = "OverlayIconPixmap";
			DBusMessageIter entryIter;
			dbus_message_iter_open_container(dictIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
			dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "a(iiay)", &variantIter);
			appendIconPixmapArray(&variantIter, nullptr);
			dbus_message_iter_close_container(&entryIter, &variantIter);
			dbus_message_iter_close_container(dictIter, &entryIter);
		}

		{
			const char *key = "AttentionIconPixmap";
			DBusMessageIter entryIter;
			dbus_message_iter_open_container(dictIter, DBUS_TYPE_DICT_ENTRY, nullptr, &entryIter);
			dbus_message_iter_append_basic(&entryIter, DBUS_TYPE_STRING, &key);
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(&entryIter, DBUS_TYPE_VARIANT, "a(iiay)", &variantIter);
			appendIconPixmapArray(&variantIter, nullptr);
			dbus_message_iter_close_container(&entryIter, &variantIter);
			dbus_message_iter_close_container(dictIter, &entryIter);
		}

		appendToolTipVariant(dictIter);
	}

	void appendMenuProperties(DBusMessageIter *dictIter)
	{
		appendUIntVariant(dictIter, "Version", 4);
		appendStringVariant(dictIter, "Status", "normal");
	}

	void appendSingleMenuItemProperties(DBusMessageIter *dictIter, const MenuItem &item)
	{
		appendStringVariant(dictIter, "label", item.title.c_str());
		appendStringVariant(dictIter, "type", "standard");
		appendBoolVariant(dictIter, "enabled", TRUE);
		appendBoolVariant(dictIter, "visible", TRUE);
	}

	void appendMenuLayoutItem(DBusMessageIter *iter, int32_t itemId, const MenuItem *item, bool includeChildren)
	{
		DBusMessageIter structIter;
		dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT, nullptr, &structIter);
		dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT32, &itemId);

		DBusMessageIter propertiesIter;
		dbus_message_iter_open_container(&structIter, DBUS_TYPE_ARRAY, "{sv}", &propertiesIter);
		if (item)
		{
			appendSingleMenuItemProperties(&propertiesIter, *item);
		}
		dbus_message_iter_close_container(&structIter, &propertiesIter);

		DBusMessageIter childrenIter;
		dbus_message_iter_open_container(&structIter, DBUS_TYPE_ARRAY, "v", &childrenIter);
		if (includeChildren && itemId == 0)
		{
			for (size_t i = 0; i < g_menuItems.size(); ++i)
			{
				DBusMessageIter childVariantIter;
				dbus_message_iter_open_container(&childrenIter, DBUS_TYPE_VARIANT, "(ia{sv}av)", &childVariantIter);
				appendMenuLayoutItem(&childVariantIter, static_cast<int32_t>(i + 1), &g_menuItems[i], false);
				dbus_message_iter_close_container(&childrenIter, &childVariantIter);
			}
		}
		dbus_message_iter_close_container(&structIter, &childrenIter);
		dbus_message_iter_close_container(iter, &structIter);
	}

	void emitSimpleSignal(const char *path, const char *interfaceName, const char *signalName)
	{
		if (!g_connection)
		{
			return;
		}
		DBusMessage *signal = dbus_message_new_signal(path, interfaceName, signalName);
		if (!signal)
		{
			return;
		}
		dbus_connection_send(g_connection, signal, nullptr);
		dbus_connection_flush(g_connection);
		dbus_message_unref(signal);
	}

	void emitLayoutUpdatedSignal()
	{
		if (!g_connection)
		{
			return;
		}
		DBusMessage *signal = dbus_message_new_signal(MENU_OBJECT_PATH, DBUSMENU_INTERFACE, "LayoutUpdated");
		if (!signal)
		{
			return;
		}
		dbus_int32_t parent = 0;
		dbus_message_append_args(signal, DBUS_TYPE_UINT32, &g_menuRevision, DBUS_TYPE_INT32, &parent, DBUS_TYPE_INVALID);
		dbus_connection_send(g_connection, signal, nullptr);
		dbus_connection_flush(g_connection);
		dbus_message_unref(signal);
	}

	void tryRegisterToWatcher()
	{
		if (!g_connection || g_serviceName.empty())
		{
			return;
		}

		const char *watcherService = getWatcherServiceName();
		const char *watcherInterface = getWatcherInterfaceName(watcherService);
		if (!watcherService || !watcherInterface)
		{
			return;
		}

		DBusMessage *message = dbus_message_new_method_call(
			watcherService,
			"/StatusNotifierWatcher",
			watcherInterface,
			"RegisterStatusNotifierItem");
		if (!message)
		{
			return;
		}

		const char *serviceName = g_serviceName.c_str();
		dbus_message_append_args(message, DBUS_TYPE_STRING, &serviceName, DBUS_TYPE_INVALID);

		DBusError error;
		dbus_error_init(&error);
		DBusMessage *reply = dbus_connection_send_with_reply_and_block(g_connection, message, 1000, &error);
		dbus_message_unref(message);
		if (reply)
		{
			dbus_message_unref(reply);
			g_registered = true;
		}
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
	}

	DBusMessage *buildUnknownPropertyError(DBusMessage *message, const char *interfaceName, const char *propertyName)
	{
		std::string errorText = std::string("Unknown property ") + interfaceName + "." + propertyName;
		return dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_PROPERTY, errorText.c_str());
	}

	bool appendItemPropertyVariantByName(DBusMessageIter *iter, const char *propertyName)
	{
		if (std::strcmp(propertyName, "Category") == 0)
		{
			const char *value = "ApplicationStatus";
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "s", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "Id") == 0)
		{
			const char *value = "BROTBOX";
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "s", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "Title") == 0)
		{
			const char *value = g_tooltip.c_str();
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "s", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "Status") == 0)
		{
			const char *value = "Active";
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "s", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "WindowId") == 0)
		{
			dbus_uint32_t value = 0;
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "u", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_UINT32, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "IconThemePath") == 0 ||
			std::strcmp(propertyName, "IconName") == 0 ||
			std::strcmp(propertyName, "OverlayIconName") == 0 ||
			std::strcmp(propertyName, "AttentionIconName") == 0 ||
			std::strcmp(propertyName, "AttentionMovieName") == 0)
		{
			const char *value = "";
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "s", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "IconPixmap") == 0 ||
			std::strcmp(propertyName, "OverlayIconPixmap") == 0 ||
			std::strcmp(propertyName, "AttentionIconPixmap") == 0)
		{
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "a(iiay)", &variantIter);
			appendIconPixmapArray(&variantIter, std::strcmp(propertyName, "IconPixmap") == 0 ? g_currentIcon : nullptr);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "ToolTip") == 0)
		{
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "(sa(iiay)ss)", &variantIter);
			DBusMessageIter structIter;
			dbus_message_iter_open_container(&variantIter, DBUS_TYPE_STRUCT, nullptr, &structIter);
			const char *iconName = "";
			dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &iconName);
			appendIconPixmapArray(&structIter, g_currentIcon);
			const char *title = g_tooltip.c_str();
			const char *description = "";
			dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &title);
			dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &description);
			dbus_message_iter_close_container(&variantIter, &structIter);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "ItemIsMenu") == 0)
		{
			dbus_bool_t value = FALSE;
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "b", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_BOOLEAN, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "Menu") == 0)
		{
			const char *value = MENU_OBJECT_PATH;
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "o", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_OBJECT_PATH, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		return false;
	}

	bool appendMenuPropertyVariantByName(DBusMessageIter *iter, const char *propertyName)
	{
		if (std::strcmp(propertyName, "Version") == 0)
		{
			dbus_uint32_t value = 4;
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "u", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_UINT32, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		if (std::strcmp(propertyName, "Status") == 0)
		{
			const char *value = "normal";
			DBusMessageIter variantIter;
			dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, "s", &variantIter);
			dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
			dbus_message_iter_close_container(iter, &variantIter);
			return true;
		}
		return false;
	}

	DBusHandlerResult handleItemMessage(DBusConnection *, DBusMessage *message, void *)
	{
		if (dbus_message_is_method_call(message, DBUS_INTROSPECTABLE_INTERFACE, "Introspect"))
		{
			static const char *introspectionXml =
				"<node>"
				"  <interface name='org.kde.StatusNotifierItem'>"
				"    <property name='Category' type='s' access='read'/>"
				"    <property name='Id' type='s' access='read'/>"
				"    <property name='Title' type='s' access='read'/>"
				"    <property name='Status' type='s' access='read'/>"
				"    <property name='WindowId' type='u' access='read'/>"
				"    <property name='IconThemePath' type='s' access='read'/>"
				"    <property name='IconName' type='s' access='read'/>"
				"    <property name='IconPixmap' type='a(iiay)' access='read'/>"
				"    <property name='OverlayIconName' type='s' access='read'/>"
				"    <property name='OverlayIconPixmap' type='a(iiay)' access='read'/>"
				"    <property name='AttentionIconName' type='s' access='read'/>"
				"    <property name='AttentionIconPixmap' type='a(iiay)' access='read'/>"
				"    <property name='AttentionMovieName' type='s' access='read'/>"
				"    <property name='ToolTip' type='(sa(iiay)ss)' access='read'/>"
				"    <property name='Menu' type='o' access='read'/>"
				"    <property name='ItemIsMenu' type='b' access='read'/>"
				"    <method name='ContextMenu'><arg type='i' direction='in'/><arg type='i' direction='in'/></method>"
				"    <method name='Activate'><arg type='i' direction='in'/><arg type='i' direction='in'/></method>"
				"    <method name='SecondaryActivate'><arg type='i' direction='in'/><arg type='i' direction='in'/></method>"
				"    <method name='Scroll'><arg type='i' direction='in'/><arg type='s' direction='in'/></method>"
				"    <signal name='NewTitle'/>"
				"    <signal name='NewIcon'/>"
				"    <signal name='NewAttentionIcon'/>"
				"    <signal name='NewOverlayIcon'/>"
				"    <signal name='NewToolTip'/>"
				"    <signal name='NewStatus'><arg type='s'/></signal>"
				"  </interface>"
				"  <interface name='org.freedesktop.StatusNotifierItem'>"
				"    <property name='Category' type='s' access='read'/>"
				"    <property name='Id' type='s' access='read'/>"
				"    <property name='Title' type='s' access='read'/>"
				"    <property name='Status' type='s' access='read'/>"
				"    <property name='WindowId' type='u' access='read'/>"
				"    <property name='IconThemePath' type='s' access='read'/>"
				"    <property name='IconName' type='s' access='read'/>"
				"    <property name='IconPixmap' type='a(iiay)' access='read'/>"
				"    <property name='OverlayIconName' type='s' access='read'/>"
				"    <property name='OverlayIconPixmap' type='a(iiay)' access='read'/>"
				"    <property name='AttentionIconName' type='s' access='read'/>"
				"    <property name='AttentionIconPixmap' type='a(iiay)' access='read'/>"
				"    <property name='AttentionMovieName' type='s' access='read'/>"
				"    <property name='ToolTip' type='(sa(iiay)ss)' access='read'/>"
				"    <property name='Menu' type='o' access='read'/>"
				"    <property name='ItemIsMenu' type='b' access='read'/>"
				"    <method name='ContextMenu'><arg type='i' direction='in'/><arg type='i' direction='in'/></method>"
				"    <method name='Activate'><arg type='i' direction='in'/><arg type='i' direction='in'/></method>"
				"    <method name='SecondaryActivate'><arg type='i' direction='in'/><arg type='i' direction='in'/></method>"
				"    <method name='Scroll'><arg type='i' direction='in'/><arg type='s' direction='in'/></method>"
				"    <signal name='NewTitle'/>"
				"    <signal name='NewIcon'/>"
				"    <signal name='NewAttentionIcon'/>"
				"    <signal name='NewOverlayIcon'/>"
				"    <signal name='NewToolTip'/>"
				"    <signal name='NewStatus'><arg type='s'/></signal>"
				"  </interface>"
				"  <interface name='org.freedesktop.DBus.Properties'>"
				"    <method name='Get'><arg type='s' direction='in'/><arg type='s' direction='in'/><arg type='v' direction='out'/></method>"
				"    <method name='GetAll'><arg type='s' direction='in'/><arg type='a{sv}' direction='out'/></method>"
				"  </interface>"
				"</node>";

			DBusMessage *reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_STRING, &introspectionXml, DBUS_TYPE_INVALID);
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUS_PROPERTIES_INTERFACE, "Get"))
		{
			const char *interfaceName = nullptr;
			const char *propertyName = nullptr;
			DBusError error;
			dbus_error_init(&error);
			if (!dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &interfaceName, DBUS_TYPE_STRING, &propertyName, DBUS_TYPE_INVALID))
			{
				DBusMessage *reply = dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, error.message ? error.message : "Invalid arguments");
				dbus_error_free(&error);
				dbus_connection_send(g_connection, reply, nullptr);
				dbus_message_unref(reply);
				return DBUS_HANDLER_RESULT_HANDLED;
			}

			DBusMessage *reply = dbus_message_new_method_return(message);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			if (!isItemInterface(interfaceName) || !appendItemPropertyVariantByName(&iter, propertyName))
			{
				dbus_message_unref(reply);
				reply = buildUnknownPropertyError(message, interfaceName, propertyName);
			}
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUS_PROPERTIES_INTERFACE, "GetAll"))
		{
			const char *interfaceName = nullptr;
			DBusError error;
			dbus_error_init(&error);
			if (!dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &interfaceName, DBUS_TYPE_INVALID))
			{
				DBusMessage *reply = dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, error.message ? error.message : "Invalid arguments");
				dbus_error_free(&error);
				dbus_connection_send(g_connection, reply, nullptr);
				dbus_message_unref(reply);
				return DBUS_HANDLER_RESULT_HANDLED;
			}
			if (!isItemInterface(interfaceName))
			{
				DBusMessage *reply = dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_INTERFACE, "Unknown interface");
				dbus_connection_send(g_connection, reply, nullptr);
				dbus_message_unref(reply);
				return DBUS_HANDLER_RESULT_HANDLED;
			}

			DBusMessage *reply = dbus_message_new_method_return(message);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			DBusMessageIter dictIter;
			dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dictIter);
			appendItemProperties(&dictIter);
			dbus_message_iter_close_container(&iter, &dictIter);
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (isItemInterface(dbus_message_get_interface(message)))
		{
			if (dbus_message_is_method_call(message, dbus_message_get_interface(message), "Activate") ||
				dbus_message_is_method_call(message, dbus_message_get_interface(message), "SecondaryActivate"))
			{
				if (g_game)
				{
					g_game->requestShowWindow();
				}
				DBusMessage *reply = dbus_message_new_method_return(message);
				dbus_connection_send(g_connection, reply, nullptr);
				dbus_message_unref(reply);
				return DBUS_HANDLER_RESULT_HANDLED;
			}
			if (dbus_message_is_method_call(message, dbus_message_get_interface(message), "ContextMenu") ||
				dbus_message_is_method_call(message, dbus_message_get_interface(message), "Scroll"))
			{
				DBusMessage *reply = dbus_message_new_method_return(message);
				dbus_connection_send(g_connection, reply, nullptr);
				dbus_message_unref(reply);
				return DBUS_HANDLER_RESULT_HANDLED;
			}
		}

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	DBusHandlerResult handleMenuMessage(DBusConnection *, DBusMessage *message, void *)
	{
		if (dbus_message_is_method_call(message, DBUS_INTROSPECTABLE_INTERFACE, "Introspect"))
		{
			static const char *introspectionXml =
				"<node>"
				"  <interface name='com.canonical.dbusmenu'>"
				"    <property name='Version' type='u' access='read'/>"
				"    <property name='Status' type='s' access='read'/>"
				"    <method name='GetLayout'><arg type='i' direction='in'/><arg type='i' direction='in'/><arg type='as' direction='in'/><arg type='u' direction='out'/><arg type='(ia{sv}av)' direction='out'/></method>"
				"    <method name='GetGroupProperties'><arg type='ai' direction='in'/><arg type='as' direction='in'/><arg type='a(ia{sv})' direction='out'/></method>"
				"    <method name='GetProperty'><arg type='i' direction='in'/><arg type='s' direction='in'/><arg type='v' direction='out'/></method>"
				"    <method name='Event'><arg type='i' direction='in'/><arg type='s' direction='in'/><arg type='v' direction='in'/><arg type='u' direction='in'/></method>"
				"    <method name='AboutToShow'><arg type='i' direction='in'/><arg type='b' direction='out'/></method>"
				"    <signal name='LayoutUpdated'><arg type='u'/><arg type='i'/></signal>"
				"    <signal name='ItemsPropertiesUpdated'><arg type='a(ia{sv})'/><arg type='a(ias)'/></signal>"
				"    <signal name='ItemActivationRequested'><arg type='i'/><arg type='u'/></signal>"
				"  </interface>"
				"  <interface name='org.freedesktop.DBus.Properties'>"
				"    <method name='Get'><arg type='s' direction='in'/><arg type='s' direction='in'/><arg type='v' direction='out'/></method>"
				"    <method name='GetAll'><arg type='s' direction='in'/><arg type='a{sv}' direction='out'/></method>"
				"  </interface>"
				"</node>";
			DBusMessage *reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_STRING, &introspectionXml, DBUS_TYPE_INVALID);
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUS_PROPERTIES_INTERFACE, "Get"))
		{
			const char *interfaceName = nullptr;
			const char *propertyName = nullptr;
			DBusError error;
			dbus_error_init(&error);
			if (!dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &interfaceName, DBUS_TYPE_STRING, &propertyName, DBUS_TYPE_INVALID))
			{
				DBusMessage *reply = dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, error.message ? error.message : "Invalid arguments");
				dbus_error_free(&error);
				dbus_connection_send(g_connection, reply, nullptr);
				dbus_message_unref(reply);
				return DBUS_HANDLER_RESULT_HANDLED;
			}

			DBusMessage *reply = dbus_message_new_method_return(message);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			if (std::strcmp(interfaceName, DBUSMENU_INTERFACE) != 0 || !appendMenuPropertyVariantByName(&iter, propertyName))
			{
				dbus_message_unref(reply);
				reply = buildUnknownPropertyError(message, interfaceName, propertyName);
			}
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUS_PROPERTIES_INTERFACE, "GetAll"))
		{
			const char *interfaceName = nullptr;
			DBusError error;
			dbus_error_init(&error);
			if (!dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &interfaceName, DBUS_TYPE_INVALID))
			{
				DBusMessage *reply = dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, error.message ? error.message : "Invalid arguments");
				dbus_error_free(&error);
				dbus_connection_send(g_connection, reply, nullptr);
				dbus_message_unref(reply);
				return DBUS_HANDLER_RESULT_HANDLED;
			}
			if (std::strcmp(interfaceName, DBUSMENU_INTERFACE) != 0)
			{
				DBusMessage *reply = dbus_message_new_error(message, DBUS_ERROR_UNKNOWN_INTERFACE, "Unknown interface");
				dbus_connection_send(g_connection, reply, nullptr);
				dbus_message_unref(reply);
				return DBUS_HANDLER_RESULT_HANDLED;
			}

			DBusMessage *reply = dbus_message_new_method_return(message);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			DBusMessageIter dictIter;
			dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dictIter);
			appendMenuProperties(&dictIter);
			dbus_message_iter_close_container(&iter, &dictIter);
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUSMENU_INTERFACE, "GetLayout"))
		{
			dbus_int32_t parentId = 0;
			dbus_int32_t recursionDepth = -1;
			DBusMessageIter argsIter;
			dbus_message_iter_init(message, &argsIter);
			if (dbus_message_iter_get_arg_type(&argsIter) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&argsIter, &parentId);
				dbus_message_iter_next(&argsIter);
			}
			if (dbus_message_iter_get_arg_type(&argsIter) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&argsIter, &recursionDepth);
			}

			DBusMessage *reply = dbus_message_new_method_return(message);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &g_menuRevision);
			if (parentId > 0 && static_cast<size_t>(parentId) <= g_menuItems.size())
			{
				appendMenuLayoutItem(&iter, parentId, &g_menuItems[static_cast<size_t>(parentId - 1)], recursionDepth != 0);
			}
			else
			{
				appendMenuLayoutItem(&iter, 0, nullptr, recursionDepth != 0);
			}
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUSMENU_INTERFACE, "GetGroupProperties"))
		{
			DBusMessage *reply = dbus_message_new_method_return(message);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			DBusMessageIter arrayIter;
			dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "(ia{sv})", &arrayIter);
			for (size_t i = 0; i < g_menuItems.size(); ++i)
			{
				DBusMessageIter structIter;
				dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_STRUCT, nullptr, &structIter);
				dbus_int32_t id = static_cast<dbus_int32_t>(i + 1);
				dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT32, &id);
				DBusMessageIter dictIter;
				dbus_message_iter_open_container(&structIter, DBUS_TYPE_ARRAY, "{sv}", &dictIter);
				appendSingleMenuItemProperties(&dictIter, g_menuItems[i]);
				dbus_message_iter_close_container(&structIter, &dictIter);
				dbus_message_iter_close_container(&arrayIter, &structIter);
			}
			dbus_message_iter_close_container(&iter, &arrayIter);
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUSMENU_INTERFACE, "GetProperty"))
		{
			dbus_int32_t id = 0;
			const char *propertyName = nullptr;
			DBusError error;
			dbus_error_init(&error);
			if (!dbus_message_get_args(message, &error, DBUS_TYPE_INT32, &id, DBUS_TYPE_STRING, &propertyName, DBUS_TYPE_INVALID))
			{
				DBusMessage *reply = dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, error.message ? error.message : "Invalid arguments");
				dbus_error_free(&error);
				dbus_connection_send(g_connection, reply, nullptr);
				dbus_message_unref(reply);
				return DBUS_HANDLER_RESULT_HANDLED;
			}

			DBusMessage *reply = dbus_message_new_method_return(message);
			DBusMessageIter iter;
			dbus_message_iter_init_append(reply, &iter);
			bool ok = false;
			if (id > 0 && static_cast<size_t>(id) <= g_menuItems.size())
			{
				const MenuItem &item = g_menuItems[static_cast<size_t>(id - 1)];
				if (std::strcmp(propertyName, "label") == 0)
				{
					DBusMessageIter variantIter;
					const char *value = item.title.c_str();
					dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "s", &variantIter);
					dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
					dbus_message_iter_close_container(&iter, &variantIter);
					ok = true;
				}
				else if (std::strcmp(propertyName, "enabled") == 0 || std::strcmp(propertyName, "visible") == 0)
				{
					DBusMessageIter variantIter;
					dbus_bool_t value = TRUE;
					dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "b", &variantIter);
					dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_BOOLEAN, &value);
					dbus_message_iter_close_container(&iter, &variantIter);
					ok = true;
				}
				else if (std::strcmp(propertyName, "type") == 0)
				{
					DBusMessageIter variantIter;
					const char *value = "standard";
					dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "s", &variantIter);
					dbus_message_iter_append_basic(&variantIter, DBUS_TYPE_STRING, &value);
					dbus_message_iter_close_container(&iter, &variantIter);
					ok = true;
				}
			}
			if (!ok)
			{
				dbus_message_unref(reply);
				reply = buildUnknownPropertyError(message, DBUSMENU_INTERFACE, propertyName);
			}
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUSMENU_INTERFACE, "Event"))
		{
			dbus_int32_t id = 0;
			const char *eventId = nullptr;
			DBusMessageIter argsIter;
			if (dbus_message_iter_init(message, &argsIter) && dbus_message_iter_get_arg_type(&argsIter) == DBUS_TYPE_INT32)
			{
				dbus_message_iter_get_basic(&argsIter, &id);
				dbus_message_iter_next(&argsIter);
			}
			if (dbus_message_iter_get_arg_type(&argsIter) == DBUS_TYPE_STRING)
			{
				dbus_message_iter_get_basic(&argsIter, &eventId);
			}
			if (eventId && std::strcmp(eventId, "clicked") == 0 && id > 0 && static_cast<size_t>(id) <= g_menuItems.size())
			{
				g_menuItems[static_cast<size_t>(id - 1)].callback();
			}
			DBusMessage *reply = dbus_message_new_method_return(message);
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		if (dbus_message_is_method_call(message, DBUSMENU_INTERFACE, "AboutToShow"))
		{
			dbus_bool_t needUpdate = FALSE;
			DBusMessage *reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &needUpdate, DBUS_TYPE_INVALID);
			dbus_connection_send(g_connection, reply, nullptr);
			dbus_message_unref(reply);
			return DBUS_HANDLER_RESULT_HANDLED;
		}

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	DBusObjectPathVTable itemVTable = {
		nullptr,
		handleItemMessage,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
	};

	DBusObjectPathVTable menuVTable = {
		nullptr,
		handleMenuMessage,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
	};
}

bbe::TrayIcon::IconHandle bbe::TrayIcon::createIcon(int width, int height, const uint32_t *argbPixels, size_t pixelCount)
{
	auto *icon = new LinuxIconHandle();
	icon->width = width;
	icon->height = height;
	icon->argb32NetworkOrder.reserve(pixelCount * 4);
	for (size_t i = 0; i < pixelCount; ++i)
	{
		const uint32_t pixel = argbPixels[i];
		icon->argb32NetworkOrder.push_back(static_cast<unsigned char>((pixel >> 24) & 0xFF));
		icon->argb32NetworkOrder.push_back(static_cast<unsigned char>((pixel >> 16) & 0xFF));
		icon->argb32NetworkOrder.push_back(static_cast<unsigned char>((pixel >> 8) & 0xFF));
		icon->argb32NetworkOrder.push_back(static_cast<unsigned char>(pixel & 0xFF));
	}
	return icon;
}

void bbe::TrayIcon::init(bbe::Game *game, const char *tooltip, IconHandle icon)
{
	g_game = game;
	g_tooltip = tooltip ? tooltip : "";
	g_currentIcon = icon;

	if (g_connection)
	{
		setIcon(icon);
		return;
	}

	DBusError error;
	dbus_error_init(&error);
	g_connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
	if (!g_connection)
	{
		if (dbus_error_is_set(&error))
		{
			dbus_error_free(&error);
		}
		return;
	}
	if (dbus_error_is_set(&error))
	{
		dbus_error_free(&error);
	}

	dbus_connection_set_exit_on_disconnect(g_connection, FALSE);
	g_serviceName = "org.freedesktop.StatusNotifierItem-" + std::to_string(::getpid()) + "-1";
	const int requestResult = dbus_bus_request_name(g_connection, g_serviceName.c_str(), DBUS_NAME_FLAG_DO_NOT_QUEUE, &error);
	if (dbus_error_is_set(&error))
	{
		dbus_error_free(&error);
	}
	if (requestResult != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		return;
	}

	dbus_connection_register_object_path(g_connection, ITEM_OBJECT_PATH, &itemVTable, nullptr);
	dbus_connection_register_object_path(g_connection, MENU_OBJECT_PATH, &menuVTable, nullptr);
	tryRegisterToWatcher();
	emitSimpleSignal(ITEM_OBJECT_PATH, KDE_ITEM_INTERFACE, "NewToolTip");
	setIcon(icon);
}

void bbe::TrayIcon::addPopupItem(const char *title, std::function<void()> callback)
{
	g_menuItems.push_back(MenuItem{ title ? title : "", std::move(callback) });
	++g_menuRevision;
	emitLayoutUpdatedSignal();
}

bool bbe::TrayIcon::isVisible()
{
	return getWatcherServiceName() != nullptr;
}

void bbe::TrayIcon::setIcon(IconHandle icon)
{
	static IconHandle previousIcon = nullptr;
	if (previousIcon == icon)
	{
		return;
	}
	previousIcon = icon;
	g_currentIcon = icon;
	emitSimpleSignal(ITEM_OBJECT_PATH, KDE_ITEM_INTERFACE, "NewIcon");
	emitSimpleSignal(ITEM_OBJECT_PATH, FREEDESKTOP_ITEM_INTERFACE, "NewIcon");
	emitSimpleSignal(ITEM_OBJECT_PATH, KDE_ITEM_INTERFACE, "NewToolTip");
	emitSimpleSignal(ITEM_OBJECT_PATH, FREEDESKTOP_ITEM_INTERFACE, "NewToolTip");
}

void bbe::TrayIcon::update()
{
	if (!g_connection)
	{
		return;
	}
	if (!g_registered)
	{
		tryRegisterToWatcher();
	}
	dbus_connection_read_write(g_connection, 0);
	while (dbus_connection_dispatch(g_connection) == DBUS_DISPATCH_DATA_REMAINS)
	{
	}
}

#else

void bbe::TrayIcon::init(bbe::Game *, const char *, IconHandle)
{
}

void bbe::TrayIcon::addPopupItem(const char *, std::function<void()>)
{
}

bool bbe::TrayIcon::isVisible()
{
	return false;
}

void bbe::TrayIcon::setIcon(IconHandle)
{
}

void bbe::TrayIcon::update()
{
}

#endif
