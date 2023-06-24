#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mst.h"
#include "mstr.h"
#ifdef MEM_LEAK_CHK
void* Malloc(size_t size);
void Free(void* ptr);
#define malloc Malloc
#define free Free
#endif
PRIVATE void put_token(char* buf, TOKEN_TYPE t, MST_Object* mst) {
  TOKEN* tok = malloc(sizeof(TOKEN));
  tok->t = t;
  tok->tok = buf;
  AddVal((uintptr_t)tok, mst->token);
}
PUBLIC MST_API Var MST_var_make_integer(char* name, int val) {
  Var r;
  char* s = (char*)malloc(strlen(name) + 1);
  strcpy(s, name);
  r.name = s;
  r.vt = INTEGER;
  r.obj = malloc(sizeof(Integer));
  ((Integer*)(r.obj))->num = val;
  return r;
}
PUBLIC MST_API Var MST_var_make_string(char* name, char* ss) {
  Var r;
  char* s = (char*)malloc(strlen(name) + 1);
  strcpy(s, name);
  r.name = s;
  r.vt = STR;
  r.obj = malloc(sizeof(String));
  char* s1 = (char*)malloc(strlen(ss) + 1);
  strcpy(s1, ss);
  ((String*)(r.obj))->str = s1;
  return r;
}

PUBLIC MST_API Array_data MST_arr_dat_make_integer(int val) {
  Array_data r;
  r.vt = INTEGER;
  r.obj = malloc(sizeof(Integer));
  ((Integer*)(r.obj))->num = val;
  return r;
}
PUBLIC MST_API Array_data MST_arr_dat_make_string(char* ss) {
  Array_data r;
  r.vt = STR;
  r.obj = malloc(sizeof(String));
  char* s1 = (char*)malloc(strlen(ss) + 1);
  strcpy(s1, ss);
  ((String*)(r.obj))->str = s1;
  return r;
}

// TODO:我们需要将name（char *）的数据送到token这个链表里面，因为我们需要Free
PUBLIC MST_API void MST_add_data_to_array(MST_Object* mst_obj,
                                          Array* arr,
                                          Array_data ad) {
  if (ad.vt == SPAC || ad.vt == ARRAY) {
    mst_obj->err = WRONG_TYPE_TO_ADD;
    return;
  }
  Array_data* v = (Array_data*)malloc(sizeof(Array_data));
  memcpy(v, &ad, sizeof(Array_data));
  if (v->vt == STR) {
    String* s = (String*)v->obj;
    put_token(s->str, STRING, mst_obj);
  }
  AddVal((uintptr_t)v, arr->the_array);
}
PUBLIC MST_API void MST_add_var_to_space(MST_Object* mst_obj,
                                         SPACE* sp,
                                         Var var) {
  if (var.vt == SPAC || var.vt == ARRAY) {
    mst_obj->err = WRONG_TYPE_TO_ADD;
    return;
  }
  put_token(var.name, WORD, mst_obj);
  Var* v = (Var*)malloc(sizeof(Var));
  memcpy(v, &var, sizeof(Var));
  if (v->vt == STR) {
    String* s = (String*)v->obj;
    put_token(s->str, STRING, mst_obj);
  }
  AddVal((uintptr_t)v, sp->the_space);
}
PUBLIC MST_API void MST_add_empty_space_to_space(MST_Object* mst_obj,
                                                 SPACE* sp,
                                                 char* name) {
  char* s = (char*)malloc(strlen(name) + 1);
  strcpy(s, name);
  Var* v = (Var*)malloc(sizeof(Var));
  v->name = s;
  v->vt = SPAC;
  v->obj = malloc(sizeof(SPACE));
  ((SPACE*)v->obj)->the_space = NewList();
  put_token(s, WORD, mst_obj);
  AddVal((uintptr_t)v, sp->the_space);
}
PUBLIC MST_API void MST_add_empty_array_to_space(MST_Object* mst_obj,
                                                 SPACE* sp,
                                                 char* name) {
  char* s = (char*)malloc(strlen(name) + 1);
  strcpy(s, name);
  Var* v = (Var*)malloc(sizeof(Var));
  v->name = s;
  v->vt = ARRAY;
  v->obj = malloc(sizeof(Array));
  ((Array*)v->obj)->the_array = NewList();
  put_token(s, WORD, mst_obj);
  AddVal((uintptr_t)v, sp->the_space);
}
PUBLIC MST_API void MST_add_empty_space_to_array(MST_Object* mst_obj,
                                                 Array* arr) {
  Array_data* v = (Array_data*)malloc(sizeof(Array_data));
  v->vt = SPAC;
  v->obj = malloc(sizeof(SPACE));
  ((SPACE*)v->obj)->the_space = NewList();
  AddVal((uintptr_t)v, arr->the_array);
}
PUBLIC MST_API void MST_add_empty_array_to_array(MST_Object* mst_obj,
                                                 Array* arr) {
  Array_data* v = (Array_data*)malloc(sizeof(Array_data));
  v->vt = ARRAY;
  v->obj = malloc(sizeof(Array));
  ((Array*)v->obj)->the_array = NewList();
  AddVal((uintptr_t)v, arr->the_array);
}

