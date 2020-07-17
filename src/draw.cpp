#include "device.h"

#include "buffer.h"
#include "pipeline.h"

void Device::finalize(
    const Buffer &indexBuffer,
    const Buffer &vertexBuffer,
    const std::vector<Pipeline*> &pipelines)
{
    const auto &primaryCommandBuffers = this->primaryCommandBuffers();
    auto secondaryCommandBuffers = this->secondaryCommandBuffers();
    const auto &commandPools = this->commandPools();
    const auto numThreads = this->numThreads();
    const size_t numIndicesEach=indexBuffer.m_numElements/this->numThreads();
    const auto renderpass=pipelines[0]->m_renderpass; // TODO: Check if this is suitable. Only one renderpass supported. Singleton?
    setupFramebuffer(*renderpass);
    const auto &framebuffers = this->framebuffers();
    const auto &clearValues = renderpass->clearValues();
    
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderpass->renderpass();
    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = this->extent();
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    const auto &numSubpasses = renderpass->subpasses().size();

    for (size_t imageIndex = 0; imageIndex < this->swapchainSize(); ++imageIndex)
    {
        auto &primaryCommandBuffer = primaryCommandBuffers[imageIndex];

        renderPassInfo.framebuffer = framebuffers[imageIndex];
        vkBeginCommandBuffer(primaryCommandBuffer, &beginInfo);

        for (size_t pass = 0; pass < numSubpasses; ++pass)
        {
            const auto &descriptorSets = pipelines[pass]->m_descriptor->sets();
            auto &pipeline = pipelines[pass]->m_pipeline;
            auto &pipelineLayout = pipelines[pass]->m_layout;

            if (pass == 0 )
                vkCmdBeginRenderPass(
                    primaryCommandBuffer,
                    &renderPassInfo,
                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            else
                vkCmdNextSubpass(
                    primaryCommandBuffer,
                    VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

            auto createDrawCommands =[&](int i)
            {
                auto &secondaryCommandBuffer = secondaryCommandBuffers[i];

                size_t numIndices=numIndicesEach;
                size_t indexOffset=numIndicesEach*i;
                if (i==(numThreads-1)) numIndices = indexBuffer.m_numElements-(i*numIndicesEach);

                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = commandPools[i];
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                allocInfo.commandBufferCount = 1;
                if (vkAllocateCommandBuffers(this->device(), &allocInfo, &secondaryCommandBuffer) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to allocate command buffers.");
                }

                VkCommandBufferInheritanceInfo inheritanceInfo = {};
                inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
                inheritanceInfo.renderPass = renderpass->renderpass();
                inheritanceInfo.framebuffer = framebuffers[imageIndex];
                inheritanceInfo.subpass=pass;

                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
                beginInfo.pInheritanceInfo = &inheritanceInfo;

                if (vkBeginCommandBuffer(secondaryCommandBuffer, &beginInfo) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to begin recording command buffer.");
                }

                vkCmdBindPipeline(secondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                VkDeviceSize offsets[] = {0};

                vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, &vertexBuffer.m_buffer, offsets);
                vkCmdBindIndexBuffer(secondaryCommandBuffer, indexBuffer.m_buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(
                    secondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                    0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
                vkCmdDrawIndexed(secondaryCommandBuffer, numIndices, 1, indexOffset, 0, 0);

                if (vkEndCommandBuffer(secondaryCommandBuffer) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to record command buffer.");
                }
            };

            int counter = 0;
            for (auto &t: this->threads())
            {
                t->addJob(std::bind(createDrawCommands,counter++));
            }
            this->wait();
            vkCmdExecuteCommands(primaryCommandBuffer, secondaryCommandBuffers.size(), secondaryCommandBuffers.data());
        }

        vkCmdEndRenderPass(primaryCommandBuffer);

        if (vkEndCommandBuffer(primaryCommandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not end primaryCommandBuffer.");   
        }
    }
}