#include "BBE/WaylandClipboard.h"

#if defined(__linux__) && defined(BBE_USE_WAYLAND_CLIPBOARD)

#include "BBE/Window.h"

#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <mutex>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace
{
	struct OfferState
	{
		wl_data_offer* offer = nullptr;
		std::vector<std::string> mimeTypes;
	};

	struct ClipboardState
	{
		std::mutex mutex;
		bool initialized = false;
		bool supported = false;
		bool cleanupRegistered = false;

		wl_display* display = nullptr;
		wl_registry* registry = nullptr;
		wl_seat* seat = nullptr;
		wl_pointer* pointer = nullptr;
		wl_keyboard* keyboard = nullptr;
		wl_data_device_manager* dataDeviceManager = nullptr;
		wl_data_device* dataDevice = nullptr;

		OfferState* selectionOffer = nullptr;
		OfferState* dragOffer = nullptr;
		wl_data_source* selectionSource = nullptr;
		std::vector<bbe::byte> selectionData;
		uint32_t lastInputSerial = 0;
	};

	ClipboardState& clipboardState()
	{
		static ClipboardState state;
		return state;
	}

	void destroyOffer(OfferState*& offerState)
	{
		if (offerState == nullptr)
		{
			return;
		}

		if (offerState->offer != nullptr)
		{
			wl_data_offer_destroy(offerState->offer);
		}

		delete offerState;
		offerState = nullptr;
	}

	void destroySelectionSource(ClipboardState& state)
	{
		if (state.selectionSource == nullptr)
		{
			return;
		}

		wl_data_source_destroy(state.selectionSource);
		state.selectionSource = nullptr;
		state.selectionData.clear();
	}

	void closeFdIfValid(int fd)
	{
		if (fd >= 0)
		{
			::close(fd);
		}
	}

	bool readAllFromFd(int fd, std::vector<bbe::byte>& outBytes)
	{
		bbe::byte buffer[4096];
		while (true)
		{
			const ssize_t bytesRead = ::read(fd, buffer, sizeof(buffer));
			if (bytesRead == 0)
			{
				return true;
			}
			if (bytesRead < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}

				return false;
			}

			outBytes.insert(outBytes.end(), buffer, buffer + bytesRead);
		}
	}

	bool writeAllToFd(int fd, const bbe::byte* data, size_t length)
	{
		size_t written = 0;
		while (written < length)
		{
			const ssize_t currentWrite = ::write(fd, data + written, length - written);
			if (currentWrite < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}

				return false;
			}

			written += static_cast<size_t>(currentWrite);
		}

		return true;
	}

	std::string getPreferredMimeType(const std::vector<std::string>& mimeTypes)
	{
		static constexpr const char* preferredMimeTypes[] =
		{
			"image/png",
			"image/bmp",
			"image/x-bmp",
			"image/jpeg",
			"image/jpg",
			"image/tga",
			"image/x-tga",
		};

		for (const char* preferredMimeType : preferredMimeTypes)
		{
			if (std::find(mimeTypes.begin(), mimeTypes.end(), preferredMimeType) != mimeTypes.end())
			{
				return preferredMimeType;
			}
		}

		return "";
	}

	void registryHandleGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		if (std::strcmp(interface, wl_seat_interface.name) == 0 && state.seat == nullptr)
		{
			const uint32_t bindVersion = std::min(version, 5u);
			state.seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, bindVersion));
		}
		else if (std::strcmp(interface, wl_data_device_manager_interface.name) == 0 && state.dataDeviceManager == nullptr)
		{
			const uint32_t bindVersion = std::min(version, 3u);
			state.dataDeviceManager = static_cast<wl_data_device_manager*>(wl_registry_bind(registry, name, &wl_data_device_manager_interface, bindVersion));
		}
	}

	void registryHandleGlobalRemove(void*, wl_registry*, uint32_t)
	{
	}

	void pointerHandleEnter(void* data, wl_pointer*, uint32_t serial, wl_surface*, wl_fixed_t, wl_fixed_t)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		state.lastInputSerial = serial;
	}

	void pointerHandleLeave(void*, wl_pointer*, uint32_t, wl_surface*)
	{
	}

	void pointerHandleMotion(void*, wl_pointer*, uint32_t, wl_fixed_t, wl_fixed_t)
	{
	}

	void pointerHandleButton(void* data, wl_pointer*, uint32_t serial, uint32_t, uint32_t, uint32_t)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		state.lastInputSerial = serial;
	}

	void pointerHandleAxis(void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t)
	{
	}

	void pointerHandleFrame(void*, wl_pointer*)
	{
	}

	void pointerHandleAxisSource(void*, wl_pointer*, uint32_t)
	{
	}

	void pointerHandleAxisStop(void*, wl_pointer*, uint32_t, uint32_t)
	{
	}

	void pointerHandleAxisDiscrete(void*, wl_pointer*, uint32_t, int32_t)
	{
	}

	void pointerHandleAxisValue120(void*, wl_pointer*, uint32_t, int32_t)
	{
	}

	void pointerHandleAxisRelativeDirection(void*, wl_pointer*, uint32_t, uint32_t)
	{
	}

	void keyboardHandleKeymap(void*, wl_keyboard*, uint32_t, int32_t fd, uint32_t)
	{
		closeFdIfValid(fd);
	}

	void keyboardHandleEnter(void* data, wl_keyboard*, uint32_t serial, wl_surface*, wl_array*)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		state.lastInputSerial = serial;
	}

	void keyboardHandleLeave(void*, wl_keyboard*, uint32_t, wl_surface*)
	{
	}

	void keyboardHandleKey(void* data, wl_keyboard*, uint32_t serial, uint32_t, uint32_t, uint32_t)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		state.lastInputSerial = serial;
	}

	void keyboardHandleModifiers(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)
	{
	}

	void keyboardHandleRepeatInfo(void*, wl_keyboard*, int32_t, int32_t)
	{
	}

	void seatHandleCapabilities(void* data, wl_seat* seat, uint32_t capabilities)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);

		const bool hasPointer = (capabilities & WL_SEAT_CAPABILITY_POINTER) != 0;
		if (hasPointer && state.pointer == nullptr)
		{
			state.pointer = wl_seat_get_pointer(seat);
			static constexpr wl_pointer_listener pointerListener =
			{
				pointerHandleEnter,
				pointerHandleLeave,
				pointerHandleMotion,
				pointerHandleButton,
				pointerHandleAxis,
				pointerHandleFrame,
				pointerHandleAxisSource,
				pointerHandleAxisStop,
				pointerHandleAxisDiscrete,
				pointerHandleAxisValue120,
				pointerHandleAxisRelativeDirection,
			};
			wl_pointer_add_listener(state.pointer, &pointerListener, &state);
		}
		else if (!hasPointer && state.pointer != nullptr)
		{
			if (wl_pointer_get_version(state.pointer) >= WL_POINTER_RELEASE_SINCE_VERSION)
			{
				wl_pointer_release(state.pointer);
			}
			else
			{
				wl_pointer_destroy(state.pointer);
			}
			state.pointer = nullptr;
		}

		const bool hasKeyboard = (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) != 0;
		if (hasKeyboard && state.keyboard == nullptr)
		{
			state.keyboard = wl_seat_get_keyboard(seat);
			static constexpr wl_keyboard_listener keyboardListener =
			{
				keyboardHandleKeymap,
				keyboardHandleEnter,
				keyboardHandleLeave,
				keyboardHandleKey,
				keyboardHandleModifiers,
				keyboardHandleRepeatInfo,
			};
			wl_keyboard_add_listener(state.keyboard, &keyboardListener, &state);
		}
		else if (!hasKeyboard && state.keyboard != nullptr)
		{
			if (wl_keyboard_get_version(state.keyboard) >= WL_KEYBOARD_RELEASE_SINCE_VERSION)
			{
				wl_keyboard_release(state.keyboard);
			}
			else
			{
				wl_keyboard_destroy(state.keyboard);
			}
			state.keyboard = nullptr;
		}
	}

	void seatHandleName(void*, wl_seat*, const char*)
	{
	}

	void dataOfferHandleOffer(void* data, wl_data_offer*, const char* mimeType)
	{
		OfferState& offerState = *reinterpret_cast<OfferState*>(data);
		offerState.mimeTypes.emplace_back(mimeType);
	}

	void dataOfferHandleSourceActions(void*, wl_data_offer*, uint32_t)
	{
	}

	void dataOfferHandleAction(void*, wl_data_offer*, uint32_t)
	{
	}

	void dataDeviceHandleDataOffer(void*, wl_data_device*, wl_data_offer* id)
	{
		OfferState* offerState = new OfferState();
		offerState->offer = id;
		static constexpr wl_data_offer_listener dataOfferListener =
		{
			dataOfferHandleOffer,
			dataOfferHandleSourceActions,
			dataOfferHandleAction,
		};
		wl_data_offer_set_user_data(id, offerState);
		wl_data_offer_add_listener(id, &dataOfferListener, offerState);
	}

	void dataDeviceHandleEnter(void* data, wl_data_device*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t, wl_data_offer* id)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		destroyOffer(state.dragOffer);
		state.dragOffer = id != nullptr ? reinterpret_cast<OfferState*>(wl_data_offer_get_user_data(id)) : nullptr;
	}

	void dataDeviceHandleLeave(void* data, wl_data_device*)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		destroyOffer(state.dragOffer);
	}

	void dataDeviceHandleMotion(void*, wl_data_device*, uint32_t, wl_fixed_t, wl_fixed_t)
	{
	}

	void dataDeviceHandleDrop(void*, wl_data_device*)
	{
	}

	void dataDeviceHandleSelection(void* data, wl_data_device*, wl_data_offer* id)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		destroyOffer(state.selectionOffer);
		state.selectionOffer = id != nullptr ? reinterpret_cast<OfferState*>(wl_data_offer_get_user_data(id)) : nullptr;
	}

	void dataSourceHandleTarget(void*, wl_data_source*, const char*)
	{
	}

	void dataSourceHandleSend(void* data, wl_data_source*, const char* mimeType, int32_t fd)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		if (std::strcmp(mimeType, "image/png") == 0 && !state.selectionData.empty())
		{
			writeAllToFd(fd, state.selectionData.data(), state.selectionData.size());
		}
		closeFdIfValid(fd);
	}

	void dataSourceHandleCancelled(void* data, wl_data_source* source)
	{
		ClipboardState& state = *reinterpret_cast<ClipboardState*>(data);
		if (state.selectionSource == source)
		{
			destroySelectionSource(state);
		}
	}

	void dataSourceHandleDnDDropPerformed(void*, wl_data_source*)
	{
	}

	void dataSourceHandleDnDFinished(void*, wl_data_source*)
	{
	}

	void dataSourceHandleAction(void*, wl_data_source*, uint32_t)
	{
	}

	void cleanupLocked();

	void cleanup()
	{
		ClipboardState& state = clipboardState();
		std::lock_guard lock(state.mutex);
		cleanupLocked();
	}

	void cleanupLocked()
	{
		ClipboardState& state = clipboardState();
		destroySelectionSource(state);
		destroyOffer(state.selectionOffer);
		destroyOffer(state.dragOffer);

		if (state.dataDevice != nullptr)
		{
			if (wl_data_device_get_version(state.dataDevice) >= WL_DATA_DEVICE_RELEASE_SINCE_VERSION)
			{
				wl_data_device_release(state.dataDevice);
			}
			else
			{
				wl_data_device_destroy(state.dataDevice);
			}
			state.dataDevice = nullptr;
		}

		if (state.pointer != nullptr)
		{
			if (wl_pointer_get_version(state.pointer) >= WL_POINTER_RELEASE_SINCE_VERSION)
			{
				wl_pointer_release(state.pointer);
			}
			else
			{
				wl_pointer_destroy(state.pointer);
			}
			state.pointer = nullptr;
		}

		if (state.keyboard != nullptr)
		{
			if (wl_keyboard_get_version(state.keyboard) >= WL_KEYBOARD_RELEASE_SINCE_VERSION)
			{
				wl_keyboard_release(state.keyboard);
			}
			else
			{
				wl_keyboard_destroy(state.keyboard);
			}
			state.keyboard = nullptr;
		}

		if (state.seat != nullptr)
		{
			if (wl_seat_get_version(state.seat) >= WL_SEAT_RELEASE_SINCE_VERSION)
			{
				wl_seat_release(state.seat);
			}
			else
			{
				wl_seat_destroy(state.seat);
			}
			state.seat = nullptr;
		}

		if (state.dataDeviceManager != nullptr)
		{
			wl_data_device_manager_destroy(state.dataDeviceManager);
			state.dataDeviceManager = nullptr;
		}

		if (state.registry != nullptr)
		{
			wl_registry_destroy(state.registry);
			state.registry = nullptr;
		}

		state.display = nullptr;
		state.lastInputSerial = 0;
		state.initialized = false;
		state.supported = false;
		state.cleanupRegistered = false;
	}

	bool ensureInitializedLocked()
	{
		ClipboardState& state = clipboardState();
		if (state.initialized)
		{
			return state.supported;
		}

		if (bbe::Window::INTERNAL_firstInstance == nullptr)
		{
			return false;
		}

		if (glfwGetPlatform() != GLFW_PLATFORM_WAYLAND)
		{
			return false;
		}

		GLFWwindow* window = bbe::Window::INTERNAL_firstInstance->getRaw();
		if (window == nullptr)
		{
			return false;
		}

		state.display = glfwGetWaylandDisplay();
		if (state.display == nullptr)
		{
			return false;
		}

		state.registry = wl_display_get_registry(state.display);
		if (state.registry == nullptr)
		{
			return false;
		}

		static constexpr wl_registry_listener registryListener =
		{
			registryHandleGlobal,
			registryHandleGlobalRemove,
		};
		wl_registry_add_listener(state.registry, &registryListener, &state);

		wl_display_roundtrip(state.display);

		if (state.seat == nullptr || state.dataDeviceManager == nullptr)
		{
			cleanupLocked();
			return false;
		}

		static constexpr wl_seat_listener seatListener =
		{
			seatHandleCapabilities,
			seatHandleName,
		};
		wl_seat_add_listener(state.seat, &seatListener, &state);
		wl_display_roundtrip(state.display);

		state.dataDevice = wl_data_device_manager_get_data_device(state.dataDeviceManager, state.seat);
		if (state.dataDevice == nullptr)
		{
			cleanupLocked();
			return false;
		}

		static constexpr wl_data_device_listener dataDeviceListener =
		{
			dataDeviceHandleDataOffer,
			dataDeviceHandleEnter,
			dataDeviceHandleLeave,
			dataDeviceHandleMotion,
			dataDeviceHandleDrop,
			dataDeviceHandleSelection,
		};
		wl_data_device_add_listener(state.dataDevice, &dataDeviceListener, &state);
		wl_display_roundtrip(state.display);

		if (!state.cleanupRegistered)
		{
			bbe::Window::INTERNAL_firstInstance->registerCloseListener(cleanup);
			state.cleanupRegistered = true;
		}

		state.initialized = true;
		state.supported = true;
		return true;
	}

	bool ensureInitialized()
	{
		ClipboardState& state = clipboardState();
		std::lock_guard lock(state.mutex);
		return ensureInitializedLocked();
	}

	void dispatchPendingLocked()
	{
		ClipboardState& state = clipboardState();
		if (!ensureInitializedLocked())
		{
			return;
		}

		wl_display_dispatch_pending(state.display);
		wl_display_flush(state.display);
	}
}

