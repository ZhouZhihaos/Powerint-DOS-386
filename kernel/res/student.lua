function show_all(o)
  for k,v in pairs(o) do
    print(string.format("Student: %s ID:%d",v.name,v.id))
  end
end
-- print("Running.")
Json = require('json')

obj_f = io.open("stu.jsn","r")
dat = obj_f:read("a")
obj_f:close()
obj_f = io.open("stu.jsn","w")
json_obj = nil
if string.len(dat) ~= 0 then
  json_obj = Json.decode(dat)
else
  print("decode ok!")
end

if json_obj ~= nil then
  show_all(json_obj)
  obj_f:write(Json.encode(json_obj))
end

obj_f:close()
