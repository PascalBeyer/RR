
#include "4coder_API/4coder_custom.h"
#include "4coder_os_comp_cracking.h"

#include "4coder_generated/command_metadata.h"

#include "4coder_lib/4coder_arena.cpp"
#include "4coder_lib/4coder_heap.cpp"

#define FSTRING_IMPLEMENTATION
#include "4coder_lib/4coder_string.h"
#include "4coder_lib/4coder_table.h"
#include "4coder_lib/4coder_utf8.h"
#include "4coder_lib/4cpp_lexer.h"


#include "4coder_ui_helper.h"
#include "4coder_helper.h"
#include "4coder_default_framework.h"
#include "4coder_config.h"
#include "4coder_seek.h"
#include "4coder_auto_indent.h"
#include "4coder_search.h"
#include "4coder_build_commands.h"
#include "4coder_jump_sticky.h"
#include "4coder_jump_lister.h"
#include "4coder_project_commands.h"
#include "4coder_function_list.h"
#include "4coder_scope_commands.h"
#include "4coder_combined_write_commands.h"

#include "4coder_default_framework_variables.cpp"
#include "4coder_buffer_seek_constructors.cpp"
#include "4coder_helper.cpp"
#include "4coder_ui_helper.cpp"
#include "4coder_font_helper.cpp"
#include "4coder_config.cpp"
#include "4coder_default_framework.cpp"
#include "4coder_seek.cpp"
#include "4coder_base_commands.cpp"
#include "4coder_lists.cpp"
#include "4coder_auto_indent.cpp"
#include "4coder_search.cpp"
#include "4coder_jumping.cpp"
#include "4coder_jump_direct.cpp"
#include "4coder_jump_sticky.cpp"
#include "4coder_jump_lister.cpp"
#include "4coder_clipboard.cpp"
#include "4coder_system_command.cpp"
#include "4coder_build_commands.cpp"
#include "4coder_project_commands.cpp"
#include "4coder_function_list.cpp"
#include "4coder_scope_commands.cpp"
#include "4coder_combined_write_commands.cpp"

#include "4coder_default_hooks.cpp"

#include "4coder_remapping_commands.cpp" // todo do we need this or is this just specified somewhere


static void set_current_keymap(Application_Links* app, int map) 
{
   uint32_t access = AccessAll;
   View_Summary view     = get_active_view(app, access);
   Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
   
   if (buffer.exists) 
   {
      buffer_set_setting(app, &buffer, BufferSetting_MapID, map);
   }
}

enum modal_mapid {
   mapid_function = default_maps_count,
   mapid_numbers,// todo make this a thing
   
};

CUSTOM_COMMAND_SIG(write_double_colon)
{
   uint8_t character = ':';
   write_character_parameter(app, &character, 1);
}

CUSTOM_COMMAND_SIG(write_semi_colon)
{
   uint8_t character = ';';
   write_character_parameter(app, &character, 1);
}

CUSTOM_COMMAND_SIG(switch_edit_mode_to_function)
{
   set_current_keymap(app, mapid_function);
   
   Theme_Color colors[ ] = {
      { Stag_Cursor, 0xffff0000 },
      { Stag_At_Cursor, 0xff000000 },
   };
   
   set_theme_colors( app, colors, ArrayCount( colors ) );
}

CUSTOM_COMMAND_SIG(switch_edit_mode_to_default)
{
   set_current_keymap(app, default_code_map);
   
   Theme_Color colors[ ] = {
      { Stag_Cursor, 0xff00FF00 },
      { Stag_At_Cursor, 0xff000000 },
   };
   
   set_theme_colors( app, colors, ArrayCount( colors ) );
}


CUSTOM_COMMAND_SIG(switch_edit_mode_to_numbers)
{
   set_current_keymap(app, mapid_numbers);
   
   Theme_Color colors[ ] = {
      { Stag_Cursor, 0xfff7b300 },
      { Stag_At_Cursor, 0xff000000 },
   };
   
   set_theme_colors( app, colors, ArrayCount( colors ) );
}


struct seek_char_result
{
   char character;
   int32_t position;
};

static seek_char_result get_next_non_whitespace_character(Application_Links *app, Buffer_Summary *buffer, uint32_t pos, bool can_return_newline = true)
{
   char data_chunk[1024];
   Stream_Chunk stream = {};
   
   int i = pos;
   if (init_stream_chunk(&stream, app, buffer, pos, data_chunk, sizeof(data_chunk))){
      bool32 still_looping = false;
      do{
         for (; i < stream.end; ++i){
            char c = stream.data[i];
            
            if(c != ' ')
            {
               if(can_return_newline)
               {
                  seek_char_result result = { c, i };
                  
                  return result;
               }
               else
               {
                  seek_char_result result = { c, i };
                  if(c != '\n') return result;
               }
            }
         }
         still_looping = forward_stream_chunk(&stream);
      }while(still_looping);
   }
   
   seek_char_result not_found_result = { -1, -1 };
   
   return not_found_result;
}


