#pragma once


#include <iostream>
#include <exception>
#include <filesystem>

#include "platforms/window.h"

#include "swapchain.h"
#include "pool.h"
#include "pipeline.h"

class Render
{
private:

    Instance instance;
    Device device;
    Renderpass renderpass;
    Pipeline pipeline;
    Pipeline compute;
    Swapchain swapchain;
    std::vector<Framebuffer> framebuffers;
    Queue present_queue;
    CommandPool command_pool;
    std::vector<CommandBuffer> command_buffers;

    auto setup(Window& window)
    {
        instance = InstanceBuilder()
        .SetStandarValidation()
        .SetEnabledExtensions(Window::GetInstanceExtensions())
        .Build();


        vk::PhysicalDeviceFeatures enabledFeatures;
        // enabledFeatures.tessellationShader = true;
        // enabledFeatures.geometryShader = true;
        // enabledFeatures.samplerAnisotropy = true;


        auto surface = SurfaceBuilder().Build(instance ,window.CreateWindowSurface(*instance));


        auto [device, queues] = DeviceBuilder()
        .SetEnabledFeatures(enabledFeatures)
        .Build(instance, surface, {QueueType::GENERAL});

        this->device = device;

        present_queue = queues.at(0);
        
        renderpass = RenderpassBuilder()
        .AddAttachments( {
            {"out_image", Attachment{
                .load = vk::AttachmentLoadOp::eDontCare,
                .store = vk::AttachmentStoreOp::eStore,
                .format = vk::Format::eB8G8R8A8Srgb,
                .samples = vk::SampleCountFlagBits::e1
            }}
        }
        )
        .AddSubpassDescription(Description()
            .AddColors({"out_image"})
        )
        .Build(device);

        pipeline = GraphicsPipelineBuilder(device)
            .AddShaderFromFile("../../shaders/vert.spv", vk::ShaderStageFlagBits::eVertex)
            .AddShaderFromFile("../../shaders/frag.spv", vk::ShaderStageFlagBits::eFragment)
            .AddPipelineLayout(vk::PipelineLayoutCreateInfo())
            .Build(renderpass, 1);

        // compute = ComputePipelineBuilder(device)
        //     .AddShaderFromFile("../shaders/comp.spv", vk::ShaderStageFlagBits::eCompute)
        //     .AddPipelineLayout(vk::PipelineLayoutCreateInfo())
        //     .Build();
        

        swapchain = SwapchainBuilder()
        .SetPresentMode(vk::PresentModeKHR::eMailbox)
        .Build(device, surface, present_queue,vk::Extent2D(800, 600));

        auto image_views = swapchain->GetImageViews();

        command_pool = CommandPoolBuilder().Build(present_queue);

        command_buffers = CommandBufferBuilder().Build(command_pool, image_views.size());
        
        for(auto x = 0; x < image_views.size(); x++)
        {
            framebuffers.emplace_back(
                FramebufferBuilder()
                .AddAttachment(image_views.at(x))
                .Build(device, swapchain->GetSize(), renderpass)
            );
            command_buffers.at(x)->begin(vk::CommandBufferBeginInfo());
            // command_buffers.at(x)->bindPipeline(compute);
            // //command_buffers.at(x)->bindDescriptorSets()
            // command_buffers.at(x)->dispatch(1024, 0,0);
            command_buffers.at(x)->bindFramebuffer(framebuffers.at(x));
            command_buffers.at(x)->bindPipeline(pipeline);
            command_buffers.at(x)->draw(3, 1, 0 ,0);
            command_buffers.at(x)->endRenderPass();
            command_buffers.at(x)->end();
        }

    }

public:
    Render(Window& window)
    {
        setup(window);
    }
    ~Render()
    {
        device->waitIdle();
    }
    

    void Resize(uint32_t width, uint32_t height)
    {
        device->waitIdle();
        swapchain->RecreateSwapchain(vk::Extent2D(width, height));
        auto image_views = swapchain->GetImageViews();

        framebuffers.clear();
        for(auto x = 0; x < image_views.size(); x++)
        {
            framebuffers.emplace_back(
                FramebufferBuilder()
                .AddAttachment(image_views.at(x))
                .Build(device, swapchain->GetSize(), renderpass)
            );
            command_buffers.at(x)->begin(vk::CommandBufferBeginInfo());
            // command_buffers.at(x)->bindPipeline(compute);
            // command_buffers.at(x)->dispatch(1024, 0,0);
            command_buffers.at(x)->bindFramebuffer(framebuffers.at(x));
            command_buffers.at(x)->bindPipeline(pipeline);
            command_buffers.at(x)->draw(3, 1, 0 ,0);
            command_buffers.at(x)->endRenderPass();
            command_buffers.at(x)->end();
        }

    }
    void DrawFrame()
    {
        
		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        const auto [index, aquire, present, submit_fence] = swapchain->AquireNextImage();
        
        auto s = vk::SubmitInfo()
            .setWaitSemaphoreCount(1)
            .setSignalSemaphoreCount(1)
            .setPWaitSemaphores(&aquire)
            .setPSignalSemaphores(&present)
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*command_buffers.at(index))
            .setPWaitDstStageMask(waitStages);

        present_queue.submit(s, submit_fence);

        swapchain->Present();


    }
};

