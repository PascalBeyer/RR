
#define _CRT_SECURE_NO_WARNINGS 
// @tempoary

#include <stdio.h>
#include <stdlib.h> // @cleanup maybe lern how to do these things with linux....
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>

typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef u8 bool;
#define true 1
#define false 0

#define max_u8  (u8)(-1)
#define max_u16 (u16)(-1)
#define max_u32 (u32)(-1)
#define max_u64 (u64)(-1)

#define cast(type) (type)
#define proc static
#define null ((void *)0)
#define func static
#define constant_array_size(a) (sizeof(a)/sizeof(*(a)))

#define kilo_bytes(a) (a) * 1024
#define mega_bytes(a) (kilo_bytes(a)) * 1024
#define giga_bytes(a) (mega_bytes(a)) * 1024

static u32 console_print(char *format, ...);
static void panic(s32 exit_code);

#define zero_struct(s) memset(&(s), 0, sizeof(s))

#if __TINYC__
#define _Debug 1
#endif

#if _Debug
#define only_debug(a) a;
#define assert(expr) if(!(expr)) {console_print("%s:%i: ASSERT FIRED: '%s' in Function '%s'.\n",  __FILE__,  __LINE__, #expr, __FUNCTION__); panic(1);}


#define concat_macro_3(a, b, c) a##b##c
#define compile_time_assert(expr) int concat_macro_3(__compile_time_assert_array, __FILE__, __LINE__)[(expr) ? 1 : -1]

#define not_implemented assert(!"not implemented!")
#define invalid_code_path assert(!"invalid code path")
#define invalid_default_case default:{ assert(!"invalid default case"); } break;

#else // _Debug
#define only_debug(a)
#define assert(a) 
#define not_implemented
#define invalid_code_path 
#define invalid_default_case
#define compile_time_assert(a)
#endif

func bool pointer_in_memory_range(void *pointer, void* base, size_t range_size)
{
   bool smaller = (base <= pointer);
   void *end    = (cast(u8 *)base + range_size);
   bool bigger  = (pointer < end);
   return smaller && bigger;
}

struct loaded_file
{
   u8 *memory;
   size_t size;
};

static struct loaded_file load_entire_file(char *directory)
{
   struct loaded_file ret; 
   zero_struct(ret);
   
   FILE *file = fopen(directory, "rb");
   if(!file)
   {
      return ret;
   }
   
   fseek(file, 0, SEEK_END);
   u64 file_size = ftell(file);
   rewind(file);
   
   u8 *buffer   = cast(u8*)malloc(file_size);
   if(!buffer)  return ret;
   fread(buffer, 1, file_size, file);
   
   fclose(file);
   
   ret.size   = file_size;
   ret.memory = buffer;
   
   return ret;
}


static void panic(s32 exit_code)
{
   bool ddh = true;
   if(ddh)
   {
      exit(exit_code);
   }
}

static u32 console_print(char *format, ...)
{
   va_list va;
   va_start(va, format);
   int chars_written = vprintf(format, va);
   va_end(va);
   
   fflush(stdout);
   
   chars_written = (chars_written > 0) ? chars_written : 0;
   
   return (u32)chars_written;
}

struct terminal
{
   u32 x;
   u32 y;
   u32 z;
};

func u32 abs_diff_u32(u32 a, u32 b)
{
   u32 ret = (a > b) ? (a - b) : (b - a);
   return ret;
}

func u32 manhatten_distance(struct terminal t1, struct terminal t2)
{
   u32 dx = abs_diff_u32(t1.x, t2.x);
   u32 dy = abs_diff_u32(t1.y, t2.y);
   u32 dz = abs_diff_u32(t1.z, t2.z);
   
   u32 ret = dx + dy + dz;
   return ret;
}

struct terminal_index_set_pair
{
   u32 index_set;
   // @note: the first 12 bits are the 400 * x_index + 20 * y_index + z_index into the terminal coordinate arrays as these are at most 19, this means this is at most 7999 < 8192 = (1 << 13); fits in 12 bits.
   // @note: the last 20 bit are the terminals set.
};

