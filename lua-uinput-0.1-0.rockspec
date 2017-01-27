package = "lua-uinput"
version = "0.1-0"
source = { url = "git://github.com/v1993/lua-uinput.git" }
description = {
   summary = "simple uinput API",
   detailed = [[Version of (s)uinput library for lua.
You can use it for emulating devices under linux.]],
   homepage = "http://vallua.ru",
   license = "MIT",
}
dependencies = { "lua >= 5.1" }

external_dependencies = {
   LIBUDEV = {
      header = "libudev.h";
      library = "libudev.so"
   }
}

build = {
	type = 'builtin';
	modules = {
		['uinput.init'] = 'src/init.lua';
		['uinput.defines'] = 'src/defines.lua';
		['uinput.mainpart'] = {
			sources = {'src/library.c', 'src/deviceconf.c'};
			libraries = {'udev'};
			incdirs = {"$(LIBUDEV_INCDIR)"},
			libdirs = {"$(LIBUDEV_LIBDIR)"}
		}
	}
}
