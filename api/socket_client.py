# Copyright 2019, Oath Inc.
# Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
from struct import *
import socket
import time
from collections import namedtuple

CheckTypes = ["HM_CHECK_DEFAULT",
    "HM_CHECK_NONE",
    "HM_CHECK_HTTP",
    "HM_CHECK_HTTPS",
    "HM_CHECK_TCP",
    "HM_CHECK_FTP",
    "HM_CHECK_DNS",
    "HM_CHECK_DNSVC",
    "HM_CHECK_HTTPS_NO_PEER_CHECK",
    "HM_CHECK_FTPS",
    "HM_CHECK_FTPS_EXPLICIT_NO_PEER_CHECK",
    "HM_CHECK_AUX_HTTP",
    "HM_CHECK_AUX_HTTPS",
    "HM_CHECK_AUX_HTTPS_NO_PEER_CHECK"]
DualStackL = ["IPv4 only", "IPv6 only" , "Both"]
Modes = [
    "HM_MODE_GROUP_RT_RR",
    "HM_MODE_GROUP_RT_RANDOM",
    "HM_MODE_BEST_RT",
    "HM_MODE_FALLBACK",
    "HM_MODE_SINGLE_ACTIVE",
    "HM_MODE_GROUP_RT_ALL",
    "HM_MODE_ALL_ACTIVE",
    "HM_MODE_GROUP_RT_LOCAL_RR",
    "HM_MODE_LOADFB",
    "HM_MODE_GROUP_RT_HASH",
    "HM_MODE_GROUP_RT_HASH_ALL",
    "HM_MODE_GROUP_LEAST_CONNS",
]

Reasons = [
    "HM_REASON_NONE",
    "HM_REASON_SUCCESS",
    "HM_REASON_DNS_NOTFOUND",
    "HM_REASON_DNS_TIMEOUT",
    "HM_REASON_DNS_FAILURE",
    "HM_REASON_YNET_NOTFOUND",
    "HM_REASON_CONNECT_TIMEOUT",
    "HM_REASON_CONNECT_FAILURE",
    "HM_REASON_REQUEST_FAILURE",
    "HM_REASON_RESPONSE_TIMEOUT",
    "HM_REASON_RESPONSE_FAILURE",
    "HM_REASON_RESPONSE_DOWN",
    "HM_REASON_RESPONSE_404",
    "HM_REASON_RESPONSE_403",
    "HM_REASON_RESPONSE_3XX",
    "HM_REASON_RESPONSE_5XX",
    "HM_REASON_INTERNAL_ERROR"
]

States = [
    "HM_CHECK_INACTIVE",
    "HM_CHECK_QUEUED",
    "HM_CHECK_IN_PROGRESS",
    "HM_CHECK_FAILED"
]


class GroupParams:
	def __init__(self,props):
		self.check_status = props[0]
		if props[0]:
			self.checkType = CheckTypes[props[1]]
			self.port = props[2]
			self.dualStack = DualStackL[props[3]]
			self.smoothingWindow = props[5]
			self.maxFlaps = props[6]
			self.flapThreshold = props[7]
			self.numCheckRetries = props[8]
			self.checkRetryDelay = props[9]
			self.groupThreshold = props[10]
			self.slowThreshold = props[11]
			self.checkTimeout = props[12]
			self.checkTTL = props[13]
			self.mode = Modes[props[14]]
			self.check_info = props[15]
		else:	
			self.checkType = None
			self.port = None
			self.dualStack = None
			self.smoothingWindow = None
			self.maxFlaps = None
			self.flapThreshold = None
			self.numCheckRetries = None
			self.checkRetryDelay = None
			self.groupThreshold = None
			self.slowThreshold = None
			self.checkTimeout = None
			self.checkTTL = None
			self.mode = None
			self.check_info = None


class HostCheck:
	def __init__(self, props):
		if props[1]:
			self.errnum = props[0]
			self.check_status = props[1]
			self.status = props[2]
			self.reason = Reasons[props[3]]
			self.connect_rt = props[4]
			self.smoothed_connect_rt = props[5]
			self.total_rt = props[6]
			self.statustime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(props[7]/1000))
		else:
			self.errnum = None
			self.check_status = props[1]
			self.status = None
			self.reason = None
			self.connect_rt = None
			self.smoothed_connect_rt = None
			self.total_rt = None
			self.statustime = None

class DNS_SchedInfo:
	def __init__(self, props):
		self.hasv4 = props[0]
		self.hasv6 = props[1]
		if self.hasv4:
			self.v4LastCheckTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(props[2]/1000))
			self.v4NextCheckTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(props[3]/1000))
			self.v4State = States[props[4]]
		else:
			self.v4LastCheckTime = None
			self.v4NextCheckTime = None
			self.v4State = None
		
		if self.hasv6:
			self.v6LastCheckTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(props[5]/1000))
			self.v6NextCheckTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(props[6]/1000))
			self.v6State = States[props[7]]
		else:
			self.v6LastCheckTime = None
			self.v6NextCheckTime = None
			self.v6State = None
		self.count = props[8]
		self.hcs = []

class HealthCheck_SchedInfo:
	def __init__(self, props):
		self.addr = None
		if props[0] == 2:
			self.addr = socket.inet_ntop(props[0] , pack('<L', props[1]))
		if props[0] == 10:
			self.addr = socket.inet_ntop(props[0] , pack('<LLLL', props[1], props[2], props[3], props[4]))
		self.lastCheckTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(props[5]/1000))
		self.nextCheckTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(props[6]/1000))
		self.state = States[props[7]]
	