func u32 pack_xyz_in_base_20(u32 x, u32 y, u32 z)
{
   assert((x < 20) && (y < 20) && (z < 20));
   return (400 * x + 20 * y + z);
}

func struct terminal_index_set_pair create_terminal_index_set_pair(u32 xyz_in_base_20, u32 set)
{
   assert(!(set >> 20));
   assert(xyz_in_base_20 < 8000);
   struct terminal_index_set_pair ret;
   ret.index_set = (xyz_in_base_20 << 20) | set;
   return ret;
}

func u32 get_xyz_from_index_set_pair(struct terminal_index_set_pair pair)
{
   u32 index = pair.index_set >> 20;
   return index;
}
func u32 get_x_from_index_set_pair(struct terminal_index_set_pair pair)
{
   u32 index = pair.index_set >> 20;
   u32 x     = (index / 400);
   
   assert(x < 20);
   return x;
}

func u32 get_y_from_index_set_pair(struct terminal_index_set_pair pair)
{
   u32 index = pair.index_set >> 20;
   u32 y     = (index / 20) % 20;
   assert(y < 20);
   return y;
}

func u32 get_z_from_index_set_pair(struct terminal_index_set_pair pair)
{
   u32 index = pair.index_set >> 20;
   u32 z     = index % 20;
   assert(z < 20);
   return z;
}

func u32 get_set_from_index_set_pair(struct terminal_index_set_pair pair)
{
   u32 set = pair.index_set & ((1<<20) - 1);
   return set;
}

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
      if(number_value > 9)// || number_value < 0)
      {
         *success = false;
         return max_u32;
      }
      ret = number_value;
   }
   
   for(u32 i = 1; i < to_eat->length; i++)
   {
      u32 number_value = *(to_eat->data + i) - '0';
      if(number_value > 9)// || number_value < 0)
      {
         to_eat->length -= i;
         to_eat->data   += i;
         return ret;
      }
      ret = 10 * ret + number_value;
   }
   
   // we ate everything.
   to_eat->data += to_eat->length;
   to_eat->length = 0;
   
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
   
   return ret;
}


struct terminal_set_label
{
   struct terminal_index_set_pair value;
   u32 label;
}; // @cleanup: name is kinda dumb, but whatever

func struct terminal_set_label create_terminal_set_label(struct terminal_index_set_pair value, u32 label)
{
   struct terminal_set_label ret;
   ret.value = value;
   ret.label = label;
   return ret;
}

struct hash_map_entry
{
   struct terminal_set_label *value;
   u32 hash;
};
struct hash_map
{
   struct hash_map_entry *entries; // this is zero terminated.
   u64 size;
   u64 log2_capacity;
};

// @cleanup think about what the _best_ hash is here.
func u32 cheap_and_dirty_hash(u32 hash, u32 number_of_bits_to_return)
{
   assert(number_of_bits_to_return > 12);
   assert(number_of_bits_to_return < 33);
   u32 lower_part = number_of_bits_to_return - 12;
   u32 top        = hash >> 20;
   u32 mask       = (1 << lower_part) - 1;
   u32 bot        = hash & mask;
   u32 index      = (top << lower_part) | bot;
   return index;
}

// returns a valid place if it does not exist
func struct hash_map_entry* hash_map_get_internal(struct hash_map *hash_map, u32 hash)
{
   u32 index    = cheap_and_dirty_hash(hash, hash_map->log2_capacity);
   u32 capacity = (1 << hash_map->log2_capacity);
   assert(index < capacity);
   struct hash_map_entry *entry = hash_map->entries + index;
   while(entry->hash && entry->hash != hash)
   {
      entry++;
   }
   
   // if we are outside of the range we reset to 0, and start again
   if(cast(u64) (entry -  hash_map->entries) > capacity)
   {
      entry= hash_map->entries;
      while(entry->hash && entry->hash != hash)
      {
         entry++;
      }
   }
   
   return entry;
}

func int yay_for_c_style_polymorphism(const void* _a, const void* _b)
{
   u32 a = *cast(u32 *)_a;
   u32 b = *cast(u32 *)_b;
   return (cast(s64)a - cast(s64) b);
}

