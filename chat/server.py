# UDP 聊天软件 服务器端
import socket
import threading
import time
CONNECT = []
def main():
    # 创建 socket 对象
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    port = int(input('请输入端口号：'))
    # 绑定端口号
    try:
      s.bind(('', port))
    except Exception as e:
      print(e)
      return
    # 接收客户端的连接
    while True:
        try:
          data, addr = s.recvfrom(1024)
        except:
          print('接收失败')
          # 关闭 socket 重新连接
          s.close()
          s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
          s.bind(('', port))
          print('重新连接成功')
          continue
        if data.decode('utf-8').split(' ')[0] == 'CONNECT':
          CONNECT.append(addr)
          try:
            s.sendto(('OK').encode('utf-8'), addr)
          except:
            print('客户端连接失败！')
            CONNECT.remove(addr)
          print('客户端连接成功！', addr)
        elif data.decode('utf-8').split(' ')[0] == 'SEND':
          str2 = ''
          str2 += str(addr)
          str2 += ': '
          for i in CONNECT:
            print('正在发送给' + str(i))
            for j in data.decode('utf-8').split(' ')[1:]:
              str2 += j
              str2 += ' '
            try:
              s.sendto(str2.encode('utf-8'), i)
            except:
              print("发送失败")
            str2 = ''
            str2 += str(addr)
            str2 += ': '
          print('发送成功！')
        elif data.decode('utf-8').split(' ')[0] == 'DISCONNECT':
          CONNECT.remove(addr)
          print('客户端断开连接！', addr)
        time.sleep(1)
main()