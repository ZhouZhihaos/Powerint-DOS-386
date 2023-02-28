Srand() -- 初始化
num = Rand(1,100) -- 数字
a = 0 -- 步数
print("Welcome to guess game, number is between 1 and 100")
while 1 do
  b = 0
  print("Guess:")
  b = tonumber(input())
  a  = a + 1
  if b == nil then
    print("NaN")
  elseif b == num then
    print("Yes!You are right You used ",a)
    break
  elseif b > num then
    print("Too big!")
  elseif b < num then
    print("Too small")
  end
end