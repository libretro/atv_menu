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

/* structs */
struct atv_icon
{
   struct nk_image normal;
   struct nk_image selected;
   struct nk_image hover;
};

struct atv_font
{
   struct nk_font *font;
   int    height;
};

/* enums */
enum atv_color_enum {
    NK_COLOR_TEXT_HOVER,
    NK_COLOR_TEXT_ACTIVE,
    NK_COLOR_CUSTOM_COUNT,
};

/* globals */
struct nk_color atv_colors[NK_COLOR_COUNT];
struct nk_color atv_colors_custom[NK_COLOR_CUSTOM_COUNT];

struct atv_icon sidebar_icons[10];
struct atv_font fonts[7];

/* helpers */
static void set_style(struct nk_context *ctx)
{
   atv_colors[NK_COLOR_TEXT] = nk_rgba(158, 158, 158, 255);
   atv_colors[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
   atv_colors[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
   atv_colors[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
   atv_colors[NK_COLOR_BUTTON] = nk_rgba(255, 112, 67, 255);
   atv_colors[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
   atv_colors[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
   atv_colors[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
   atv_colors[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
   atv_colors[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
   atv_colors[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
   atv_colors[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
   atv_colors[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
   atv_colors[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
   atv_colors[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
   atv_colors[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
   atv_colors[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
   atv_colors[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
   atv_colors[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
   atv_colors[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
   atv_colors[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
   atv_colors[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
   atv_colors[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
   atv_colors[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
   atv_colors[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
   atv_colors[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
   atv_colors[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
   atv_colors[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
   nk_style_from_table(ctx, atv_colors);
   atv_colors_custom[NK_COLOR_TEXT_HOVER] = nk_rgba(255, 255, 255, 255);
   atv_colors_custom[NK_COLOR_TEXT_ACTIVE] = nk_rgba(0, 0, 0, 255);
   nk_style_set_font(ctx, &fonts[0].font->handle);
   ctx->style.button.text_alignment = NK_TEXT_ALIGN_CENTERED;
}

static struct nk_image icon_load(const char *filename)
{
   int x,y,n;
   GLuint tex;
   unsigned char *data = stbi_load(filename, &x, &y, &n, 0);

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
   glGenerateMipmap(GL_TEXTURE_2D);
   stbi_image_free(data);
   return nk_image_id((int)tex);
}

static bool atv_icon_load(struct atv_icon *icon, const char *filename)
{
   char buf[256];
   fflush(stdout);
   snprintf(buf, sizeof(buf), "png/%s_idle.png", filename);
   icon->normal = icon_load(buf);
   snprintf(buf, sizeof(buf), "png/%s_active.png", filename);
   icon->selected = icon_load(buf);
   snprintf(buf, sizeof(buf), "png/%s_hover.png", filename);
   icon->hover = icon_load(buf);
   fflush(stdout);
}

/* widgets */
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
   bool image, struct atv_font* f, void *data)
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


int sidebar_button_text(struct nk_context *ctx, struct atv_icon img,
    const char *text, int len, nk_flags align);

void sidebar_draw_button_text_image(struct nk_command_buffer *out,
    const struct nk_rect *bounds, const struct nk_rect *label,
    const struct nk_rect *image, nk_flags state, const struct nk_style_button *style,
    const char *str, int len, const struct nk_user_font *font,
    struct atv_icon *icon)
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
        text.text = atv_colors_custom[NK_COLOR_TEXT_HOVER];
        img = &icon->hover;
    }
    else if (state & NK_WIDGET_STATE_ACTIVED)
    {
        text.text = style->text_active;
        text.text = atv_colors_custom[NK_COLOR_TEXT_ACTIVE];
        img = &icon->selected;
    }
    else text.text = style->text_normal;

    text.padding = nk_vec2(font->height * 1.5 ,0);
    nk_widget_text(out, *label, str, len, &text, NK_TEXT_LEFT, font);
    nk_draw_image(out, *image, img, nk_white);
}

int sidebar_do_button_text_styled(nk_flags *state,
    struct nk_command_buffer *out, struct nk_rect bounds,
    struct atv_icon img, const char* str, int len, nk_flags align,
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
    const struct nk_style_button *style, struct atv_icon img, const char *text,
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

int sidebar_button_text(struct nk_context *ctx, struct atv_icon img,
    const char *text, int len, nk_flags align)
{return sidebar_button_text_styled(ctx, &ctx->style.button, img, text, len, align);}
