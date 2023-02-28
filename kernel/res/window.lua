
ui = require('pluaui')
function about()
   gui.MsgBox("Powerint DOS 386 0.7a by zhouzhihao & min0911_","info")
end
function click1()
   --gui.MsgBox(t:getText(),"Info")
   if (t:getText() == s) then
      gui.MsgBox("Right","Info")
   else
      gui.MsgBox("Error","Info")
   end
end
function click2()
   gui.MsgBox(tostring(eval(t1:getText())),"The result")
   --if (t1:getText() == s) then
   --   gui.MsgBox("Right","Info")
   --else
   --   gui.MsgBox("Error","Info")
   --end
end
function click3()
   s = t:getText()
   gui.MsgBox("OK!","Info")
end
function errorfunc(err)
   gui.MsgBox(err,"error")
end
function click4(self)
   xpcall(load(t2:getText()),errorfunc)
end
function click5()
   exec("halt")
end

function click6()
   exec("reboot")
end
if (gui.is_gui() == false) then
   print("This program cannot run in dos mode")
   os.exit()
end

-- ui = require('pluaui')
os.forever()
win = ui.CreateWindow(0,0,550,180,"Tool Box")
win:add(ui.CreateButton(0,0,100,20,"about",about))
t=ui.CreateTextBox(5,28+20+5,150,20)
t1=ui.CreateTextBox(5,28+20+5+20+5+6,150,20)
t2 = ui.CreateTextBox(5,28+20+5+20+5+6+20+5+6,320,20)
s = 'Fuck you'
win:add(t)
win:add(t1)
win:add(t2)
win:add(ui.CreateButton(155,25,150,20,"check",click1))
win:add(ui.CreateButton(155,50+6,150,20,"Calc",click2))
win:add(ui.CreateButton(310,25,150,20,"Set",click3))
win:add(ui.CreateButton(348,20+5+20+5+6+20+5+6,150,20,"Run",click4))
win:add(ui.CreateButton(0,20+5+20+5+6+20+5+6+20+5,150,20,"Halt",click5))
win:add(ui.CreateButton(155,20+5+20+5+6+20+5+6+20+5,150,20,"Reboot",click6))
win:loop()