
#include "../o/src/utilities.c"
#include <stdio.h>
#include <stdlib.h>


static loaded_file load_entire_file(char *directory, memory_arena *arena)
{
   loaded_file ret = zero_struct;
   
   FILE *file = fopen(directory, "rb");
   if(!file)
   {
      return ret;
   }
   
   fseek(file, 0, SEEK_END);
   u64 file_size = ftell(file);
   rewind(file);
   
   u8 *buffer = push_data(arena, u8, file_size);
   fread(buffer, 1, file_size, file);
   
   fclose(file);
   
   
   ret.size = file_size;
   ret.memory    = buffer;
   
   return ret;
}


static void panic(s32 exit_code)
{
   exit(exit_code);
}

static u32 console_print(char *format, ...)
{
   char buffer[1024];
   va_list va;
   va_start(va, format);
   int chars_written = vprintf(format, va);
   va_end(va);
   
   fflush(stdout);
   
   chars_written = (chars_written > 0) ? chars_written : 0;
   
   return (u32)chars_written;
}

typedef union terminal
{
   struct t
   {
      u32 x;
      u32 y;
      u32 z;
   };
   
   u32 values[3];
   
} terminal;

typedef struct terminal_array
{
   terminal *data;
   u64 amount_of_terminals;
} terminal_array;


typedef struct 
{
   char *data;
   u64 length;
} string;

func string create_string(char *data, u64 length)
{
   string ret;
   ret.data   = data;
   ret.length = length;
   return ret;
}

//@reserche what are \v and \f
func bool is_whitespace_or_newline(char c)
{
   return ((c ==' ') || (c =='\t') || (c =='\n') || (c == '\r') || (c == '\v') || (c == '\f'));
}

func bool is_whitespace(char c)
{
   return ((c ==' ') || (c =='\t') || (c == '\v') || (c == '\f'));
}

func bool is_only_whitespaces_or_newlines(string s)
{
   for(u64 i = 0; i < s.length; i++)
   {
      if(!is_whitespace_or_newline(s.data[i]))
      {
         return false;
      }
   }
   return true;
}

func bool is_only_whitespaces(string s)
{
   for(u64 i = 0; i < s.length; i++)
   {
      if(!is_whitespace(s.data[i]))
      {
         return false;
      }
   }
   return true;
}

func string eat_leading_whitespaces(string *to_eat)
{
   string ret = *to_eat;
   
   for(u64 it_index = 0; it_index < to_eat->length; it_index++)
   {
      char *it = to_eat->data + it_index;
      if(!is_whitespace(*it))
      {
         ret.length      = it_index;
         to_eat->length -= it_index;
         to_eat->data   += it_index;
         
         return ret;
      }
   }
   
   to_eat->data  += to_eat->length;
   to_eat->length = 0;
   
   return ret;
}

func string eat_leading_whitespaces_and_newlines(string *to_eat)
{
   string ret = *to_eat;
   
   for(u64 it_index = 0; it_index < to_eat->length; it_index++)
   {
      char *it = to_eat->data + it_index;
      if(!is_whitespace_or_newline(*it))
      {
         ret.length      = it_index;
         to_eat->length -= it_index;
         to_eat->data   += it_index;
         
         return ret;
      }
   }
   
   to_eat->data  += to_eat->length;
   to_eat->length = 0;
   
   return ret;
}

// @cleanup overflow checking? u64 only?
func u32 eat_a_u32(string *to_eat, bool *success)
{
   if(!to_eat->length)
   {
      *success = false;
      return max_u32;
   }
   
   u32 ret = max_u32;
   
   { // fail if the first character is not a number
      u32 number_value = *to_eat->data - '0';
      if(number_value > 9 || number_value < 0)
      {
         *success = false;
         return max_u32;
      }
      ret = number_value;
   }
   
   for(u32 i = 1; i < to_eat->length; i++)
   {
      u32 number_value = *(to_eat->data + i) - '0';
      if(number_value > 9 || number_value < 0)
      {
         to_eat->length -= i;
         to_eat->data   += i;
         return ret;
      }
      ret = 10 * ret + number_value;
   }
   
   return ret;
}

func string eat_a_line(string *to_eat)
{
   string ret = *to_eat;
   
   for(u64 it_index = 0; it_index < to_eat->length; it_index++)
   {
      char *it = to_eat->data + it_index;
      if(*it   == '\n')
      {
         // eat the newline as well
         u64 length_to_eat = it_index + 1;
         ret.length      = length_to_eat;
         to_eat->length -= length_to_eat;
         to_eat->data   += length_to_eat;
         
         return ret;
      }
   }
   
   to_eat->data  += to_eat->length;
   to_eat->length = 0;
   
}

typedef struct
{
   void *value;
   u64   key;
} key_value_pair;

typedef struct
{
   key_value_pair *root_minus_one; 
   u64 heap_size; // we are using the elements [1, heap_size]
   u64 capacity;
   memory_arena *arena;
} min_heap;

// @hmm make this bool and handle errors outside?
proc void min_heap_insert(min_heap *heap, key_value_pair to_insert) 
{
   if(heap->heap_size + 1 >= heap->capacity)
   {
      not_implemented;
      // @incomplete realloc or something?
      // think about how to do memory with the crt.
   }
   
   heap->heap_size++;
   heap->root_minus_one[heap->heap_size] = to_insert; 
   u64 current_index = heap->heap_size;
   
   while (true) 
   {
      u64 old_index = current_index;
      current_index >>= 1;
      if(!current_index)
      {
         break;
      }
      key_value_pair parent = heap->root_minus_one[current_index];
      if(parent.key >= to_insert.key)
      {
         break;
      }
      heap->root_minus_one[old_index]     = parent;
      heap->root_minus_one[current_index] = to_insert;
   }
}

