package gfx.gui

import gfx.vk.Device
import gfx.vk.ImageView
import gfx.vk.UploadActivity
import gfx.vk.VulkanTexture
import org.lwjgl.nuklear.NkUserFont
import org.lwjgl.nuklear.NkUserFontGlyph
import org.lwjgl.nuklear.Nuklear.*
import org.lwjgl.stb.*
import org.lwjgl.stb.STBTruetype.stbtt_GetCodepointHMetrics
import org.lwjgl.stb.STBTruetype.stbtt_GetPackedQuad
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.vulkan.VK10.*
import org.lwjgl.vulkan.VkCommandBuffer
import java.io.File
import java.nio.ByteBuffer

class NuklearFont(
    private val device: Device,
    val path: String,
    val fontHeight: Float,
    val bitmapW: Int,
    val bitmapH: Int
) : UploadActivity {
    val nkFont: NkUserFont = NkUserFont.create()
    private var fontTexture: VulkanTexture = VulkanTexture(
        device, bitmapW, bitmapH, 1,
        VK_FORMAT_R8_UINT, 1, 1,
        VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL
    )
    private var fontTextureView: ImageView = fontTexture.image.view()
    private val fontInfo = STBTTFontinfo.malloc()
    private val charData = STBTTPackedchar.malloc(95)

    init {
        MemoryStack.stackPush().use { stack ->
            // Read the font
            val ttfBytes = File(path).readBytes()
            val ttf = stack.malloc(ttfBytes.size)
            ttf.put(ttfBytes)
            ttf.flip()
            // Initialize
            STBTruetype.stbtt_InitFont(fontInfo, ttf)
            val scale = STBTruetype.stbtt_ScaleForPixelHeight(fontInfo, fontHeight)
            val pDescent = stack.mallocInt(1)
            STBTruetype.stbtt_GetFontVMetrics(fontInfo, null, pDescent, null)
            val descent = pDescent[0] * scale
            // Pack font into bitmap
            val bitmap = packFont(stack, ttf)
            fontTexture.prepareStagingBuffer()
            fontTexture.loadPixels(bitmap)
            nkFont
                .width { _, _, text, len -> calcTextWidth(fontInfo, text, len, scale) }
                .height(fontHeight)
                .query { _, _, glyph, codepoint, _ -> fontQuery(codepoint, glyph, descent, scale) }
                .texture { it.ptr(fontTextureView.vkImageView) }
        }

    }

    override fun recordUpload(cmdBuf: VkCommandBuffer) {
        fontTexture.recordUpload(cmdBuf)
    }

    override fun finalizeAfterUpload() {
        fontTexture.finalizeAfterUpload()
    }

    private fun packFont(stack: MemoryStack, ttf: ByteBuffer): ByteBuffer {
        val bitmap = stack.malloc(bitmapW * bitmapH)
        val pc = STBTTPackContext.malloc(stack)
        STBTruetype.stbtt_PackBegin(
            pc,
            bitmap,
            bitmapW,
            bitmapH,
            0,
            1,
            MemoryUtil.NULL
        )
        STBTruetype.stbtt_PackSetOversampling(pc, 4, 4)
        STBTruetype.stbtt_PackFontRange(pc, ttf, 0, fontHeight, FIRST_CHAR, charData)
        STBTruetype.stbtt_PackEnd(pc)
        return bitmap
    }

    fun cleanup() {
        fontTextureView.cleanup()
        fontTexture.cleanup()
        fontInfo.free()
        charData.free()
        nkFont.free()
    }

    private fun fontQuery(codepoint: Int, pGlyph: Long, descent: Float, scale: Float) {
        MemoryStack.stackPush().use { stack ->
            val x = stack.floats(0.0f)
            val y = stack.floats(0.0f)

            val q = STBTTAlignedQuad.malloc(stack)
            val advance = stack.mallocInt(1)

            stbtt_GetPackedQuad(charData,
                bitmapW, bitmapH,
                codepoint - FIRST_CHAR,
                x, y,
                q, false
            )
            stbtt_GetCodepointHMetrics(fontInfo, codepoint, advance, null)
            // Fill the glyph structure
            val glyph = NkUserFontGlyph.create(pGlyph)
            glyph.width(q.x1() - q.x0())
            glyph.height(q.y1() - q.y0())
            glyph.offset()[q.x0()] = q.y0() + (fontHeight + descent)
            glyph.xadvance(advance.get(0) * scale)
            glyph.uv(0)[q.s0()] = q.t0()
            glyph.uv(1).set(q.s1(), q.t1())
        }
    }

    companion object {
        const val FIRST_CHAR = 32
    }
}

private fun calcTextWidth(fontInfo: STBTTFontinfo, pText: Long, len: Int, scale: Float): Float {
    var textWidth = 0f
    MemoryStack.stackPush().use { stack ->
        // Decode unicode glyphs one by one and accumulate their widths
        val textBuf = MemoryUtil.memByteBuffer(pText, len)
        while (textBuf.hasRemaining()) {
            // Decode unicode glyph
            val unicode = stack.mallocInt(1)
            val glyphSize = nk_utf_decode(textBuf, unicode)
            if (unicode[0] == NK_UTF_INVALID || glyphSize == 0) {
                break
            }
            // Text width
            val advanceWidth = stack.mallocInt(1)
            stbtt_GetCodepointHMetrics(fontInfo, unicode[0], advanceWidth, null)
            // Accumulate and next
            textWidth += advanceWidth[0] * scale
            textBuf.position(textBuf.position() + glyphSize)
        }
    }
    return textWidth
}