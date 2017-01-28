#define _GNU_SOURCE
#include "declarations.h"
#include "deviceconf.h"

// This library is based on suinput library from Tuomas Jorma Juhani Räsänen

static void chopen(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	if (d->closed) {
		luaerr(L, "device is closed");
	}
};

static void chcreat(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	if (!(d->created)) {
		luaerr(L, "device is created");
	}
};

static void chncreat(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	if (d->created) {
		luaerr(L, "device isn't created");
	}
};

// Some extra functions for dirty work

static int write_event(int uinput_fd, const struct input_event *event_p, lua_State *L)
{
	ssize_t bytes;

	bytes = write(uinput_fd, event_p, sizeof(struct input_event));
	if (bytes != sizeof(struct input_event)) {
		if (L != NULL) {
			luaerrmsg(L, "Low-level error while writing");
		} else {
			return false;
		}
	}
	if (L != NULL) {
		return 0;
	} else {
		return true;
	}
}

static int orig_emit(int uinput_fd, uint16_t ev_type, uint16_t ev_code,
				 int32_t ev_value, lua_State *L)
{
		struct input_event event;

		memset(&event, 0, sizeof(event));
		gettimeofday(&event.time, 0);
		event.type = ev_type;
		event.code = ev_code;
		event.value = ev_value;

		return write_event(uinput_fd, &event, L);
}

static int suinput_syn(int uinput_fd, lua_State *L)
{
		return orig_emit(uinput_fd, EV_SYN, SYN_REPORT, 0, L);
}

static char *get_uinput_path(lua_State *L)
{
		struct udev *udev;
		struct udev_device *udev_dev;
		const char *devnode;
		char *retval = NULL;
		int orig_errno;

		if ((udev = udev_new()) == NULL) {
				goto err;
		}

		udev_dev = udev_device_new_from_subsystem_sysname(udev, "misc",
														  "uinput");
		if (udev_dev == NULL) {
				goto out;
		}

		if ((devnode = udev_device_get_devnode(udev_dev)) == NULL) {
				goto out;
		}

		if ((retval = malloc(strlen(devnode) + 1)) == NULL) {
				goto out;
		}

		strcpy(retval, devnode);
out:
		orig_errno = errno;
		udev_device_unref(udev_dev);
		udev_unref(udev);
		errno = orig_errno;
		if (retval == NULL) {
			err:
			luaerrmsg(L, "Unable to get path to uinput iface file");
		}
		return retval;
};

// Main functions

static int uinp_open (lua_State *L) {
	int fd;
	UinpDevice *d;
	char *uinput_path;
	uinput_path = get_uinput_path(L);
	fd = open(uinput_path, O_WRONLY | O_NONBLOCK);
	free(uinput_path);
	if (fd < 0) {
		luaerrmsg(L, "Unable to open uinput iface file");
	}
	d = (UinpDevice *)lua_newuserdata(L, sizeof(UinpDevice));
	d->fd = fd;
	d->autosync = false;
	d->created = false;
	d->closed = false;
	luaL_getmetatable(L, "uinput");
	lua_setmetatable(L, -2);
	return 1;
};

static int uinp_destroy(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	int uinput_fd = d->fd;
	if (d->closed) { return 0; }
	if (ioctl(uinput_fd, UI_DEV_DESTROY) == -1) {
			int original_errno = errno;
			close(uinput_fd);
			d->closed = true;
			errno = original_errno;
			luaerrmsg(L, "Error while destroying (on ioctl)");
	}
	int clres = close(uinput_fd);
	d->closed = true;
	if (clres == -1) {
		luaerrmsg(L, "Error while destroying (on close)");
	}
//	printf("%p destrroyd\n", d);
	return 0;
};

static int uinp_sync(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	if (!d->closed) {
		suinput_syn(d->fd, L);
	};
	return 0;
};

static void uinp_autosync(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	if (d->autosync) {
		uinp_sync(L);
	}
};

static int uinp_enable_events(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	luaL_checktype(L, 2, LUA_TTABLE);
	chopen(L);
	chncreat(L);
	lua_pushnil(L); // First iteration
	while (lua_next(L, 2) != 0) {
		uint16_t ev_type = luaL_checkunsigned(L, -1);
		uint16_t ev_code = luaL_checkunsigned(L, -2);
		unsigned long io;
		if (ioctl(d->fd, UI_SET_EVBIT, ev_type) == -1) {
			luaerrmsg(L, "Unable to enable type of events");
		}
		switch (ev_type) {
		case EV_KEY:
				io = UI_SET_KEYBIT;
				break;
		case EV_REL:
				io = UI_SET_RELBIT;
				break;
		case EV_ABS:
				io = UI_SET_ABSBIT;
				break;
		case EV_MSC:
				io = UI_SET_MSCBIT;
				break;
		case EV_SW:
				io = UI_SET_SWBIT;
				break;
		case EV_LED:
				io = UI_SET_LEDBIT;
				break;
		case EV_SND:
				io = UI_SET_SNDBIT;
				break;
		case EV_FF:
				io = UI_SET_FFBIT;
				break;
		default:
				luaerr(L, "invalid event type");
		};
		if (ioctl(d->fd, io, ev_code) == -1) {
			luaerrmsg(L, "Unable to enable event");
		};
		lua_pop(L, 1);
	};
	return 0;
};