static key_value_pair min_heap_pop_minimal_element(min_heap *heap)
{
   key_value_pair ret = zero_struct;
   if(heap->heap_size == 0)
   {
      return ret;
   }
   
   ret = heap->root_minus_one[1]; // heap[1] is our root.
   
   key_value_pair last_element = heap->root_minus_one[heap->heap_size];
   heap->heap_size--;
   
   u64 current_index = 1;
   
   while (true)
   {
      key_value_pair *left_child  = heap->root_minus_one + 2 * current_index;
      key_value_pair *right_child = heap->root_minus_one + 2 * current_index + 1;
      
      key_value_pair *minimal_child = (left_child->key < right_child->key) ? left_child : right_child;
      u64 minimal_index  = (left_child->key < right_child->key) ? (2 * current_index) : (2 * current_index + 1);
      if(last_element.key < minimal_child->key)
      {
         heap->root_minus_one[current_index] = last_element;
         break;
      }
      heap->root_minus_one[current_index] = *minimal_child;
      *minimal_child                      =  last_element;
      
      current_index = minimal_index;
   }
   
   return ret;
}


int main(int argc, char* argv[]) 
{
   
   if(argc < 2)
   {
      console_print("ERROR: Wrong no arguments detected. Please specify an input file. \n");
      return 1;
   }
   
   if(argc > 2)
   {
      console_print("WARNING: More then one argument detected. Ignored arguments are: \n");
      for(int arg = 2; arg < argc; arg++)
      {
         console_print("'");
         console_print(argv[arg]);
         console_print("' ");
      }
      console_print("\n");
      //return 1;
   }
   
   memory_arena *arena;
   { // @cleanup implement a real arena api, that we then just implement via malloc garbage
      // @cleanup how much do we need exacly someting like (2<<20)*(sizeof(key_value_pair) + sizeof(???)) or something...?
      u64 arena_size = mega_bytes(100);
      u8 *buffer = cast(u8 *)malloc(arena_size);
      if(!buffer) // lame bullshit
      {
         console_print("Error: Unable to allocate memory. \n");
         return 1;
      }
      arena       = cast(memory_arena *)buffer;
      arena->size = arena_size;
      arena->current = (u8 *)buffer + sizeof(memory_arena);
   }
   
   char *file_directory = argv[1];
   loaded_file file = load_entire_file(file_directory, arena);
   
   if(!file.memory)
   {
      console_print("ERROR: Could not open the file: '%s' \n", file_directory);
      return 1;
   }
   
   console_print("\n");
   console_print("LOG: Beginning to parse file '%s'.\n", file_directory);
   console_print("\n");
   
   terminal_array terminals = zero_struct;
   {
      string remaining = create_string(file.memory, file.size);
      
      if(!remaining.length)
      {
         console_print("ERROR: Loaded file was empty. \n");
         return 1;
      }
      
      bool success = true;
      eat_leading_whitespaces_and_newlines(&remaining);
      
      u32 specified_amount_of_terminals = eat_a_u32(&remaining, &success);
      
      if(specified_amount_of_terminals > 20)
      {
         console_print("ERROR: More then 20 terminals is not supported. \n");
         console_print("Requested amount of terminals was %u. \n", specified_amount_of_terminals);
         return 1;
      }
      
      eat_a_line(&remaining);
      
      //daisy chain the terminals into a buffer
      terminal *terminal_data = push_data(arena, terminal, 0);
      u64 line                = 1;
      while(remaining.length)
      {
         line++;
         terminal *t = push_struct(arena, terminal);
         for(u64 i = 0; i < 3; i++)
         {
            eat_leading_whitespaces(&remaining);
            t->values[i] = eat_a_u32(&remaining, &success);
         }
         
         string rest_line = eat_a_line(&remaining);
         
         if(!success)
         {
            console_print("ERROR: Could not read a number in line: %u \n", line);
            return 1;
         }
         
         if(!is_only_whitespaces_or_newlines(rest_line))
         {
            console_print("ERROR: Junk after line: %u. Junk: '%s'\n", line, rest_line.data);
            return 1;
         }
         
      }
      u64 amount_of_terminals = cast(u64)((terminal *)arena->current - terminal_data);
      
      if(amount_of_terminals != specified_amount_of_terminals)
      {
         // @reserche, what are the right %bla's here? we need 64 bit and 32 bit?
         console_print("ERROR: Intended amount of terminals was %u, but we got %u.", specified_amount_of_terminals, amount_of_terminals);
         return 1;
      }
      
      terminals.data                = terminal_data;
      terminals.amount_of_terminals = amount_of_terminals;
   }
   
   
   min_heap heap = zero_struct;
   {
      // @incomplete init heap
   }
   
   typedef struct {
      void *value;
      //u32 terminal_index;  we probably just want to cast the void * to this...
      //u32 terminal_bitset;
      u64 label;
   } value_label_pair;
   
   
   
   while (heap.heap_size > 0)
   {
      key_value_pair min = min_heap_pop_minimal_element(&heap);
      
   }
   
   u64 steiner_tree_length = 0;
   
   console_print("SUCCESS: Length of a minimal Steiner tree is: %u \n", steiner_tree_length);
   console_print("\n");
   
   
   
	return 0;
}
