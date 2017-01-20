#define _GNU_SOURCE
#include "declarations.h"
#include "deviceconf.h"

static void chopen(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	if (!(d->closed)) {
		luaerr(L, "device is closed");
	}
};

static void chcreat(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	if (d->created) {
		luaerr(L, "device is created");
	}
};

static void chncreat(lua_State *L) {
	UinpDevice *d = checkuinput(L);
	if (!(d->created)) {
		luaerr(L, "device isn't created");
	}
};

// Three important functions from suinput library

static int suinput_write_event(int uinput_fd, const struct input_event *event_p, lua_State *L)
{
        ssize_t bytes;

        bytes = write(uinput_fd, event_p, sizeof(struct input_event));
        if (bytes != sizeof(struct input_event)) {
                luaerrmsg(L, "Low-level error while writing");
        }
        return 0;
}

static int suinput_emit(int uinput_fd, uint16_t ev_type, uint16_t ev_code,
                 int32_t ev_value, lua_State *L)
{
        struct input_event event;

        memset(&event, 0, sizeof(event));
        gettimeofday(&event.time, 0);
        event.type = ev_type;
        event.code = ev_code;
        event.value = ev_value;

        return suinput_write_event(uinput_fd, &event, L);
}

static int suinput_syn(int uinput_fd, lua_State *L)
{
        return suinput_emit(uinput_fd, EV_SYN, SYN_REPORT, 0, L);
}

static char *suinput_get_uinput_path(lua_State *L)
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

static int uinp_open (lua_State *L) {
	int fd;
	UinpDevice *d;
	fd = open(suinput_get_uinput_path(L), O_WRONLY | O_NONBLOCK);
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
			luaerrmsg(L, "Error while destroying");
	}
	int clres = close(uinput_fd);
	d->closed = true;
	if (clres == -1) {
		luaerrmsg(L, "Error while destroying");
	}
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

static const struct luaL_Reg lib [] = {
	{"open", uinp_open},
	{"destroy", uinp_destroy},
	{"sync", uinp_sync},
	{"newdevice", uinp_dev_create},
	{NULL, NULL} // TODO: fill it!
};

//int luaopen_uinput_library (lua_State *L) {
int luaopen_library (lua_State *L) {
	luaL_newmetatable(L, "uinput");
	uinp_dev_luaopen(L);
	luaL_newlib(L, lib);
	return 1;
};