bool bbe::INTERNAL::waylandClipboard::isSupported()
{
	return ensureInitialized();
}

bool bbe::INTERNAL::waylandClipboard::isImageInClipboard()
{
	ClipboardState& state = clipboardState();
	std::lock_guard lock(state.mutex);
	dispatchPendingLocked();
	if (state.selectionSource != nullptr && !state.selectionData.empty())
	{
		return true;
	}

	return state.selectionOffer != nullptr && !getPreferredMimeType(state.selectionOffer->mimeTypes).empty();
}

bbe::ByteBuffer bbe::INTERNAL::waylandClipboard::getClipboardImageData()
{
	ClipboardState& state = clipboardState();
	std::lock_guard lock(state.mutex);
	dispatchPendingLocked();
	if (state.selectionSource != nullptr && !state.selectionData.empty())
	{
		return bbe::ByteBuffer(state.selectionData.data(), state.selectionData.size());
	}

	if (state.selectionOffer == nullptr)
	{
		return {};
	}

	const std::string mimeType = getPreferredMimeType(state.selectionOffer->mimeTypes);
	if (mimeType.empty())
	{
		return {};
	}

	int pipeFds[2] = { -1, -1 };
	if (::pipe(pipeFds) != 0)
	{
		return {};
	}

	wl_data_offer_receive(state.selectionOffer->offer, mimeType.c_str(), pipeFds[1]);
	closeFdIfValid(pipeFds[1]);
	pipeFds[1] = -1;

	wl_display_flush(state.display);

	std::vector<bbe::byte> data;
	const bool readSuccess = readAllFromFd(pipeFds[0], data);
	closeFdIfValid(pipeFds[0]);

	if (!readSuccess || data.empty())
	{
		return {};
	}

	return bbe::ByteBuffer(data.data(), data.size());
}

