/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2017 - Andrés Suárez
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
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

static bool content_active, sidebar_active, favorites_active, 
            recent_active, folders_active, netplay_active, settings_active,
            details_active, dialog_active, game_active = false;

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

void favorites_entry_cb(struct atv_content_entry *entry)
{
   printf("Do something with this: %s\n", entry->label);
   details_active = true;
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
   static bool activate = false;


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
      nk_begin(ctx, "header", nk_rect(WINDOW_WIDTH * 20 / 100 + 1, 0, WINDOW_WIDTH * 80 / 100 - 1, content_title_height), 0);
      { 
         set_style(ctx);
         ctx->style.button.normal = nk_style_item_color(atv_colors[NK_COLOR_WINDOW]);
         ctx->style.button.hover  = nk_style_item_color(atv_colors[NK_COLOR_BUTTON_HOVER]);
         ctx->style.button.active = nk_style_item_color(atv_colors[NK_COLOR_BUTTON]);
         ctx->style.button.border_color = nk_rgba(0,0,0,0);
         ctx->style.button.text_alignment = NK_TEXT_ALIGN_LEFT;

         if (nk_window_is_active(ctx, "content"))
            content_title(ctx, menu_entries.entry[sidebar_current_id].label, &fonts[6], NK_TEXT_ALIGN_RIGHT);
         else
            content_title(ctx, "RetroArch", &fonts[6], NK_TEXT_ALIGN_RIGHT);
         set_style(ctx);
      }
      nk_end(ctx);

      ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(38, 50, 56, 255));

      nk_begin(ctx, "content", nk_rect(content_view_position_x, content_view_position_y, content_view_width, WINDOW_HEIGHT - content_title_height), 0);
      { 
            content_entries = 0;
            set_style(ctx);
            ctx->style.button.normal = nk_style_item_color(atv_colors[NK_COLOR_WINDOW]);
            ctx->style.button.hover  = nk_style_item_color(atv_colors[NK_COLOR_BUTTON_HOVER]);
            ctx->style.button.active = nk_style_item_color(atv_colors[NK_COLOR_BUTTON]);
            ctx->style.button.border_color = nk_rgba(0,0,0,0);
            ctx->style.button.text_alignment = NK_TEXT_ALIGN_LEFT;
            ctx->style.button.rounding = 0;

            if (favorites_active || (sidebar_active && recent_active))
            {
               int col = 0;
               if (sidebar_active)
               {
                  content_subtitle(ctx, "Favorites", &fonts[4], NK_TEXT_ALIGN_LEFT);
                  items = 8;
               }
               else
                  items = content_view_width / 400;
               
               nk_layout_row_template_begin(ctx, 280);
               nk_layout_row_template_push_dynamic(ctx);
               for (col = 0; col < items; col++)
                  nk_layout_row_template_push_variable(ctx, 400);
               nk_layout_row_template_push_dynamic(ctx);
               nk_layout_row_template_end(ctx);

               col = 0;
               for (int row = 1; row <= favorites_entries.count / items + 1; row++)
               {
                  if (row == 1)
                     nk_spacing(ctx, 1);
                  for (; col < items * row && col < favorites_entries.count; col++)
                     content_entry_widget(ctx, &favorites_entries.entry[col], content_current_id, activate && content_active, favorites_entry_cb);
                  if(col != favorites_entries.count)
                  {
                     nk_spacing(ctx, 1);
                     nk_spacing(ctx, 1);
                  }
               }
               content_entries += favorites_entries.count;
            }

            if (recent_active || (sidebar_active && favorites_active))
            {
               int col = 0;
               if (sidebar_active)
               {
                  content_subtitle(ctx, "Recent", &fonts[4], NK_TEXT_ALIGN_LEFT);
                  items = 8;
               }
               else
                  items = content_view_width / 400;

               nk_layout_row_template_begin(ctx, 280);
               nk_layout_row_template_push_dynamic(ctx);
               for (col = 0; col < items; col++)
                  nk_layout_row_template_push_variable(ctx, 400);
               nk_layout_row_template_push_dynamic(ctx);
               nk_layout_row_template_end(ctx);

               col = 0;
               for (int row = 1; row <= recent_entries.count / items + 1; row++)
               {
                  if (row == 1)
                     nk_spacing(ctx, 1);
                  for (; col < items * row && col < recent_entries.count; col++)
                     content_entry_widget(ctx, &recent_entries.entry[col], content_current_id, activate && content_active, favorites_entry_cb);
                  if(col != recent_entries.count)
                  {
                     nk_spacing(ctx, 1);
                     nk_spacing(ctx, 1);
                  }
               }
               content_entries += recent_entries.count;
            }

            if (folders_active)
            {
               int col = 0;
               if (sidebar_active)
               {
                  content_subtitle(ctx, "File Browser", &fonts[4], NK_TEXT_ALIGN_LEFT);
                  items = 12;
               }
               else
                  items = content_view_width / 160;

               nk_layout_row_template_begin(ctx, 160);
               nk_layout_row_template_push_dynamic(ctx);
               for (col = 0; col < items; col++)
                  nk_layout_row_template_push_variable(ctx, 160);
               nk_layout_row_template_push_dynamic(ctx);
               nk_layout_row_template_end(ctx);

               col = 0;
               for (int row = 1; row <= file_browser_entries.count / items + 1; row++)
               {
                  if (row == 1)
                     nk_spacing(ctx, 1);
                  for (; col < items * row && col < file_browser_entries.count; col++)
                     content_entry_widget(ctx, &file_browser_entries.entry[col], content_current_id, activate && content_active,  favorites_entry_cb);
                  if(col != file_browser_entries.count)
                  {
                     nk_spacing(ctx, 1);
                     nk_spacing(ctx, 1);
                  }
               }
               content_entries += file_browser_entries.count;
            }

            if (netplay_active)
            {
               int col = 0;
               if (sidebar_active)
               {
                  content_subtitle(ctx, "Netplay", &fonts[4], NK_TEXT_ALIGN_LEFT);
                  items = 12;
               }
               else
                  items = content_view_width / 160;

               nk_layout_row_template_begin(ctx, 160);
               nk_layout_row_template_push_dynamic(ctx);
               for (col = 0; col < items; col++)
                  nk_layout_row_template_push_variable(ctx, 160);
               nk_layout_row_template_push_dynamic(ctx);
               nk_layout_row_template_end(ctx);

               col = 0;
               for (int row = 1; row <= netplay_rooms_entries.count / items + 1; row++)
               {
                  if (row == 1)
                     nk_spacing(ctx, 1);
                  for (; col < items * row && col < netplay_rooms_entries.count; col++)
                     content_entry_widget(ctx, &netplay_rooms_entries.entry[col], content_current_id, activate && content_active,  favorites_entry_cb);
                  if(col != netplay_rooms_entries.count)
                  {
                     nk_spacing(ctx, 1);
                     nk_spacing(ctx, 1);
                  }
               }
               content_entries += netplay_rooms_entries.count;
            }
            if (settings_active)
            {
               int col = 0;
               if (sidebar_active)
               {
                  content_subtitle(ctx, "Settings", &fonts[4], NK_TEXT_ALIGN_LEFT);
                  items = 12;
               }
               else
                  items = content_view_width / 160;

               static char buf[MAX_SIZE];
               snprintf(buf, sizeof(buf), "%s", settings_entries.entry[col].sublabel);
               content_subtitle(ctx, buf, &fonts[3], NK_TEXT_ALIGN_LEFT);

               nk_layout_row_template_begin(ctx, 160);
               nk_layout_row_template_push_dynamic(ctx);
               for (col = 0; col < items; col++)
                  nk_layout_row_template_push_variable(ctx, 160);
               nk_layout_row_template_push_dynamic(ctx);
               nk_layout_row_template_end(ctx);

               col = 0;
               for (int row = 1; row <= settings_entries.count / items + 1; row++)
               {
                  if (row == 1)
                     nk_spacing(ctx, 1);
                  for (; col < items * row && col < settings_entries.count; col++)
                  {
                     if (!strcmp(buf, settings_entries.entry[col].sublabel) == 0)
                     {
                        snprintf(buf, sizeof(buf), "%s", settings_entries.entry[col].sublabel);
                        content_subtitle(ctx, buf, &fonts[3], NK_TEXT_ALIGN_LEFT);
                        nk_layout_row_template_begin(ctx, 160);
                        nk_layout_row_template_push_dynamic(ctx);
                        for (int n = 0; n < items; n++)
                           nk_layout_row_template_push_variable(ctx, 160);
                        nk_layout_row_template_push_dynamic(ctx);
                        nk_layout_row_template_end(ctx);
                        nk_spacing(ctx, 1);
                     }
                     content_entry_widget(ctx, &settings_entries.entry[col], content_current_id, activate && content_active,  favorites_entry_cb);
                  }
                  
                  if(col != settings_entries.count)
                  {
                     nk_spacing(ctx, 1);
                     nk_spacing(ctx, 1);
                  }
               }
               
               
               content_entries += settings_entries.count;
            }

            set_style(ctx);
         }
      nk_end(ctx);

      nk_begin(ctx, "sidebar", nk_rect(0, 0, sidebar_width, WINDOW_HEIGHT), 0);
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
            sidebar_entry_widget(ctx, &menu_entries.entry[i], sidebar_current_id, activate && nk_window_is_active(ctx, "sidebar"), menu_entries.offset, menu_entry_cb);
            if (menu_entries.entry[i].spacer)
               sidebar_spacer(ctx, 16);
         }
         for (int i=0; i < playlist_entries.count; i++)
         {
            sidebar_entry_widget(ctx, &playlist_entries.entry[i], sidebar_current_id, activate, playlist_entries.offset, playlist_entry_cb);
            if (playlist_entries.entry[i].spacer)
               sidebar_spacer(ctx, 16);
         }
         set_style(ctx);
      }
      nk_end(ctx);
      if (details_active)
      {
         int col = 0;
         nk_window_close(ctx, "content");
         nk_window_close(ctx, "header");
         nk_begin(ctx, "details", nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 2), 0);
         {
               /* no borders, and no selection colors for the sidebar */
               ctx->style.button.normal = nk_style_item_color(nk_rgba(0,0,0,0));
               ctx->style.button.hover  = nk_style_item_color(nk_rgba(0,0,0,0));
               ctx->style.button.active = nk_style_item_color(nk_rgba(0,0,0,0));
               ctx->style.button.border_color = nk_rgba(0,0,0,0);
               ctx->style.button.text_alignment = NK_TEXT_ALIGN_LEFT;

               nk_layout_row_static(ctx, 300, WINDOW_WIDTH, 1);
               //content_entry_widget(ctx, &recent_entries.entry[0], content_current_id, activate && content_active, favorites_entry_cb);
               content_entry_screenshots_widget(ctx, &favorites_entries.entry[1]);
         }
         nk_end(ctx);
      }

      const struct nk_input *in = &ctx->input;
      const int delta = WINDOW_WIDTH * 5 / 100;
      int lip = WINDOW_WIDTH * 0.3 / 100;
      activate = false;
      if (nk_window_is_active(ctx, "content"))
      {
         if (sidebar_width > lip )
         {
            sidebar_width -= delta;
            content_view_width += delta;
            content_view_position_x = sidebar_width + 1;
         }
         else if (sidebar_width < lip)
         {
            sidebar_width += 1;
            content_view_width -= 1;
            content_view_position_x = sidebar_width + 1;
         }
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_LEFT))
         {
            if (content_current_id % items == 0)
               nk_window_set_focus(ctx, "sidebar");
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
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_ENTER))
         {
            activate = true;
            lock_keys = true;
         }
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_BACKSPACE))
         {
            nk_window_set_focus(ctx, "sidebar");
            lock_keys = true;
         }
         
      }
      else if (nk_window_is_active(ctx, "sidebar"))
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
            nk_window_set_focus(ctx, "content");
            lock_keys = true;
         }
         if (!lock_keys && nk_input_is_key_pressed(in, NK_KEY_ENTER))
         {
            activate = true;
            lock_keys = true;
         }
      }
      if (frames % 16 == 0)
         lock_keys = false;
      content_active = nk_window_is_active(ctx, "content");
      sidebar_active = nk_window_is_active(ctx, "sidebar");
      folders_active = strcmp(menu_entries.entry[sidebar_current_id].name, "folder") == 0 ? true : false;
      favorites_active = strcmp(menu_entries.entry[sidebar_current_id].name, "favorites") == 0 ? true : false;
      recent_active = strcmp(menu_entries.entry[sidebar_current_id].name, "recent") == 0 ? true : false;
      netplay_active = strcmp(menu_entries.entry[sidebar_current_id].name, "netplay") == 0 ? true : false;
      settings_active = strcmp(menu_entries.entry[sidebar_current_id].name, "settings") == 0 ? true : false;

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

