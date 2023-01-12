package gfx.gui

import gfx.vk.Device
import gfx.vk.ImageView
import gfx.vk.VulkanTexture
import org.lwjgl.nuklear.NkUserFont
import org.lwjgl.nuklear.NkUserFontGlyph
import org.lwjgl.nuklear.Nuklear
import org.lwjgl.stb.*
import org.lwjgl.system.MemoryStack
import org.lwjgl.system.MemoryUtil
import org.lwjgl.vulkan.VK10
import org.lwjgl.vulkan.VK10.VK_FORMAT_R8_UINT
import java.io.File

class NuklearFont(private val device: Device, val path: String, val fontHeight: Float, val bitmapW: Int, val bitmapH: Int) {
    val nkFont = NkUserFont.create()
    private var fontTexture: VulkanTexture = VulkanTexture(
        device, bitmapW, bitmapH, 1,
        VK_FORMAT_R8_UINT, 1, 1, VK10.VK_SAMPLE_COUNT_1_BIT, VK10.VK_IMAGE_TILING_OPTIMAL
    )
    private var fontTextureView: ImageView = fontTexture.view()

    init {
        MemoryStack.stackPush().use { stack ->
            val fontInfo = STBTTFontinfo.create()
            val cdata = STBTTPackedchar.create(95)
            // Read the font
            val ttfBytes = File(path).readBytes()
            val ttf = stack.malloc(ttfBytes.size)
            ttf.put(ttfBytes)
            ttf.flip()
            // Initialize
            STBTruetype.stbtt_InitFont(fontInfo, ttf)
            val scale = STBTruetype.stbtt_ScaleForPixelHeight(fontInfo, fontHeight)

            val d = stack.mallocInt(1)
            STBTruetype.stbtt_GetFontVMetrics(fontInfo, null, d, null)
            val descent = d[0] * scale

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
            STBTruetype.stbtt_PackFontRange(pc, ttf, 0, fontHeight, 32, cdata)
            STBTruetype.stbtt_PackEnd(pc)

            nkFont
                .width { _, _, text, len -> calcTextWidth(fontInfo, text, len, scale) }
                .height(fontHeight)
                .query { handle, font_height, glyph, codepoint, next_codepoint ->
                    MemoryStack.stackPush().use { stack ->
                        val x = stack.floats(0.0f)
                        val y = stack.floats(0.0f)

                        val q = STBTTAlignedQuad.malloc(stack)
                        val advance = stack.mallocInt(1)

                        STBTruetype.stbtt_GetPackedQuad(
                            cdata,
                            bitmapW,
                            bitmapH,
                            codepoint - 32,
                            x,
                            y,
                            q,
                            false
                        )
                        STBTruetype.stbtt_GetCodepointHMetrics(fontInfo, codepoint, advance, null)

                        val ufg = NkUserFontGlyph.create(glyph)

                        ufg.width(q.x1() - q.x0())
                        ufg.height(q.y1() - q.y0())
                        ufg.offset()[q.x0()] = q.y0() + (fontHeight + descent)
                        ufg.xadvance(advance.get(0) * scale)
                        ufg.uv(0)[q.s0()] = q.t0()
                        ufg.uv(1).set(q.s1(), q.t1())
                    }
                }
                .texture{ it.ptr(fontTexture.vkImage)}
            fontTexture.prepareUpdate()
            fontTexture.loadPixels(bitmap)
        }

    }
    fun cleanup() {
        fontTextureView.cleanup()
        fontTexture.cleanup()
        nkFont.free()
    }
    private fun calcTextWidth(fontInfo: STBTTFontinfo, text: Long, len: Int, scale: Float): Float {
        var textWidth = 0f
        MemoryStack.stackPush().use { stack ->
            val unicode = stack.mallocInt(1)
            var glyphLen = Nuklear.nnk_utf_decode(text, MemoryUtil.memAddress(unicode), len)
            var textLen = glyphLen
            if (glyphLen == 0) {
                return@use
            }
            val advance = stack.mallocInt(1)
            while (textLen <= len && glyphLen != 0) {
                if (unicode[0] == Nuklear.NK_UTF_INVALID) {
                    break
                }
                // Currently drawn glyph info
                STBTruetype.stbtt_GetCodepointHMetrics(fontInfo, unicode[0], advance, null)
                textWidth += advance[0] * scale
                // Offset next glyph
                glyphLen = Nuklear.nnk_utf_decode(text + textLen, MemoryUtil.memAddress(unicode), len - textLen)
                textLen += glyphLen
            }
        }
        return textWidth
    }
}