PRIVATE void mst_add_str(mstr* result, char* str, int space_no) {
  for (int i = 0; i < space_no; i++) {
    mstr_add_char(result, '\t');
  }
  mstr_add_str(result, str);
}
PRIVATE void build_space(mstr* result, SPACE* space, int spaces_no);
PRIVATE void build_array(mstr* result,
                         Array* arr,
                         int spaces_no_for_start,
                         int spaces_no) {
  int flag = 0;
  mst_add_str(result, "[", spaces_no_for_start);
  for (int i = 0; MST_Array_Get(arr, i) != NULL; i++) {
    flag = 1;
    // printf(".\n");
    Array_data* ad = (Array_data*)MST_Array_Get(arr, i);
    switch (ad->vt) {
      case SPAC:
        mstr_add_str(result, "{\n");
        build_space(result, MST_Array_get_space(ad), spaces_no + 1);
        mst_add_str(result, "}", spaces_no);
        break;
      case INTEGER: {
        char num_buff[50];
        sprintf(num_buff, "%d", MST_Array_get_integer(ad));
        mstr_add_str(result, num_buff);
        break;
      }
      case STR: {
        char* buff = (char*)malloc(strlen(MST_Array_get_str(ad)) + 3);
        strcpy(buff, MST_Array_get_str(ad));
        sprintf(buff, "\"%s\"", MST_Array_get_str(ad));
        mstr_add_str(result, buff);
        free(buff);
        break;
      }
      case ARRAY:
        build_array(result, MST_Array_get_array(ad), 0, spaces_no);
        break;
      default:
        break;
    }
    mstr_add_char(result, ',');
  }
  if (flag) {
    mstr_backspace(result);
  }
  mstr_add_char(result, ']');
}
PRIVATE void build_space(mstr* result, SPACE* space, int spaces_no) {
  for (int i = 1; FindForCount(i, space->the_space) != NULL; i++) {
    Var* sp = (Var*)FindForCount(i, space->the_space)->val;
    //  printf("name found!\n");
    char* n = (char*)malloc(strlen(sp->name) + 6);
    strcpy(n, sp->name);
    sprintf(n, "\"%s\" = ", sp->name);
    // printf("n=%s %d\n",n,i);
    mst_add_str(result, n, spaces_no);
    free(n);
    switch (sp->vt) {
      case SPAC:
        mst_add_str(result, "{\n", spaces_no);
        build_space(result, MST_Space_GetSpace(sp), spaces_no + 1);
        mst_add_str(result, "}", spaces_no);
        break;
      case INTEGER: {
        char num_buff[50];
        sprintf(num_buff, "%d", MST_Space_GetInteger(sp));
        mstr_add_str(result, num_buff);
        break;
      }
      case STR: {
        char* buff = (char*)malloc(strlen(MST_Space_GetStr(sp)) + 3);
        strcpy(buff, MST_Space_GetStr(sp));
        sprintf(buff, "\"%s\"", MST_Space_GetStr(sp));
        mstr_add_str(result, buff);
        free(buff);
        break;
      }
      case ARRAY:
        build_array(result, MST_Space_GetArray(sp), 0, spaces_no);
        break;
      default:
        break;
    }
    mstr_add_char(result, '\n');
  }
}
PUBLIC MST_API void MST_change_var(MST_Object* mst_obj, Var v, Var* v1) {
  free(v1->obj);
  put_token(v.name, WORD, mst_obj);
  memcpy(v1, &v, sizeof(Var));
  if (v1->vt == STR) {
    String* s = (String*)v1->obj;
    put_token(s->str, STRING, mst_obj);
  }
}
PUBLIC MST_API void MST_change_arr(MST_Object* mst_obj, Array_data v, Array_data* v1) {
  free(v1->obj);
  memcpy(v1, &v, sizeof(Var));
  if (v1->vt == STR) {
    String* s = (String*)v1->obj;
    put_token(s->str, STRING, mst_obj);
  }
}
PUBLIC MST_API char* MST_build_to_string(MST_Object* mst_obj) {
  mstr* ms = mstr_init();
  // printf("mstr init ok!\n");
  build_space(ms, MST_GetRootSpace(mst_obj), 0);
  mstr_backspace(ms);
  // printf("build_space!\n");
  char* s = (char*)malloc(strlen(mstr_get(ms)) + 1);
#ifdef MEM_LEAK_CHK
  // s这个指针需要用户自己去释放，因此不计算在malloc次数中
  extern int _malloc_times;
  _malloc_times--;
#endif
  strcpy(s, mstr_get(ms));
  mstr_free(ms);
  return s;
}
