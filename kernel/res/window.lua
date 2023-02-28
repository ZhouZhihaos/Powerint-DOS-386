count = 0
function click(self)
  count = count + 1
  self.win:box(5,80,100,20,0xffffff)
  self.win:str(5,80,tostring(count),0)
end
function clear(self)
  count = 0
  self.win:box(5,80,100,20,0xffffff)
  self.win:str(5,80,tostring(count),0)
end
ui = mod("pluaui.lua")
os.forever()
window = ui.CreateWindow(0,0,320,200,"hello")
window:box(5,80,100,20,0xffffff)
window:add(ui.CreateButton(0,0,100,20,"Count",click))
window:add(ui.CreateButton(0,25,100,20,"Clear",clear))
window:loop()