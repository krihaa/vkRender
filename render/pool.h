#pragma once
#include "pipeline.h"

namespace inner
{
    class Framebuffer : public vk::Framebuffer
    {
        private:
        std::shared_ptr<Device> device;
        std::shared_ptr<Renderpass> renderpass;
        vk::Extent2D size;
        public:
        Framebuffer(std::shared_ptr<Device> device, std::shared_ptr<Renderpass> renderpass, vk::Framebuffer framebuffer, vk::Extent2D size):
        vk::Framebuffer(framebuffer), device(device), renderpass(renderpass), size(size)
        {

        }

        auto Renderpass()
        {
            return renderpass;
        }

        auto Size()
        {
            return size;
        }

        ~Framebuffer()
        {
            device->destroyFramebuffer(*this);
        }
    };

    class CommandPool : public vk::CommandPool
    {
        private:
        std::shared_ptr<Device> device;

        public:

        CommandPool(std::shared_ptr<Device> device, vk::CommandPool pool):
        device(device),
        vk::CommandPool(pool)
        {}

        ~CommandPool()
        {
            device->destroyCommandPool(*this);
        }

        auto Device()
        {
            return device;
        }

        auto Reset()
        {
            device->resetCommandPool(*this, vk::CommandPoolResetFlags());
        }

        auto Release()
        {
            device->resetCommandPool(*this, vk::CommandPoolResetFlagBits::eReleaseResources);
        }
    };
    
    class CommandBuffer : public vk::CommandBuffer
    {
        private:
        std::shared_ptr<CommandPool> pool;
        public:
        CommandBuffer(vk::CommandBuffer buffer, std::shared_ptr<CommandPool> pool):
        vk::CommandBuffer(buffer), pool(pool)
        {}

        void bindFramebuffer(std::shared_ptr<Framebuffer> framebuffer, vk::SubpassContents content = vk::SubpassContents::eInline)
        {	   
            auto size = framebuffer->Size();
            auto view = vk::Viewport()
                .setWidth((float)size.width)
                .setHeight((float)size.height)
                .setMaxDepth(1.0f);
            auto scissor = vk::Rect2D()
                .setOffset(vk::Offset2D(0, 0))
                .setExtent(size);
            setViewport(0, view);
            setScissor(0, scissor);
            beginRenderPass(
                vk::RenderPassBeginInfo()
                    .setFramebuffer(*framebuffer)
                    .setRenderArea(scissor)
                    .setRenderPass(*framebuffer->Renderpass())
                    .setClearValues({}) // TODO
            , content);
        }

        void bindPipeline(std::shared_ptr<Pipeline> pipeline)
        {
            static_cast<const vk::CommandBuffer&>(*this).bindPipeline(pipeline->bind(), *pipeline);
        }
    };

};

using CommandPool = std::shared_ptr<inner::CommandPool>;
using CommandBuffer = std::shared_ptr<inner::CommandBuffer>;
using Framebuffer = std::shared_ptr<inner::Framebuffer>;

class FramebufferBuilder
{
    private:
    std::vector<vk::ImageView> attachments;
    public:
    auto AddAttachment(vk::ImageView attachment)
    {
        attachments.push_back(attachment);
        return *this;
    }
    auto Build(Device device, vk::Extent2D size, Renderpass renderpass)
    {
        auto f = vk::FramebufferCreateInfo()
        .setAttachments(attachments)
        .setHeight(size.height)
        .setWidth(size.width)
        .setLayers(1)
        .setRenderPass(*renderpass);

        return std::make_shared<inner::Framebuffer>(device, renderpass, device->createFramebuffer(f), size);
    }
};

class CommandPoolBuilder
{
    public:
    auto Build(Queue queue, vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
    {
        return std::make_shared<inner::CommandPool>(queue.Device(), queue.Device()->createCommandPool(vk::CommandPoolCreateInfo()
            .setQueueFamilyIndex(queue.Family())
            .setFlags(flags)
        ));
    }   
};

class CommandBufferBuilder
{
    private:
    vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary;
    public:
    auto SetLevel(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary)
    {
        this->level = level;
    }
    auto Build(CommandPool pool, uint32_t count)
    {
        auto buffers = pool->Device()->allocateCommandBuffers(
            vk::CommandBufferAllocateInfo()
            .setCommandBufferCount(count)
            .setCommandPool(*pool)
            .setLevel(level)
        );

        std::vector<CommandBuffer> ret;
        for (auto buffer : buffers)
        {
            ret.emplace_back(
                std::make_shared<inner::CommandBuffer>(buffer, pool)
            );
        }
        return ret;
    }
};
