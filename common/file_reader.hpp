#ifndef RS_FILE_READER_H
#define RS_FILE_READER_H
#include <common/core.hpp>
#include <common/reader.hpp>

class FileReader: public Reader
{

public:
    FileReader();
    virtual ~FileReader();

public:
    virtual int32_t Open(const std::string &f) override;
    virtual void Close() override;

public:
    virtual bool IsOpen() override;
    virtual int64_t Tellg() override;
    virtual void Skip(int64_t size) override;
    virtual int64_t Lseek(int64_t offset) override;
    virtual int64_t FileSize() override;
    virtual int32_t Read(void *buf, size_t size, ssize_t *nread) override;

private:
    int32_t fd_;
};

#endif
