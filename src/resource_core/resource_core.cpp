#include "resource_core.h"

namespace lab4::resource_core
{
ResourceError::ResourceError(const std::string& message) : std::runtime_error(message) {}

FileHandle::FileHandle(const std::string& path)
{
    FileHandle::open(path);
}

FileHandle::~FileHandle()
{
    FileHandle::close();
}

FileHandle::FileHandle(FileHandle&& other) noexcept
{
    path_ = std::move(other.path_);
    file_ = other.file_;
    other.file_ = nullptr;
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept
{
    if (this != &other)
    {
        close();
        path_ = std::move(other.path_);
        file_ = other.file_;
        other.file_ = nullptr;
    }
    return *this;
}

void FileHandle::open(const std::string& path)
{
    FileHandle::close();
    file_ = std::fopen(path.c_str(), "r");
    if (!file_)
    {
        throw ResourceError("open error: " + path);
    }
    path_ = path;
}

void FileHandle::close()
{
    if (file_ != nullptr)
    {
        std::fclose(file_);
        file_ = nullptr;
        path_ = "";
    }
}

bool FileHandle::isOpen() const noexcept
{
    return file_ != nullptr;
}

const std::string& FileHandle::getPath() const noexcept
{
    return path_;
}

std::FILE* FileHandle::getFile() const noexcept
{
    return file_;
}

std::shared_ptr<FileHandle> ResourceManager::caching(const std::string& path)
{
    auto it = Cache_.find(path);
    if (it != Cache_.end())
    {
        if (auto shared = (*it).second.lock()) // превращение weak_ptr в shared_ptr
        {
            return shared;
        }
        Cache_.erase(it);
    }
    auto handle = std::make_shared<FileHandle>(path);
    Cache_[path] = handle; // сохранение в кэш с преобразованием в weak_ptr
    return handle;
}

void ResourceManager::clearCache()
{
    Cache_.clear();
}
void ResourceManager::deleteDead()
{
    for (auto it = Cache_.begin(); it != Cache_.end();)
    {
        if ((*it).second.expired())
        {
            it = Cache_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
size_t ResourceManager::sizeCache() const noexcept
{
    return Cache_.size();
}

} // namespace lab4::resource_core
