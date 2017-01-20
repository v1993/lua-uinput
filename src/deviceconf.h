#include "declarations.h"
#ifndef DVCONF_H_INCLUDED
	#define DVCONF_H_INCLUDED
	typedef struct uinput_user_dev uinput_user_dev;
	typedef struct input_id input_id;
	#define checkdev(L) \
	(uinput_user_dev *)luaL_checkudata(L, 1, "uinp_dev")
	#define checkdevarr(L) \
	luaL_checkudata(L, 1, "uinp_dev_arr")
	#define checkdevid(L) \
	luaL_checkudata(L, 1, "uinp_dev_id")
	#define checkarrpos(L, num, anum) \
	luaL_argcheck(L, num <= ABS_CNT&&num>=0, anum, "incorrect size")
	#define dev_child(L) \
	lua_setmetatable(L, -2); \
	lua_pushvalue(L, 1); \
	lua_pushvalue(L, -2); \
	uinp_dev_addlink(L); \
	lua_pop(L, 2)
	#define dev_get_arr(L) \
	luaL_getmetatable(L, "uinp_dev_arr"); \
	dev_child(L)

	int uinp_dev_create(lua_State *L);
	void uinp_dev_luaopen(lua_State *L);
	void uinp_dev_addlink(lua_State *L); // Bonus
#endif
