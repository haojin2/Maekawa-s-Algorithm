from socket import *
from threading import *
from sys import *
TCP_IP=''
class message(object):
	""""""
	def __init__(self,command,src,dest,arg1,arg2,orig=None):
		if (orig == None):
			self.command = command
			self.src = src
			self.dest = dest
			self.arg1 = arg1
			self.arg2 = arg2
		else:
			self.command = orig.command
			self.src = orig.src
			self.dest = orig.dest
			self.arg1 = orig.arg1
			self.arg2 = orig.arg2
	def setargs(command,src,dest,arg1,arg2):
		self.command = command
		self.src = src
		self.dest = dest
		self.arg1 = arg1
		self.arg2 = arg2

BUFFER_SIZE=getsizeof(message)

def get_port(id):
	return 10000+id*10
def server_thread():
	s = socket(AF_INET,SOCK_STREAM)
	s.bind((TCP_IP,5005))
	s.listen(1)
	conn,addr = s.accept()
	print addr
	print conn
	data=conn.recv(1024)
	print data
def node_thread(node_id):
	a = message(1,1,1,1,1)
	print a.command
	print node_id
	s = socket(AF_INET,SOCK_STREAM)
	s.connect(('localhost',5005))
	s.sendall("hello world")
print BUFFER_SIZE
a=Thread(target=server_thread,args=())
b=Thread(target=node_thread,args=(10,))
a.start()
b.start()

a.join()
b.join()