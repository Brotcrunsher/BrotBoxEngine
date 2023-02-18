#pragma once

#include "../BBE/Array.h"
#include "../BBE/Async.h"
#include "../BBE/DynamicArray.h"
#include "../BBE/Hash.h"
#include "../BBE/HashMap.h"
#include "../BBE/List.h"
#include "../BBE/Span.h"
#include "../BBE/Grid.h"
#include "../BBE/RingArray.h"
#include "../BBE/Stack.h"

#include "../BBE/ExceptionHelper.h"
#include "../BBE/Exceptions.h"

#include "../BBE/SimpleFile.h"
#include "../BBE/SimpleUrlRequest.h"

#include "../BBE/EngineSettings.h"
#include "../BBE/Utf8Helpers.h"
#include "../BBE/Utf8Iterator.h"
#include "../BBE/String.h"

#include "../BBE/Color.h"
#include "../BBE/PixelObserver.h"
#include "../BBE/CursorMode.h"
#include "../BBE/Game.h"
#include "../BBE/LightFalloffMode.h"
#include "../BBE/PointLight.h"
#include "../BBE/PrimitiveBrush2D.h"
#include "../BBE/PrimitiveBrush3D.h"
#include "../BBE/VertexWithNormal.h"
#include "../BBE/Window.h"

#include "imgui.h"

#ifdef BBE_RENDERER_VULKAN
#include "../BBE/Vulkan/VWDepthImage.h"

#include "../BBE/Vulkan/VulkanBuffer.h"
#include "../BBE/Vulkan/VulkanCommandPool.h"
#include "../BBE/Vulkan/VulkanDescriptorPool.h"
#include "../BBE/Vulkan/VulkanDevice.h"
#include "../BBE/Vulkan/VulkanFence.h"
#include "../BBE/Vulkan/VulkanHelper.h"
#include "../BBE/Vulkan/VulkanInstance.h"
#include "../BBE/Vulkan/VulkanManager.h"
#include "../BBE/Vulkan/VulkanPhysicalDevices.h"
#include "../BBE/Vulkan/VulkanPipeline.h"
#include "../BBE/Vulkan/VulkanRenderPass.h"
#include "../BBE/Vulkan/VulkanSemaphore.h"
#include "../BBE/Vulkan/VulkanShader.h"
#include "../BBE/Vulkan/VulkanSurface.h"
#include "../BBE/Vulkan/VulkanSwapchain.h"
#endif

#include "../BBE/CameraControlNoClip.h"
#include "../BBE/Keyboard.h"
#include "../BBE/KeyboardKeys.h"
#include "../BBE/Mouse.h"

#include "../BBE/Math.h"
#include "../BBE/Matrix4.h"
#include "../BBE/ValueNoise2D.h"
#include "../BBE/Vector2.h"
#include "../BBE/Vector3.h"
#include "../BBE/Vector4.h"
#include "../BBE/BezierCurve2.h"
#include "../BBE/Line2.h"

#include "../BBE/DefaultDestroyer.h"
#include "../BBE/NewDeleteAllocator.h"
#include "../BBE/STLAllocator.h"
#include "../BBE/UniquePointer.h"

#include "../BBE/LinearCongruentialGenerator.h"
#include "../BBE/MersenneTwister.h"
#include "../BBE/Random.h"

#include "../BBE/Circle.h"
#include "../BBE/Cube.h"
#include "../BBE/IcoSphere.h"
#include "../BBE/Rectangle.h"
#include "../BBE/RectangleRotated.h"

#include "../BBE/CPUWatch.h"
#include "../BBE/GameTime.h"
#include "../BBE/StopWatch.h"

#include "../BBE/DataType.h"
#include "../BBE/EmptyClass.h"
#include "../BBE/STLCapsule.h"
#include "../BBE/Unconstructed.h"
#include "../BBE/UtilDebug.h"
#include "../BBE/UtilTest.h"

#include "../BBE/Font.h"
#include "../BBE/FragmentShader.h"

#include "../BBE/PhysWorld.h"
#include "../BBE/PhysRectangle.h"
#include "../BBE/PhysCircle.h"