CUSTOM_COMMAND_SIG(write_double_curly_braces)
{
   View_Summary view = get_active_view(app, AccessProtected);
   Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
   uint32_t pos = view.cursor.pos;
   
   seek_char_result next_char = get_next_non_whitespace_character(app, &buffer, pos);
   
   if(next_char.character == '\n')
   {
      uint8_t character1 = '{';
      char *character2 = "\n}";
      write_character_parameter(app, &character1, 1);
      write_character_parameter(app, (uint8_t *)character2, 2);
      int32_t new_pos = view.cursor.pos + 1;
      view_set_cursor(app, &view, seek_pos(new_pos), true);
   }
   else
   {
      exec_command(app, write_character);
   }
}

CUSTOM_COMMAND_SIG(write_double_index_braces)
{
   View_Summary view = get_active_view(app, AccessProtected);
   Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
   uint32_t pos = view.cursor.pos;
   
   seek_char_result next_char = get_next_non_whitespace_character(app, &buffer, pos);
   
   if(next_char.character == '\n'){
      
      uint8_t character1 = '[';
      uint8_t character2 = ']';
      
      write_character_parameter(app, &character1, 1);
      write_character_parameter(app, &character2, 1);
      
      int32_t new_pos = view.cursor.pos + 1;
      view_set_cursor(app, &view, seek_pos(new_pos), true);
   }
   else
   {
      exec_command(app, write_character);
   }
   
}

CUSTOM_COMMAND_SIG(write_double_round_braces)
{
   View_Summary view     = get_active_view(app, AccessProtected);
   Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
   int32_t pos           = view.cursor.pos;
   
   seek_char_result next_char = get_next_non_whitespace_character(app, &buffer, pos);
   
   if(next_char.character == '\n'){
      
      uint8_t character1 = '(';
      uint8_t character2 = ')';
      write_character_parameter(app, &character1, 1);
      write_character_parameter(app, &character2, 1);
      int32_t new_pos = view.cursor.pos + 1;
      view_set_cursor(app, &view, seek_pos(new_pos), true);
   }
   else
   {
      exec_command(app, write_character);
   }
}

CUSTOM_COMMAND_SIG(seek_past_next_curly_brace)
{
   uint32_t access = AccessOpen;
   View_Summary view = get_active_view(app, access);
   Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
   int32_t pos = view.cursor.pos;
   
   bool can_return_newline = false;
   seek_char_result next_char = get_next_non_whitespace_character(app, &buffer, pos, can_return_newline);
   
   if(next_char.character == '}')
   {
      int32_t new_pos = next_char.position;
      if(new_pos != buffer.size) new_pos += 1;
      view_set_cursor(app, &view, seek_pos(new_pos), true);
   }
   else
   {
      exec_command(app, write_character);
   }
}

CUSTOM_COMMAND_SIG(seek_past_next_round_brace)
{
   uint32_t access = AccessOpen;
   View_Summary view = get_active_view(app, access);
   Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
   int32_t pos = view.cursor.pos;
   
   bool can_return_newline = false;
   seek_char_result next_char = get_next_non_whitespace_character(app, &buffer, pos, can_return_newline);
   
   if(next_char.character == ')')
   {
      int32_t new_pos = next_char.position;
      if(new_pos != buffer.size) new_pos += 1;
      view_set_cursor(app, &view, seek_pos(new_pos), true);
   }
   else
   {
      exec_command(app, write_character);
   }
}

CUSTOM_COMMAND_SIG(write_semicolon_and_switch_to_default)
{
   uint8_t character = ';';
   write_character_parameter(app, &character, 1);
   exec_command(app, switch_edit_mode_to_default);
}

CUSTOM_COMMAND_SIG(seek_past_next_index_brace)
{
   uint32_t access = AccessOpen;
   View_Summary view = get_active_view(app, access);
   Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
   int32_t pos = view.cursor.pos;
   
   bool can_return_newline = false;
   seek_char_result next_char = get_next_non_whitespace_character(app, &buffer, pos, can_return_newline);
   
   if(next_char.character == ']')
   {
      int32_t new_pos = next_char.position;
      if(new_pos != buffer.size) new_pos += 1;
      view_set_cursor(app, &view, seek_pos(new_pos), true);
   }
   else
   {
      exec_command(app, write_character);
   }
}