static int uinp_create(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	luaL_checktype(L, 2, LUA_TTABLE);
	chopen(L);
	chncreat(L);
	uinput_user_dev conf;
	uinp_read_conf(L, 2, &conf);
	ssize_t bytes;
	bytes = write(d->fd, &conf, sizeof(uinput_user_dev));
	if (bytes != sizeof(uinput_user_dev)) {
		luaerrmsg(L, "Error while creating device (on write)");
	};
	if (ioctl(d->fd, UI_DEV_CREATE) == -1) {
		luaerrmsg(L, "Error while creating device (on ioctl)");
	};
	d->created = true;
	return 0;
};

static int uinp_emit(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	uint16_t ev_type = luaL_checkunsigned(L, 2);
	uint16_t ev_code = luaL_checkunsigned(L, 3);
	int32_t ev_value = luaL_checkunsigned(L, 4);
	chopen(L);
	chcreat(L);
	orig_emit(d->fd, ev_type, ev_code, ev_value, L);
	uinp_autosync(L);
	return 0;
};

static int uinp_emit_click(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	uint16_t key_code = luaL_checkunsigned(L, 2);
	chopen(L);
	chcreat(L);
	bool ok;
	ok = orig_emit(d->fd, EV_KEY, key_code, 1, NULL);
	ok &= orig_emit(d->fd, EV_KEY, key_code, 0, NULL); // Try to unpress it always
	uinp_autosync(L);
	if (L == NULL) {
		return ok;
	};
	if (!ok) {
		luaerrmsg(L, "Unable to emit click");
	};
	return 0;
};

static int uinp_emit_combo(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	luaL_checktype(L, 2, LUA_TTABLE);
	int errors = false;
	size_t i;
	int len = luaL_len(L, 2);
	for(i = 1; i <= len; i++) {
		lua_pushinteger(L, i);
		lua_gettable(L, 2);
		if (!(orig_emit(d->fd, EV_KEY, luaL_checkunsigned(L, -1), 1, NULL))) {
			errors = true;
			break;
		};
	};
	while (--i) {
		lua_pushinteger(L, i);
		lua_gettable(L, 2);
		if (!(orig_emit(d->fd, EV_KEY, luaL_checkunsigned(L, -1), 0, NULL))) {
			errors = true;
		}
	};
	if (errors) {
		luaerrmsg(L, "Some errors occurred while writing combo, it's last one");
	}
	return 0;
};

// Metatable for objects

static int uinp_get(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	const char *str = luaL_checkstring(L, 2);
	if streq(str, "destroy") {
		lua_pushcfunction(L, uinp_destroy);
	} else if streq(str, "sync") {
		lua_pushcfunction(L, uinp_sync);
	} else if streq(str, "enable") {
		lua_pushcfunction(L, uinp_enable_events);
	} else if streq(str, "create") {
		lua_pushcfunction(L, uinp_create);
	} else if streq(str, "emit") {
		lua_pushcfunction(L, uinp_emit);
	} else if streq(str, "emit_click") {
		lua_pushcfunction(L, uinp_emit_click);
	} else if streq(str, "emit_combo") {
		lua_pushcfunction(L, uinp_emit_combo);
	} else if streq(str, "fd") {
		lua_pushinteger(L, d->fd);
	} else if streq(str, "autosync") {
		lua_pushboolean(L, d->autosync);
	} else if streq(str, "created") {
		lua_pushboolean(L, d->created);
	} else if streq(str, "closed") {
		lua_pushboolean(L, d->closed);
	} else {
		lua_pushnil(L);
	};
	return 1;
};

static int uinp_set(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	const char *str = luaL_checkstring(L, 2);
	if (!streq(str, "autosync")) { luaerr(L, "you can change only autosync"); };
	luaL_checktype(L, 3, LUA_TBOOLEAN);
	bool val = lua_toboolean(L, 3);
	d->autosync = val;
	return 0;
};

// Library openning

static const struct luaL_Reg lib [] = {
	{"open", uinp_open},
	{"destroy", uinp_destroy},
	{"sync", uinp_sync},
	{"enable", uinp_enable_events},
	{"create", uinp_create},
	{"emit", uinp_emit},
	{"emit_click", uinp_emit_click},
	{"emit_combo", uinp_emit_combo},
	{NULL, NULL}
};

static const struct luaL_Reg uinp_meta [] = {
	{"__index", uinp_get},
	{"__newindex", uinp_set},
	{"__gc", uinp_destroy},
	{NULL, NULL}
};

//int luaopen_library (lua_State *L) {
int luaopen_uinput_mainpart (lua_State *L) {
	luaL_newmetatable(L, "uinput");
	luaL_setfuncs(L, uinp_meta, 0);
//	uinp_dev_luaopen(L);
	luaL_newlib(L, lib);
	return 1;
};
