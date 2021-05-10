#pragma once

#include "../log/log.h"
#include "instance.h"
#include "vulkan/vulkan.hpp"



namespace inner
{
    class Device : public vk::Device
    {
        private:
        std::shared_ptr<Instance> instance;
        vk::PhysicalDevice _physical;

        public:

        Device(vk::Device device, vk::PhysicalDevice physical, std::shared_ptr<inner::Instance> instance):
        vk::Device(device), _physical(physical), instance(instance)
        {}

        ~Device()
        {
            waitIdle();
            destroy();
        }

        auto physical()
        {
            return _physical;
        }

    };
};

using Device = std::shared_ptr<inner::Device>;

enum QueueType
{
    GENERAL,
    COMPUTE,
    TRANSFER,
};

class Queue : public vk::Queue
{
    private:
    uint32_t family;
    Device device;
    public:
    Queue() 
    {}
    Queue(vk::Queue queue, Device device, uint32_t family):
    vk::Queue(queue),
    family(family),
    device(device)
    {}

    auto Family()
    {
        return family;
    }

    auto Device()
    {
        return device;
    }

};




class DeviceBuilder
{
private:
    auto ComparePhysicalDeviceFeatures(vk::PhysicalDeviceFeatures check, vk::PhysicalDeviceFeatures target)
    {
        bool ret = true;
        if (check.robustBufferAccess)
        {
            ret = ret && target.robustBufferAccess;
        }
        if (check.fullDrawIndexUint32)
        {
            ret = ret && target.fullDrawIndexUint32;
        }
        if (check.imageCubeArray)
        {
            ret = ret && target.imageCubeArray;
        }
        if (check.independentBlend)
        {
            ret = ret && target.independentBlend;
        }
        if (check.geometryShader)
        {
            ret = ret && target.geometryShader;
        }
        if (check.tessellationShader)
        {
            ret = ret && target.tessellationShader;
        }
        if (check.sampleRateShading)
        {
            ret = ret && target.sampleRateShading;
        }
        if (check.dualSrcBlend)
        {
            ret = ret && target.dualSrcBlend;
        }
        if (check.logicOp)
        {
            ret = ret && target.logicOp;
        }
        if (check.multiDrawIndirect)
        {
            ret = ret && target.multiDrawIndirect;
        }
        if (check.drawIndirectFirstInstance)
        {
            ret = ret && target.drawIndirectFirstInstance;
        }
        if (check.depthClamp)
        {
            ret = ret && target.depthClamp;
        }
        if (check.depthBiasClamp)
        {
            ret = ret && target.depthBiasClamp;
        }
        if (check.fillModeNonSolid)
        {
            ret = ret && target.fillModeNonSolid;
        }
        if (check.depthBounds)
        {
            ret = ret && target.depthBounds;
        }
        if (check.wideLines)
        {
            ret = ret && target.wideLines;
        }
        if (check.largePoints)
        {
            ret = ret && target.largePoints;
        }
        if (check.alphaToOne)
        {
            ret = ret && target.alphaToOne;
        }
        if (check.multiViewport)
        {
            ret = ret && target.multiViewport;
        }
        if (check.samplerAnisotropy)
        {
            ret = ret && target.samplerAnisotropy;
        }
        if (check.textureCompressionETC2)
        {
            ret = ret && target.textureCompressionETC2;
        }
        if (check.textureCompressionASTC_LDR)
        {
            ret = ret && target.textureCompressionASTC_LDR;
        }
        if (check.textureCompressionBC)
        {
            ret = ret && target.textureCompressionBC;
        }
        if (check.occlusionQueryPrecise)
        {
            ret = ret && target.occlusionQueryPrecise;
        }
        if (check.pipelineStatisticsQuery)
        {
            ret = ret && target.pipelineStatisticsQuery;
        }
        if (check.vertexPipelineStoresAndAtomics)
        {
            ret = ret && target.vertexPipelineStoresAndAtomics;
        }
        if (check.fragmentStoresAndAtomics)
        {
            ret = ret && target.fragmentStoresAndAtomics;
        }
        if (check.shaderTessellationAndGeometryPointSize)
        {
            ret = ret && target.shaderTessellationAndGeometryPointSize;
        }
        if (check.shaderImageGatherExtended)
        {
            ret = ret && target.shaderImageGatherExtended;
        }
        if (check.shaderStorageImageExtendedFormats)
        {
            ret = ret && target.shaderStorageImageExtendedFormats;
        }
        if (check.shaderStorageImageMultisample)
        {
            ret = ret && target.shaderStorageImageMultisample;
        }
        if (check.shaderStorageImageReadWithoutFormat)
        {
            ret = ret && target.shaderStorageImageReadWithoutFormat;
        }
        if (check.shaderStorageImageWriteWithoutFormat)
        {
            ret = ret && target.shaderStorageImageWriteWithoutFormat;
        }
        if (check.shaderUniformBufferArrayDynamicIndexing)
        {
            ret = ret && target.shaderUniformBufferArrayDynamicIndexing;
        }
        if (check.shaderSampledImageArrayDynamicIndexing)
        {
            ret = ret && target.shaderSampledImageArrayDynamicIndexing;
        }
        if (check.shaderStorageBufferArrayDynamicIndexing)
        {
            ret = ret && target.shaderStorageBufferArrayDynamicIndexing;
        }
        if (check.shaderStorageImageArrayDynamicIndexing)
        {
            ret = ret && target.shaderStorageImageArrayDynamicIndexing;
        }
        if (check.shaderClipDistance)
        {
            ret = ret && target.shaderClipDistance;
        }
        if (check.shaderCullDistance)
        {
            ret = ret && target.shaderCullDistance;
        }
        if (check.shaderFloat64)
        {
            ret = ret && target.shaderFloat64;
        }
        if (check.shaderInt64)
        {
            ret = ret && target.shaderInt64;
        }
        if (check.shaderInt16)
        {
            ret = ret && target.shaderInt16;
        }
        if (check.shaderResourceResidency)
        {
            ret = ret && target.shaderResourceResidency;
        }
        if (check.shaderResourceMinLod)
        {
            ret = ret && target.shaderResourceMinLod;
        }
        if (check.sparseBinding)
        {
            ret = ret && target.sparseBinding;
        }
        if (check.sparseResidencyBuffer)
        {
            ret = ret && target.sparseResidencyBuffer;
        }
        if (check.sparseResidencyImage2D)
        {
            ret = ret && target.sparseResidencyImage2D;
        }
        if (check.sparseResidencyImage3D)
        {
            ret = ret && target.sparseResidencyImage3D;
        }
        if (check.sparseResidency2Samples)
        {
            ret = ret && target.sparseResidency2Samples;
        }
        if (check.sparseResidency4Samples)
        {
            ret = ret && target.sparseResidency4Samples;
        }
        if (check.sparseResidency8Samples)
        {
            ret = ret && target.sparseResidency8Samples;
        }
        if (check.sparseResidency16Samples)
        {
            ret = ret && target.sparseResidency16Samples;
        }
        if (check.sparseResidencyAliased)
        {
            ret = ret && target.sparseResidencyAliased;
        }
        if (check.variableMultisampleRate)
        {
            ret = ret && target.variableMultisampleRate;
        }
        if (check.inheritedQueries)
        {
            ret = ret && target.inheritedQueries;
        }
        return ret;
    }
    
