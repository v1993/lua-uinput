#include "declarations.h"
#include "deviceconf.h" // Local header
//struct uinput_user_dev uinput_user_dev;
/*
static void lua_pushvalue(lua_State *L, int idx) {
	lua_insert(L, idx);
	if (idx < 0) {
		idx = idx-1;
	};
	lua_remove(L, idx);
}; */

void uinp_dev_addlink(lua_State *L) { // You must place parrent at -2 idx, child at -1 idx
	int curtop = lua_gettop(L);
	lua_getfield(L, LUA_REGISTRYINDEX, "uinp_dev_link");
	lua_pushvalue(L, -2); // Offset -1
	lua_pushvalue(L, -4); // Offset -2
	lua_settable (L, -3); // Offest -3
	lua_settop(L, curtop);
};

int uinp_dev_create(lua_State *L) {
	uinput_user_dev *s;
	s = (uinput_user_dev *)lua_newuserdata(L, sizeof(uinput_user_dev));
	memset(s, 0, sizeof(uinput_user_dev));
	luaL_getmetatable(L, "uinp_dev");
	lua_setmetatable(L, -2);
	return 1;
};

static int uinp_dev_get(lua_State *L) {
	uinput_user_dev *s;
	s = checkdev(L);
	const char *key = luaL_checkstring(L, 2);
	void **pointer;
	if (strcmp(key, "absmax") == 0) {
		pointer = lua_newuserdata(L, sizeof(__s32 *));
		(*pointer) = s->absmax;
		dev_get_arr(L);
	} else if (strcmp(key, "absmin") == 0) {
		pointer = lua_newuserdata(L, sizeof(__s32 *));
		(*pointer) = s->absmin;
		dev_get_arr(L);
	} else if (strcmp(key, "absfuzz") == 0) {
		pointer = lua_newuserdata(L, sizeof(__s32 *));
		(*pointer) = s->absfuzz;
		dev_get_arr(L);
	} else if (strcmp(key, "absflat") == 0) {
		pointer = lua_newuserdata(L, sizeof(__s32 *));
		(*pointer) = s->absflat;
		dev_get_arr(L);
	} else if (strcmp(key, "name") == 0) {
		lua_pushstring(L, s->name);
	} else if (strcmp(key, "id") == 0) {
		pointer = lua_newuserdata(L, sizeof(__s32 *));
		(*pointer) = &s->id;
		luaL_getmetatable(L, "uinp_dev_id");
		dev_child(L);
	} else {
		luaL_argcheck(L, false, 2, "invalid filed");
	};
	return 1;
};

static int uinp_dev_set(lua_State *L) {
	uinput_user_dev *s;
	s = checkdev(L);
	const char *key = luaL_checkstring(L, 2);
	luaL_argcheck(L, strcmp(key, "name") == 0, 2, "invalid filed");
	const char *val =  luaL_checkstring(L, 3);
	luaL_argcheck(L, strlen(val) <= UINPUT_MAX_NAME_SIZE, 3, "too long name");
	strcpy(s->name, val);
	return 0;
};

static int uinp_dev_setarr(lua_State *L) {
	__s32 **arr = checkdevarr(L);
	int pos = luaL_checkint(L, 2);
	checkarrpos(L, pos, 2);
	int val = luaL_checkint(L, 3);
	(*arr)[pos] = val;
	return 0;
};

static int uinp_dev_getarr(lua_State *L) {
	__s32 **arr = checkdevarr(L);
	int pos = luaL_checkint(L, 2);
	checkarrpos(L, pos, 2);
	lua_pushinteger(L, (*arr)[pos]);
	return 1;
};

static int uinp_dev_setid(lua_State *L) {
	input_id **id_pointer = checkdevid(L);
	input_id *id = *id_pointer;
	const char *key = luaL_checkstring(L, 2);
	__u32 val = luaL_checkint(L, 3);
	if (strcmp(key, "bustype") == 0) {
		id->bustype = val;
	} else if (strcmp(key, "vendor") == 0) {
		id->vendor = val;
	} else if (strcmp(key, "product") == 0) {
		id->product = val;
	} else if (strcmp(key, "version") == 0) {
		id->version = val;
	} else {
		luaL_argcheck(L, false, 2, "invalid filed");
	};
	return 0;
};

static int uinp_dev_getid(lua_State *L) {
	input_id **id_pointer = checkdevid(L);
	input_id *id = *id_pointer;
	const char *key = luaL_checkstring(L, 2);
	__u32 ret;
	if (strcmp(key, "bustype") == 0) {
		ret = id->bustype;
	} else if (strcmp(key, "vendor") == 0) {
		ret = id->vendor;
	} else if (strcmp(key, "product") == 0) {
		ret = id->product;
	} else if (strcmp(key, "version") == 0) {
		ret = id->version;
	} else {
		luaL_argcheck(L, false, 2, "invalid filed");
	};
	lua_pushinteger(L, ret);
	return 1;
};

static const struct luaL_Reg devarray_m [] = {
	{"__newindex", uinp_dev_setarr},
	{"__index", uinp_dev_getarr},
	{NULL, NULL}
};

static const struct luaL_Reg devid_m [] = {
	{"__newindex", uinp_dev_setid},
	{"__index", uinp_dev_getid},
	{NULL, NULL}
};

static const struct luaL_Reg uinpdev_m [] = {
	{"__newindex", uinp_dev_set},
	{"__index", uinp_dev_get},
	{NULL, NULL}
};

void uinp_dev_luaopen(lua_State *L) {
	lua_newtable(L);
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "k");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, "uinp_dev_link");
	luaL_newmetatable(L, "uinp_dev");
	luaL_setfuncs(L, uinpdev_m, 0);
	luaL_newmetatable(L, "uinp_dev_arr");
	luaL_setfuncs(L, devarray_m, 0);
	luaL_newmetatable(L, "uinp_dev_id");
	luaL_setfuncs(L, devid_m, 0);
};
