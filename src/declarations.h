#ifndef DECL_H_INCLUDED
#define DECL_H_INCLUDED
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <linux/limits.h>

#include <libudev.h>
#include <linux/uinput.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define checkuinput(L) \
(UinpDevice *)luaL_checkudata(L, 1, "uinput")

#define getfiled(L, key); \
lua_getfiled(L, -1, key)

#define luaerrmsg(L, msg) \
luaL_error(L, "%s: %m", msg)

#define luaerr(L, msg) \
luaL_error(L, "Error in uinput: %s", msg)

typedef struct UinpDevice {
	int fd;
	bool autosync;
	bool created;
	bool closed;
} UinpDevice;
#endif

typedef struct uinput_user_dev uinput_user_dev;
