#pragma once
#include "device.h"

namespace inner
{
    class Swapchain
    {
        private:
        vk::SurfaceFormatKHR m_SurfaceFormat;
        vk::PresentModeKHR m_PresentMode;
        uint32_t m_Images;
        std::shared_ptr<Surface> surface;
        vk::SwapchainKHR swapchain;
        vk::Extent2D m_Size;
        std::vector<vk::UniqueImageView> m_ImageViews;    
        std::shared_ptr<Device> device;
        std::vector<vk::UniqueSemaphore> m_AquireSemaphores;
        std::vector<vk::UniqueSemaphore> m_PresentSemaphores;
        std::vector<vk::UniqueFence> m_SubmitFences;
        vk::Queue m_PresentQueue;
        uint32_t m_AquireIndex;
        uint32_t image_index;

        auto CreateSwapchainImageViews()
        {
            auto images = device->getSwapchainImagesKHR(swapchain);
            m_ImageViews.clear();
            for(auto image : images)
            {
                m_ImageViews.emplace_back(
                    device->createImageViewUnique(
                        vk::ImageViewCreateInfo()
                        .setImage(image)
                        .setViewType(vk::ImageViewType::e2D)
                        .setComponents(vk::ComponentMapping())
                        .setFormat(m_SurfaceFormat.format)
                        .setSubresourceRange(
                            vk::ImageSubresourceRange()
                            .setAspectMask(vk::ImageAspectFlagBits::eColor)
                            .setBaseMipLevel(0)
                            .setLevelCount(1)
                            .setBaseArrayLayer(0)
                            .setLayerCount(1)
                        )
                    )
                );
            }
        }
        public:
    
        Swapchain(std::shared_ptr<Device> device, vk::SurfaceFormatKHR surface_format, vk::PresentModeKHR present_mode, uint32_t image_count, 
        std::shared_ptr<Surface> surface, vk::Queue present_queue, vk::Extent2D size):
        device(device),
        m_SurfaceFormat(surface_format),
        m_PresentMode(present_mode),
        m_Images(image_count),
        surface(surface),
        m_PresentQueue(present_queue),
        m_Size(size),
        m_AquireIndex(0)
        {
            RecreateSwapchain(m_Size);
            for(int x = 0; x < m_ImageViews.size(); x++)
            {
                m_AquireSemaphores.emplace_back( device->createSemaphoreUnique(vk::SemaphoreCreateInfo()));

                m_PresentSemaphores.emplace_back(device->createSemaphoreUnique(vk::SemaphoreCreateInfo()));
                m_SubmitFences.emplace_back(device->createFenceUnique(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled)));
            }
        }

        ~Swapchain()
        {
            device->destroySwapchainKHR(swapchain);
        }

        auto AquireNextImage() //todo: handle failure
        {
            vk::Semaphore sem = m_AquireSemaphores.at(m_AquireIndex).get();
            image_index = device->acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), sem, nullptr);
            m_AquireIndex = (m_AquireIndex + 1) % m_ImageViews.size();

            device->waitForFences(m_SubmitFences.at(image_index).get(), true, std::numeric_limits<uint64_t>::max());
            device->resetFences(m_SubmitFences.at(image_index).get());

            return std::tuple(image_index, sem, m_PresentSemaphores.at(image_index).get(), m_SubmitFences.at(image_index).get());
        }

        auto Present() // todo: handle failure
        {
            m_PresentQueue.presentKHR(
                vk::PresentInfoKHR()
                .setWaitSemaphoreCount(1)
                .setPWaitSemaphores(&m_PresentSemaphores.at(image_index).get())
                .setSwapchainCount(1)
                .setPSwapchains(&swapchain)
                .setPImageIndices(&image_index)
            );
        }

        auto GetSize()
        {
            return m_Size;
        }

        auto GetImageViews()
        {
            std::vector<vk::ImageView> image_views;
            for(auto& view : m_ImageViews)
            {
                image_views.push_back(view.get());
            }
            return image_views;
        }

        void RecreateSwapchain(vk::Extent2D size)
        {

            auto capabilities = device->physical().getSurfaceCapabilitiesKHR(*surface);

            size.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, size.width));
            size.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, size.height));

            m_Size = size;
            auto tmp = swapchain;
            auto cinfo = vk::SwapchainCreateInfoKHR()
                .setImageArrayLayers(1)
                .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                .setImageSharingMode(vk::SharingMode::eExclusive)
                .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                .setClipped(true)
                .setSurface(*surface)
                .setPreTransform(capabilities.currentTransform)
                .setMinImageCount(m_Images)
                .setImageExtent(size)
                .setImageFormat(m_SurfaceFormat.format)
                .setImageColorSpace(m_SurfaceFormat.colorSpace)
                .setPresentMode(m_PresentMode)
                .setOldSwapchain(tmp);

            swapchain = device->createSwapchainKHR(cinfo);

            device->destroySwapchainKHR(tmp);

            CreateSwapchainImageViews();
        }
    };
};

using Swapchain = std::shared_ptr<inner::Swapchain>;

class SwapchainBuilder
{
    private:
    vk::SurfaceFormatKHR m_SurfaceFormat = vk::SurfaceFormatKHR(vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear);
    vk::PresentModeKHR m_PresentMode = vk::PresentModeKHR::eFifo;
    uint32_t m_RequestedImages = 3;
    public:
    SwapchainBuilder()
    {}

    auto SetFormat(vk::SurfaceFormatKHR surface_format = vk::SurfaceFormatKHR(vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear))
    {
        m_SurfaceFormat = surface_format;
        return *this;
    }
    auto SetPresentMode(vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo)
    {
        m_PresentMode = present_mode;
        return *this;
    }
    auto SetRequestedImages(uint32_t images = 3)
    {
        m_RequestedImages = images;
        return *this;
    }
    auto Build(Device device, Surface surface, vk::Queue present_queue, vk::Extent2D size)
    {
        auto physical = device->physical();
        auto surfaceFormats = physical.getSurfaceFormatsKHR(*surface);
        auto supported = false;
        for(auto surfaceFormat : surfaceFormats)
            if(surfaceFormat.format == m_SurfaceFormat)
                {supported = true; break;}

        if(!supported)
            throw(std::exception("Surface format not supported"));

        supported = false;
        for(auto mode : physical.getSurfacePresentModesKHR(*surface))
            if(mode == m_PresentMode)
                {supported = true; break;}

        if(!supported)
            throw(std::exception("Present mode not supported"));
        
        auto capabilities = physical.getSurfaceCapabilitiesKHR(*surface);

        auto image_count = m_RequestedImages;

        image_count = image_count < capabilities.minImageCount ? capabilities.minImageCount + 1 : image_count;
        image_count = image_count > capabilities.maxImageCount ? capabilities.maxImageCount : image_count;

        size.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, size.width));
        size.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, size.height));


        return std::make_shared<inner::Swapchain>(device, m_SurfaceFormat, m_PresentMode, image_count, surface, present_queue, size);
    }
};