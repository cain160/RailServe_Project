'''
    Simple udp socket server
'''

import socket
import sys

HOST = ''   # Symbolic name meaning all available interfaces
PORT = 1995 # Arbitrary non-privileged port

# Datagram (udp) socket
try :
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print ('Socket created')
except socket.error as msg :
    print ('Failed to create socket. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
    sys.exit()


# Bind socket to local host and port
try:
    s.bind((HOST, PORT))
except socket.error as msg:
    print ('Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
    sys.exit()

print ('Socket bind complete')

#now keep talking with the client
while 1:
    # receive data from client (data, addr)
    d = s.recvfrom(1024)
    data = d[0]
    addr = d[1]

    if not data:
        break

    reply = 'OK...' + data.decode('utf-8')

    print(reply)
    s.sendto(bytes(reply,'utf8') , addr)
    print ('Message[' + addr[0] + ':' + str(addr[1].decode('utf-8')) + '] - ' + data.strip())

s.close()