func struct terminal_set_label *hash_map_get(struct hash_map *hash_map, u32 hash)
{
   struct hash_map_entry *entry = hash_map_get_internal(hash_map, hash);
   assert(!entry->hash || entry->hash == hash);
   
   return entry->value;
}

// @incomplete guard all allocation points?
func void hash_map_grow(struct hash_map* hash_map)
{
   struct hash_map_entry *old_entries = hash_map->entries;
   u64 old_capacity                   = (1 << hash_map->log2_capacity);
   
   hash_map->log2_capacity += 1;
   void *cleared_memory     = calloc((1 << hash_map->log2_capacity) + 1, sizeof(struct hash_map_entry));
   assert(cleared_memory); // @Tempoary (maybe)
   hash_map->entries = cast(struct hash_map_entry *)cleared_memory;
   
   for(u32 i = 0; i < old_capacity; i++)
   {
      struct hash_map_entry to_insert = old_entries[i];
      if(!to_insert.hash) continue;
      
      u32 hash = to_insert.hash;
      struct hash_map_entry *location = hash_map_get_internal(hash_map, hash);
      *location = to_insert;
   }
   
   free(old_entries);
}

func void hash_map_change_or_create(struct hash_map *hash_map, struct terminal_set_label *value)
{
   u32 hash = value->value.index_set;
   assert(hash); // @note, as we are never considering empty sets, this will never fire
   struct hash_map_entry *location = hash_map_get_internal(hash_map, hash);
   
   if(!location->hash)
   {
      hash_map->size++;
   }
   
   struct hash_map_entry entry_to_add;
   entry_to_add.hash  = hash;
   entry_to_add.value = value;
   
   *location = entry_to_add;
   
   if(hash_map->size > (1 << (hash_map->log2_capacity - 3))) // @cleanup: which condtition here?
   {
      hash_map_grow(hash_map);
   }
}

struct min_heap
{
   struct terminal_set_label *root_minus_one;
   struct hash_map *hash_map;
   u32 heap_size; // we are using the elements [1, heap_size]
   u32 heap_capacity; // heap capacity includes the pre_root element
   
   // for future costs
   u32 *x_values;
   u32 *y_values;
   u32 *z_values;
   u32 *terminal_indices;
   u32 amount_of_terminals;
};


func void min_heap_change_element(struct min_heap *heap, u32 index, struct terminal_set_label change_to)
{
   assert(index != 0); // our root is at one
   assert(index <= heap->heap_size);
   
   struct terminal_set_label *label = heap->root_minus_one + index;
   *label                           = change_to;
   
   hash_map_change_or_create(heap->hash_map, label);
}

func u32 calculate_future_cost(struct min_heap *heap, struct terminal_index_set_pair pair)
{
   u32 *x_values           = heap->x_values;
   u32 *y_values           = heap->y_values;
   u32 *z_values           = heap->z_values;
   u32 *terminal_indices   = heap->terminal_indices;
   u32 amount_of_terminals = heap->amount_of_terminals;
   u32 all_terminals = (1 << (amount_of_terminals)) - 1;
   
   u32 x_index = get_x_from_index_set_pair(pair);
   u32 y_index = get_y_from_index_set_pair(pair);
   u32 z_index = get_z_from_index_set_pair(pair);
   u32 set     = get_set_from_index_set_pair(pair);
   
   u32 complement = all_terminals & (~set);
   
   u32 min_x = x_values[x_index];
   u32 max_x = x_values[x_index];
   
   u32 min_y = y_values[y_index];
   u32 max_y = y_values[y_index];
   
   u32 min_z = z_values[z_index];
   u32 max_z = z_values[z_index];
   
   for(u32 j = 0; j < amount_of_terminals; j++)
   {
      if(complement & (1 << j))
      {
         u32 terminal_index = terminal_indices[j];
         
         // @cleanup: formulas copied
         u32 x = x_values[(terminal_index / 400)];
         u32 y = y_values[(terminal_index / 20) % 20];
         u32 z = z_values[terminal_index % 20];
         max_x = (x > max_x) ? x : max_x;
         max_y = (y > max_y) ? y : max_y;
         max_z = (z > max_z) ? z : max_z;
         
         min_x = (x < min_x) ? x : min_x;
         min_y = (y < min_y) ? y : min_y;
         min_z = (z < min_z) ? z : min_z;
      }
   }
   
   u32 future_cost = (max_x - min_x) + (max_y - min_y) + (max_z - min_z);
   return future_cost;
}

