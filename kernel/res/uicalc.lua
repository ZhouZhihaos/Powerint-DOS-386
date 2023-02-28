function none(self)
  print("Button")
end
ui = require("pluaui")
os.forever()
func_tbl = {}
s = ""
func_tbl[1] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(1)
  self.win:str(5,28,s,0)
end
func_tbl[2] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(2)
  self.win:str(5,28,s,0)
end
func_tbl[3] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(3)
  self.win:str(5,28,s,0)
end
func_tbl[4] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(4)
  self.win:str(5,28,s,0)
end
func_tbl[5] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(5)
  self.win:str(5,28,s,0)
end
func_tbl[6] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(6)
  self.win:str(5,28,s,0)
end
func_tbl[7] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(7)
  self.win:str(5,28,s,0)
end
func_tbl[8] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(8)
  self.win:str(5,28,s,0)
end
func_tbl[9] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(9)
  self.win:str(5,28,s,0)
end
func_tbl[10] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. tostring(0)
  self.win:str(5,28,s,0)
end
func_tbl[11] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. '+'
  self.win:str(5,28,s,0)
end
func_tbl[12] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. '-'
  self.win:str(5,28,s,0)
end
func_tbl[13] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. '*'
  self.win:str(5,28,s,0)
end
func_tbl[14] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = s .. '/'
  self.win:str(5,28,s,0)
end
func_tbl[15] = function (self)
  self.win:box(5,28,195,68,0xffffff)
  s = tostring(eval(s))
  self.win:str(5,28,s,0)
  s = ""
end
func_tbl[16] = function (self)
  s = ""
  self.win:box(5,28,195,68,0xffffff)
end
window = ui.CreateWindow(0,0,200,400,"calc")
window:box(5,28,195,68,0xffffff)
for i = 0,2 do
  for j = 0,2 do
    window:add(ui.CreateButton(j * 52, i * 52 + 68 + 10, 50, 50,tostring(i * 3 + j + 1),func_tbl[i * 3 + j + 1]))
  end
end
window:add(ui.CreateButton(1 * 52, 3 * 52 + 68 + 10, 50, 50, "0",func_tbl[10]))
window:add(ui.CreateButton(5, 400 - 28 - 20, 32, 16, "+",func_tbl[11]))
window:add(ui.CreateButton(5 + 32 + 2, 400 - 28 - 20, 32, 16, "-",func_tbl[12]))
window:add(ui.CreateButton(5 + 32 + 2 + 32 + 2, 400 - 28 - 20, 32, 16, "*",func_tbl[13]))
window:add(ui.CreateButton(5 + 32 + 2 + 32 + 2 + 32 + 2, 400 - 28 - 20, 32, 16, "/",func_tbl[14]))
window:add(ui.CreateButton(5 + 32 + 2 + 32 + 2 + 32 + 2 + 32 + 2, 400 - 28 - 20, 32,16, "=",func_tbl[15]))
window:add(ui.CreateButton(5 + 32 + 2 + 32 + 2 + 32 + 2 + 32 + 2 + 32 + 2,400 - 28 - 20, 20, 16, "CE",func_tbl[16]))
window:loop() -- 进入消息循环