// Copyright 2019, Oath Inc.
// Licensed under the terms of the Apache 2.0 license. See LICENSE file in the root of the distribution for licensing details.
#ifndef HMSOCKETUTIL_BASE_H_
#define HMSOCKETUTIL_BASE_H_
//! Class to help socket communication.
class HMSocketUtilBase
{
public:
    HMSocketUtilBase(bool connected, int sock) :
        m_connected(connected),
        m_socket(sock) {}
    virtual ~HMSocketUtilBase() {}
    HMSocketUtilBase& operator=(const HMSocketUtilBase&) = delete;        // Disallow copying
    HMSocketUtilBase(const HMSocketUtilBase&) = delete;


    /*!
         Called to receive command.
         \param string to receive.
         \param wait time for the data.
     */
    bool receiveCommand(std::string& cmd, timeval& tv);


    /*!
         Called to send command.
         \param string to send.
     */
    bool sendCommand(const std::string& cmd);


    /*!
         Called to send a response message.
         \param data to send.
         \param size of data.
     */
    bool sendMessage(const char* data, uint64_t size);

    /*!
         Called to receive data over socket. wait time is 3 seconds
         \param string to receive.
         \param size of string.
     */
    bool receiveMessage(char *data, uint64_t size);

protected:
    bool m_connected;
    int m_socket;

private:
    /*!
         Called to send data across the socket.
         \param data buffer.
         \param size of the data buffer.
     */
    virtual bool sendData(const char* buffer, uint64_t size) = 0;
    /*!
         Called to receive data across the socket.
         \param data buffer.
         \param size of the data buffer.
         \param wait time for the data.
     */
    virtual bool recvData(char* data, uint64_t size, timeval& tv) = 0;
};

#endif /* HMSOCKETUTIL_BASE_H_ */
