#pragma once


#include <fstream>
#include <unordered_map>

#include "renderpass.h"



namespace inner
{
	class Pipeline : public vk::Pipeline
	{
		private:
		std::shared_ptr<Device> device;
		std::shared_ptr<Renderpass> renderpass;
		vk::PipelineLayout layout;
		vk::PipelineBindPoint _bind;
		public:
		Pipeline(vk::PipelineLayout layout, vk::Pipeline pipeline, vk::PipelineBindPoint bind, std::shared_ptr<Device> device, std::shared_ptr<Renderpass> renderpass):
		layout(layout), vk::Pipeline(pipeline), _bind(bind), device(device), renderpass(renderpass)
		{}

		Pipeline(vk::PipelineLayout layout, vk::Pipeline pipeline, vk::PipelineBindPoint bind, std::shared_ptr<inner::Device> device):
		layout(layout), vk::Pipeline(pipeline), _bind(bind), device(device)
		{}

		~Pipeline()
		{
			device->destroyPipeline(*this);
			device->destroyPipelineLayout(layout);
		}

		auto bind()
		{
			return _bind;
		}

	};

};

using Pipeline = std::shared_ptr<inner::Pipeline>;

enum VertexInput
{
	VEC2,
	VEC3,
	VEC4,
	FLOAT,
	INT,
};
class GraphicsPipelineBuilder
{
	std::vector<vk::VertexInputBindingDescription> m_VertexBindings;
	std::vector<vk::VertexInputAttributeDescription> m_VertexAttributes;
	std::vector<vk::PipelineShaderStageCreateInfo> m_ShaderStages;
	std::vector<vk::ShaderModule> m_ShaderModules;
	vk::PipelineLayout m_Layout;
	vk::PipelineBindPoint m_Bind;
	Device device;
	public:
	GraphicsPipelineBuilder(Device device):
	device(device)
	{}
	
	auto AddVertexInput(std::vector<VertexInput> input, vk::VertexInputRate rate)
	{
		uint32_t c = 0;
		for (auto t : input)
		{
			vk::Format format;
			switch (t)
			{
			case VEC2:
				format = vk::Format::eR32G32Sfloat; break;
			case VEC4:
				format = vk::Format::eR32G32B32A32Sfloat; break;
			case VEC3:
				format = vk::Format::eR32G32B32Sfloat; break;
			case FLOAT:
				format = vk::Format::eR32Sfloat; break;
			case INT:
				format = vk::Format::eR32Sint; break;
			default:
				throw(std::exception("Unsupported format!"));
				return std::move(*this);

			}
			uint32_t offset = 0;
			for (uint32_t x = 0; x < c; x++)
			{
				switch (input[x])
				{
				case VEC2:
					offset += sizeof(float) * 2; break;
				case VEC4:
					offset += sizeof(float) * 4; break;
				case VEC3:
					offset += sizeof(float) * 3; break;
				case FLOAT:
					offset += sizeof(float); break;
				case INT:
					offset += sizeof(int32_t); break;
				}

			}
			m_VertexAttributes.emplace_back(
				vk::VertexInputAttributeDescription()
					.setBinding(static_cast<uint32_t>(m_VertexBindings.size()))
					.setLocation(c)
					.setFormat(format)
					.setOffset(offset)
			);
			c++;
		}
		uint32_t offset = 0;
		for (auto t : input)
		{
			switch (t)
			{
				case VEC2:
					offset += sizeof(float) * 2; break;
				case VEC4:
					offset += sizeof(float) * 4; break;
				case VEC3:
					offset += sizeof(float) * 3; break;
				case FLOAT:
					offset += sizeof(float); break;
				case INT:
					offset += sizeof(int32_t); break;
			}
		}

		m_VertexBindings.emplace_back(
			vk::VertexInputBindingDescription()
				.setBinding(static_cast<uint32_t>(m_VertexBindings.size()))
				.setStride(offset)
				.setInputRate(rate)
		);
		return std::move(*this);
	}
	
	auto AddShaderFromFile(const std::string path, vk::ShaderStageFlagBits stage, const char* entry_point = "main")
	{
		std::vector<char> bytes;

		auto file = std::ifstream(path, std::ios::ate | std::ios::binary);
		if (!file)
			throw(std::exception(("File not found: " + path + ".").data()));
		if (file.is_open())
		{
			size_t fileSize = (size_t)file.tellg();
			bytes.resize(fileSize);
			file.seekg(0);
			file.read(bytes.data(), fileSize);
			file.close();
		}
		m_ShaderModules.emplace_back(
			device->createShaderModule(
				vk::ShaderModuleCreateInfo()
					.setCodeSize(bytes.size())
					.setPCode((uint32_t*)bytes.data())
			)
		);
		m_ShaderStages.emplace_back(
			vk::PipelineShaderStageCreateInfo()
			.setModule(m_ShaderModules.at(m_ShaderModules.size() - 1))
			.setStage(stage)
			.setPName(entry_point)
		);

		switch (stage)
		{
			case vk::ShaderStageFlagBits::eCompute:
			m_Bind = vk::PipelineBindPoint::eCompute;
			break;
			default:
			m_Bind = vk::PipelineBindPoint::eGraphics;

		}

		return std::move(*this);
	}

