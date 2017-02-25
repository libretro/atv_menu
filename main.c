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

void menu_entry_cb(struct atv_menu_entry *entry)
{
   printf("Do something with this: %s\n", entry->label);
   fflush(stdout);
}

void playlist_entry_cb(struct atv_menu_entry *entry)
{
   printf("Do something with this: %s\n", entry->label);
   fflush(stdout);
}

void history_entry_cb(struct atv_content_entry *entry)
{
   printf("Do something with this: %s\n", entry->label);
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
   static int sidebar_current_id = 1;
   static int content_current_id = 0;
   static bool lock_keys = false;
   static int items = 0;
   static int content_entries = 0;

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

   dummy_data_load();

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
         items = content_view_width / 280;
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
      nk_begin(ctx, "Content", nk_rect(content_view_position_x, content_view_position_y, content_view_width, WINDOW_HEIGHT - content_title_height), 0);
      { 
         set_style(ctx);
         ctx->style.button.normal = nk_style_item_color(atv_colors[NK_COLOR_WINDOW]);
         ctx->style.button.hover  = nk_style_item_color(atv_colors[NK_COLOR_BUTTON_HOVER]);
         ctx->style.button.active = nk_style_item_color(atv_colors[NK_COLOR_BUTTON]);
         ctx->style.button.border_color = nk_rgba(0,0,0,0);
         ctx->style.button.text_alignment = NK_TEXT_ALIGN_LEFT;
         {
            nk_layout_row_static(ctx, 220, 280, items);
            for (int i=0; i < history_entries.count; i++)
               content_entry_widget(ctx, &history_entries.entry[i], content_current_id, items, history_entry_cb);
            set_style(ctx);
         }
         content_entries = history_entries.count;
      }
      nk_end(ctx);
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
            sidebar_entry_widget(ctx, &menu_entries.entry[i], sidebar_current_id, menu_entries.offset, menu_entry_cb);
            if (menu_entries.entry[i].spacer)
               sidebar_spacer(ctx, 16);
         }
         for (int i=0; i < playlist_entries.count; i++)
         {
            sidebar_entry_widget(ctx, &playlist_entries.entry[i], sidebar_current_id, playlist_entries.offset, playlist_entry_cb);
            if (playlist_entries.entry[i].spacer)
               sidebar_spacer(ctx, 16);
         }
         set_style(ctx);
      }
      nk_end(ctx);

      const struct nk_input *in = &ctx->input;
      const int delta = 60;
      if (nk_window_is_active(ctx, "Content"))
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
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_LEFT))
         {
            if (content_current_id % items == 0)
               nk_window_set_focus(ctx, "Sidebar");
            if (content_current_id >= 1)
               content_current_id -= 1;
            lock_keys = true;
         }
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_RIGHT))
         {
            if (content_current_id < content_entries)
               content_current_id += 1;
            else
               content_current_id = 0;
            lock_keys = true;
         }
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_UP))
         {
            if (content_current_id >= items)
               content_current_id -= items;
            lock_keys = true;
         }
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_DOWN))
         {
            if (content_current_id + items < content_entries)
               content_current_id += items;
            lock_keys = true;
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
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_UP))
         {
            sidebar_current_id = MAX(0, sidebar_current_id - 1);
            lock_keys = true;
         }
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_DOWN))
         {
            sidebar_current_id = MIN(sidebar_current_id + 1, menu_entries.count + playlist_entries.count - 1);
            lock_keys = true;
         }
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_RIGHT))
         {
            nk_window_set_focus(ctx, "Content");
            printf("help!!!");
            fflush(stdout);
            lock_keys = true;
         }
      }
      if (frames % 16 == 0)
         lock_keys = false;

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

