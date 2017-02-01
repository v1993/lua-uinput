#include "declarations.h"
#include "deviceconf.h" // Local header

// Everyting for config table parsing

static void parse_name(lua_State *L, int idx, uinput_user_dev *dev) {
	lua_getfield(L, idx, "name");
	if (!(lua_isnil(L, -1))) {
		const char *name = luaL_checkstring(L,-1);
		luaL_argcheck(L, strlen(name) > 0, 3, "name is empty");
		luaL_argcheck(L, strlen(name) <= UINPUT_MAX_NAME_SIZE, 3, "too long name");
		strcpy(dev->name, name);
	} else {
		luaerr(L, "no \"name\" field in description table");
	};
	lua_pop(L,1);
};

static void parse_max(lua_State *L, int idx, uinput_user_dev *dev) {
	lua_getfield(L, idx, "absmax");
	if (lua_isnil(L, -1)) {
		goto out;
	} else if (lua_istable(L, -1)) {
		int i;
		for (i=0;i<=ABS_CNT;i++) {
			lua_pushnumber(L, i);
			lua_gettable(L, -2);
			if (!(lua_isnil(L, -1))) {
				int num = luaL_checknumber(L, -1);
				dev->absmax[i] = num;
			}
			lua_pop(L, 1);
		}
	} else {
		luaerr(L, "invalid absmax");
	}
	out:
	lua_pop(L,1);
};

static void parse_min(lua_State *L, int idx, uinput_user_dev *dev) {
	lua_getfield(L, idx, "absmin");
	if (lua_isnil(L, -1)) {
		goto out;
	} else if (lua_istable(L, -1)) {
		int i;
		for (i=0;i<=ABS_CNT;i++) {
			lua_pushnumber(L, i);
			lua_gettable(L, -2);
			if (!(lua_isnil(L, -1))) {
				int num = luaL_checknumber(L, -1);
				dev->absmin[i] = num;
			}
			lua_pop(L, 1);
		}
	} else {
		luaerr(L, "invalid absmin");
	}
	out:
	lua_pop(L,1);
};

static void parse_fuzz(lua_State *L, int idx, uinput_user_dev *dev) {
	lua_getfield(L, idx, "absfuzz");
	if (lua_isnil(L, -1)) {
		goto out;
	} else if (lua_istable(L, -1)) {
		int i;
		for (i=0;i<=ABS_CNT;i++) {
			lua_pushnumber(L, i);
			lua_gettable(L, -2);
			if (!(lua_isnil(L, -1))) {
				int num = luaL_checknumber(L, -1);
				dev->absfuzz[i] = num;
			}
			lua_pop(L, 1);
		}
	} else {
		luaerr(L, "invalid absfuzz");
	}
	out:
	lua_pop(L,1);
};

static void parse_flat(lua_State *L, int idx, uinput_user_dev *dev) {
	lua_getfield(L, idx, "absflat");
	if (lua_isnil(L, -1)) {
		goto out;
	} else if (lua_istable(L, -1)) {
		int i;
		for (i=0;i<=ABS_CNT;i++) {
			lua_pushnumber(L, i);
			lua_gettable(L, -2);
			if (!(lua_isnil(L, -1))) {
				int num = luaL_checknumber(L, -1);
				dev->absflat[i] = num;
			}
			lua_pop(L, 1);
		}
	} else {
		luaerr(L, "invalid absflat");
	}
	out:
	lua_pop(L,1);
};

static void parse_id(lua_State *L, int idx, uinput_user_dev *dev) {
	lua_getfield(L, idx, "bustype");
	if (!(lua_isnil(L, -1))) {
		dev->id.bustype = luaL_checkint(L,-1);
	};
	lua_pop(L,1);
	lua_getfield(L, idx, "vendor");
	if (!(lua_isnil(L, -1))) {
		dev->id.vendor = luaL_checkint(L,-1);
	};
	lua_pop(L,1);
	lua_getfield(L, idx, "product");
	if (!(lua_isnil(L, -1))) {
		dev->id.product = luaL_checkint(L,-1);
	};
	lua_pop(L,1);
	lua_getfield(L, idx, "version");
	if (!(lua_isnil(L, -1))) {
		dev->id.version = luaL_checkint(L,-1);
	};
	lua_pop(L,1);
};

int uinp_read_conf(lua_State *L, int idx, uinput_user_dev *dev) {
	memset(dev, 0, sizeof(uinput_user_dev));
	parse_name(L, idx, dev);
	parse_max(L, idx, dev);
	parse_min(L, idx, dev);
	parse_fuzz(L, idx, dev);
	parse_flat(L, idx, dev);
	parse_id(L, idx, dev);
	return 0;
};
/* You must use table like this:
 * tab = {
 * nam = 'device name',
 * absmin = {
 * 	[uinput.ABS_X] = 0,
 * 	[uinput.ABS_Y] = 0
 * },
 * absmax = {
 * 	[uinput.ABS_X] = 1023,
 * 	[uinput.ABS_Y] = 1023
 * },
 * bustype = uinput.BUS_USB
 * }
 */