#if 0
CUSTOM_COMMAND_SIG(write_character)
CUSTOM_DOC("Inserts whatever character was used to trigger this command.")
{
   User_Input in = get_command_input(app);
   uint8_t character[4];
   uint32_t length = to_writable_character(in, character);
   write_character_parameter(app, character, length);
}
#endif

static void write_single_character(Application_Links *app, uint8_t to_write)
{
   write_character_parameter(app, &to_write, 1);
}


CUSTOM_COMMAND_SIG(write_shift_character)
{
   User_Input in = get_command_input(app);
   uint8_t character[4];
   uint32_t length = to_writable_character(in, character);
   if(length != 1)
   {
      write_character_parameter(app, character, length);
      return;
   }
   switch(character[0])
   {
      
      case '1':
      {
         write_single_character(app, '!');
      }break;
      case '2':
      {
         write_single_character(app, '"'); // todo make this into a brace thing????
      }break;
      case '3':
      {
         //this is end curly brace
      }break;
      case '4':
      {
         write_single_character(app, '\'');
      }break;
      case '5':
      {
         write_single_character(app, '%');
      }break;
      case '6':
      {
         write_single_character(app, '&');
      }break;
      case ',':
      {
         write_single_character(app, ';');
      }break;
      case ';':
      {
         write_single_character(app, ',');
      }break;
      case '<':
      {
         write_single_character(app, '>');
      }break;
      case '>':
      {
         write_single_character(app, '<');
      }break;
   }
}

CUSTOM_COMMAND_SIG(write_equals_and_line_up_previous)
{
   write_single_character(app, '=');
   View_Summary view = get_active_view(app, AccessOpen);
   Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
   
   int32_t start_line = view.cursor.line;
   int32_t max_amount_of_chars = 0;
   
   int32_t min_line = start_line;
   const int32_t max_edits = 300;
   int32_t edit_index = 0;
   Buffer_Edit edits[max_edits];
   {
      int32_t current_line = start_line;
      
      Partial_Cursor start = {};
      Partial_Cursor end = {};
      while(current_line > 0)
      {
         buffer_compute_cursor(app, &buffer, seek_line_char(current_line, 1), &start);
         buffer_compute_cursor(app, &buffer, seek_line_char(current_line, -1), &end);
         
         static const int32_t chunk_size = 1024;
         char chunk[chunk_size];
         Stream_Chunk stream = {};
         
         int32_t i = start.pos;
         stream.max_end = end.pos;
         
         int32_t found_index                   = -1;
         int32_t found_white_spaces            = 0;
         bool found_non_whitespace             = false;
         int32_t first_non_whitespace_position = 0;
         
         if (init_stream_chunk(&stream, app, &buffer, i, chunk, chunk_size))
         {
            bool32 still_looping = false;
            do
            {
               for (;i < stream.end; ++i)
               {
                  char c = stream.data[i];
                  if (c == '=')
                  {
                     // todo handle other tokens, i.e. just break
                     found_index = i;
                     goto double_break;
                  }
                  else if(c == ' ')
                  {
                     found_white_spaces++;
                  }
                  else
                  {
                     if(!found_non_whitespace)
                     {
                        first_non_whitespace_position = i;
                        found_non_whitespace = true;
                     }
                     found_white_spaces = 0;
                  }
               }
               still_looping = forward_stream_chunk(&stream);
            }while(still_looping);
         }
         double_break:;
         if(found_index == -1) break;
         
         if(found_white_spaces) found_white_spaces--;
         int32_t size = (found_index - first_non_whitespace_position) - found_white_spaces;
         if(size > max_amount_of_chars) max_amount_of_chars = size;
         
         min_line = current_line;
         
         Buffer_Edit *edit = edits + edit_index++;
         edit->str_start = 0;
         edit->start = first_non_whitespace_position + size;
         edit->end   = edit->start + found_white_spaces;
         edit->len   = -size;
         
         current_line--;
         if(edit_index == max_edits) break;
      }
   }
   
   int32_t line_count = start_line - min_line;
   
   Partition *part = &global_part;
   Temp_Memory temp = begin_temp_memory(part);
   char *empty_spaces = push_array(part, char, max_amount_of_chars);
   memset(empty_spaces, ' ', max_amount_of_chars);
   
   for(int32_t line = 0; line <= line_count; line++)
   {
      edits[line].len += max_amount_of_chars;
      buffer_replace_range(app, &buffer, edits[line].start, edits[line].end, empty_spaces, edits[line].len);
   }
   
   end_temp_memory(temp);
}

