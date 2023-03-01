-- l_x = 0
-- l_y = 0
function show(map)
  local s = ""
  for i = 1,19 do
    for j = 1,19 do
      if map[i][j] == 1 then
        s = s .. "@"
      elseif map[i][j] == 2 then
        s = s .. "O"
      else
        s = s .. " "
      end
    end
    s = s .. "\n"
  end
  print(s)
end
function win(map)
  for i = 1,19 do
    for j = 1,19 do
      local k_w = 0
      local k_b = 0
      for k=0,5 do
        if map[i][j+k] == 1 then
          k_b = k_b + 1
        elseif map[i][j+k] == 2 then
          k_w = k_w + 1
        end
      end
      if k_b == 5 then
        return 1
      elseif k_w == 5 then
        return 2
      end
      k_w = 0
      k_b = 0
      for k=0,5 do
        if i+k > 19 then
          break
        end
        if map[i+k][j] == 1 then
          k_b = k_b + 1
        elseif map[i+k][j] == 2 then
          k_w = k_w + 1
        end
      end
      if k_b == 5 then
        return 1
      elseif k_w == 5 then
        return 2
      end
      k_w = 0
      k_b = 0
      for k=0,5 do
        if i+k > 19 then
          break
        end
        if map[i+k][j+k] == 1 then
          k_b = k_b + 1
        elseif map[i+k][j+k] == 2 then
          k_w = k_w + 1
        end
      end
      if k_b == 5 then
        return 1
      elseif k_w == 5 then
        return 2
      end
      k_w = 0
      k_b = 0
      for k=0,5 do
        if i+k > 19 then
          break
        end
        if map[i+k][j-k] == 1 then
          k_b = k_b + 1
        elseif map[i+k][j-k] == 2 then
          k_w = k_w + 1
        end
      end
      if k_b == 5 then
        return 1
      elseif k_w == 5 then
        return 2
      end
    end
  end
end
function g_xy(map,l_x,l_y)
  io.gotoxy(0,0)
  show(map)
  --l_x = 0
  --l_y = 0
  io.gotoxy(l_x,l_y)
  while 1 do
     c = io.getch()
     -- io.putstr(string.char(c))
     if(string.char(c) == 'a') then
        if(l_x ~= 0) then
          l_x = l_x - 1
        end
     elseif(string.char(c) == 's') then
        if(l_y ~= 18) then
          l_y = l_y + 1
        end
     elseif(string.char(c) == 'w') then
        if(l_y ~= 0) then
          l_y = l_y -1
        end
     elseif(string.char(c) == 'd') then
        if(l_x ~= 18) then
          l_x = l_x + 1
        end
     end
     if(c == 0x0a) then
        break
     end
     io.gotoxy(l_x,l_y)
  end
  return l_x+1,l_y+1
end
map = {}
for i = 1,19 do
  map[i] = {}
end
now = 1
l_x = 0
l_y = 0
exec("cls")
while 1 do
  x,y = g_xy(map,l_x,l_y)
  -- print(x,y)
  if map[y][x] == nil then
    -- print("Luo!")
    -- while 1 do end
    map[y][x] = now
    if now == 1 then
      now = 2
    elseif now == 2 then
      now = 1
    end
  end
  l_x = x - 1
  l_y = y - 1
  show(map)
  io.gotoxy(0,0)
  w = win(map)
  if w == 1 then
    exec("cls")
    print("Black Win!")
    break
  elseif w == 2 then
    exec("cls")
    print("White Win!")
    break
  end
end