proc void min_heap_heapify_up(struct min_heap *heap, u32 current_index, struct terminal_set_label to_heapify)
{
   u32 compare_value = calculate_future_cost(heap, to_heapify.value);
   while (true)
   {
      u64 parent_index = current_index >> 1;
      if(!parent_index)
      {
         break;
      }
      struct terminal_set_label parent = heap->root_minus_one[parent_index];
      
      u32 future_cost = calculate_future_cost(heap, parent.value);
      if(parent.label + future_cost <= to_heapify.label + compare_value)
      {
         break;
      }
      
      min_heap_change_element(heap, current_index, parent);
      current_index = parent_index;
   }
   min_heap_change_element(heap, current_index, to_heapify);
}

proc void min_heap_heapify_down(struct min_heap *heap, u32 current_index, struct terminal_set_label to_heapify)
{
   
   u32 compare_value = calculate_future_cost(heap, to_heapify.value);
   while ((2 * current_index + 1 <= heap->heap_size))
   {
      struct terminal_set_label *left_child  = heap->root_minus_one + 2 * current_index;
      struct terminal_set_label *right_child = heap->root_minus_one + 2 * current_index + 1;
      
      u32 future_cost_left  = calculate_future_cost(heap, left_child->value);
      u32 future_cost_right = calculate_future_cost(heap, right_child->value);
      
      u32 left_child_smaller = (left_child->label + future_cost_left) < (right_child->label + future_cost_right);
      
      struct terminal_set_label *minimal_child = (left_child_smaller) ? left_child : right_child;
      u64 minimal_index = (left_child_smaller) ? (2 * current_index) : (2 * current_index + 1);
      
      u32 minimal_cost = left_child_smaller ? (left_child->label + future_cost_left) : (right_child->label + future_cost_right);
      
      if(to_heapify.label + compare_value <= minimal_cost)
      {
         break;
      }
      
      min_heap_change_element(heap, current_index, *minimal_child);
      current_index = minimal_index;
   }
   
   if(2 * current_index == heap->heap_size)
   {
      struct terminal_set_label *left_child  = heap->root_minus_one + 2 * current_index;
      u32 future_cost = calculate_future_cost(heap, left_child->value);
      
      if(to_heapify.label + compare_value > left_child->label + future_cost)
      {
         min_heap_change_element(heap, current_index, *left_child);
         current_index *= 2;
      }
   }
   
   min_heap_change_element(heap, current_index, to_heapify);
}

proc void min_heap_insert(struct min_heap *heap, struct terminal_set_label to_insert)
{
   heap->heap_size++;
   
   if(heap->heap_size >= heap->heap_capacity)
   {
      // this is where we pray to god that malloc decided to allocate far apart
      
      u64 old_heap_capacity                 = heap->heap_capacity;
      heap->heap_capacity <<= 1;
      heap->root_minus_one = cast(struct terminal_set_label *)realloc(heap->root_minus_one,heap->heap_capacity * sizeof(struct terminal_set_label));
      
      assert(heap->root_minus_one);
      
      // @incomplte long jump to cleanup code on failure?
      // this shit really makes me want to lern the linux virtual memory api T.T, then we would not have to do this
      for(u64 i = 1; i < old_heap_capacity; i++)
      {
         struct terminal_set_label *cur = heap->root_minus_one + i;
         hash_map_change_or_create(heap->hash_map, cur);
      }
   }
   
   u32 num_hash = heap->hash_map->size;
   
   u64 current_index = heap->heap_size;
   min_heap_heapify_up(heap, current_index, to_insert);
   
   assert((num_hash + 1) == heap->hash_map->size);
}

