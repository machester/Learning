# -*- coding: utf-8 -*-

import socket

IP_ADDR="127.0.0.1"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# 建立连接:
s.connect(('127.0.0.1', 8888))
print("connect to", IP_ADDR, "Waiting for message")
# 接收欢迎消息:
while  True:
    net_rec_data = s.recv(1024).decode('utf-8')

    net_rec_data = ("REC:<---" + net_rec_data)
    print("net_rec_data", net_rec_data)
    # change str to byte
    net_rec_data = net_rec_data.encode('utf-8')
    # send all data byte mode
    length = s.sendall(net_rec_data)
   
s.close()
