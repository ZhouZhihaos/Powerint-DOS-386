pluaui = {}
local now_window = {}
local function WindowAdd(self,UITbl)
  self.uiNo = self.uiNo + 1
  self.uiArray[self.uiNo] = UITbl
end
local function Ui_Handler(uiArr)
  local len = os.MessageLength(-1)
  if len ~= 4 then
    return
  end
  local id = os.GetIPCMessage(0,-1)
  for i,v in pairs(uiArr) do
    if id == v.bid then
      v:func()
    end
  end
end
local function mainloop(self)
  while 1 do
    local len_window = os.MessageLength(-2)
    if len_window == 4 then
      self:Close()
      os.exit()
    end
    Ui_Handler(self.uiArray)
  end
end
local function close(self)
  for i,v in pairs(self.uiArray) do
    v:Delete()
  end
  gui.close_window(self.id)
end
local function Draw_box(self,x,y,w,h,c)
  gui.box(self.id,x,y,x+w,y+h,c,self.xsize)
end
local function Draw_px(self,x,y,c)
  gui.px(self.id,x,y,c,self.xsize)
end
local function Draw_str(self,x,y,s,c)
  gui.str(self.id,s,x,y,c,self.xsize)
end
function pluaui.CreateWindow(x,y,w,h,title)
  now_window.uiArray = {}
  now_window.id = 0
  now_window.add = WindowAdd
  now_window.id = gui.OpenWindow(x,y,w,h,title)
  now_window.xsize = w
  now_window.uiNo = 0
  now_window.loop = mainloop
  now_window.Close = close
  now_window.box = Draw_box
  now_window.px = Draw_px
  now_window.str= Draw_str
  return now_window
end

local function ButtonDelete(self)
  gui.delete_button(self.bid)
end
function pluaui.CreateButton(x,y,w,h,title,func)
  return {
    t = 1,
    win = now_window,
    bid = gui.CreateButton(x,y,w,h,now_window.id,title),
    func = func,
    Delete = ButtonDelete
  }
end
local function TextBoxDelete(self)
  gui.delete_textBox(self.bid)
end
local function TextBoxGetText(self)
   return gui.TextBox_GetText(self.bid)
end
function pluaui.CreateTextBox(x,y,w,h)
  return {
    t = 2,
    win = now_window,
    bid = gui.CreateTextBox(x,y,w,h,now_window.id),
    func = nil,
    Delete = TextBoxDelete,
    getText = TextBoxGetText
  }
end
return pluaui