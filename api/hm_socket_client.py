# -*- coding: utf-8 -*-
# Copyright 2019, Oath Inc.
# Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
from struct import *
import socket
import time
from collections import namedtuple

from netchasm  import threadinfo_pb2
from netchasm import generalparams_pb2
from netchasm import datahostgroup_pb2
from netchasm import ipaddress_pb2
from netchasm import datacheckresult_pb2
from netchasm import hostschdinfo_pb2
from netchasm import hashinfo_pb2

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

Responses = [
    "RESPONSE_NONE",
    "RESPONSE_CONNECTED",
    "RESPONSE_FAILED",
    "RESPONSE_DNS_FAILED"
]


def getAddress(x):
	addr = None
	if x.type == 2:
		addr = socket.inet_ntop(x.type, pack('<L', x.addr0))
	if x.type == 10:
		addr = socket.inet_ntop(x.type ,  pack('<LLLL', x.addr0, x.addr1, x.addr2, x.addr3))
	return addr

class HashHGPair:
	def __init__(self, pHash = None):
		if pHash is None:
			self.groupName=''
			self.size = 0
			self.hash = b''
		else:
			self.groupName = pHash.hostgroupname
			self.size = pHash.size
			self.hash = pHash.hash

	def packData(self):
		pHash = hashinfo_pb2.HashHGPair()
		pHash.hostgroupname = self.groupName
		pHash.size = self.size
		pHash.hash = self.hash
		return pHash


class GroupParams:
	def __init__(self, x = None):
		if x is None:
			self.groupName = ''
			self.checkType = 0
			self.port = 0
			self.dualStack = 1
			self.smoothingWindow = 10
			self.maxFlaps = 4
			self.flapThreshold = 60000
			self.numCheckRetries = 0
			self.checkRetryDelay = 0
			self.groupThreshold = 20
			self.slowThreshold = 20
			self.checkTimeout = 10000
			self.checkTTL = 30000
			self.passthroughInfo = 0
			self.checkInfo = ''
			self.hosts = []
			self.hostgroups = []
			self.remoteCheckType = 0
			self.distributedFallback = 0
			self.measurementOptions = 0
			self.remoteCheck = ''
		else:
			self.checkType = CheckTypes[x.checkType]
			self.port = x.port
			self.dualStack = DualStackL[x.dualstack]
			self.smoothingWindow = x.smoothingWindow
			self.maxFlaps = x.maxFlaps
			self.flapThreshold = x.flapThreshold
			self.numCheckRetries = x.numCheckRetries
			self.checkRetryDelay = x.checkRetryDelay
			self.groupThreshold = x.groupThreshold
			self.slowThreshold = x.slowThreshold
			self.checkTimeout = x.checkTimeout
			self.checkTTL = x.checkTTL
			self.passthroughInfo = x.passthroughInfo
			self.checkInfo = x.checkInfo
			self.hosts = x.hosts
			self.hostgroups = x.hostgroups

	def packData(self):
		hostgroupparams = datahostgroup_pb2.DataHostGroup()
		hostgroupparams.checkType = self.checkType
		hostgroupparams.port = self.port
		hostgroupparams.dualstack = self.dualStack
		hostgroupparams.smoothingWindow = self.smoothingWindow
		hostgroupparams.maxFlaps = self.maxFlaps
		hostgroupparams.flapThreshold = self.flapThreshold
		hostgroupparams.numCheckRetries = self.numCheckRetries
		hostgroupparams.checkRetryDelay = self.checkRetryDelay
		hostgroupparams.groupThreshold = self.groupThreshold
		hostgroupparams.slowThreshold = self.slowThreshold
		hostgroupparams.checkTimeout = self.checkTimeout
		hostgroupparams.checkTTL = self.checkTTL
		hostgroupparams.passthroughInfo = self.passthroughInfo
		hostgroupparams.measurementOptions = self.measurementOptions 
		hostgroupparams.checkInfo = self.checkInfo
		hostgroupparams.remoteCheckType = self.remoteCheckType
		hostgroupparams.distributedFallback = self.distributedFallback
		hostgroupparams.remoteCheck = self.remoteCheck
		for host in self.hosts:
			hostgroupparams.hosts.append(host)
		for hostg in self.hostgroups:
			hostgroupparams.hostgroups.append(hostg)
		return hostgroupparams

class HostCheck:
	def __init__(self, x):
		self.address = getAddress(x.address)
		self.start = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x.start/1000))
		self.end = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x.end/1000))
		self.responseTime = x.responseTime
		self.totalResponseTime = x.totalResponseTime;
		self.minResponseTime = x.minResponseTime;
		self.maxResponseTime = x.maxResponseTime;
		self.smoothedResponseTime = x.smoothedResponseTime;
		self.sumResponseTime = x.sumResponseTime;
		self.numChecks = x.numChecks;
		self.numResponses = x.numResponses;
		self.numConnectFailures = x.numConnectFailures;
		self.numFailures = x.numFailures;
		self.numTimeouts = x.numTimeouts;
		self.numFlaps = x.numFlaps;
		self.status = x.status;
		self.response = Responses[x.response];
		self.reason = Reasons[x.reason];
		self.numFailedChecks = x.numFailedChecks;
		self.numSlowResponses = x.numSlowResponses;
		self.port = x.port;
		self.changeTime = x.changeTime;
		self.forceHostDown = x.forceHostDown;
		self.queueCheckTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x.queueCheckTime/1000));
		self.checkTime = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(x.checkTime/1000));