bool bbe::INTERNAL::waylandClipboard::setClipboardImageData(const bbe::byte* data, size_t length, const char* mimeType)
{
	ClipboardState& state = clipboardState();
	std::lock_guard lock(state.mutex);
	if (!ensureInitializedLocked() || state.lastInputSerial == 0)
	{
		return false;
	}

	destroySelectionSource(state);

	state.selectionData.assign(data, data + length);
	state.selectionSource = wl_data_device_manager_create_data_source(state.dataDeviceManager);
	if (state.selectionSource == nullptr)
	{
		state.selectionData.clear();
		return false;
	}

	static constexpr wl_data_source_listener dataSourceListener =
	{
		dataSourceHandleTarget,
		dataSourceHandleSend,
		dataSourceHandleCancelled,
		dataSourceHandleDnDDropPerformed,
		dataSourceHandleDnDFinished,
		dataSourceHandleAction,
	};
	wl_data_source_add_listener(state.selectionSource, &dataSourceListener, &state);
	wl_data_source_offer(state.selectionSource, mimeType);
	wl_data_device_set_selection(state.dataDevice, state.selectionSource, state.lastInputSerial);
	wl_display_flush(state.display);
	return true;
}

#else

bool bbe::INTERNAL::waylandClipboard::isSupported()
{
	return false;
}

bool bbe::INTERNAL::waylandClipboard::isImageInClipboard()
{
	return false;
}

bbe::ByteBuffer bbe::INTERNAL::waylandClipboard::getClipboardImageData()
{
	return {};
}

bool bbe::INTERNAL::waylandClipboard::setClipboardImageData(const bbe::byte*, size_t, const char*)
{
	return false;
}

#endif
