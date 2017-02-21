/* nuklear - 1.32.0 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "widgets.h"
#include "nuklear_glfw_gl3.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])

struct nk_font *font_16;
struct nk_font *font_24;
struct nk_font *font_32;
struct nk_font *font_40;
struct nk_font *font_48;
struct nk_font *font_56;

enum theme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK};


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

static bool mui_icon_load(struct mui_icon *icon, const char *filename)
{
   char buf[256];
   printf(buf, filename);
   fflush(stdout);
   snprintf(buf, sizeof(buf), "png/%s_white.png", filename);
   icon->normal = icon_load(buf);
   snprintf(buf, sizeof(buf), "png/%s_black.png", filename);
   icon->selected = icon_load(buf);
   snprintf(buf, sizeof(buf), "png/%s_grey.png", filename);
   icon->hover = icon_load(buf);
   printf("buffer: %s %s\n", filename, buf);
   fflush(stdout);
}

void set_style(struct nk_context *ctx, enum theme theme)
{
   if (theme == THEME_BLUE) 
   {
      table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
      table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
      table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
      table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
      table[NK_COLOR_BUTTON] = nk_rgba(255, 112, 67, 255);
      table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
      table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
      table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
      table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
      table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
      table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
      table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
      table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
      table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
      table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
      table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
      table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
      table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
      table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
      table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
      table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
      table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
      table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
      table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
      table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
      table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
      table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
      table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
      nk_style_from_table(ctx, table);
      colors_custom[NK_COLOR_TEXT_HOVER] = nk_rgba(255,213,79 ,255);
   }
   else 
   {
      nk_style_default(ctx);
   }
   nk_style_set_font(ctx, &font_16->handle);
   ctx->style.button.text_alignment = NK_TEXT_ALIGN_CENTERED;
   
}

static void reset_style(struct nk_context *ctx)
{
   set_style(ctx, THEME_BLUE);
}


void test()
{
   exit(1);
}

static int button_sidebar(struct nk_context *ctx, int img_idx, 
   char* label, bool image, struct nk_font* font, void *data)
{
   ctx->style.button.normal = nk_style_item_color(nk_rgba(0,0,0,0));
   ctx->style.button.border_color = nk_rgba(0,0,0,0);
   ctx->style.button.text_alignment = NK_TEXT_ALIGN_LEFT;
   nk_style_set_font(ctx, &font->handle);
   if (image)
   {
      if (sidebar_button_text(ctx, sidebar_icons[0], label, strlen(label), NK_TEXT_RIGHT))
         printf("hello");
   }
   else
   {
      if (nk_button_label(ctx, label))
         printf("hello");
   }
   reset_style(ctx);
}

static void button_placeholder(struct nk_context *ctx)
{
   ctx->style.button.normal = nk_style_item_color(nk_rgba(0,0,0,0));
   ctx->style.button.border_color = nk_rgba(0,0,0,0);
   nk_button_text(ctx, "", 0);
   reset_style(ctx);
}

static void sidebar_row(struct nk_context *ctx, int img_idx, char* label, 
   bool image, struct nk_font* font, void *data)
{
   nk_layout_row_begin(ctx, NK_DYNAMIC, 32, 3);
   nk_layout_row_end(ctx);
   nk_layout_row_begin(ctx, NK_DYNAMIC, 64, 3);
   nk_layout_row_push(ctx, 0.05f);
   button_placeholder(ctx);
   nk_layout_row_push(ctx, 0.9f);
   button_sidebar(ctx, img_idx, label, image, font, data);
   nk_layout_row_push(ctx, 0.05f);
   button_placeholder(ctx);
   nk_layout_row_end(ctx);
}

static void error_callback(int e, const char *d)
{printf("Error %d: %s\n", e, d);}

int main(void)
{
   static GLFWwindow *win;
   int width = 0, height = 0;
   struct nk_context *ctx;
   struct nk_color background;

   glfwSetErrorCallback(error_callback);
   if (!glfwInit())
   {
      fprintf(stdout, "[GFLW] failed to init!\n");
         exit(1);
   }
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
   win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Demo", NULL, NULL);
   glfwMakeContextCurrent(win);
   glfwGetWindowSize(win, &width, &height);

   /* OpenGL */
   glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
   glewExperimental = 1;
   if (glewInit() != GLEW_OK)
   {
      fprintf(stderr, "Failed to setup GLEW\n");
      exit(1);
   }

   mui_icon_load(&sidebar_icons[0], "search");
   //sidebar_icons[0].normal = icon_load("png/search_white.png");

   ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);
   /* Load Fonts: if none of these are loaded a default font will be used  */
   /* Load Cursor: if you uncomment cursor loading please hide the cursor */
   {
      struct nk_font_atlas *atlas;
      nk_glfw3_font_stash_begin(&atlas);
      font_16 = nk_font_atlas_add_from_file(atlas, "fonts/DroidSans.ttf", 16, 0);
      font_24 = nk_font_atlas_add_from_file(atlas, "fonts/DroidSans.ttf", 24, 0);
      font_32 = nk_font_atlas_add_from_file(atlas, "fonts/DroidSans.ttf", 32, 0);
      font_40 = nk_font_atlas_add_from_file(atlas, "fonts/DroidSans.ttf", 40, 0);
      font_48 = nk_font_atlas_add_from_file(atlas, "fonts/DroidSans.ttf", 48, 0);
      font_56 = nk_font_atlas_add_from_file(atlas, "fonts/DroidSans.ttf", 56, 0);
      nk_glfw3_font_stash_end();
      nk_style_set_font(ctx, &font_16->handle);
   }

   set_style(ctx, THEME_BLUE);

   background = nk_rgb(38, 50, 56);
    while (!glfwWindowShouldClose(win))
    {
      /* Input */
      glfwPollEvents();
      nk_glfw3_new_frame();

      /* sidebar */
      nk_begin(ctx, "Sidebar", nk_rect(0, 0, WINDOW_WIDTH * 30 / 100, WINDOW_HEIGHT),NULL);
      {
         sidebar_row(ctx, 0, "", false, font_32, (void*)test);
         sidebar_row(ctx, 0, "Search", true, font_56, (void*)test);
         sidebar_row(ctx, 0, "", false, font_32, (void*)test);
         sidebar_row(ctx, 0, "History", false, font_40, (void*)test);
         sidebar_row(ctx, 0, "Collections", false, font_40, (void*)test);
         sidebar_row(ctx, 0, "File Browser", false, font_40, (void*)test);
         sidebar_row(ctx, 0, "", false, font_32, (void*)test);
         sidebar_row(ctx, 0, "Settings", false, font_40, (void*)test);
      }
      nk_end(ctx);

      {
         float bg[4];
         nk_color_fv(bg, background);
         glfwGetWindowSize(win, &width, &height);
         glViewport(0, 0, width, height);
         glClear(GL_COLOR_BUFFER_BIT);
         glClearColor(bg[0], bg[1], bg[2], bg[3]);
         /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
          * with blending, scissor, face culling, depth test and viewport and
          * defaults everything back into a default state.
          * Make sure to either a.) save and restore or b.) reset your own state after
          * rendering the UI. */
         nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
         glfwSwapBuffers(win);
      }
   }
   nk_glfw3_shutdown();
   glfwTerminate();
   return 0;
}

