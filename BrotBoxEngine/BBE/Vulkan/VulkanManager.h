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
#include "../BBE/RenderManager.h"
#include "../Vulkan/VulkanImage.h"

struct ImFont;

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

			enum class PipelineRecord3D
			{
				NONE, PRIMITIVE
			};

			enum class DrawRecord
			{
				NONE, CUBE, ICOSPHERE
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
				VulkanPipeline m_pipeline3DPrimitive;

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
				VulkanDescriptorPool      m_descriptorPool;
				VulkanDescriptorSet       m_setVertexLight;
				VulkanDescriptorSet       m_setFragmentLight;
				bbe::List<VulkanDescriptorSet> m_setViewProjectionMatrixLights;
				GLFWwindow               *m_pwindow = nullptr;
				PrimitiveBrush2D          m_primitiveBrush2D;
				bbe::List<PrimitiveBrush3D> m_primitiveBrushes3D;
				bbe::List<VulkanBuffer>   m_uboMatrices;

				bbe::List<std::future<void>> screenshotFutures;
				bbe::List<std::shared_future<void>> videoFutures;

				uint32_t m_screenWidth;
				uint32_t m_screenHeight;
				uint32_t m_imageIndex;

				//VulkanStopWatch m_renderPassStopWatch;

				FILE* videoFile = nullptr;


				bbe::List<bbe::List<bbe::AutoRef>> imageDatas;

				PipelineRecord2D m_pipelineRecord2D = PipelineRecord2D::NONE;
				PipelineRecord3D m_pipelineRecord3D = PipelineRecord3D::NONE;

				DrawRecord m_lastDraw3D = DrawRecord::NONE;

				constexpr static uint32_t m_imguiMinImageCount = 2;
				bool m_imguiInitSuccessful = false;
				ImFont* imguiFontSmall = nullptr;
				ImFont* imguiFontBig = nullptr;

				struct BufferMemoryPair
				{
					VkBuffer       m_buffer;
					VkDeviceMemory m_memory;
				};
				bbe::List<bbe::List<BufferMemoryPair>> m_delayedBufferDeletes;
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

				void bindPipelinePrimitive3D();

				virtual void setColor2D(const bbe::Color& color) override;
				virtual void fillRect2D(const Rectangle& rect, float rotation, FragmentShader* shader) override;
				virtual void fillCircle2D(const Circle& circle) override;
				virtual void drawImage2D(const Rectangle& rect, const Image& image, float rotation) override;
				virtual void fillVertexIndexList2D(const uint32_t* indices, size_t amountOfIndices, const bbe::Vector2* vertices, size_t amountOfVertices, const bbe::Vector2& pos, const bbe::Vector2& scale) override;

				virtual void setColor3D(const bbe::Color& color) override;
				virtual void setCamera3D(const bbe::Vector3& cameraPos, const bbe::Matrix4& m_view, const bbe::Matrix4& m_projection) override;
				virtual void fillCube3D(const Cube& cube) override;
				virtual void fillSphere3D(const bbe::IcoSphere& sphere) override;
				virtual void addLight(const bbe::Vector3& pos, float lightStrenght, bbe::Color lightColor, bbe::Color specularColor, LightFalloffMode falloffMode) override;

				virtual void imguiStart() override;
				virtual void imguiStop() override;
				virtual void imguiStartFrame() override;
				virtual void imguiEndFrame() override;

			};
		}
	}
}
