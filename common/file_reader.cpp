
#include <common/file_reader.hpp>
#include <common/error.hpp>

FileReader::FileReader()
{

}

FileReader::~FileReader()
{

}

int FileReader::Open(const std::string &p)
{
    int ret = ERROR_SUCCESS;
    if (fd_ > 0)
    {
        return ERROR_SYSTEM_FILE_ALREADY_OPENED;
    }
    return ret;
}
