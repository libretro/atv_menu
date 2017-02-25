/* Copyright (C) 2010-2017 The RetroArch team
 * Copyright (C) 2016 - Andrés Suárez
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this menu driver (atv_menu).
 * ---------------------------------------------------------------------------------------
 * MIT License
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


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

void test(void)
{
   printf("test");
   fflush(stdout);
}

void menu_entry_cb(struct atv_sidebar_entry entry)
{
   printf("Do something with this: %s\n", entry.label);
   fflush(stdout);
}

static void error_callback(int e, const char *d)
{printf("Error %d: %s\n", e, d);}

int main(void)
{
   static GLFWwindow *win;
   int width = 0, height = 0;
   struct nk_context *ctx;
   struct nk_color background;
   static int frames = 0;
   static int active = 0;
   glfwSwapInterval(1);

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

   sidebar_data_load();

   ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);
   /* Load Fonts: if none of these are loaded a default font will be used  */
   /* Load Cursor: if you uncomment cursor loading please hide the cursor */
   {
      struct nk_font_atlas *atlas;
      nk_glfw3_font_stash_begin(&atlas);
      for (int i = 0 ; i < 7; i++)
      {
         fonts[i].font = nk_font_atlas_add_from_file(atlas, 
            "fonts/DroidSans.ttf", (i + 1) * 8, 0);
         fonts[i].height = (i + 1) * 8;
      }

      nk_glfw3_font_stash_end();
      nk_style_set_font(ctx, &fonts[0].font->handle);
   }

   set_style(ctx);

   int  sidebar_width = WINDOW_WIDTH * 20 / 100;
   int  content_view_width = WINDOW_WIDTH * 80 / 100 - 1;
   int  content_title_height = WINDOW_HEIGHT * 10 / 100;
   int  content_view_position_x = sidebar_width + 1;
   int  content_view_position_y = content_title_height + 1;

   background = nk_rgb(38, 50, 56);
    while (!glfwWindowShouldClose(win))
    {
      frames ++;
      /* Input */
      glfwPollEvents();
      nk_glfw3_new_frame();

      ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(38, 50, 56, 255));
      nk_begin(ctx, "Header", nk_rect(WINDOW_WIDTH * 20 / 100 + 1, 0, WINDOW_WIDTH * 80 / 100 - 1, content_title_height), 0);
      { 
         set_style(ctx);
         ctx->style.button.normal = nk_style_item_color(atv_colors[NK_COLOR_WINDOW]);
         ctx->style.button.hover  = nk_style_item_color(atv_colors[NK_COLOR_BUTTON_HOVER]);
         ctx->style.button.active = nk_style_item_color(atv_colors[NK_COLOR_BUTTON]);
         ctx->style.button.border_color = nk_rgba(0,0,0,0);
         ctx->style.button.text_alignment = NK_TEXT_ALIGN_LEFT;

         content_title(ctx, "History", &fonts[6]);
         set_style(ctx);
      }
      nk_end(ctx);

      ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(38, 50, 56, 255));
      nk_begin(ctx, "History", nk_rect(content_view_position_x, content_view_position_y, content_view_width, WINDOW_HEIGHT - content_title_height), 0);
      { 
         set_style(ctx);
         ctx->style.button.normal = nk_style_item_color(atv_colors[NK_COLOR_WINDOW]);
         ctx->style.button.hover  = nk_style_item_color(atv_colors[NK_COLOR_BUTTON_HOVER]);
         ctx->style.button.active = nk_style_item_color(atv_colors[NK_COLOR_BUTTON]);
         ctx->style.button.border_color = nk_rgba(0,0,0,0);
         ctx->style.button.text_alignment = NK_TEXT_ALIGN_LEFT;

         {
            int items = content_view_width / 260;
            nk_layout_row_static(ctx, 260, 280, items);
            content_entry(ctx, "Super Metroid", "Super Nintendo Entertainment System", &fonts[2], &fonts[1], test_entry, test);
            content_entry(ctx, "Label 2", "Sublabel 2",  &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 3", "Sublabel 3",  &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 4", "Sublabel 4",  &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 5", "Sublabel 5",  &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 6", "Sublabel 6",  &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 7", "Sublabel 7",  &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 8", "Sublabel 8",  &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 9", "Sublabel 9",  &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 10", "Sublabel 10", &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 11", "Sublabel 11", &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 12", "Sublabel 12", &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 13", "Sublabel 13", &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 14", "Sublabel 14", &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 15", "Sublabel 15", &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 16", "Sublabel 16", &fonts[2], &fonts[1], color_bars, test);
            content_entry(ctx, "Label 17", "Sublabel 17", &fonts[2], &fonts[1], color_bars, test);
            set_style(ctx);
         }
      }
      nk_end(ctx);
      static bool lock_keys = false;
      nk_begin(ctx, "Sidebar", nk_rect(0, 0, sidebar_width, WINDOW_HEIGHT), 0);
      {
         /* no borders, and no selection colors for the sidebar */
         ctx->style.button.normal = nk_style_item_color(nk_rgba(0,0,0,0));
         ctx->style.button.hover  = nk_style_item_color(nk_rgba(0,0,0,0));
         ctx->style.button.active = nk_style_item_color(nk_rgba(0,0,0,0));
         ctx->style.button.border_color = nk_rgba(0,0,0,0);
         ctx->style.button.text_alignment = NK_TEXT_ALIGN_LEFT;

         sidebar_spacer(ctx, 8);
         for (int i=0; i < menu_entries.count; i++)
         {
            sidebar_entry_widget(ctx, menu_entries.entry[i], active, menu_entries.offset, menu_entry_cb);
            if (menu_entries.entry[i].spacer)
               sidebar_spacer(ctx, 16);
         }
         for (int i=0; i < playlist_entries.count; i++)
         {
            sidebar_entry_widget(ctx, playlist_entries.entry[i], active, playlist_entries.offset, menu_entry_cb);
            if (playlist_entries.entry[i].spacer)
               sidebar_spacer(ctx, 16);
         }
         set_style(ctx);

         const struct nk_input *in = &ctx->input;

         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_UP))
         {
            active = MAX(0, active - 1);
            lock_keys = true;
         }
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_DOWN))
         {
            active = MIN(active+1, menu_entries.count + playlist_entries.count - 1);
            lock_keys = true;
         }
      }
      if (frames % 16 == 0)
         lock_keys = false;
      nk_end(ctx);

      const int delta = 60;
      if (nk_window_is_active(ctx, "History"))
      {
         if (sidebar_width > 15 )
         {
            sidebar_width -= delta;
            content_view_width += delta;
            content_view_position_x = sidebar_width + 1;
         }
         else if (sidebar_width < 15)
         {
            sidebar_width += 1;
            content_view_width -= 1;
            content_view_position_x = sidebar_width + 1;
         }
      }
      else
      {
         if (sidebar_width < WINDOW_WIDTH * 20 / 100)
         {
            sidebar_width += delta;
            content_view_width -= delta;
            content_view_position_x = sidebar_width + 1;
         }
         else if (sidebar_width > WINDOW_WIDTH * 20 / 100)
         {
            sidebar_width -= 1;
            content_view_width += 1;
            content_view_position_x = sidebar_width + 1;
         }
      }

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