class DNS_SchedInfo:
	def __init__(self, x):
		self.hcs = []
		self.hasv4 = x.hasv4
		self.hasv6 = x.hasv6
		if self.hasv4:
			self.v4LastCheckTime = time.localtime(x.v4LastCheckTime/1000)
			self.v4NextCheckTime = time.localtime(x.v4NextCheckTime/1000)
			self.v4State = States[x.v4State]
		else:
			self.v4LastCheckTime = None
			self.v4NextCheckTime = None
			self.v4State = None
		
		if self.hasv6:
                        self.v6LastCheckTime = time.localtime(x.v6LastCheckTime/1000)
                        self.v6NextCheckTime = time.localtime(x.v6NextCheckTime/1000)
                        self.v6State = States[x.v6State]
		else:
			self.v6LastCheckTime = None
			self.v6NextCheckTime = None
			self.v6State = None

class Host_SchedInfo:
	def __init__(self, x):
		self.addr = getAddress(x.address)
		self.lastCheckTime = time.localtime(x.lastCheckTime/1000)
		self.nextCheckTime = time.localtime(x.nextCheckTime/1000)
		self.state = States[x.state]

class HealthMonClient:
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

	def __receivePacketSize(self, sock):
		data = self.__receiveData(sock, 8)
		(x,) = unpack('!Q', data)
		return x

	def __receiveUInt(self, sock):
		size = self.__receivePacketSize(sock)
		if size != 0:
			data = self.__receiveData(sock, size)
			gp = generalparams_pb2.Uint()
			if gp.ParseFromString(data):
				return gp.data
		return 0

	def __sendMessage(self, sock, msg):
		msgLen = len(msg)
		sock.send(pack('!Q', msgLen))
		sock.send(msg.encode())

	def __sendBlob(self, sock, blob):
		sock.send(blob)

	def __receiveList(self, sock):
		size = self.__receivePacketSize(sock)
		if size != 0:
			data = self.__receiveData(sock, size)
			gp = generalparams_pb2.List()
			if gp.ParseFromString(data):
				return gp.items
		return []

	def workInfo(self):
		sock = self.__createSocket()
		if sock:
			self.__sendMessage(sock, '1 workqueueinfo')
			x = self.__receiveUInt(sock)
			self.__closeSocket(sock)
			return x
		return None

	def threadInfo(self):
		sock = self.__createSocket() 
		if sock:
			self.__sendMessage(sock,'1 threadinfo')
			size = self.__receivePacketSize(sock)
			if size != 0:
				data = self.__receiveData(sock, size)
				ti = threadinfo_pb2.ThreadInfo()
				if ti.ParseFromString(data):
					return(ti.numThreads, ti.numIdleThreads)
			self.__closeSocket(sock)
		return (None,None)

	def scheduleQueueInfo(self):
		sock = self.__createSocket()
		if sock:
			self.__sendMessage(sock, '1 schdqueueinfo')
			x = self.__receiveUInt(sock)
			self.__closeSocket(sock)
			return x
		return None


	def hostGroupList(self):
		sock = self.__createSocket()
		if sock:
			self.__sendMessage(sock,'1 hostgrouplist')
			data = self.__receiveList(sock)
			self.__closeSocket(sock)
			return data
		return None


	def hostList(self, hostGroup):
		sock = self.__createSocket()
		cmd = '1 hostlist ' + hostGroup
		if sock:
			self.__sendMessage(sock, cmd)
			data = self.__receiveList(sock)
			self.__closeSocket(sock)
			return data
		return None

	def getHostCheck(self, hostGroup, host):
		sock = self.__createSocket()
		cmd = '1 hostcheck ' + hostGroup + ' ' + host
		if sock:
			self.__sendMessage(sock, cmd)
			size = self.__receivePacketSize(sock)
			if size:
				hostchecks = []
				data = self.__receiveData(sock, size)	
				self.__closeSocket(sock)
				results = datacheckresult_pb2.DataCheckResults()
				if results.ParseFromString(data):
					for x in results.results:
						hstCheck = HostCheck(x)
						hostchecks.append(hstCheck)
				return hostchecks
		return None


	def getHostSchdInfo(self, hostGroup, host):
		sock = self.__createSocket()
		cmd = '1 hostschdinfo ' + hostGroup + ' ' + host
		if sock:
			self.__sendMessage(sock, cmd)
			size = self.__receivePacketSize(sock)
			if size:
				data = self.__receiveData(sock, size)
				dnsSchedInfo = hostschdinfo_pb2.DNSSchedInfo()
				self.__closeSocket(sock)
				if dnsSchedInfo.ParseFromString(data):
					dnsSched = DNS_SchedInfo(dnsSchedInfo)
					for y in dnsSchedInfo.hostSchedInfo:
						dnsSched.hcs.append(Host_SchedInfo(y))
					return dnsSched
		return None

	def getHostGroupParams(self, hostGroup):
		sock = self.__createSocket()
		cmd = '1 hostgroupparams ' + hostGroup
		if sock:
			self.__sendMessage(sock, cmd)
			size = self.__receivePacketSize(sock)
			if size:
				data = self.__receiveData(sock, size)	
				self.__closeSocket(sock)
				hostgroupparams = datahostgroup_pb2.DataHostGroup()
				if hostgroupparams.ParseFromString(data):
					grpParams = GroupParams(hostgroupparams)
					return grpParams
		return None

	def removeHostGroup(self, hostGroupName):
		sock = self.__createSocket()
		cmd = '1 removehostgroup ' + hostGroupName
		if sock:
			self.__sendMessage(sock, cmd)
			size = self.__receivePacketSize(sock)
			if size:
				data = self.__receiveData(sock, size)
				self.__closeSocket(sock)
				boolPacket = generalparams_pb2.Bool()
				if boolPacket.ParseFromString(data):
					return boolPacket.data
		return False

	def getConfigHash(self):
		sock = self.__createSocket()
		cmd = '1 gettransconfighash'
		if sock:
			self.__sendMessage(sock, cmd)
			size = self.__receivePacketSize(sock)
			if size:
				data = self.__receiveData(sock, size)
				self.__closeSocket(sock)
				hashPacket = hashinfo_pb2.HashHGPair()
				if hashPacket.ParseFromString(data):
					return HashHGPair(hashPacket)
		return HashHGPair()			

	def commitTransaction(self, hashValue):
		sock = self.__createSocket()
		pHash = hashValue.packData()
		bytesData = pHash.SerializeToString()
		cmd = '1 committransation ' + str(len(bytesData))
		if sock:
			self.__sendMessage(sock, cmd)
			self.__sendBlob(sock, bytesData)
			x = self.__receiveUInt(sock)
			self.__closeSocket(sock)
			return x
		return 0

	def clearTransaction(self):
		sock = self.__createSocket()
		cmd = '1 cleartransation'
		if sock:
			self.__sendMessage(sock, cmd)
			size = self.__receivePacketSize(sock)
			if size:
				data = self.__receiveData(sock, size)
				self.__closeSocket(sock)
				boolPacket = generalparams_pb2.Bool()
				if boolPacket.ParseFromString(data):
					return boolPacket.data
		return False

	def addHostGroup(self, hostGroup):
		sock = self.__createSocket()
		pHostGroup = hostGroup.packData()
		bytesData = pHostGroup.SerializeToString()
		cmd = '1 addhostgroup ' + hostGroup.groupName + ' ' + str(len(bytesData))
		if sock:
			self.__sendMessage(sock, cmd)
			self.__sendBlob(sock, bytesData)
			size = self.__receivePacketSize(sock)
			if size:
				data = self.__receiveData(sock, size)
				self.__closeSocket(sock)
				boolPacket = generalparams_pb2.Bool()
				if boolPacket.ParseFromString(data):
					return boolPacket.data
		return False

