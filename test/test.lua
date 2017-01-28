pcall(require, "luacov")


print("------------------------------------")
print("Lua version: " .. (jit and jit.version or _VERSION))
print("------------------------------------")
print("")

local HAS_RUNNER = not not lunit
local lunit = require "lunit"
local TEST_CASE = lunit.TEST_CASE

local LUA_VER = _VERSION
local u = require "uinput"

local _ENV = TEST_CASE "main_test_case"
local dev

function setup()
	dev = assert_userdata(u.open(), 'open returned not userdata')
end

function teardown()
	dev:destroy()
end

function test_enable()
	assert_false(pcall(dev.enable, dev), 'enable OK without arg')
	assert_false(pcall(dev.enable, dev, 'string'), 'enable OK with non-table arg')
	dev:enable({[u.ABS_X] = u.EV_ABS, [u.KEY_A] = u.EV_KEY})
end;

function test_create()
	assert_false(pcall(dev.create, dev), 'create OK without arg')
	assert_false(pcall(dev.create, dev, 'string'), 'create OK with non-table arg')
	dev:create({name = 'testdev'})
end;

function test_abs()
	dev:enable({[u.ABS_X] = u.EV_ABS})
	dev:create({name = 'testdev_abs', absmax = {[u.ABS_X] = 1024}, absmin = {[u.ABS_X] = 0}})
	dev:emit(u.EV_ABS, u.ABS_X, 512)
end;

function test_kbd()
	dev:enable({[u.KEY_A] = u.EV_KEY, [u.KEY_B] = u.EV_KEY})
	dev:create({name = 'testdev_kbd'})
	dev:emit_click(u.KEY_A)
	dev:emit_combo({u.KEY_A, u.KEY_B})
end;

if not HAS_RUNNER then lunit.run() end

collectgarbage() -- Finalize all userdata