static struct terminal_set_label min_heap_pop_minimal_element(struct min_heap *heap)
{
   struct terminal_set_label ret = heap->root_minus_one[1]; // this is our root.
   if(1 == heap->heap_size)
   {
      heap->heap_size = 0;
      return ret;
   }
   
   struct terminal_set_label last_element = heap->root_minus_one[heap->heap_size];
   heap->heap_size--;
   min_heap_heapify_down(heap, 1, last_element);
   
#if _Debug
   zero_struct(heap->root_minus_one[heap->heap_size + 1]);
#endif
   
   return ret;
}

// @note this decreases the _key_, i.e the label of the index.
static void min_heap_dikrisky(struct min_heap *heap, u32 index, u32 new_label)
{
   assert(index <= heap->heap_size);
   
   struct terminal_set_label to_change = heap->root_minus_one[index];
   
   assert(to_change.label > new_label);
   
   u32 num_hash = heap->hash_map->size;
   
   to_change.label = new_label;
   min_heap_heapify_up(heap, index, to_change);
   
   assert(num_hash == heap->hash_map->size);
}

static void change_label_or_insert_into_min_heap(struct min_heap *heap, struct terminal_index_set_pair to_change, struct terminal_set_label *label, u32 maybe_new_label)
{
   if(!label)
   {
      struct terminal_set_label to_insert = create_terminal_set_label(to_change, maybe_new_label);
      min_heap_insert(heap, to_insert);
      return;
   }
   
   if(pointer_in_memory_range(label, heap->root_minus_one, sizeof(struct terminal_set_label) * heap->heap_capacity))
   {
      u32 maybe_old_label = label->label;
      if(maybe_old_label > maybe_new_label)
      {
         u32 index = cast(u32)(label - heap->root_minus_one);
         min_heap_dikrisky(heap, index, maybe_new_label);
      }
      
      return;
   }
   
   // if we get here then we are in the finished array.
   // so by some property there is nothing to do anymore.
   assert(label->label <= maybe_new_label);
}

struct finished_array
{
   struct terminal_set_label *data;
   u32 amount;
   u32 capacity;
};

static void finished_array_add(struct hash_map *map, struct finished_array *arr, struct terminal_set_label data)
{
   if(arr->amount + 1 > arr->capacity)
   {
      arr->capacity = 2 * arr->capacity + 2;
      arr->data = cast(struct terminal_set_label *)realloc(arr->data, arr->capacity * sizeof(struct terminal_set_label));
      
      
      for(u64 i = 0; i < arr->amount; i++)
      {
         struct terminal_set_label *cur = arr->data + i;
         hash_map_change_or_create(map, cur);
      }
   }
   
   struct terminal_set_label *to_add = arr->data + arr->amount;
   arr->data[arr->amount++] = data;
   
   u32 num_hash = map->size;
   hash_map_change_or_create(map, to_add);
   assert(num_hash == map->size);
}

