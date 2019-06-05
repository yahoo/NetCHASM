# Copyright 2019, Oath Inc.
# Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#!/usr/bin/perl
use Socket;
use strict;
use Encode qw/encode/;
use Class::Struct;
use Socket qw(:DEFAULT inet_ntop);
my @CheckTypes = ("HM_CHECK_DEFAULT",
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
    "HM_CHECK_AUX_HTTPS_NO_PEER_CHECK");

my @DualStackL = ("IPv4 only", "IPv6 only" , "Both");
my @Modes = (
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
);

my @Reasons = (
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
);

my @States = (
    "HM_CHECK_INACTIVE",
    "HM_CHECK_QUEUED",
    "HM_CHECK_IN_PROGRESS",
    "HM_CHECK_FAILED"
);

my $server_path = '/home/y/var/run/netchasm/controlsocket';

sub setServerPath {
	$server_path = $_[0];
}

sub createSocket {
        my $name = '/home/y/var/run/netchasm/controlsocket';
        my $sock_addr = sockaddr_un($server_path);
        socket(my $sock, AF_UNIX, SOCK_SEQPACKET,0) || die "socket: $!";
        connect($sock, $sock_addr) || die "connect: $!";
        return $sock;
}

sub sendMessage {
        my $sock = $_[0];
        my $message = $_[1];
        send $sock,$message,0;
}

sub receiveData {
	my $sock = $_[0];
	my $expected = $_[1];
	my $result = '';
	while (length($result) != $expected) {
		recv $sock,my $data, $expected,0;
		$result = $result.$data;
	}
	return $result;
}


sub closeSocket {
	my $sock = $_[0];
	close($sock) if $sock
}

sub threadInfo {
	my $sock = createSocket();
	if($sock) {
		sendMessage($sock, "threadinfo");
		my $line = receiveData($sock,16);
		my @result = unpack("QQ", $line);
		close($sock);
		return @result;
	}
	return undef;	
}

sub workInfo {
        my $sock = createSocket();
	if($sock) {
        	sendMessage($sock, "workqueueinfo");
        	my $line = receiveData($sock,4);
        	my @result = unpack("V", $line);
		close($sock);
 		return $result[0];
	}
        return undef;
}

sub scheduleQueueInfo {
        my $sock = createSocket();
	if($sock) {
        	sendMessage($sock, "schdqueueinfo");
        	my $line = receiveData($sock,8);
        	my @result = unpack("Q", $line);
		close($sock);
 		return $result[0];
	}
        return undef;
}

sub hostGroupList {
	my $sock = createSocket();
	if($sock) {
        	sendMessage($sock, "hostgrouplist");
        	my $line = receiveData($sock,8);
        	my @size = unpack("Q", $line);
		my $result = receiveData($sock, $size[0]);
        	my @hostgroups = split /,/ , $result;
		close($sock);
		return @hostgroups; 
	}
        return undef;
}

sub hostList {
        my $sock = createSocket();
	if ($sock) {
		my $hostGroup = $_[0];
       		sendMessage($sock, "hostlist " . $hostGroup);
       	 	my $line = receiveData($sock,8);
        	my @size = unpack("Q", $line);
        	my $result = receiveData($sock, $size[0]);
        	my @hosts = split /,/ , $result;
		close($sock);
		return @hosts;
	}
        return undef;
}

sub getHostCheck {
	my $sock = createSocket();
	if($sock) {
        	my $hostGroup = $_[0];
		my $hostName = $_[1];
        	sendMessage($sock, "hostcheck " . $hostGroup . " " . $hostName);
        	my $result = receiveData($sock,48);
        	my($errnum, $cs, $dum, $dum1, $status, $reason, $cr, $scr, $tr, $st)  = unpack("VCCSVVQQQQ", $result);
        	my $hc = {};
		$hc->{CheckStatus} = $cs;
		if($cs) {
			$hc->{ErrorNum} = $errnum;
			$hc->{Status} = $status;
			$hc->{Reason} = $Reasons[$reason];
			$hc->{ConnectRT} = $cr;
			$hc->{SmoothConnectRT} = $scr;
			$hc->{TotalRT} = $tr;
			$hc->{StatusTime} = $st;
 		} else {
			$hc->{ErrorNum} = undef;
			$hc->{Status} = undef;
			$hc->{Reason} = undef;
			$hc->{ConnectRT} = undef;
			$hc->{SmoothConnectRT} = undef;
			$hc->{TotalRT} = undef;
			$hc->{StatusTime} = undef;
		}
	       	close($sock);
		return $hc;
	}
        return undef;
}