OPEN_FILE_HOOK_SIG(pascal_default_file_settings){
   // NOTE(allen|a4.0.8): The get_parameter_buffer was eliminated
   // and instead the buffer is passed as an explicit parameter through
   // the function call.  That is where buffer_id comes from here.
   Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
   Assert(buffer.exists);
   
   bool32 treat_as_code = false; // just always do.
   bool32 treat_as_todo = false;
   bool32 lex_without_strings = false;
   
   CString_Array extensions = get_code_extensions(&global_config.code_exts);
   
   Parse_Context_ID parse_context_id = 0;
   
   if (buffer.file_name != 0 && buffer.size < (16 << 20)){
      String name = make_string(buffer.file_name, buffer.file_name_len);
      String ext = file_extension(name);
      for (int32_t i = 0; i < extensions.count; ++i){
         if (match(ext, extensions.strings[i])){
            treat_as_code = true;
            
            if (match(ext, "cs")){
               if (parse_context_language_cs == 0){
                  init_language_cs(app);
               }
               parse_context_id = parse_context_language_cs;
            }
            
            if (match(ext, "java")){
               if (parse_context_language_java == 0){
                  init_language_java(app);
               }
               parse_context_id = parse_context_language_java;
            }
            
            if (match(ext, "rs")){
               if (parse_context_language_rust == 0){
                  init_language_rust(app);
               }
               parse_context_id = parse_context_language_rust;
               lex_without_strings = true;
            }
            
            if (match(ext, "cpp") || match(ext, "h") || match(ext, "c") || match(ext, "hpp") || match(ext, "cc")){
               if (parse_context_language_cpp == 0){
                  init_language_cpp(app);
               }
               parse_context_id = parse_context_language_cpp;
            }
            
            // TODO(NAME): Real GLSL highlighting
            if (match(ext, "glsl")){
               if (parse_context_language_cpp == 0){
                  init_language_cpp(app);
               }
               parse_context_id = parse_context_language_cpp;
            }
            
            // TODO(NAME): Real Objective-C highlighting
            if (match(ext, "m")){
               if (parse_context_language_cpp == 0){
                  init_language_cpp(app);
               }
               parse_context_id = parse_context_language_cpp;
            }
            
            break;
         }
      }
      
   }
   
   int32_t map_id = mapid_function;
   int32_t map_id_query = 0;
   
   buffer_set_setting(app, &buffer, BufferSetting_MapID, default_lister_ui_map);
   buffer_get_setting(app, &buffer, BufferSetting_MapID, &map_id_query);
   Assert(map_id_query == default_lister_ui_map);
   
   buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, global_config.default_wrap_width);
   buffer_set_setting(app, &buffer, BufferSetting_MinimumBaseWrapPosition, global_config.default_min_base_width);
   buffer_set_setting(app, &buffer, BufferSetting_MapID, map_id);
   buffer_get_setting(app, &buffer, BufferSetting_MapID, &map_id_query);
   Assert(map_id_query == map_id);
   buffer_set_setting(app, &buffer, BufferSetting_ParserContext, parse_context_id);
   
   // NOTE(allen): Decide buffer settings
   bool32 wrap_lines = true;
   bool32 use_virtual_whitespace = false;
   bool32 use_lexer = false;
   if (treat_as_todo){
      lex_without_strings = true;
      wrap_lines = true;
      use_virtual_whitespace = true;
      use_lexer = true;
   }
   else if (treat_as_code){
      wrap_lines = global_config.enable_code_wrapping;
      use_virtual_whitespace = global_config.enable_virtual_whitespace;
      use_lexer = true;
   }
   
   //if (buffer.size >= (192 << 10)){
   if (buffer.size >= (128 << 10)){
      wrap_lines = false;
      use_virtual_whitespace = false;
   }
   
   // NOTE(allen|a4.0.12): There is a little bit of grossness going on here.
   // If we set BufferSetting_Lex to true, it will launch a lexing job.
   // If a lexing job is active when we set BufferSetting_VirtualWhitespace, the call can fail.
   // Unfortunantely without tokens virtual whitespace doesn't really make sense.
   // So for now I have it automatically turning on lexing when virtual whitespace is turned on.
   // Cleaning some of that up is a goal for future versions.
   buffer_set_setting(app, &buffer, BufferSetting_LexWithoutStrings, lex_without_strings);
   buffer_set_setting(app, &buffer, BufferSetting_WrapLine, wrap_lines);
   buffer_set_setting(app, &buffer, BufferSetting_VirtualWhitespace, use_virtual_whitespace);
   buffer_set_setting(app, &buffer, BufferSetting_Lex, use_lexer);
   
   // no meaning for return
   return(0);
}


