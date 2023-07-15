local function show_all(o)
  for k,v in pairs(o) do
    print(string.format("Student: %s ID:%d",v.name,v.id))
  end
end
local function add_student(o,name)
  local index = 0
  for k,v in pairs(o) do
    index = index + 1
  end
  o[index + 1] = {}
  o[index + 1].name = name 
  o[index + 1].id = index
end
function string.split(str, sep)
  local result = {}
  if str == nil or sep == nil or type(str) ~= "string" or type(sep) ~= "string" then
      return result
  end

  if string.len(sep) == 0 then
      return result
  end
  local pattern = string.format("([^%s]+)", sep)
  --print(pattern)

  string.gsub(
      str,
      pattern,
      function(c)
          result[#result + 1] = c
      end
  )

  return result
end
local json = require('json')
local obj_f = io.open("stu.jsn","r")
local dat = nil
if obj_f ~= nil then
  dat = obj_f:read("a")
  obj_f:close()
else
  dat = ""
end
obj_f = io.open("stu.jsn","w")
assert(obj_f ~= nil)
local json_obj = nil
if string.len(dat) ~= 0 then
  json_obj = json.decode(dat)
end

if json_obj == nil then
  json_obj = {}
end
while 1 do
  io.write(">>>")
  local s = input()
  local arr = string.split(s," ")
  if arr[1] == "exit" then
    break
  elseif arr[1] == "add" then
    assert(arr[2] ~= nil)
    add_student(json_obj,arr[2])
  elseif arr[1] == "show" then
    show_all(json_obj)
  end
end
obj_f:write(json.encode(json_obj))
obj_f:close()
