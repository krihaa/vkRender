#pragma once

#include <memory>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 0

#include "vulkan/vulkan.hpp"
#include "../log/log.h"

namespace inner
{
    class Instance : public vk::Instance
    {
        private:
        VkDebugReportCallbackEXT debug;

        static VKAPI_ATTR VkBool32 VKAPI_CALL print_debug(
            VkDebugReportFlagsEXT flags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t obj,
            size_t location,
            int32_t code,
            const char *prefix,
            const char *msg,
            void *userData)
        {
            warn(std::string("[Validation]: ") + std::string(prefix) + std::string(msg));
            return VK_FALSE;
        }
        auto enable_validation()
        {
            auto vkCreate = (PFN_vkCreateDebugReportCallbackEXT) getProcAddr("vkCreateDebugReportCallbackEXT");
            auto info = vk::DebugReportCallbackCreateInfoEXT()
                .setFlags(vk::DebugReportFlagBitsEXT::ePerformanceWarning | 
                vk::DebugReportFlagBitsEXT::eWarning | 
                vk::DebugReportFlagBitsEXT::eError)
                .setPfnCallback(print_debug);
            if ( vkCreate(static_cast<VkInstance>(*this), reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*>(&info), nullptr, &debug) != VK_SUCCESS)
            {
                throw(std::exception("CreateDebugReportCallback unsuccessfull"));
            }

        }
        public:
        Instance(vk::Instance instance, bool validation):
        vk::Instance(instance)
        {
            if(validation)
                enable_validation();
        }

        ~Instance()
        {
            if(debug)
            {
                auto vkDestroy = (PFN_vkDestroyDebugReportCallbackEXT) getProcAddr("vkDestroyDebugReportCallbackEXT");
                vkDestroy(static_cast<VkInstance>(*this), debug, nullptr);
            }

            destroy();
            
        }
    };
    
    class Surface : public vk::SurfaceKHR
    {
        private:
        std::shared_ptr<Instance> instance;

        public:
        Surface(vk::SurfaceKHR surface, std::shared_ptr<Instance> instance):
        vk::SurfaceKHR(surface), instance(instance) 
        {}

        ~Surface()
        {
            instance->destroySurfaceKHR(*this);
        }

    };

};


using Instance = std::shared_ptr<inner::Instance>;
using Surface = std::shared_ptr<inner::Surface>;

class SurfaceBuilder
{
    public:
    auto Build(Instance instance, vk::SurfaceKHR surface)
    {
        return std::make_shared<inner::Surface>(surface, instance);
    }
};

class InstanceBuilder
{
private:
    std::vector<const char*> m_ValidationLayers;
    std::vector<const char*> m_Extensions;
    template <typename T>
    auto CheckLayerSupport(T layers)
    {
        std::vector<vk::LayerProperties> available = vk::enumerateInstanceLayerProperties();
        std::vector<const char *> supported;
        for (auto layer : layers)
        {
            bool found = false;
            for (const auto &properties : available)
                if (strcmp(layer, properties.layerName) == 0)
                {
                    supported.emplace_back(layer);
                    found = true;
                    break;
                }
            if (!found)
            {
                warn("Layer not found: " + std::string(layer));
            }
        }
        m_ValidationLayers = supported;
    }
public:    

    template <typename T>
    auto SetValidationLayers(T layers)
    {
        CheckLayerSupport(layers);
        return *this;
    }

    auto SetStandarValidation()
    {
        CheckLayerSupport(
            std::array<const char*, 2>{
                "VK_LAYER_KHRONOS_validation",
                "VK_LAYER_LUNARG_monitor"
            }
        );
        return *this;
    }

    auto SetExtendedValidation()
    {
        CheckLayerSupport(
            std::array<const char*, 4>{
                "VK_LAYER_KHRONOS_validation", 
                "VK_LAYER_RENDERDOC_capture", 
                "VK_LAYER_LUNARG_api_dump", 
                "VK_LAYER_LUNARG_monitor"
            }
        );
        return *this;
    }    

    auto SetEnabledExtensions(std::vector<const char*> extensions)
    {
        m_Extensions = extensions;
        return *this;
    } 

    auto Build()
    {
        auto instanceCreateInfo = vk::InstanceCreateInfo();
        bool validation = false;
        if (m_ValidationLayers.size() > 0)
        {
            instanceCreateInfo.setPEnabledLayerNames(m_ValidationLayers);
            m_Extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            validation = true;

        }
        instanceCreateInfo.setPEnabledExtensionNames(m_Extensions);
        return std::make_shared<inner::Instance>(vk::createInstance(instanceCreateInfo), validation);
    }
};