# Usage Example
'''
hm = HealthMonClient()

print(hm.workInfo())
print(hm.threadInfo())
print(hm.scheduleQueueInfo())

print(hm.hostGroupList())
ha = hm.getConfigHash()
print(vars(ha))
print(hm.removeHostGroup("ymaven.b1.b.corp.yahoo.com"))
ha = hm.getConfigHash()
print(vars(ha))
print(hm.commitTransaction(ha))
gp = GroupParams()
gp.groupName = 'ymaven2.b1.b.corp.yahoo.com'
gp.checkInfo = '//<host:port>/akamai'
gp.port = 9999
gp.checkRetries = 3
gp.checkRetryDelay = 5000
gp.checkType = 2
gp.passthroughInfo = 0
gp.ttl = 100000
gp.timeout = 10000
gp.hosts.append('ymaven-01.infra.corp.gq1.yahoo.com')
gp.hosts.append('ymaven-01.infra.corp.ne1.yahoo.com')
gp.hosts.append('ymaven-02.infra.corp.gq1.yahoo.com')
gp.hosts.append('ymaven-02.infra.corp.ne1.yahoo.com')
print(hm.addHostGroup(gp))
ha = hm.getConfigHash()
print(hm.commitTransaction(ha))

hgl = hm.hostGroupList()

for hg in hgl:
	print(hg)
	hl = hm.hostList(hg)
	print(hl)
	print(vars(hm.getHostGroupParams(hg)))
#	print(hl)
	for h in hl:
		res = hm.getHostCheck(hg, h)
		for r in res:
			print(vars(r))
		x = hm.getHostSchdInfo(hg,h)
		print(vars(x))
		for y in x.hcs:
			print(vars(y))	
'''
