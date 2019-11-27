# NetCHASM
> An Automated health checking and server status verifiction system.

[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

NetCHASM conducts periodic health checks and retrieves load-feed-back data from distributed servers. It stores the information in a backend database and exposes the results through an API. The daemon allows checks over all major protocols HTTP, HTTPS, FTP, FTPS, TCP, and DNS. NetCHASM provides C++ APIS to access the backend databases, and APIs to query the running Daemon in C++, Python and Perl. The Daemon retrieve load-feedback in addition to out-of-band information to control server status for traffic load balancers. The Daemon is configurage using simple YAML configs.


## Table of Contents

- [Background](#background)
- [Install](#install)
- [Configuration](#configuration)
- [Usage](#usage)
- [Security](#security)
- [API](#api)
- [Contribute](#contribute)
- [License](#license)

## Background

Load balancers need to make routing decisions based on health, load, and manual control flags in near realtime. NetCHASM provides an easy, extensible framework to collect health, load, and control signals periodically from remote servers aggregating the data for our load balancers. The daemon can be configured to check HTTP, HTTPS, FTP, FTPS, TCP and DNS. Remote information about load, and a generic out-of-band XML files can also be periodically retrieved. The Daemon automatically combines redundant checks, schedules checks, and combines the information into a persistent backend and responds through an API. The Daemon conducts checks through a dedicated thread pool allowing checks to be made efficiently in parallel.

While NetCHASM can do a lot out of the box, the code is also highly extensible with new check types, back ends, loggers, and control APIs having base classes allowing for easy additions of new types with all base functionality already available. 

## Install

NetCHASM uses CMake. For convenience, a Makefile is included allowing a default make, a unit test make, and a doxygen generation. 

Dependency:
NetCHASM requires: curl, ssl, cares, MDBM, yaml-cpp, cppunit, and libevent. 

MDBM is available at: [https://github.com/yahoo/mdbm](https://github.com/yahoo/mdbm).

Other dependencies can be installed via package utilities.
For example, on Fedora/CentOS:

```
dnf install openssl-devel c-ares c-ares-devel yaml-cpp yaml-cpp-devel libevent libevent-devel make cmake gcc-c++ rapidxml-devel curl-devel cppunit-devel lcov protobuf protobuf-devel librdkafka librdkafka-devel

make build
sudo make install
```

Unit tests can be built and run using:
```
make test
```

Doxygen can be created using:
```
make docs
```


## Configuration
The Daemon is configured to run through a master configuration. An example with details on parameters can be found in [master-config-sample.yaml](master-config-sample.yaml).

Host checks are configured through check configuration files. An example showing all valid parameters are include in [check-conf-sample.yaml](check-conf-sample.yaml).

## Usage

Once the master and check configs are ready, the daemon can be run using:

```
NetCHASMDaemon <path to master config>
```

A variety of tools are included to interact with the daemon.

The hm_command tool issues commands to the daemon through the built in control socket. 
The hm_configure tool allows modifying the master config options on the daemon in real time.
The hm_dumpconfig tool will write out the check configuration of the running daemon.
The hm_reload tool will have the daemon reload updated configs without an interruption in checking.
The hm_set tool allows an individual host to be manually set to down.


## API

Additionally, NetCHASM provides a number of APIs. Python and Perl libraries that send commands through the control interface can be found in the API directory. NetCHASM exposes two C++ APIs. The first is the HMStorageAPI that allows a program to access the persistent backend database regardless of the daemon being running. The second API is a client control interface allowing a program full access to the control API of the running daemon.


## Contribute

Please refer to [the contributing.md file](Contributing.md) for information about how to get involved. We welcome issues, questions, and pull requests. Pull Requests are welcome.

## Maintainers
Joshua Juen: juen1jp@verizonmedia.com
Uthira Mohan: umohan@verizonmedia.com
Raghavendra Nataraj: raghavendran@verizonmedia.com

## License

This project is licensed under the terms of the [Apache 2.0](LICENSE-Apache-2.0) open source license. Please refer to [LICENSE](LICENSE) for the full terms.


