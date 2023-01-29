map = {}
for i = 1,19 do
  map[i] = {}
end

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
      k_w = 0
      k_b = 0
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
now = 1
while 1 do
  x = 0
  y = 0
  print("Please input x:")
  x = tonumber(input())
  print("Please input y:")
  y = tonumber(input())
  map[y][x] = now
  if now == 1 then
    now = 2
  elseif now == 2 then
    now = 1
  end
  show(map)
  
  w = win(map)
  if w == 1 then
    print("Black Win!")
    break
  elseif w == 2 then
    print("White Win!")
    break
  end
end