#pragma once

#include "device.h"

namespace inner
{
	class Renderpass : public vk::RenderPass
	{
		private:
		std::shared_ptr<Device> device;
		public:

		uint32_t subpass_count;

		Renderpass(vk::RenderPassCreateInfo create_info, std::shared_ptr<Device> device):
		vk::RenderPass(device->createRenderPass(create_info)), device(device), subpass_count(0)
		{}

		~Renderpass()
		{
			device->destroyRenderPass(*this);
		}

	};
};

using Renderpass = std::shared_ptr<inner::Renderpass>;


struct Attachment
{
	public:
	vk::AttachmentLoadOp load;
	vk::AttachmentStoreOp store;
	vk::Format format;
	vk::SampleCountFlagBits samples;
};

struct Description
{
	public:
	std::vector<std::string> color;
	std::vector<std::string> input;
	std::string depth;
	auto AddColors(std::vector<std::string> colors)
	{
		color = colors;
		return *this;
	}
};

class RenderpassBuilder
{
	std::vector<Description> m_Subpasses;
	std::vector<std::pair<std::string, Attachment>> m_Attachments;
	uint32_t m_SubpassCount = 0;
	struct SubpassData
	{
		public:
		std::vector<vk::AttachmentReference> color;
		std::vector<vk::AttachmentReference> input;
		vk::AttachmentReference depth;
	};
	std::vector<SubpassData> m_PipelineData;
	std::vector<vk::SubpassDependency> m_Dependencies;
	public:

	RenderpassBuilder()
	{}

	
	auto AddAttachments(std::vector<std::pair<std::string, Attachment>> attachments)
	{
		m_Attachments = attachments;
		return std::move(*this);
	}
	
	auto AddSubpassDescription(Description description)
	{

		auto get_index = [&](std::string name) {
			for(uint32_t index = 0; index < m_Attachments.size(); index++)
			{
				if(m_Attachments.at(index).first == name)
					return index;
			}
			throw(std::exception("Unable to find attachment"));
			return (uint32_t)0;
		};

		SubpassData d;
		for(auto& color : description.color)
		{	
			d.color.emplace_back(
				vk::AttachmentReference{get_index(color), vk::ImageLayout::eColorAttachmentOptimal}
			);
		}
		for(auto& input : description.input)
		{	
			auto index = get_index(input);
			d.input.emplace_back(
				vk::AttachmentReference{index, vk::ImageLayout::eShaderReadOnlyOptimal}
			);

			for(uint32_t i = 0; i < m_PipelineData.size(); i++)
			{
				auto data = m_PipelineData.at(i);
				for(auto& c : data.color)
				{
					if (c.attachment == index)
					{
						m_Dependencies.emplace_back(
							vk::SubpassDependency()
							.setSrcSubpass(i)
							.setDstSubpass((uint32_t)m_PipelineData.size())
							.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
							.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
							.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
							.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
							.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
						);
						break;
					}
				}
				if (d.depth.attachment == index)
				{
					m_Dependencies.emplace_back(
						vk::SubpassDependency()
						.setSrcSubpass(i)
						.setDstSubpass((uint32_t)m_PipelineData.size())
						.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
						.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
						.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
						.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
						.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
					);
				}
			}
		}
		if(description.depth.length() > 0)
		{	
			d.depth = vk::AttachmentReference{get_index(description.depth), vk::ImageLayout::eDepthStencilAttachmentOptimal};
		}
		m_PipelineData.push_back(d);
		return std::move(*this);

	}

	auto Build(Device device)
	{
		std::vector<vk::AttachmentDescription> attachments;

		for(auto [name, attach] : m_Attachments)
		{
			auto a = vk::AttachmentDescription()
				.setFormat(attach.format)
				.setSamples(attach.samples)
				.setInitialLayout(vk::ImageLayout::eUndefined);
			switch(attach.format)
			{
				case vk::Format::eD16Unorm:
				case vk::Format::eD16UnormS8Uint:
				case vk::Format::eD24UnormS8Uint:
				case vk::Format::eD32Sfloat:
				case vk::Format::eD32SfloatS8Uint:
				a.setLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(attach.load)
				.setStencilStoreOp(attach.store)
				.setFinalLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);
				attachments.push_back(a);
				break;
				default:
				a.setLoadOp(attach.load)
				.setStoreOp(attach.store)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
				attachments.push_back(a);
				break;
			}
		}
		m_Dependencies.push_back(
			vk::SubpassDependency()
			.setSrcSubpass(VK_SUBPASS_EXTERNAL)
			.setDstSubpass(0)
			.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
			.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests)
			.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
				| vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite));

		m_Dependencies.push_back(
			vk::SubpassDependency()
			.setSrcSubpass(0)
			.setDstSubpass(VK_SUBPASS_EXTERNAL)
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests)
			.setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
				| vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
			.setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)

		);

		std::vector<vk::SubpassDescription> subpasses;

		for(auto& d : m_PipelineData)
		{
			auto i = vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setColorAttachments(d.color)
				.setInputAttachments(d.input);
			if(d.depth != NULL)
			{
				i.setPDepthStencilAttachment(&d.depth);
			}
			subpasses.emplace_back(i);
		}

		auto renderPassInfo = vk::RenderPassCreateInfo()
			.setAttachments(attachments)
			.setDependencies(m_Dependencies)
			.setSubpasses(subpasses);


		return std::make_shared<inner::Renderpass>(renderPassInfo, device);

	}
};