
#include <common/file.hpp>
#include <common/error.hpp>
#include <common/log.hpp>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

FileReader::FileReader()
{

}

FileReader::~FileReader()
{

}

int32_t FileReader::Open(const std::string &p)
{
    int32_t ret = ERROR_SUCCESS;
    if (fd_ > 0)
    {
        return ERROR_SYSTEM_FILE_ALREADY_OPENED;
    }
    return ret;
}

FileWriter::FileWriter()
{
    stfd_ = nullptr;
}

FileWriter::~FileWriter()
{
    Close();
}

void FileWriter::Close()
{
    int ret = ERROR_SUCCESS;

    if (st_netfd_close(stfd_) < 0)
    {
        ret = ERROR_SYSTEM_FILE_CLOSE;
        rs_error("close file %s failed. ret=%d", path_.c_str(), ret);
    }
}

int FileWriter::Open(const std::string &path, bool append)
{
    int ret = ERROR_SUCCESS;

    if (stfd_)
    {
        ret = ERROR_SYSTEM_FILE_ALREADY_OPENED;
        rs_error("file %s already open. ret=%d", path.c_str(), ret);
        return ret;
    }

    int flags = O_CREAT | O_WRONLY | O_TRUNC;
    if (append)
    {
        flags = O_APPEND | O_WRONLY;
    }

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP  | S_IROTH;
    st_netfd_t stfd = st_open(path.c_str(), flags, mode);

    if (!stfd)
    {
        ret = ERROR_SYSTEM_FILE_OPENE;
        rs_error("open file %s failed. ret=%d", path.c_str(), ret);
        return ret;
    }
    path_ = path;
    stfd_ = stfd;

    return ret;
}

bool FileWriter::IsOpen()
{
    return stfd_ != nullptr;
}

void FileWriter::Lseek(int64_t offset)
{
    ::lseek(st_netfd_fileno(stfd_), (off_t)offset, SEEK_SET);
}
