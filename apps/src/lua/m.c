#include <stdio.h>
#include <syscall.h>
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

/* Global typedef */

/* Global define */

/* Global Variable */
static int lua_input(lua_State* L) {
  char *buf = malloc(100);
  scan(buf, 100);
  lua_pushstring(L, buf);
  free(buf);
  return 1;
}
static int num(lua_State* L) {
  //size_t l;
  //const char* s = lua_tolstring(L, 1, &l);
  //printf("%s\n",s);
  lua_pushinteger(L,1);
  return 1;
}
static const struct luaL_Reg my_lib_bsp[] = {
    {"input", lua_input},
    {"num", num},
    {NULL, NULL}
};

char *LUA_SCRIPT_GLOBAL;
static lua_State* luaL;
void lua_run(void) {
  luaL = luaL_newstate();
  luaopen_base(luaL);
  luaL_setfuncs(luaL, my_lib_bsp, 0);
  luaL_dostring(luaL, LUA_SCRIPT_GLOBAL);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(int argc,char **argv) {
    if(argc==1) {
        printf("no input file.\n");
        return 0;
    }
    if(filesize(argv[1]) == -1) {
        printf("File not found\n");
        return 0;
    }
  LUA_SCRIPT_GLOBAL = malloc(filesize(argv[1])+1);
  fopen(argv[1],LUA_SCRIPT_GLOBAL);
  lua_run();
}