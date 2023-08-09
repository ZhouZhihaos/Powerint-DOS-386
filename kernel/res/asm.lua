
print('asm.lua')
local nas_file = arg[1]
local out_file = arg[2]
local name = nil
if nas_file == nil or out_file == nil then
  print("arg error.")
  os.exit()
end
local idx = nas_file:match(".+()%.%w+$")
if idx then 
  name = nas_file:sub(1, idx - 1) 
else
  name = nas_file
end
print('nask')
exec(string.format("nask %s %s.obj %s.lst",nas_file,name,name))
print('obj2bim')
exec(string.format("obj2bim @/make.rul out:%s.bim stack:1k map:%s.map %s.obj",name,name,name))
print('bim2hrb')
exec(string.format("bim2hrb %s.bim %s 0",name,out_file))
print("Build OK!")
os.exit()