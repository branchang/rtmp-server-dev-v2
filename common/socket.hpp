#ifndef RS_SOCKET_HPP
#define RS_SOCKET_HPP

#include <common/core.hpp>
#include <common/io.hpp>
#include <st.h>

extern int SendLargeIovs(IProtocolReaderWriter* rw,
                           iovec *iovs,
                           int size,
                           ssize_t* pnwrite);

class StSocket : public IProtocolReaderWriter
{
public:
    StSocket(st_netfd_t client_stfd);
    virtual ~StSocket();

public:
    virtual bool IsNeverTimeout(int64_t timeout_us) override;
    virtual void SetRecvTimeout(int64_t timeout_us) override;
    virtual int64_t GetRecvTimeout() override;
    virtual void SetSendTimeout(int64_t timeout_us) override;
    virtual int64_t GetSendTimeout() override;
    virtual int64_t GetSendBytes() override;
    virtual int64_t GetRecvBytes() override;

public:
    virtual int32_t Read(void *buf, size_t size, ssize_t *nread) override;
    virtual int32_t ReadFully(void *buf, size_t size, ssize_t *nread) override;
    virtual int32_t Write(void *buf, size_t size, ssize_t *nread) override;
    virtual int32_t WriteEv(const struct iovec *iov, size_t iov_size, ssize_t *nwrite) override;

private:
    st_netfd_t stfd_;
    int64_t send_timeout_;
    int64_t recv_timeout_;
    int64_t send_bytes_;
    int64_t recv_bytes_;
};

#endif
