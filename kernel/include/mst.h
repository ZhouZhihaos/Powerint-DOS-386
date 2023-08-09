#ifndef _MST_H
#define _MST_H
#define PRIVATE static
#define PUBLIC
#define MST_API
#include <dos.h>
#include <stddef.h>

//#define MEM_LEAK_CHK

typedef enum {
  NUMBER,
  STRING,
  SPACE_START,
  SPACE_END,
  ARRAY_START,
  ARRAY_END,
  OP,
  WORD
} TOKEN_TYPE;
typedef struct {
  char* tok;
  TOKEN_TYPE t;
} TOKEN;
typedef enum {
  STRING_OP_ERROR = 1,
  UNKNOW_TOKEN,
  ERROR_TOKEN,
  SYNTAX_ERROR,
  WRONG_TYPE_TO_ADD
} ERR_TYPE;
typedef enum { INTEGER, ARRAY, SPAC, STR } VAR_TYPE;
typedef struct {
  List* the_space;
} SPACE;
typedef struct {
  char* string;
  unsigned len;
  List* token;
  SPACE* root_space;
  int err;
} MST_Object;
typedef struct {
  uint32_t num;
} Integer;
typedef struct {
  char* str;
} String;
typedef struct {
  List* the_array;
} Array;
typedef struct {
  VAR_TYPE vt;
  char* name;
  void* obj;
} Var;
typedef struct {
  VAR_TYPE vt;
  void* obj;
} Array_data;
/*
    mini set 解析路线
    Space->var->Array->string/integer
    Space->var->Space……
    Space->var->string/integer
    Space中放Var，Var中可以放Space、Array、integer、string
    Parser会设置一个叫做`root_space`的空间以存放Var
 */

PUBLIC MST_API MST_Object* Init_MstObj(char* string);
PUBLIC MST_API Var* MST_GetVar(char* name, SPACE* space);
PUBLIC MST_API int MST_Space_GetInteger(Var* var);
PUBLIC MST_API SPACE* MST_Space_GetSpace(Var* var);
PUBLIC MST_API Array* MST_Space_GetArray(Var* var);
PUBLIC MST_API char* MST_Space_GetStr(Var* var);
PUBLIC MST_API Array_data* MST_Array_Get(Array* arr, int idx);
PUBLIC MST_API int MST_Array_get_integer(Array_data* ad);
PUBLIC MST_API char* MST_Array_get_str(Array_data* ad);
PUBLIC MST_API SPACE* MST_Array_get_space(Array_data* ad);
PUBLIC MST_API Array* MST_Array_get_array(Array_data* ad);
PUBLIC MST_API void MST_FreeObj(MST_Object* mst);

PUBLIC MST_API const char* MST_strerror(MST_Object* mst);
PUBLIC MST_API void MST_add_data_to_array(MST_Object* mst_obj,
                                          Array* arr,
                                          Array_data ad);
PUBLIC MST_API char* MST_build_to_string(MST_Object* mst_obj);
PUBLIC MST_API void MST_add_var_to_space(MST_Object* mst_obj,
                                         SPACE* sp,
                                         Var var);
PUBLIC MST_API Var MST_var_make_integer(char* name, int val);
PUBLIC MST_API Var MST_var_make_string(char* name, char* ss);
PUBLIC MST_API Array_data MST_arr_dat_make_integer(int val);
PUBLIC MST_API Array_data MST_arr_dat_make_string(char* ss);
PUBLIC MST_API void MST_add_empty_array_to_array(MST_Object* mst_obj,
                                                 Array* arr);
PUBLIC MST_API void MST_add_empty_space_to_array(MST_Object* mst_obj,
                                                 Array* arr);
PUBLIC MST_API void MST_add_empty_array_to_space(MST_Object* mst_obj,
                                                 SPACE* sp,
                                                 char* name);
PUBLIC MST_API void MST_add_empty_space_to_space(MST_Object* mst_obj,
                                                 SPACE* sp,
                                                 char* name);
PUBLIC MST_API void MST_change_var(MST_Object* mst_obj, Var v, Var* v1);
PUBLIC MST_API void MST_change_arr(MST_Object* mst_obj,
                                   Array_data v,
                                   Array_data* v1);
#define MST_GetRootSpace(m) (m)->root_space
// MST_get_space_in_space
#define MST_get_space_in_space(m, name, space)          \
  MST_GetVar((name), (space))                           \
      ? MST_Space_GetSpace(MST_GetVar((name), (space))) \
      : NULL
#define MST_get_space_in_array(m, idx, arr)              \
  MST_Array_Get((arr), (idx))                            \
      ? MST_Array_get_space(MST_Array_Get((arr), (idx))) \
      : NULL  // MST_get_space_in_array
#define MST_get_integer_in_space(m, name, space)          \
  MST_GetVar((name), (space))                             \
      ? MST_Space_GetInteger(MST_GetVar((name), (space))) \
      : -1  // MST_get_integer_in_space
#define MST_get_integer_in_array(m, idx, arr)              \
  MST_Array_Get((arr), (idx))                              \
      ? MST_Array_get_integer(MST_Array_Get((arr), (idx))) \
      : -1  // MST_get_integer_in_array
#define MST_get_string_in_space(m, name, space)                               \
  MST_GetVar((name), (space)) ? MST_Space_GetStr(MST_GetVar((name), (space))) \
                              : NULL  // MST_get_string_in_space
#define MST_get_string_in_array(m, idx, arr)                                   \
  MST_Array_Get((arr), (idx)) ? MST_Array_get_str(MST_Array_Get((arr), (idx))) \
                              : NULL  // MST_get_string_in_array
#define MST_change_var_for_name(m, v, name, space)           \
  {                                                          \
    if (MST_GetVar((name), (space))) {                       \
      MST_change_var((m), (v), MST_GetVar((name), (space))); \
    }                                                        \
  }
#define MST_change_arr_for_idx(m, v, idx, arr)               \
  {                                                          \
    if (MST_Array_Get((arr), (idx))) {                       \
      MST_change_arr((m), (v), MST_Array_Get((arr), (idx))); \
    }                                                        \
  }
#endif
