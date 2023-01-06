#include "gui.h"
#include <nuklear.h>
#include <vulkan/vulkan.h>

#include "vk/device.h"
#include "vk/image.h"

typedef struct GuiContext {
    struct nk_context ctx;
    const Image fontAtlasImage;
    const Device* pDevice;
} GuiContext;

GuiContext newGuiContext(const Device* pDevice)
{
    GuiContext context;
    struct nk_font_atlas atlas;
    struct nk_font* font;
    const void* image;
    int w, h;
    nk_font_atlas_init_default(&atlas);
    font = nk_font_atlas_add_default(&atlas, 13, 0);
    image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    // TODO: upload to device
    nk_font_atlas_end(&atlas, nk_handle_id(0), NULL);
    nk_init_default(&context.ctx, &font->handle);
    return context;
}
