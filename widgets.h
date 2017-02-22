#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

struct mui_icon
{
   struct nk_image normal;
   struct nk_image selected;
   struct nk_image hover;
};

struct mui_font
{
   struct nk_font *font;
   int    height;
};

enum nk_style_colors_custom {
    NK_COLOR_TEXT_HOVER,
    NK_COLOR_TEXT_ACTIVE,
    NK_COLOR_CUSTOM_COUNT,
};

struct nk_color table[NK_COLOR_COUNT];
struct nk_color colors_custom[NK_COLOR_CUSTOM_COUNT];

struct mui_icon sidebar_icons[10];
struct mui_font fonts[7];

static int sidebar_button(struct nk_context *ctx, int img_idx, 
   char* label, bool image, struct nk_font* font, void *data)
{
   nk_style_set_font(ctx, &font->handle);
   if (image)
   {
      if (sidebar_button_text(ctx, sidebar_icons[img_idx], label, strlen(label), NK_TEXT_RIGHT))
         printf("hello");
   }
   else
   {
      if (nk_button_label(ctx, label))
         printf("hello");
   }
}

static void sidebar_placeholder(struct nk_context *ctx)
{
   ctx->style.button.normal = nk_style_item_color(nk_rgba(0,0,0,0));
   ctx->style.button.border_color = nk_rgba(0,0,0,0);
   nk_button_text(ctx, "", 0);
}

static void sidebar_spacer(struct nk_context *ctx, int height)
{
   nk_layout_row_begin(ctx, NK_DYNAMIC, height, 1);
   nk_layout_row_end(ctx);
   nk_layout_row_begin(ctx, NK_DYNAMIC, height, 1);
   nk_layout_row_push(ctx, 1.0f);
   sidebar_placeholder(ctx);
   nk_layout_row_end(ctx);
}

static void sidebar_row(struct nk_context *ctx, int img_idx, char* label, 
   bool image, struct mui_font* f, void *data)
{
   nk_layout_row_begin(ctx, NK_DYNAMIC, f->height, 1);
   nk_layout_row_end(ctx);
   nk_layout_row_begin(ctx, NK_DYNAMIC, f->height * 1.5f, 2);
   nk_layout_row_push(ctx, 0.05f);
   sidebar_placeholder(ctx);
   nk_layout_row_push(ctx, 0.95f);
   sidebar_button(ctx, img_idx, label, image, f->font, data);
   nk_layout_row_end(ctx);
}


int sidebar_button_text(struct nk_context *ctx, struct mui_icon img,
    const char *text, int len, nk_flags align);

void sidebar_draw_button_text_image(struct nk_command_buffer *out,
    const struct nk_rect *bounds, const struct nk_rect *label,
    const struct nk_rect *image, nk_flags state, const struct nk_style_button *style,
    const char *str, int len, const struct nk_user_font *font,
    struct mui_icon *icon)
{
    struct nk_text text;
    struct nk_image *img;
    const struct nk_style_item *background;
    background = nk_draw_button(out, bounds, state, style);
    img = &icon->normal;
    /* select correct colors */
    if (background->type == NK_STYLE_ITEM_COLOR)
        text.background = background->data.color;
    else text.background = style->text_background;
    if (state & NK_WIDGET_STATE_HOVER)
    {
        text.text = style->text_hover;
        text.text = colors_custom[NK_COLOR_TEXT_HOVER];
        img = &icon->hover;
    }
    else if (state & NK_WIDGET_STATE_ACTIVED)
    {
        text.text = style->text_active;
        text.text = colors_custom[NK_COLOR_TEXT_ACTIVE];
        img = &icon->selected;
    }
    else text.text = style->text_normal;

    text.padding = nk_vec2(font->height * 1.5 ,0);
    nk_widget_text(out, *label, str, len, &text, NK_TEXT_LEFT, font);
    nk_draw_image(out, *image, img, nk_white);
}

int sidebar_do_button_text_styled(nk_flags *state,
    struct nk_command_buffer *out, struct nk_rect bounds,
    struct mui_icon img, const char* str, int len, nk_flags align,
    enum nk_button_behavior behavior, const struct nk_style_button *style,
    const struct nk_user_font *font, const struct nk_input *in)
{
    int ret;
    struct nk_rect icon;
    struct nk_rect content;

    NK_ASSERT(style);
    NK_ASSERT(state);
    NK_ASSERT(font);
    NK_ASSERT(out);
    if (!out || !font || !style || !str)
        return nk_false;

    ret = nk_do_button(state, out, bounds, style, in, behavior, &content);
    icon.y = bounds.y + style->padding.y;
    icon.w = icon.h = bounds.h - 2 * style->padding.y;
    if (align & NK_TEXT_ALIGN_LEFT) {
        icon.x = (bounds.x + bounds.w) - (2 * style->padding.x + icon.w);
        icon.x = NK_MAX(icon.x, 0);
    } else icon.x = bounds.x + 2 * style->padding.x;

    icon.x += style->image_padding.x;
    icon.y += style->image_padding.y;
    icon.w -= 2 * style->image_padding.x;
    icon.h -= 2 * style->image_padding.y;

    if (style->draw_begin) style->draw_begin(out, style->userdata);
    sidebar_draw_button_text_image(out, &bounds, &content, &icon, *state, style, str, len, font, &img);
    if (style->draw_end) style->draw_end(out, style->userdata);
    return ret;
}

int sidebar_button_text_styled(struct nk_context *ctx,
    const struct nk_style_button *style, struct mui_icon img, const char *text,
    int len, nk_flags align)
{
    struct nk_window *win;
    struct nk_panel *layout;
    const struct nk_input *in;

    struct nk_rect bounds;
    enum nk_widget_layout_states state;

    NK_ASSERT(ctx);
    NK_ASSERT(ctx->current);
    NK_ASSERT(ctx->current->layout);
    if (!ctx || !ctx->current || !ctx->current->layout)
        return 0;

    win = ctx->current;
    layout = win->layout;

    state = nk_widget(&bounds, ctx);
    if (!state) return 0;
    in = (state == NK_WIDGET_ROM || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
    return sidebar_do_button_text_styled(&ctx->last_widget_state, &win->buffer,
            bounds, img, text, len, align, ctx->button_behavior,
            style, ctx->style.font, in);
}

int sidebar_button_text(struct nk_context *ctx, struct mui_icon img,
    const char *text, int len, nk_flags align)
{return sidebar_button_text_styled(ctx, &ctx->style.button, img, text, len, align);}
