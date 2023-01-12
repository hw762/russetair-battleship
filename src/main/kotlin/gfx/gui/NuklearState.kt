package gfx.gui

import gfx.vk.*
import org.lwjgl.nuklear.*
import org.lwjgl.nuklear.Nuklear.*
import org.lwjgl.stb.STBTTAlignedQuad
import org.lwjgl.stb.STBTTFontinfo
import org.lwjgl.stb.STBTTPackContext
import org.lwjgl.stb.STBTTPackedchar
import org.lwjgl.stb.STBTruetype.*
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.system.MemoryUtil.NULL
import org.lwjgl.system.MemoryUtil.memAddress
import org.lwjgl.util.vma.Vma
import org.lwjgl.vulkan.*
import org.lwjgl.vulkan.VK10.*
import java.io.File

class NuklearState(private val device: Device) : CommandRecordable {
    private val ctx = NkContext.create()
    private val defaultFont = NkUserFont.create()
    private val cmds = NkBuffer.create()
    private val nullTexture = NkDrawNullTexture.create()
    private lateinit var fontTexture: VulkanTexture
    private lateinit var fontTextureView: ImageView
    private val vkSampler: Long

    private val vertices = VulkanBuffer(
        device, MAX_VERTEX_BUFFER, VK10.VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        Vma.VMA_MEMORY_USAGE_AUTO, Vma.VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    )
    private val elements = VulkanBuffer(
        device, MAX_ELEMENT_BUFFER, VK10.VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        Vma.VMA_MEMORY_USAGE_AUTO, Vma.VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
    )
    private val vkDescriptorPool: Long
    private val projMatDescriptorSetLayout = DescriptorSetLayout.SimpleDescriptorSetLayout(
        device, VK10.VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, VK10.VK_SHADER_STAGE_VERTEX_BIT
    )
    private val textureDescriptorSetLayout = DescriptorSetLayout.SimpleDescriptorSetLayout(
        device, VK10.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, VK10.VK_SHADER_STAGE_FRAGMENT_BIT
    )

    init {
        nk_init(ctx, ALLOCATOR, defaultFont)
        MemoryStack.stackPush().use { stack ->
            vkDescriptorPool = createDescriptorPool(stack, device)
            vkSampler = createTextureSampler(stack)
            initFont()
            nk_style_set_font(ctx, defaultFont)
        }

    }

    private fun initFont() {
        fontTexture = VulkanTexture(
            device, FONT_BITMAP_W, FONT_BITMAP_H, 1,
            VK_FORMAT_R8_UINT, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL
        )
        MemoryStack.stackPush().use { stack ->
            val FONT_HEIGHT = 18.0f
            val fontInfo = STBTTFontinfo.create()
            val cdata = STBTTPackedchar.create(95)
            // Read the font
            val ttfBytes = File(DEFAULT_FONT).readBytes()
            val ttf = stack.malloc(ttfBytes.size)
            ttf.put(ttfBytes)
            ttf.flip()
            // Initialize
            stbtt_InitFont(fontInfo, ttf)
            val scale = stbtt_ScaleForPixelHeight(fontInfo, FONT_HEIGHT)

            val d = stack.mallocInt(1)
            stbtt_GetFontVMetrics(fontInfo, null, d, null)
            val descent = d[0] * scale

            val bitmap = stack.malloc(FONT_BITMAP_W * FONT_BITMAP_H)

            val pc = STBTTPackContext.malloc(stack)
            stbtt_PackBegin(pc, bitmap, FONT_BITMAP_W, FONT_BITMAP_H, 0, 1, NULL)
            stbtt_PackSetOversampling(pc, 4, 4)
            stbtt_PackFontRange(pc, ttf, 0, FONT_HEIGHT, 32, cdata)
            stbtt_PackEnd(pc)

            defaultFont
                .width { _, _, text, len -> calcTextWidth(fontInfo, text, len, scale) }
                .height(FONT_HEIGHT)
                .query { handle, font_height, glyph, codepoint, next_codepoint ->
                    MemoryStack.stackPush().use { stack ->
                        val x = stack.floats(0.0f)
                        val y = stack.floats(0.0f)

                        val q = STBTTAlignedQuad.malloc(stack)
                        val advance = stack.mallocInt(1)

                        stbtt_GetPackedQuad(cdata, FONT_BITMAP_W, FONT_BITMAP_H, codepoint - 32, x, y, q, false)
                        stbtt_GetCodepointHMetrics(fontInfo, codepoint, advance, null)

                        val ufg = NkUserFontGlyph.create(glyph)

                        ufg.width(q.x1() - q.x0())
                        ufg.height(q.y1() - q.y0())
                        ufg.offset()[q.x0()] = q.y0() + (FONT_HEIGHT + descent)
                        ufg.xadvance(advance.get(0) * scale)
                        ufg.uv(0)[q.s0()] = q.t0()
                        ufg.uv(1).set(q.s1(), q.t1())
                    }
                }
                .texture{ it.ptr(fontTexture.vkImage)}

            fontTexture.prepareStagingBuffer()
            fontTexture.loadPixels(bitmap)
            fontTextureView = fontTexture.view()
        }
    }

