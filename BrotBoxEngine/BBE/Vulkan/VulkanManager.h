#pragma once
#include <future>

#include "GLFW/glfw3.h"

#include "../BBE/PrimitiveBrush2D.h"
#include "../BBE/PrimitiveBrush3D.h"
#include "../Vulkan//VulkanInstance.h"
#include "../Vulkan//VulkanSurface.h"
#include "../Vulkan//VulkanPhysicalDevices.h"
#include "../Vulkan//VulkanDevice.h"
#include "../Vulkan//VulkanSwapchain.h"
#include "../Vulkan//VulkanRenderPass.h"
#include "../Vulkan//VulkanShader.h"
#include "../Vulkan//VulkanPipeline.h"
#include "../Vulkan//VulkanCommandPool.h"
#include "../Vulkan//VulkanSemaphore.h"
#include "../Vulkan//VulkanDescriptorPool.h"
#include "../Vulkan//VulkanDescriptorSet.h"
#include "../Vulkan//VulkanDescriptorSetLayout.h"
#include "../Vulkan//VWDepthImage.h"
#include "../Vulkan//VulkanFence.h"
#include "../BBE/Stack.h"
#include "../BBE/Image.h"
#include "../Vulkan/VulkanStopWatch.h"
#include "../BBE/ImguiManager.h"
#include "../BBE/RenderManager.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			enum class PipelineRecord2D
			{
				NONE, PRIMITIVE, IMAGE
			};

			class VulkanManager 
				: public RenderManager {
			public:
				struct ScreenshotFirstStage
				{
					VkDevice            device;
					VkDeviceMemory      dstImageMemory;
					VkImage             dstImage;
					VkSubresourceLayout subResourceLayout;
					VkFormat            format;
					uint32_t            height;
					uint32_t            width;

					unsigned char* toPixelData(bool* outRequiresSwizzle);
				};
			public:
				static VulkanManager* s_pinstance;
			private:
				VulkanInstance          m_instance;
				VulkanSurface           m_surface;
				PhysicalDeviceContainer m_physicalDeviceContainer;
				VulkanDevice            m_device;
				VulkanSwapchain         m_swapchain;
				VulkanRenderPass        m_renderPass;

				VulkanShader   m_vertexShader2DPrimitive;
				VulkanShader   m_fragmentShader2DPrimitive;
				VulkanPipeline m_pipeline2DPrimitive;

				VulkanShader   m_fragmentShader2DImage;
				VulkanPipeline m_pipeline2DImage;

				VulkanShader   m_vertexShader3DPrimitive;
				VulkanShader   m_fragmentShader3DPrimitive;
				VulkanShader   m_vertexShader3DTerrain;
				VulkanShader   m_fragmentShader3DTerrain;
				VulkanShader   m_teseShader3DTerrain;
				VulkanShader   m_tescShader3DTerrain;
				VulkanPipeline m_pipeline3DPrimitive;
				VulkanPipeline m_pipeline3DTerrain;

				VulkanCommandPool         m_commandPool;
				VulkanSemaphore           m_semaphoreImageAvailable;
				VulkanSemaphore           m_semaphoreRenderingDone;
				VWDepthImage              m_depthImage;
				VkCommandBuffer           m_currentFrameDrawCommandBuffer1 = VK_NULL_HANDLE;
				VulkanFence               m_presentFence1;
				VkCommandBuffer           m_currentFrameDrawCommandBuffer2 = VK_NULL_HANDLE;
				VulkanFence               m_presentFence2;
				VkCommandBuffer           *m_currentFrameDrawCommandBuffer = VK_NULL_HANDLE;
				VulkanFence               *m_presentFence;
				VulkanDescriptorSetLayout m_setLayoutVertexLight;
				VulkanDescriptorSetLayout m_setLayoutFragmentLight;
				VulkanDescriptorSetLayout m_setLayoutViewProjectionMatrix;
				VulkanDescriptorSetLayout m_setLayoutSampler;
				VulkanDescriptorSetLayout m_setLayoutTerrainHeightMap;
				VulkanDescriptorSetLayout m_setLayoutTerrainAdditionalTexture;
				VulkanDescriptorSetLayout m_setLayoutTerrainAdditionalTextureWeight;
				VulkanDescriptorSetLayout m_setLayoutTerrainViewFrustum;
				VulkanDescriptorPool      m_descriptorPool;
				VulkanDescriptorSet       m_setVertexLight;
				VulkanDescriptorSet       m_setFragmentLight;
				bbe::List<VulkanDescriptorSet> m_setViewProjectionMatrixLights;
				GLFWwindow               *m_pwindow = nullptr;
				PrimitiveBrush2D          m_primitiveBrush2D;
				bbe::List<PrimitiveBrush3D> m_primitiveBrushes3D;

				bbe::List<std::future<void>> screenshotFutures;
				bbe::List<std::shared_future<void>> videoFutures;

				uint32_t m_screenWidth;
				uint32_t m_screenHeight;
				uint32_t m_imageIndex;

				//VulkanStopWatch m_renderPassStopWatch;

				ImguiManager m_imguiManager;
				FILE* videoFile = nullptr;

				bbe::List<bbe::List<bbe::Image::VulkanData*>> imageDatas;

				PipelineRecord2D m_pipelineRecord = PipelineRecord2D::NONE;
				FillMode m_fillMode = FillMode::SOLID;

			private:
				ScreenshotFirstStage getRawScreenshot();

			public:
				VulkanManager();

				VulkanManager(const VulkanManager& other) = delete;
				VulkanManager(VulkanManager&& other) = delete;
				VulkanManager& operator=(const VulkanManager& other) = delete;
				VulkanManager& operator=(VulkanManager&& other) = delete;

				void init(const char *appName, uint32_t major, uint32_t minor, uint32_t patch, GLFWwindow *window, uint32_t initialWindowWidth, uint32_t initialWindowHeight) override;

				void destroy() override;
				void preDraw2D() override;
				void preDraw3D() override;
				void preDraw() override;
				void postDraw() override;
				void waitEndDraw() override;
				void waitTillIdle() override;

				bool isReadyToDraw() const override;

				bbe::PrimitiveBrush2D &getBrush2D() override;
				bbe::PrimitiveBrush3D &getBrush3D() override;

				void createPipelines();
				void resize(uint32_t width, uint32_t height) override;
				void recreateSwapchain();

				VulkanDevice& getVulkanDevice();
				VulkanRenderPass& getVulkanRenderPass();
				VulkanShader& getVertexShader2DPrimitive();

				void screenshot(const bbe::String& path) override;
				void saveVideoFrame();
				void setVideoRenderingMode(const char* path) override;
				void stopRecording();

				void bindRectBuffers();

				virtual void setColor2D(const bbe::Color& color) override;
				virtual void fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader) override;
				virtual void fillCircle2D(const Circle& circle) override;
				virtual void setFillMode2D(bbe::FillMode fm) override;
				virtual bbe::FillMode getFillMode2D() override;
				virtual void drawImage2D(const Rectangle& rect, const Image& image, float rotation) override;
};
		}
	}
}
