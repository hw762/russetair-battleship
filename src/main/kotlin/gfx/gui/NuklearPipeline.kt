package gfx.gui

import gfx.vk.*
import gfx.vk.VulkanUtils.Companion.vkCheck
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.util.shaderc.Shaderc
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK10.*

class NuklearPipeline(private val device: Device,
                      private val pipelineCache: PipelineCache,
                      colorFormat: Int) {
    val vkPipeline: Long
    private val vertexShader: ShaderProgram
    private val fragmentShader: ShaderProgram

    init {
        ShaderCompiler.compileShaderIfChanged(NK_VERTEX_SHADER_FILE_GLSL, Shaderc.shaderc_glsl_vertex_shader)
        ShaderCompiler.compileShaderIfChanged(NK_FRAGMENT_SHADER_FILE_GLSL, Shaderc.shaderc_glsl_fragment_shader)
        vertexShader = ShaderProgram(device, VK_SHADER_STAGE_VERTEX_BIT, NK_VERTEX_SHADER_FILE_SPV)
        fragmentShader = ShaderProgram(device, VK_SHADER_STAGE_FRAGMENT_BIT, NK_FRAGMENT_SHADER_FILE_SPV)
        MemoryStack.stackPush().use { stack ->
            val main = stack.UTF8("main")
            val shaderStages = VkPipelineShaderStageCreateInfo.calloc(2, stack)
            shaderStages[0]
                .`sType$Default`()
                .stage(VK_SHADER_STAGE_VERTEX_BIT)
                .module(vertexShader.shaderModule.handle)
                .pName(main)
                .pSpecializationInfo(vertexShader.shaderModule.specInfo)
            shaderStages[1]
                .`sType$Default`()
                .stage(VK_SHADER_STAGE_FRAGMENT_BIT)
                .module(fragmentShader.shaderModule.handle)
                .pName(main)
                .pSpecializationInfo(fragmentShader.shaderModule.specInfo)
            val viAttrs = VkVertexInputAttributeDescription.calloc(3)
            viAttrs[0]
                .binding(0)
                .location(0)
                .format(VK_FORMAT_R32G32_SFLOAT)
                .offset(0)
            viAttrs[1]
                .binding(0)
                .location(1)
                .format(VK_FORMAT_R32G32_SFLOAT)
                .offset(2 * GraphConstants.FLOAT_SIZE)
            viAttrs[2]
                .binding(0)
                .location(2)
                .format(VK_FORMAT_R8G8B8A8_UINT)
                .offset(4* GraphConstants.FLOAT_SIZE)
            val viBindings = VkVertexInputBindingDescription.calloc(1)
            viBindings[0]
                .binding(0)
                .stride(4* GraphConstants.FLOAT_SIZE + 4)
                .inputRate(VK_VERTEX_INPUT_RATE_VERTEX)
            val vertexInputStateCI = VkPipelineVertexInputStateCreateInfo.calloc()
                .`sType$Default`()
                .pVertexAttributeDescriptions(viAttrs)
                .pVertexBindingDescriptions(viBindings)
            val inputAssemblyStateCI = VkPipelineInputAssemblyStateCreateInfo.calloc(stack)
                .`sType$Default`()
                .topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            val viewportStateCI = VkPipelineViewportStateCreateInfo.calloc(stack)
                .`sType$Default`()
                .viewportCount(1)
                .scissorCount(1)
            val rasterizationStateCI = VkPipelineRasterizationStateCreateInfo.calloc(stack)
                .`sType$Default`()
                .polygonMode(VK_POLYGON_MODE_FILL)
                .cullMode(VK_CULL_MODE_NONE)
                .frontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
                .lineWidth(1f)
            val multisampleStateCI = VkPipelineMultisampleStateCreateInfo.calloc(stack)
                .`sType$Default`()
                .rasterizationSamples(VK_SAMPLE_COUNT_1_BIT)
            val attachments = VkPipelineColorBlendAttachmentState.calloc(1, stack)
            attachments[0]
                .colorWriteMask(VK_COLOR_COMPONENT_R_BIT or VK_COLOR_COMPONENT_G_BIT or VK_COLOR_COMPONENT_B_BIT or VK_COLOR_COMPONENT_A_BIT)
                .colorBlendOp(VK_BLEND_OP_ADD)
                .alphaBlendOp(VK_BLEND_OP_ADD)
                .srcColorBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA)
                .dstColorBlendFactor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
                .srcAlphaBlendFactor(VK_BLEND_FACTOR_ONE)
                .dstAlphaBlendFactor(VK_BLEND_FACTOR_ZERO)
            val colorBlendState = VkPipelineColorBlendStateCreateInfo.calloc(stack)
                .`sType$Default`()
                .pAttachments(attachments)
            val dynamicStateCI = VkPipelineDynamicStateCreateInfo.calloc(stack)
                .`sType$Default`()
                .pDynamicStates(stack.ints(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR))
            val pColorFormats = stack.mallocInt(1)
            pColorFormats.put(0, colorFormat)
            val renderingCI = VkPipelineRenderingCreateInfo.calloc(stack)
                .`sType$Default`()
                .pColorAttachmentFormats(pColorFormats)
            val pipelineCIs = VkGraphicsPipelineCreateInfo.calloc(1, stack)
                .`sType$Default`()
                .pNext(renderingCI)
                .pStages(shaderStages)
                .pVertexInputState(vertexInputStateCI)
                .pInputAssemblyState(inputAssemblyStateCI)
                .pViewportState(viewportStateCI)
                .pRasterizationState(rasterizationStateCI)
                .pMultisampleState(multisampleStateCI)
                .pColorBlendState(colorBlendState)
                .pDynamicState(dynamicStateCI)
            val lp = stack.mallocLong(1)
            vkCheck(
                vkCreateGraphicsPipelines(device.vkDevice, pipelineCache.vkPipelineCache, pipelineCIs, null, lp),
                "Failed to create pipeline"
            );
            vkPipeline = lp[0]
        }
    }

    fun cleanup() {
        vertexShader.cleanup()
        fragmentShader.cleanup()
        vkDestroyPipeline(device.vkDevice, vkPipeline, null)
    }

    companion object {
        private const val NK_FRAGMENT_SHADER_FILE_GLSL = "resources/shaders/nuklear.frag.glsl"
        private const val NK_FRAGMENT_SHADER_FILE_SPV = "$NK_FRAGMENT_SHADER_FILE_GLSL.spv"
        private const val NK_VERTEX_SHADER_FILE_GLSL = "resources/shaders/nuklear.vert.glsl"
        private const val NK_VERTEX_SHADER_FILE_SPV = "$NK_VERTEX_SHADER_FILE_GLSL.spv"
    }
}