    private fun calcTextWidth(fontInfo: STBTTFontinfo, text: Long, len: Int, scale: Float): Float {
        var textWidth = 0f
        MemoryStack.stackPush().use { stack ->
            val unicode = stack.mallocInt(1)
            var glyphLen = nnk_utf_decode(text, memAddress(unicode), len)
            var textLen = glyphLen
            if (glyphLen == 0) {
                return@use
            }
            val advance = stack.mallocInt(1)
            while (textLen <= len && glyphLen != 0) {
                if (unicode[0] == NK_UTF_INVALID) {
                    break
                }
                // Currently drawn glyph info
                stbtt_GetCodepointHMetrics(fontInfo, unicode[0], advance, null)
                textWidth += advance[0] * scale
                // Offset next glyph
                glyphLen = nnk_utf_decode(text + textLen, memAddress(unicode), len - textLen)
                textLen += glyphLen
            }
        }
        return textWidth
    }

    private fun createTextureSampler(stack: MemoryStack): Long {
        val lp = stack.mallocLong(1)
        val samplerCI = VkSamplerCreateInfo.calloc(stack)
            .`sType$Default`()
            .magFilter(VK10.VK_FILTER_NEAREST)
            .minFilter(VK10.VK_FILTER_NEAREST)
            .mipmapMode(VK10.VK_SAMPLER_MIPMAP_MODE_NEAREST)
            .addressModeU(VK10.VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .addressModeV(VK10.VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
            .addressModeW(VK10.VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
        VulkanUtils.vkCheck(
            VK10.vkCreateSampler(device.vkDevice, samplerCI, null, lp),
            "Failed to create sampler"
        )
        return lp[0]
    }

    fun cleanup() {
        vkDestroySampler(device.vkDevice, vkSampler, null)
        fontTextureView.cleanup()
        fontTexture.cleanup()
        vertices.cleanup()
        elements.cleanup()
        nullTexture.free()
        defaultFont.free()
        nk_buffer_free(cmds)
        projMatDescriptorSetLayout.cleanup()
        textureDescriptorSetLayout.cleanup()
        vkDestroyDescriptorPool(device.vkDevice, vkDescriptorPool, null)
    }


    companion object {
        private const val DEFAULT_FONT = "resources/fonts/FiraSans-Regular.otf"
        private const val FONT_BITMAP_W = 1024
        private const val FONT_BITMAP_H = 1024
        private const val MAX_VERTEX_BUFFER = 512 * 1024L
        private const val MAX_ELEMENT_BUFFER = 128 * 1024L
        private const val MAX_TEXTURES = 128

        private val VERTEX_LAYOUT: NkDrawVertexLayoutElement.Buffer = NkDrawVertexLayoutElement.create(4)
            .position(0).attribute(Nuklear.NK_VERTEX_POSITION).format(Nuklear.NK_FORMAT_FLOAT).offset(0)
            .position(1).attribute(Nuklear.NK_VERTEX_TEXCOORD).format(Nuklear.NK_FORMAT_FLOAT).offset(8)
            .position(2).attribute(Nuklear.NK_VERTEX_COLOR).format(Nuklear.NK_FORMAT_R8G8B8A8).offset(16)
            .position(3).attribute(Nuklear.NK_VERTEX_ATTRIBUTE_COUNT).format(Nuklear.NK_FORMAT_COUNT).offset(0)
            .flip();
        private val ALLOCATOR: NkAllocator = NkAllocator.create()
            .alloc { handle: Long, old: Long, size: Long ->
                MemoryUtil.nmemAllocChecked(
                    size
                )
            }
            .mfree { handle: Long, ptr: Long -> MemoryUtil.nmemFree(ptr) }

        fun createDescriptorPool(stack: MemoryStack, device: Device): Long {
            val poolSizes = VkDescriptorPoolSize.calloc(2, stack)
            poolSizes[0]
                .type(VK10.VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                .descriptorCount(1)
            poolSizes[1]
                .type(VK10.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                .descriptorCount(MAX_TEXTURES)

            val descriptorPoolCI = VkDescriptorPoolCreateInfo.calloc(stack)
                .`sType$Default`()
                .maxSets(MAX_TEXTURES + 1)
                .pPoolSizes(poolSizes)
            val lp = stack.mallocLong(1)
            VulkanUtils.vkCheck(
                VK10.vkCreateDescriptorPool(device.vkDevice, descriptorPoolCI, null, lp),
                "Failed to create descriptor pool"
            )
            return lp[0]
        }
    }

    override fun prepareRecord() {
    }

    override fun recordCommands(cmdBuf: VkCommandBuffer) {
        fontTexture.recordCommands(cmdBuf)
    }

    override fun finalizeAfterSubmit() {
        fontTexture.finalizeAfterSubmit()
    }
}