pcall(require, "luacov")


print("------------------------------------")
print("Lua version: " .. (jit and jit.version or _VERSION))
print("------------------------------------")
print("")

local HAS_RUNNER = not not lunit
local lunit = require "lunit"
local TEST_CASE = lunit.TEST_CASE

local u = require "uinput"
local function pcall_m(...)
	local arg = {...};
	return pcall(arg[1][arg[2]], arg[1], select(3, ...));
end

local _ENV = TEST_CASE "main"
local dev

function setup()
	dev = assert_userdata(u.open(), 'open returned not userdata')
end

function teardown()
	assert_true(pcall_m(dev, 'destroy'))
end

function test_enable()
	assert_false(pcall_m(dev, 'enable'), 'enable OK without arg')
	assert_false(pcall_m(dev, 'enable', 'string'), 'enable OK with non-table arg')
	assert_true(pcall_m(dev, 'enable', {[u.ABS_X] = u.EV_ABS, [u.KEY_A] = u.EV_KEY}))
end;

function test_sync()
	assert_true(pcall_m(dev, 'create', {name = 'testdev_sync'}))
	assert_true(pcall_m(dev, 'sync'))
	dev.autosync = true
end;

function test_abs()
	assert_true(pcall_m(dev, 'enable', {[u.ABS_X] = u.EV_ABS}))
	assert_true(pcall_m(dev, 'create', {name = 'testdev_abs', absmax = {[u.ABS_X] = 1024}, absmin = {[u.ABS_X] = 0}}))
	assert_true(pcall_m(dev, 'emit', u.EV_ABS, u.ABS_X, 512))
	assert_true(pcall_m(dev, 'sync'))
end;

function test_kbd()
	assert_true(pcall_m(dev, 'enable', {[u.KEY_A] = u.EV_KEY, [u.KEY_B] = u.EV_KEY}))
	assert_true(pcall_m(dev, 'create', {name = 'testdev_kbd'}))
	assert_true(pcall_m(dev, 'emit_click', u.KEY_A))
	assert_true(pcall_m(dev, 'sync'))
	assert_true(pcall_m(dev, 'emit_combo', {u.KEY_A, u.KEY_B}))
	assert_true(pcall_m(dev, 'sync'))
end;

local _ENV = TEST_CASE "deviceconf"
local dev

function setup()
	dev = assert_userdata(u.open(), 'open returned not userdata')
end

function teardown()
	assert_true(pcall_m(dev, 'destroy'))
end

function test_onlyname()
	assert_true(pcall_m(dev, 'create', {name = 'testdev'}))
end;

function test_noarg()
	assert_false(pcall_m(dev, 'create'), 'create OK without arg')
end

function test_nontable()
	assert_false(pcall_m(dev, 'create', 'string'), 'create OK with non-table arg')
end

function test_empty()
	assert_false(pcall_m(dev, 'create', {}), 'create OK with empty table')
end

function test_emptyname()
	assert_false(pcall_m(dev, 'create', {name = ''}), 'create OK with empty name')
end

function test_emptytabs()
	assert_true(pcall_m(dev, 'create', {name = 'testdev', absmin = {}, absmax = {}, absfuzz = {}, absflat = {}}))
end

function test_tabs()
	assert_true(pcall_m(dev, 'create', {name = 'testdev', absmin = {[u.ABS_X] = 0}, absmax = {[u.ABS_X] = 0}, absfuzz = {[u.ABS_X] = 0}, absflat = {[u.ABS_X] = 0}}))
end

function test_id()
	assert_true(pcall_m(dev, 'create', {name = 'testdev', bustype = u.BUS_USB, vendor = 0x12, product = 0x12, version = 0x12}))
end

local _ENV = TEST_CASE "libmetatable"

function test_meta()
	local dev = u()
	dev:destroy()
end

if not HAS_RUNNER then lunit.run() end

collectgarbage() -- Finalize all userdata, it can show hidden bugs...
