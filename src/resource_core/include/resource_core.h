#pragma once
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace lab4::resource_core
{
class ResourceError : public std::runtime_error
{
  public:
    ResourceError(const std::string& message);
};
class FileHandle
{
  public:
    FileHandle() = default;
    FileHandle(const std::string& path);
    ~FileHandle();
    void open(const std::string& path);
    void close();
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;
    bool isOpen() const noexcept;
    const std::string& getPath() const noexcept;
    std::FILE* getFile() const noexcept;

  private:
    std::string path_;
    std::FILE* file_ = nullptr; // указатель на структуру (std::string path, формат открытия)
};
class ResourceManager
{
  public:
    std::shared_ptr<FileHandle> caching(const std::string& path);
    void clearCache();
    void deleteDead();
    size_t sizeCache() const noexcept;

  private:
    std::unordered_map<std::string, std::weak_ptr<FileHandle>> Cache_; // поиск ресурса
};

} // namespace lab4::resource_core