int main(int argc, char* argv[])
{
   clock_t begintime = clock();
   
   if(argc < 2)
   {
      console_print("ERROR: NO arguments detected. Please specify an input file. \n");
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
   
   char *file_directory = argv[1];
   struct loaded_file file = load_entire_file(file_directory);
   
   if(!file.memory)
   {
      console_print("ERROR: Could not open the file: '%s' \n", file_directory);
      return 1;
   }
   
   console_print("\n");
   console_print("LOG: Beginning to parse file '%s'.\n", file_directory);
   console_print("\n");
   
   u32 amount_of_terminals;
   struct terminal terminals[20];
   zero_struct(terminals);
   
   { // daisy chain terminals
      string remaining = create_string(cast(char *)file.memory, file.size);
      
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
      u64 line                = 1;
      u32 terminal_indexer    = 0;
      while(remaining.length)
      {
         // @cleanup: allow empty lines?
         line++;
         struct terminal *terminal = terminals + terminal_indexer++;
         
         {
            eat_leading_whitespaces(&remaining);
            terminal->x = eat_a_u32(&remaining, &success);
            eat_leading_whitespaces(&remaining);
            terminal->y = eat_a_u32(&remaining, &success);
            eat_leading_whitespaces(&remaining);
            terminal->z = eat_a_u32(&remaining, &success);
         }
         
         string rest_line = eat_a_line(&remaining);
         
         if(!success)
         {
            console_print("ERROR: Could not read a number in line: %u \n", line);
            return 1;
         }
         
         
         if(!is_only_whitespaces_or_newlines(rest_line))
         {
            console_print("ERROR: Junk after line: %u. Junk: '%.*s'\n", line, rest_line.length, rest_line.data);
            
            return 1;
         }
         
         if(terminal_indexer > 20)
         {
            console_print("ERROR: We got more then 20 terminals.\n");
            return 1;
         }
         
         // @cleanup: debug
         if(remaining.data && remaining.data[0] == '#')
         {
            break;
         }
         
      }
      amount_of_terminals = terminal_indexer;
      
      //console_print("LOG: amount_of_terminals is %u\n", amount_of_terminals);
      
      if(amount_of_terminals  != specified_amount_of_terminals)
      {
         // @reserche, what are the right %bla's here? we need 64 bit and 32 bit?
         console_print("ERROR: Intended amount of terminals was %u, but we got %u.\n", specified_amount_of_terminals, amount_of_terminals);
         return 1;
      }
   }
   free(file.memory);
   
   console_print("LOG: %u terminals detected.\n", amount_of_terminals);
   
#if 0   
   for(u32 i = 0; i < specified_amount_of_terminals; i++)
   {
      struct terminal *t = terminal + i;
      console_print("Terminal %u: (%u, %u, %u)\n", i, t->x, t->y, t->z);
   }
#endif
   
   // setup the hannan grid
   u32 amount_of_x_values;
   u32 x_values[20];
   u32 amount_of_y_values;
   u32 y_values[20];
   u32 amount_of_z_values;
   u32 z_values[20];
   u32 terminal_indices[20];
   
   {
      for(u32 i = 0; i < amount_of_terminals; i++)
      {
         x_values[i] = terminals[i].x;
         y_values[i] = terminals[i].y;
         z_values[i] = terminals[i].z;
      }
      
      qsort(x_values, amount_of_terminals, sizeof(u32), yay_for_c_style_polymorphism);
      qsort(y_values, amount_of_terminals, sizeof(u32), yay_for_c_style_polymorphism);
      qsort(z_values, amount_of_terminals, sizeof(u32), yay_for_c_style_polymorphism);
      
      // remove doublicate values
      u32 dx = 0;
      u32 dy = 0;
      u32 dz = 0;
      for(u32 i = 1; i < amount_of_terminals; i++)
      {
         if(x_values[dx] != x_values[i])
         {
            x_values[++dx] = x_values[i];
         }
         if(y_values[dy] != y_values[i])
         {
            y_values[++dy] = y_values[i];
         }
         if(z_values[dz] != z_values[i])
         {
            z_values[++dz] = z_values[i];
         }
      }
      amount_of_x_values = dx + 1;
      amount_of_y_values = dy + 1;
      amount_of_z_values = dz + 1;
      
      for(u32 i = 0; i < amount_of_terminals; i++)
      {
         u32 x_index = 0; // for speed purposes we should not initialize these, but -Wall @sigh
         u32 y_index = 0;
         u32 z_index = 0;
         for(u32 k = 0; k < amount_of_x_values; k++)
         {
            if(x_values[k] == terminals[i].x)
            {
               x_index = k;
               break;
            }
         }
         for(u32 k = 0; k < amount_of_y_values; k++)
         {
            if(y_values[k] == terminals[i].y)
            {
               y_index = k;
               break;
            }
         }
         for(u32 k = 0; k < amount_of_z_values; k++)
         {
            if(z_values[k] == terminals[i].z)
            {
               z_index = k;
               break;
            }
         }
         terminal_indices[i] = pack_xyz_in_base_20(x_index, y_index, z_index);
      }
      
   }
   
   
   struct hash_map _hash_map;
   struct hash_map *map = &_hash_map;
   {
      map->log2_capacity = 17; // @cleanup, what number here?
      map->size          = 0;
      // one pad so we can go circular
      u64 map_capacity = (1 << map->log2_capacity) + 1;
      map->entries = cast(struct hash_map_entry *)calloc(map_capacity, sizeof(struct hash_map_entry));
   }
   
   // @cleanup: the good version of this does not take the hash_map...
   struct min_heap _heap;
   struct min_heap *heap = &_heap;
   {
      heap->heap_capacity = (1 << 13);
      heap->heap_size     = 0;
      heap->hash_map      = map;
      u32 memory_needed   = (heap->heap_capacity * sizeof(struct terminal_set_label));
      heap->root_minus_one = cast(struct terminal_set_label *)malloc(memory_needed);
      
      // future cost stuff
      heap->x_values            = x_values;
      heap->y_values            = y_values;
      heap->z_values            = z_values;
      heap->terminal_indices    = terminal_indices;
      heap->amount_of_terminals = amount_of_terminals;
      
   }
   
   struct finished_array *finished = cast(struct finished_array *)calloc(8000, sizeof(struct finished_array));
   
   // step 1: the last terminal is our t
   for(u32 i = 0; i < amount_of_terminals - 1; i++) 
   {
      struct terminal_index_set_pair inp;
      inp = create_terminal_index_set_pair(terminal_indices[i], (1 << i));
      
      u32 label = 0;
      struct terminal_set_label to_insert = create_terminal_set_label(inp, label);
      
      min_heap_insert(heap, to_insert);
   }
   
   // step 2: this is unneccesary, as we never touch anything with zero set
#if 0 // @note: we need this fact now, as we regard the set index pair = 0 as invalid hash
   for(u32 i = 0; i < amount_of_terminals; i++)
   {
      struct terminal_index_set_pair inp;
      inp = create_terminal_index_set_pair(terminal_indices[i], 0);
      
      struct terminal_set_label to_insert = create_terminal_set_label(inp, max_u32);
      
      struct terminal_set_label *to_add = push_struct(finished_set, struct terminal_set_label);
      *to_add = to_insert;
      hash_map_change_or_create(map, to_add);
   }
#endif
   
   u32 steiner_tree_length            = max_u32;
   u32 minimal_steiner_tree_found     = false;
   
   // t corresponds to the _top-bit_ terminal
   u32 all_terminals_but_t                        = (1 << (amount_of_terminals - 1)) - 1;
   struct terminal_index_set_pair t_and_all_but_t = create_terminal_index_set_pair(terminal_indices[amount_of_terminals - 1], all_terminals_but_t);
   
   while (heap->heap_size > 0)
   {
      // step 3:
      struct terminal_set_label min = min_heap_pop_minimal_element(heap);
      
      u32 min_xyz     = get_xyz_from_index_set_pair(min.value);
      u32 min_x_index = get_x_from_index_set_pair(min.value);
      u32 min_y_index = get_y_from_index_set_pair(min.value);
      u32 min_z_index = get_z_from_index_set_pair(min.value);
      u32 min_set     = get_set_from_index_set_pair(min.value);
      u32 min_label   = min.label;
      
      assert(min_xyz < 8000);
      
      // step 4:
      { 
         struct finished_array *array = finished + min_xyz;
         finished_array_add(map, array, min);
      }
      
      // step 5:
      if(min.value.index_set == t_and_all_but_t.index_set)
      {
         steiner_tree_length        = min_label;
         minimal_steiner_tree_found = true;
         break;
      }
      
      // step 6: 
      // @cleanup: I realized that terminal is a bad name...
      struct terminal current;
      current.x = x_values[min_x_index];
      current.y = y_values[min_y_index];
      current.z = z_values[min_z_index];
      
      for(u32 i = 0; i < 6; i++)
      {
         // @cleanup: I realized that terminal is a bad name...
         struct terminal other;
         u32 other_xyz;
         switch(i)
         {
            case 0:// +x
            {
               if(min_x_index + 1 >= amount_of_x_values) continue;
               other.x   = x_values[min_x_index + 1];
               other.y   = y_values[min_y_index];
               other.z   = z_values[min_z_index];
               other_xyz = pack_xyz_in_base_20(min_x_index + 1, min_y_index, min_z_index);
            }break;
            case 1:// -x
            {
               if(min_x_index == 0) continue;
               other.x   = x_values[min_x_index - 1];
               other.y   = y_values[min_y_index];
               other.z   = z_values[min_z_index];
               other_xyz = pack_xyz_in_base_20(min_x_index - 1, min_y_index, min_z_index);
            }break;
            case 2: // +y 
            {
               if(min_y_index + 1 >= amount_of_y_values) continue;
               other.x   = x_values[min_x_index];
               other.y   = y_values[min_y_index + 1];
               other.z   = z_values[min_z_index];
               other_xyz = pack_xyz_in_base_20(min_x_index, min_y_index + 1, min_z_index);
            }break;
            case 3: // -y
            {
               if(min_y_index == 0) continue;
               other.x   = x_values[min_x_index];
               other.y   = y_values[min_y_index - 1];
               other.z   = z_values[min_z_index];
               other_xyz = pack_xyz_in_base_20(min_x_index, min_y_index - 1, min_z_index);
            }break;
            case 4: // +z
            {
               if(min_z_index + 1 >= amount_of_z_values) continue;
               other.x   = x_values[min_x_index];
               other.y   = y_values[min_y_index];
               other.z   = z_values[min_z_index + 1];
               other_xyz = pack_xyz_in_base_20(min_x_index, min_y_index, min_z_index + 1);
            }break;
            case 5: // -z
            {
               if(min_z_index == 0) continue;
               other.x   = x_values[min_x_index];
               other.y   = y_values[min_y_index];
               other.z   = z_values[min_z_index - 1];
               other_xyz = pack_xyz_in_base_20(min_x_index, min_y_index, min_z_index - 1);
            }break;
         }
         
         struct terminal_index_set_pair to_change = create_terminal_index_set_pair(other_xyz, min_set);
         struct terminal_set_label *label         = hash_map_get(map, to_change.index_set);
         u32 maybe_new_label = manhatten_distance(current, other) + min_label;
         
         change_label_or_insert_into_min_heap(heap, to_change, label, maybe_new_label);
      }
      
      // step 7:
      u32 T_without_min_set_and_t = all_terminals_but_t & (~min_set);
      
      struct finished_array array_to_iterate = finished[min_xyz];
      
      for(u32 i = 0; i < array_to_iterate.amount; i++)
      {
         struct terminal_set_label *other_label = array_to_iterate.data + i;
         assert(get_xyz_from_index_set_pair(other_label->value) == min_xyz);
         
         u32 other_set = get_set_from_index_set_pair(other_label->value);
         assert(other_set); // this should never be the empty set
         u32 union_set = other_set | T_without_min_set_and_t;
         if(union_set != T_without_min_set_and_t) continue;
         
         struct terminal_index_set_pair to_change = create_terminal_index_set_pair(min_xyz, other_set | min_set);
         
         struct terminal_set_label *label_to_change = hash_map_get(map, to_change.index_set);
         
         u32 maybe_new_label = min_label + other_label->label;
         change_label_or_insert_into_min_heap(heap, to_change, label_to_change, maybe_new_label);
      }
   }
   
   console_print("SUCCESS: Length of a minimal Steiner tree is: %u \n", steiner_tree_length);
   console_print("\n");
   
   free(heap->root_minus_one);
   free(map->entries);
   for(u32 i = 0; i < sizeof(finished); i++)
   {
      free(finished[i].data);
   }
   free(finished);
   
   clock_t endtime = clock(); // this has terrible accuracy....
   
   if(minimal_steiner_tree_found)
   {
      console_print("TIME: %f seconds.\n", (float)(endtime - begintime) / (float)CLOCKS_PER_SEC);
   }
   else
   {
      console_print("ERROR: An error accured while exectuting the algorithm.\n");
   }
   
   return 0;
}
