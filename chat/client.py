# 客户端
from audioop import add
from glob import glob
import socket
import threading
import time

from torch import addr
s1=None
def thread():
  global s1
  while True:
    data,addr = s1.recvfrom(1024)
    print(data.decode('utf-8'))
    time.sleep(1)
def main():
    global s1
    # 创建 socket 对象
    addr = input('请输入IP地址：')
    port = int(input('请输入端口号：'))
    s1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s1.bind(('', 0))
    s1.sendto(('CONNECT ' + addr + ' ' + str(port)).encode('utf-8'), (addr, port))
    data1, addr1 = s1.recvfrom(1024)
    if data1.decode('utf-8') == 'OK':
      print('连接成功！')
    # 创建线程
    t = threading.Thread(target=thread)
    t.start()
    while True:
      str1 = input('请输入消息：')
      s1.sendto(('SEND ' + str(str1)).encode('utf-8'), (addr, port))
    s1.sendto(('DISCONNECT ' + addr + ' ' + str(port)).encode('utf-8'), (addr, port))
    
main()