START_HOOK_SIG(pascal_start){
   named_maps = named_maps_values;
   named_map_count = ArrayCount(named_maps_values);
   
   default_4coder_initialize(app);
   default_4coder_side_by_side_panels(app, files, file_count);
   
   if (global_config.automatically_load_project){
      load_project(app);
   }
   
   
   Theme_Color colors[ ] = {
      { Stag_Cursor, 0xffff0000 },
      { Stag_At_Cursor, 0xff000000 },
   };
   
   set_theme_colors( app, colors, ArrayCount( colors ) );
   
   // no meaning for return
   return(0);
}




extern "C" int32_t get_bindings(void *data, int32_t size)
{
   Bind_Helper context_ = begin_bind_helper(data, size);
   Bind_Helper *context = &context_;
   
   { // set hooks
      set_start_hook(context, pascal_start);
      set_command_caller(context, default_command_caller);
      set_render_caller(context, default_render_caller);
      
      set_hook(context, hook_exit, default_exit);
      set_hook(context, hook_view_size_change, default_view_adjust);
      
      set_open_file_hook(context, pascal_default_file_settings);
      set_new_file_hook(context, default_new_file);
      set_save_file_hook(context, default_file_save);
      
      set_end_file_hook(context, end_file_close_jump_list);
      
      set_input_filter(context, default_suppress_mouse_filter);
      set_scroll_rule(context, smooth_scroll_rule);
      set_buffer_name_resolver(context, default_buffer_name_resolution);
   }
   
   { // set keybindings
      
      begin_map(context, mapid_global);
      {
         bind(context, 'n', MDFR_CTRL, interactive_new);
         bind(context, 'o', MDFR_CTRL, interactive_open_or_new);
         bind(context, 'o', MDFR_ALT, open_in_other);
         bind(context, 'k', MDFR_CTRL, interactive_kill_buffer);
         bind(context, 'i', MDFR_CTRL, interactive_switch_buffer);
         bind(context, 'h', MDFR_CTRL, project_go_to_root_directory);
         bind(context, 'S', MDFR_CTRL, save_all_dirty_buffers);
         bind(context, '.', MDFR_ALT, change_to_build_panel);
         bind(context, ',', MDFR_ALT, close_build_panel);
         bind(context, 'n', MDFR_ALT, goto_next_jump_sticky);
         bind(context, 'N', MDFR_ALT, goto_prev_jump_sticky);
         bind(context, 'M', MDFR_ALT, goto_first_jump_sticky);
         bind(context, 'm', MDFR_ALT, build_in_build_panel);
         bind(context, 'b', MDFR_ALT, toggle_filebar);
         bind(context, 'z', MDFR_ALT, execute_any_cli);
         bind(context, 'Z', MDFR_ALT, execute_previous_cli);
         bind(context, 'x', MDFR_ALT, command_lister);
         bind(context, 'X', MDFR_ALT, project_command_lister);
         bind(context, 'I', MDFR_CTRL, list_all_functions_all_buffers_lister);
         bind(context, 'E', MDFR_ALT, exit_4coder);
         
         bind(context, key_f1, MDFR_NONE, project_fkey_command);
         bind(context, key_f2, MDFR_NONE, project_fkey_command);
         bind(context, key_f3, MDFR_NONE, project_fkey_command);
         bind(context, key_f4, MDFR_NONE, project_fkey_command);
         bind(context, key_f5, MDFR_NONE, project_fkey_command);
         bind(context, key_f6, MDFR_NONE, project_fkey_command);
         bind(context, key_f7, MDFR_NONE, project_fkey_command);
         bind(context, key_f8, MDFR_NONE, project_fkey_command);
         bind(context, key_f9, MDFR_NONE, project_fkey_command);
         bind(context, key_f10, MDFR_NONE, project_fkey_command);
         bind(context, key_f11, MDFR_NONE, project_fkey_command);
         bind(context, key_f12, MDFR_NONE, project_fkey_command);
         bind(context, key_f13, MDFR_NONE, project_fkey_command);
         bind(context, key_f14, MDFR_NONE, project_fkey_command);
         bind(context, key_f15, MDFR_NONE, project_fkey_command);
         bind(context, key_f16, MDFR_NONE, project_fkey_command);
         bind(context, key_mouse_wheel, MDFR_NONE, mouse_wheel_scroll);
         bind(context, key_mouse_wheel, MDFR_CTRL, mouse_wheel_change_face_size);
      }
      end_map(context);
      
      begin_map(context, mapid_file);
      {
         bind_vanilla_keys(context, write_character);
         
         bind(context, get_key_code("ß"), MDFR_NONE, switch_edit_mode_to_function);
         bind(context, get_key_code("ä"), MDFR_NONE, switch_edit_mode_to_numbers); 
         
         bind(context, get_key_code("ü"), MDFR_NONE, write_underscore);
         bind(context, get_key_code("ö"), MDFR_NONE, write_semi_colon);
         
         bind(context, key_mouse_left, MDFR_NONE, click_set_cursor_and_mark);
         bind(context, key_click_activate_view, MDFR_NONE, click_set_cursor_and_mark);
         bind(context, key_mouse_left_release, MDFR_NONE, click_set_cursor);
         bind(context, key_mouse_move, MDFR_NONE, click_set_cursor_if_lbutton);
         
         bind(context, key_del, MDFR_NONE, delete_char);
         bind(context, key_del, MDFR_SHIFT, delete_char);
         bind(context, key_back, MDFR_NONE, backspace_char);
         bind(context, key_back, MDFR_SHIFT, backspace_char);
         //bind(context, key_page_up, MDFR_CTRL, goto_beginning_of_file);
         //bind(context, key_page_down, MDFR_CTRL, goto_end_of_file);
         bind(context, key_page_up, MDFR_NONE, page_up);
         bind(context, key_page_down, MDFR_NONE, page_down);
         bind(context, key_page_up, MDFR_CTRL|MDFR_SHIFT, goto_beginning_of_file);bind(context, key_page_down, MDFR_CTRL|MDFR_SHIFT, goto_end_of_file);
         
         bind(context, key_up, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_up_end_line);
         bind(context, key_down, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_down_end_line);
         bind(context, key_left, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_left);
         bind(context, key_right, MDFR_CTRL|MDFR_SHIFT, seek_whitespace_right);
         bind(context, key_up, MDFR_ALT, move_line_up);
         bind(context, key_down, MDFR_ALT, move_line_down);
         bind(context, key_back, MDFR_CTRL, backspace_word);
         bind(context, key_del, MDFR_CTRL, delete_word);
         bind(context, key_back, MDFR_ALT, snipe_token_or_word);
         bind(context, key_del, MDFR_ALT, snipe_token_or_word_right);
         
         bind(context, 'D', MDFR_CTRL, delete_line);
         bind(context, 'e', MDFR_CTRL, center_view);
         bind(context, 'E', MDFR_CTRL, left_adjust_view);
         
         bind(context, 'F', MDFR_ALT, list_all_substring_locations_case_insensitive);
         bind(context, 'g', MDFR_CTRL, goto_line);
         bind(context, 'G', MDFR_CTRL, list_all_locations_of_selection);
         bind(context, 'j', MDFR_CTRL, snippet_lister);
         bind(context, 'K', MDFR_CTRL, kill_buffer);
         bind(context, 'L', MDFR_CTRL, duplicate_line);
         
         bind(context, 'O', MDFR_CTRL, reopen);
         bind(context, 'Q', MDFR_CTRL, query_replace_identifier);
         bind(context, 'q', MDFR_ALT, query_replace_selection);
         
         bind(context, 's', MDFR_CTRL, save);
         bind(context, 't', MDFR_CTRL, search_identifier);
         bind(context, 'T', MDFR_CTRL, list_all_locations_of_identifier);
         bind(context, 'v', MDFR_CTRL, paste_and_indent);
         bind(context, 'V', MDFR_CTRL, paste_next_and_indent);
         
         bind(context, '1', MDFR_CTRL, view_buffer_other_panel);
         bind(context, '2', MDFR_CTRL, swap_buffers_between_panels);
         bind(context, '\n', MDFR_NONE, newline_or_goto_position_sticky);
         bind(context, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel_sticky);
         bind(context, ' ', MDFR_SHIFT, write_character);
      }
      end_map(context);
      
      begin_map(context, mapid_function);
      {
         bind(context, '\t', MDFR_NONE, switch_edit_mode_to_default);
         bind(context, get_key_code("ä"), MDFR_NONE, switch_edit_mode_to_numbers); 
         
         bind(context, '0', MDFR_NONE, interactive_open_or_new);
         bind(context, 'n', MDFR_NONE, interactive_new);
         bind(context, 'v', MDFR_NONE, paste_and_indent);
         bind(context, '1', MDFR_NONE, list_all_locations);
         bind(context, 'p', MDFR_NONE, seek_end_of_line);
         bind(context, 'i', MDFR_NONE, seek_beginning_of_line);
         
         bind(context, '<', MDFR_NONE, change_active_panel_backwards);
         bind(context, 'r', MDFR_NONE, reverse_search);
         bind(context, ' ', MDFR_NONE, set_mark);
         bind(context, '2', MDFR_NONE, replace_in_range);
         bind(context, 'c', MDFR_NONE, copy);
         bind(context, '-', MDFR_NONE, delete_current_scope);
         bind(context, key_back, MDFR_NONE, backspace_word);
         bind(context, 'e', MDFR_NONE, delete_range);
         bind(context, 'q', MDFR_NONE, query_replace);
         bind(context, 'x', MDFR_NONE, cut);
         bind(context, 'z', MDFR_NONE, undo);
         bind(context, 'y', MDFR_NONE, redo);
         bind(context, 'f', MDFR_NONE, search);
         bind(context, 'm', MDFR_NONE, cursor_mark_swap);
         
         bind(context, 'w', MDFR_NONE, move_up);
         bind(context, 's', MDFR_NONE, move_down);
         bind(context, 'a', MDFR_NONE, move_left);
         bind(context, 'd', MDFR_NONE, move_right);
         
         bind(context, 'o', MDFR_NONE, seek_whitespace_up_end_line);
         bind(context, 'l', MDFR_NONE, seek_whitespace_down_end_line);
         bind(context, 'k', MDFR_NONE, seek_white_or_token_left);
         bind(context, get_key_code("ö"), MDFR_NONE, seek_white_or_token_right);
         
         bind(context, ',', MDFR_NONE, comment_line_toggle);
         
         bind(context, '7', MDFR_NONE, place_in_scope);
         bind(context, '8', MDFR_NONE, if0_off);
         
         bind(context, '\n', MDFR_NONE, newline_or_goto_position_sticky);
         bind(context, '\n', MDFR_SHIFT, newline_or_goto_position_same_panel_sticky);
         
         bind(context, key_end, MDFR_NONE, seek_end_of_line);
         bind(context, key_home, MDFR_NONE, seek_beginning_of_line);
      }
      
      end_map(context);
      
      begin_map(context, default_code_map);
      {
         inherit_map(context, mapid_file);
         
         //bind(context, '{', MDFR_NONE, write_double_curly_braces);
         //bind(context, '(', MDFR_NONE, write_double_round_braces);
         bind(context, '[', MDFR_NONE, write_double_index_braces);
         bind(context, '\n', MDFR_NONE, write_and_auto_tab);
         bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
         
         //bind(context, '=', MDFR_NONE, write_equals_and_line_up_previous);
         //bind(context, '}', MDFR_NONE, seek_past_next_curly_brace);
         //bind(context, ')', MDFR_NONE, seek_past_next_round_brace);
         bind(context, ']', MDFR_NONE, seek_past_next_index_brace);
         
         bind(context, ';', MDFR_NONE, write_and_auto_tab);
         bind(context, '#', MDFR_NONE, write_and_auto_tab);
         
         bind(context, '0', MDFR_NONE, write_equals_and_line_up_previous);
         bind(context, '1', MDFR_NONE, write_shift_character);
         bind(context, '2', MDFR_NONE, write_shift_character);
         bind(context, '3', MDFR_NONE, seek_past_next_curly_brace);
         bind(context, '4', MDFR_NONE, write_shift_character);
         bind(context, '5', MDFR_NONE, write_shift_character);
         bind(context, '6', MDFR_NONE, write_shift_character);
         bind(context, '7', MDFR_NONE, write_double_curly_braces);
         bind(context, '8', MDFR_NONE, write_double_round_braces);
         bind(context, '9', MDFR_NONE, seek_past_next_round_brace);
         bind(context, ',', MDFR_NONE, write_shift_character);
         bind(context, '<', MDFR_NONE, write_shift_character);
         bind(context, '>', MDFR_NONE, write_shift_character);
         bind(context, ';', MDFR_NONE, write_shift_character);
         
         bind(context, '\t', MDFR_NONE, word_complete);
         bind(context, '\t', MDFR_CTRL, auto_tab_range);
         bind(context, '\t', MDFR_SHIFT, auto_tab_line_at_cursor);
         
         bind(context, 'r', MDFR_ALT, write_block);
         bind(context, 't', MDFR_ALT, write_todo);
         bind(context, 'y', MDFR_ALT, write_note);
         bind(context, 'D', MDFR_ALT, list_all_locations_of_type_definition);
         bind(context, 'T', MDFR_ALT, list_all_locations_of_type_definition_of_identifier);
         bind(context, '[', MDFR_CTRL, open_long_braces);
         bind(context, '{', MDFR_CTRL, open_long_braces_semicolon);
         bind(context, '}', MDFR_CTRL, open_long_braces_break);
         bind(context, '[', MDFR_ALT, select_surrounding_scope);
         bind(context, ']', MDFR_ALT, select_prev_scope_absolute);
         bind(context, '\'', MDFR_ALT, select_next_scope_absolute);
         
         bind(context, '-', MDFR_ALT, delete_current_scope);
         bind(context, 'j', MDFR_ALT, scope_absorb_down);
         
         bind(context, '1', MDFR_ALT, open_file_in_quotes);
         bind(context, '2', MDFR_ALT, open_matching_file_cpp);
         bind(context, '0', MDFR_CTRL, write_zero_struct);
      }
      
      end_map(context);
      
      begin_map(context, default_lister_ui_map);
      {
         bind_vanilla_keys(context, lister__write_character);
         bind(context, get_key_code("ü"), MDFR_NONE, write_underscore);
         bind(context, get_key_code("ö"), MDFR_NONE, write_semi_colon);
         bind(context, get_key_code("ä"), MDFR_NONE, write_double_colon);
         
         bind(context, key_esc, MDFR_NONE, lister__quit);
         bind(context, '\n', MDFR_NONE, lister__activate);
         bind(context, '\t', MDFR_NONE, lister__activate);
         bind(context, key_back, MDFR_NONE, lister__backspace_text_field);
         bind(context, key_up, MDFR_NONE, lister__move_up);
         bind(context, 'k', MDFR_ALT, lister__move_up);
         bind(context, key_page_up, MDFR_NONE, lister__move_up);
         bind(context, key_down, MDFR_NONE, lister__move_down);
         bind(context, 'j', MDFR_ALT, lister__move_down);
         bind(context, key_page_down, MDFR_NONE, lister__move_down);
         bind(context, key_mouse_wheel, MDFR_NONE, lister__wheel_scroll);
         bind(context, key_mouse_left, MDFR_NONE, lister__mouse_press);
         bind(context, key_mouse_left_release, MDFR_NONE, lister__mouse_release);
         bind(context, key_mouse_move, MDFR_NONE, lister__repaint);
         bind(context, key_animate, MDFR_NONE, lister__repaint);
      }
      end_map(context);
      
      begin_map(context, mapid_numbers);
      {
         bind(context, '\t', MDFR_NONE, switch_edit_mode_to_default);
         //bind(context, '\t', MDFR_NONE, switch_edit_mode_to_default); todo semicolon should also do this.
         bind(context, get_key_code("ß"), MDFR_NONE, switch_edit_mode_to_function);
         bind(context, ',', MDFR_NONE, write_semicolon_and_switch_to_default);
         
         
         bind(context, '\n', MDFR_NONE, write_and_auto_tab);
         bind(context, '\n', MDFR_SHIFT, write_and_auto_tab);
         bind(context, key_back, MDFR_NONE, backspace_char);
         bind(context, key_back, MDFR_SHIFT, backspace_char);
         
         bind(context, '0', MDFR_NONE, write_character);
         bind(context, '1', MDFR_NONE, write_character);
         bind(context, '2', MDFR_NONE, write_character);
         bind(context, '3', MDFR_NONE, write_character);
         bind(context, '4', MDFR_NONE, write_character);
         bind(context, '5', MDFR_NONE, write_character);
         bind(context, '6', MDFR_NONE, write_character);
         bind(context, '7', MDFR_NONE, write_character);
         bind(context, '8', MDFR_NONE, write_character);
         bind(context, '9', MDFR_NONE, write_character);
         
         bind(context, 'x', MDFR_NONE, write_character);
         bind(context, 'a', MDFR_NONE, write_character);
         bind(context, 'b', MDFR_NONE, write_character);
         bind(context, 'c', MDFR_NONE, write_character);
         bind(context, 'd', MDFR_NONE, write_character);
         bind(context, 'e', MDFR_NONE, write_character);
         bind(context, 'f', MDFR_NONE, write_character);
         
         bind(context, 'm', MDFR_NONE, cursor_mark_swap);
         
         //bind(context, 'w', MDFR_NONE, move_up);
         //bind(context, 's', MDFR_NONE, move_down);
         //bind(context, 'a', MDFR_NONE, move_left);
         //bind(context, 'd', MDFR_NONE, move_right);
         
         bind(context, 'o', MDFR_NONE, seek_whitespace_up_end_line);
         bind(context, 'l', MDFR_NONE, seek_whitespace_down_end_line);
         bind(context, 'k', MDFR_NONE, seek_white_or_token_left);
         bind(context, get_key_code("ö"), MDFR_NONE, seek_white_or_token_right);
         
      }
      end_map(context);
   }
   
   
   int32_t result = end_bind_helper(context);
   return(result);
}