	auto AddPipelineLayout(vk::PipelineLayoutCreateInfo info)
	{
		m_Layout = device->createPipelineLayout(info);
		return std::move(*this);
	}

	auto AddShaderStage(vk::PipelineShaderStageCreateInfo info)
	{
		m_ShaderStages.push_back(info);
		return std::move(*this);
	}
	auto Build(Renderpass renderpass, uint32_t colorblend_count)
	{
		auto depth = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(false)
			.setDepthWriteEnable(false)
			.setDepthCompareOp(vk::CompareOp::eGreater)
			.setFront(vk::StencilOpState().setCompareOp(vk::CompareOp::eAlways))
			.setBack(vk::StencilOpState().setCompareOp(vk::CompareOp::eAlways));
		auto rasterizer = vk::PipelineRasterizationStateCreateInfo()
			.setDepthClampEnable(vk::Bool32(false))
			.setRasterizerDiscardEnable(vk::Bool32(false))
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eClockwise)
			.setDepthBiasEnable(vk::Bool32(false));
		auto multisample = vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(vk::Bool32(false))
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);
		const std::array<vk::DynamicState, 2> dynamicStates =
		{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
		};
		auto dynamic = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStates(dynamicStates);
		auto assembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);
		auto viewport = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setScissorCount(1);
		std::vector<vk::PipelineColorBlendAttachmentState> blends;
		for (uint32_t x = 0; x < colorblend_count; x++)
		{
			auto s = vk::PipelineColorBlendAttachmentState()
				.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
				.setBlendEnable(true)
				.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setColorBlendOp(vk::BlendOp::eAdd)
				.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
				.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
				.setAlphaBlendOp(vk::BlendOp::eAdd);
			blends.push_back(s);
		}
		auto blend = vk::PipelineColorBlendStateCreateInfo()
			.setAttachments(blends);

		auto vertexInput = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptions(m_VertexBindings)
			.setVertexAttributeDescriptions(m_VertexAttributes);
		
		
		auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
			.setStages(m_ShaderStages)
			.setPVertexInputState(&vertexInput)
			.setPInputAssemblyState(&assembly)
			.setPViewportState(&viewport)
			.setPDepthStencilState(&depth)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisample)
			.setPColorBlendState(&blend)
			.setPDynamicState(&dynamic)
			.setLayout(m_Layout)
			.setSubpass(renderpass->subpass_count)
			.setRenderPass(*renderpass);

		renderpass->subpass_count++;

		auto pipeline = device->createGraphicsPipeline(nullptr, pipelineInfo);

		for(auto mod : m_ShaderModules)
		{
			device->destroyShaderModule(mod);
		}

		return std::make_shared<inner::Pipeline>(m_Layout, pipeline, m_Bind, device, renderpass);
	}
};


class ComputePipelineBuilder
{
	private:
	vk::PipelineShaderStageCreateInfo shader_stage;
	std::vector<vk::ShaderModule> shader_modules;
	vk::PipelineLayout layout;
	Device device;
	public:
	ComputePipelineBuilder(Device device):
	device(device)
	{}

	auto AddShaderFromFile(const std::string path, vk::ShaderStageFlagBits stage, const char* entry_point = "main")
	{
		std::vector<char> bytes;

		auto file = std::ifstream(path, std::ios::ate | std::ios::binary);
		if (!file)
			throw(std::exception(("File not found: " + path + ".").data()));
		if (file.is_open())
		{
			size_t fileSize = (size_t)file.tellg();
			bytes.resize(fileSize);
			file.seekg(0);
			file.read(bytes.data(), fileSize);
			file.close();
		}
		shader_modules.emplace_back(
			device->createShaderModule(
				vk::ShaderModuleCreateInfo()
					.setCodeSize(bytes.size())
					.setPCode((uint32_t*)bytes.data())
			)
		);
		shader_stage = 
			vk::PipelineShaderStageCreateInfo()
			.setModule(shader_modules.at(shader_modules.size() - 1))
			.setStage(stage)
			.setPName(entry_point);

		return std::move(*this);
	}

	auto AddPipelineLayout(vk::PipelineLayoutCreateInfo info)
	{
		layout = device->createPipelineLayout(info);
		return std::move(*this);
	}

	auto AddShaderStage(vk::PipelineShaderStageCreateInfo info)
	{
		shader_stage = info;
		return std::move(*this);
	}
	auto Build()
	{
		auto i = vk::ComputePipelineCreateInfo()
				.setLayout(layout)
				.setStage(shader_stage);

		auto pipeline = device->createComputePipeline(nullptr, i);
		for(auto mod : shader_modules)
		{
			device->destroyShaderModule(mod);
		}
		
		return std::make_shared<inner::Pipeline>(layout, pipeline, vk::PipelineBindPoint::eCompute ,device);
	}
};