    auto FindPhysicalDevice(vk::Instance instance)
    {
        vk::PhysicalDevice physical_device;
        auto findSuitableDevice = [&](vk::PhysicalDeviceType type)
        {
            if (!physical_device)
            {
                for (auto p : instance.enumeratePhysicalDevices())
                {
                    if ((p.getProperties().deviceType == type) && ComparePhysicalDeviceFeatures(m_Features, p.getFeatures()) )
                    {
                        physical_device = p;
                        return;
                    }
                }
            }
            return;
        };
        findSuitableDevice(vk::PhysicalDeviceType::eDiscreteGpu);
        findSuitableDevice(vk::PhysicalDeviceType::eIntegratedGpu);
        findSuitableDevice(vk::PhysicalDeviceType::eVirtualGpu);
        findSuitableDevice(vk::PhysicalDeviceType::eCpu);
        findSuitableDevice(vk::PhysicalDeviceType::eOther);
        if(!physical_device)
        {
            throw(std::exception("Unable to find a suitable device"));
        }

        return physical_device;
    }

    auto FindGeneralQueue(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
    {
        for (auto index = 0; index < physical_device.getQueueFamilyProperties().size(); index++)
        {
            auto family = physical_device.getQueueFamilyProperties()[index];
            if (family.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                if (physical_device.getSurfaceSupportKHR(index, surface))
                {
                    return index;
                }
            }
        }
        throw(std::exception("Unable to find a queue family supporting present and graphics"));
        return 0;
    }

    auto FindComputeQueue(vk::PhysicalDevice physical_device)
    {
        for (auto index = 0; index < physical_device.getQueueFamilyProperties().size(); index++)
        {
            auto family = physical_device.getQueueFamilyProperties()[index];
            if (family.queueFlags == vk::QueueFlagBits::eCompute)
            {
                return index;
            }
        }
        throw(std::exception("Unable to find a dedicated compute family"));
        return 0;
    }

    auto FindTransferQueue(vk::PhysicalDevice physical_device)
    {
        for (auto index = 0; index < physical_device.getQueueFamilyProperties().size(); index++)
        {
            auto family = physical_device.getQueueFamilyProperties()[index];
            if (family.queueFlags == vk::QueueFlagBits::eTransfer)
            {
                return index;
            }
        }
        throw(std::exception("Unable to find a dedicated transfer family"));
        return 0;
    }

    vk::PhysicalDeviceFeatures m_Features;
public:
    auto SetEnabledFeatures(vk::PhysicalDeviceFeatures features)
    {
        m_Features = features;
        return *this;
    }

    auto Build(Instance instance, Surface surface, std::vector<QueueType> queues)
    {
        auto physical_device = FindPhysicalDevice(*instance);
        float priority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queue_infos;

        int family;
        std::vector<int> families;
        for (auto& queue : queues) 
        {
            switch(queue)
            {
                case QueueType::GENERAL:
                    family = FindGeneralQueue(physical_device, *surface);
                    break;
                case QueueType::COMPUTE:
                    family = FindComputeQueue(physical_device);
                    break;
                case QueueType::TRANSFER:
                    family = FindTransferQueue(physical_device);
                    break;
            }
            families.push_back(family);
            queue_infos.emplace_back(
                vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(family)
                .setQueueCount(1)
                .setPQueuePriorities(&priority)
            );
        }

        std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        auto i = vk::DeviceCreateInfo()
            .setQueueCreateInfos(queue_infos)
            .setPEnabledExtensionNames(deviceExtensions)
            .setPEnabledFeatures(&m_Features);

        auto device = physical_device.createDevice(i);
        if(!device)
        {
            throw(std::exception("Could not create device"));
        }

        auto r_device = std::make_shared<inner::Device>(device, physical_device, instance);

        std::vector<Queue> d_queues;
        for (auto family : families) {
            d_queues.emplace_back(
                Queue(r_device->getQueue(family, 0) ,r_device, family)
            );
        }

        return std::pair(r_device, d_queues);
        
    }
};