sub getHostSchdInfo {
	my $sock = createSocket();
	if($sock) {
		my $hostGroup = $_[0];
		my $hostName = $_[1];
                sendMessage($sock, "hostschdinfo " . $hostGroup . " " . $hostName);
		my $result = receiveData($sock, 56);
		my($hasv4, $hasv6, $dummy, $dummy5, $v4LCT, $v4NCT, $v4S, $dummy1, $dummy2, $dummy7, $v6LCT, $v6NCT, $v6S, $dummy3, $dummy4, $count) = unpack("CCSVQQCCSVQQCCSV", $result);
		my $dns_sched = {};
		$dns_sched->{hasv4} = $hasv4;
		$dns_sched->{hasv6} = $hasv6;
		if($hasv4){
			$dns_sched->{v4LastCheckTime} = localtime($v4LCT/1000);
			$dns_sched->{v4NextCheckTime} = localtime($v4NCT/1000);
			$dns_sched->{v4State} = $States[$v4S];
		} else {
			$dns_sched->{v4LastCheckTime} = undef;
			$dns_sched->{v4NextCheckTime} = undef;
			$dns_sched->{v4State} = undef;
		}
		if($hasv6) {
			$dns_sched->{v6LastCheckTime} = localtime($v6LCT/1000);
                        $dns_sched->{v6NextCheckTime} = localtime($v6NCT/1000);
                        $dns_sched->{v6State} = $States[$v6S];
		} else {
			$dns_sched->{v5LastCheckTime} = undef;
                        $dns_sched->{v5NextCheckTime} = undef;
                        $dns_sched->{v5State} = undef;
		}
		$dns_sched->{count} = $count;
		for(my $i=0 ; $i< $count; $i++ ) {
			$result = receiveData($sock, 41);
			my($addrType, $dummy1, $dummy2, $addr1, $addr2, $addr3, $addr4, $dummy3, $lct, $nct, $state ) = unpack("CCSVIIIIQQC",$result);
			if($addrType == AF_INET){
				$dns_sched->{$i}{addr} = inet_ntop($addrType, pack("V",$addr1));
			}
			if($addrType == AF_INET6){
                                $dns_sched->{$i}{addr} = inet_ntop($addrType, pack("VVVV",$addr1, $addr2, $addr3, $addr4));
                        }
			$dns_sched->{$i}{lastCheckTime} = localtime($lct/1000);
			$dns_sched->{$i}{nextCheckTime} = localtime($nct/1000);
			$dns_sched->{$i}{state} = $States[$state]
		}
		close($sock);
		return $dns_sched;
	}
	return undef; 
}


sub getHostGroupParams {
	my $sock = createSocket();
	if($sock) {
		my $hostGroup = $_[0];
		sendMessage($sock, "hostgroupparams " . $hostGroup);
		my $line = receiveData($sock,8);
		my @size = unpack("Q", $line);
		my $hgi = {};
		if($size[0]) {
			my $result = receiveData($sock, $size[0]);
			my($status, $checktype, $port, $ds, $dummy, $dummy1, $cis ) = unpack("CCSCCSV", $result);
			my($status, $checktype, $port, $ds, $dummy, $dummy1, $cis , $sw, $mf, $ft, $ncr, $dummy2, $dummy3, $crd, $gt, $st, $ct, $cttl, $mode, $checkinfo) = unpack("CCSCCSVVVVCCSVVVQQCa$cis", $result);
			$hgi->{Status} = $status;
			$hgi->{CheckType} = $CheckTypes[$checktype];
			$hgi->{Port} = $port;
			$hgi->{DualStack} = $DualStackL[$ds];
			$hgi->{SmoothingWindow} = $sw;
			$hgi->{MaxFlaps} = $mf;
			$hgi->{FlappingThreshold} = $ft;
			$hgi->{NumCheckRetries} = $ncr;
			$hgi->{CheckRetryDelay} = $crd;
			$hgi->{GroupThreshold} = $gt;
			$hgi->{SlowThreshold} = $st;
			$hgi->{CheckTimeout} = $ct;
			$hgi->{CheckTTL} = $cttl;
			$hgi->{Mode} = $Modes[$mode];
			$hgi->{CheckInfo} = $checkinfo;
		} else {
			$hgi->{Status} = 0;
			$hgi->{CheckType} = undef;
			$hgi->{Port} = undef;
			$hgi->{DualStack} = undef;
			$hgi->{SmoothingWindow} = undef;
			$hgi->{MaxFlaps} = undef;
			$hgi->{FlappingThreshold} = undef;
			$hgi->{NumCheckRetries} = undef;
			$hgi->{CheckRetryDelay} = undef;
			$hgi->{GroupThreshold} = undef;
			$hgi->{SlowThreshold} = undef;
			$hgi->{CheckTimeout} = undef;
			$hgi->{CheckTTL} = undef;
			$hgi->{Mode} = undef;
			$hgi->{CheckInfo} = undef;

		}
		close($sock);
		return $hgi;
	}
	return undef;
}

# Usage Example
=begin
print(threadInfo(). "\n");
print(workInfo(), "\n");
print(scheduleQueueInfo()."\n");
my @hostg = hostGroupList();
foreach my $hostgr (@hostg) {
	my @hosts = hostList($hostgr);
	print(@hosts."\n");
	print(getHostGroupParams($hostgr)."\n");
	foreach my $host (@hosts) {
		print(getHostCheck($hostgr, $host)."\n");
		my $hsi = getHostSchdInfo($hostgr, $host);
		print($hsi->{0}{addr}."\n");
	}
}
=end
