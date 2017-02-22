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
   fflush(stdout);
   snprintf(buf, sizeof(buf), "png/%s_idle.png", filename);
   icon->normal = icon_load(buf);
   snprintf(buf, sizeof(buf), "png/%s_active.png", filename);
   icon->selected = icon_load(buf);
   snprintf(buf, sizeof(buf), "png/%s_hover.png", filename);
   icon->hover = icon_load(buf);
   fflush(stdout);
}

void set_style(struct nk_context *ctx, enum theme theme)
{
   if (theme == THEME_BLUE) 
   {
      table[NK_COLOR_TEXT] = nk_rgba(158, 158, 158, 255);
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
      colors_custom[NK_COLOR_TEXT_HOVER] = nk_rgba(255, 255, 255, 255);
      colors_custom[NK_COLOR_TEXT_ACTIVE] = nk_rgba(0, 0, 0, 255);
   }
   else 
   {
      nk_style_default(ctx);
   }
   
   nk_style_set_font(ctx, &fonts[0].font->handle);
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
   win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "RetroArch", NULL, NULL);
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

   mui_icon_load(&sidebar_icons[0], "search_fab");
   mui_icon_load(&sidebar_icons[1], "history");
   mui_icon_load(&sidebar_icons[2], "list");
   mui_icon_load(&sidebar_icons[3], "folder");
   mui_icon_load(&sidebar_icons[4], "settings");
   //sidebar_icons[0].normal = icon_load("png/search_hover.png");

   ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);
   /* Load Fonts: if none of these are loaded a default font will be used  */
   /* Load Cursor: if you uncomment cursor loading please hide the cursor */
   {
      struct nk_font_atlas *atlas;
      nk_glfw3_font_stash_begin(&atlas);
      for (int i = 0 ; i < 7; i++)
      {
         fonts[i].font = nk_font_atlas_add_from_file(atlas, "fonts/DroidSans.ttf", (i + 1) * 8, 0);
         fonts[i].height = (i + 1) * 8;
      }

      nk_glfw3_font_stash_end();
      nk_style_set_font(ctx, &fonts[0].font->handle);
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
         ctx->style.button.normal = nk_style_item_color(nk_rgba(0,0,0,0));
         ctx->style.button.hover = nk_style_item_color(nk_rgba(0,0,0,0));
         ctx->style.button.active = nk_style_item_color(nk_rgba(0,0,0,0));
         ctx->style.button.border_color = nk_rgba(0,0,0,0);
         ctx->style.button.text_alignment = NK_TEXT_ALIGN_LEFT;

         sidebar_placeholder(ctx);
         sidebar_spacer(ctx, 32);
         sidebar_row(ctx, 0, "", true, &fonts[6], (void*)test);
         sidebar_spacer(ctx, 32);
         sidebar_row(ctx, 1, "History", true, &fonts[3], (void*)test);
         sidebar_row(ctx, 2, "Collections", true, &fonts[3], (void*)test);
         sidebar_row(ctx, 3, "File Browser", true, &fonts[3], (void*)test);
         sidebar_spacer(ctx, 32);
         sidebar_row(ctx, 4, "Settings", true, &fonts[3], (void*)test);

         reset_style(ctx);
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