class NetCHASMClient:
	def __init__(self, path = '/home/y/var/run/netchasm/controlsocket'):
		self.server_address = path

	def __createSocket(self):
		sock = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
		try:
			sock.connect(self.server_address)
			return sock
		except socket.error as msg:
			print(msg)
		return None	

	def __closeSocket(self, socket):
		socket.close()

	def __receiveData(self, sock, expected):
		result = b''
		while len(result) != expected:
			data = sock.recv(expected)
			result+=data
		return result

	def workInfo(self):
		sock = self.__createSocket()
		if sock:
			sock.sendall(b'workqueueinfo')
			data = self.__receiveData(sock, 4)
			(x,) = unpack('I',data);   
			#print(type(x))
			self.__closeSocket(sock)
			return x

	def threadInfo(self):
		sock = self.__createSocket() 
		if sock:
			sock.sendall(b'threadinfo')
			data = self.__receiveData(sock, 16)
			(total,idle) = unpack('QQ',data)
			self.__closeSocket(sock)
		return (total,idle)

	def scheduleQueueInfo(self):
		sock = self.__createSocket()
		if sock:
			sock.sendall(b'schdqueueinfo')
			data = self.__receiveData(sock, 8)
			(x,) = unpack('Q',data)
			self.__closeSocket(sock)
		return x


	def hostGroupList(self):
		sock = self.__createSocket()
		if sock:
			sock.sendall(b'hostgrouplist')
			data = self.__receiveData(sock, 8)
			(size,) = unpack('Q', data)
			data = self.__receiveData(sock, size)
			self.__closeSocket(sock)
		return data.decode().split(',')


	def hostList(self, hostGroup):
		sock = self.__createSocket()
		cmd = 'hostlist ' + hostGroup
		if sock:
			sock.sendall(cmd.encode())
			data = self.__receiveData(sock, 8)
			(size,) = unpack('Q', data)
			data = self.__receiveData(sock, size)
			self.__closeSocket(sock)
		return data.decode().split(',')

	def getHostCheck(self, hostGroup, host):
		sock = self.__createSocket()
		hostCheck = namedtuple('HostCheck', 'error check_status status reason connect_rt smooth_connect_rt total_rt status_time')
		cmd = 'hostcheck ' + hostGroup + ' ' + host
		if sock:
			sock.sendall(cmd.encode())
			data = self.__receiveData(sock, 48)	
			x = hostCheck._make(unpack('i?IIQQQQ', data))
			hstCheck = HostCheck(x)
			self.__closeSocket(sock)
		return hstCheck


	def getHostSchdInfo(self, hostGroup, host):
		sock = self.__createSocket()
		dnsCheck = namedtuple('DNSCheck', 'hasv4 hasv6 v4LastCheckTime v4NextCheckTime v4State v6LastCheckTime  v6NextCheckTime v6State count')
		healthCheck = namedtuple('HealthCheck', 'addrtype addr1, addr2, addr3 addr4 lastCheckTime nextCheckTime state')
		cmd = 'hostschdinfo ' + hostGroup + ' ' + host
		if sock:
			sock.sendall(cmd.encode())
			data = self.__receiveData(sock, 56)	
			x = dnsCheck._make(unpack('??QQBQQBI', data))
			dns_sched = DNS_SchedInfo(x)
			for i in range(dns_sched.count):
				data_hc = self.__receiveData(sock, 41) 
				y = healthCheck._make(unpack('BIIIIQQB', data_hc))
				#print(HealthCheck_SchedInfo(y))
				dns_sched.hcs.append(HealthCheck_SchedInfo(y))
			self.__closeSocket(sock)
		return dns_sched


	def getHostGroupParams(self, hostGroup):
		sock = self.__createSocket()
		frmt = '?BHBIIIIBIIIQQB'
		hostCheck = namedtuple('HostCheck', 'check_status checkType port dualStack checkInfoSize smoothingWindow  maxFlaps flapThreshold numCheckRetries checkRetryDelay groupThreshold slowThreshold checkTimeout checkTTL mode check_info')
		cmd = 'hostgroupparams ' + hostGroup
		if sock:
			sock.sendall(cmd.encode())
			data = self.__receiveData(sock, 8)
			(size,) = unpack('Q', data)
			if size:
				datasize = size - calcsize(frmt)
				data = self.__receiveData(sock, size)	
				x = hostCheck._make(unpack(frmt + str(datasize) + 's', data))
				x = x._replace(check_status = True);
				checkinfo = x[15][:x[4]-1]
				self.__closeSocket(sock)
				x = x._replace(check_info = checkinfo.decode())
				grpParams = GroupParams(x)
				return grpParams
			else:
				x = [False]
				grpParams = GroupParams(x)
				return grpParams;

# Usage Example
'''
hm = NetCHASMClient()
print(hm.workInfo())
print(hm.threadInfo())
print(hm.scheduleQueueInfo())
#print(hm.hostGroupList())
hgl = hm.hostGroupList()
print("size = " , len(hgl))

for hg in hgl:
	print(hg)
	hl = hm.hostList(hg)
	print(hm.getHostGroupParams(hg))
	print(hl)
	for h in hl:
		print(hm.getHostCheck(hg, h))
		print(hm.getHostSchdInfo(hg, h).hcs[0].addr)
'''			
