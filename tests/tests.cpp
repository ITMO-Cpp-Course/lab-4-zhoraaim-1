#include <catch2/catch_all.hpp>
#include <fstream>
#include <string>
#include "resource_core.h"

namespace {
void createTestFile(const std::string& path, const std::string& content = ":)") {
    std::ofstream out(path);
    out << content;
}

void removeTestFile(const std::string& path) {
    std::remove(path.c_str());
}
}

TEST_CASE("std::runtime_error")
{
    REQUIRE_THROWS_AS(
        throw lab4::resource_core::ResourceError("test message"),
        lab4::resource_core::ResourceError
    );

    try {
        throw lab4::resource_core::ResourceError("custom error");
    } catch (const std::runtime_error& e) {
        REQUIRE(std::string(e.what()) == "custom error");
    }
}

TEST_CASE("FileHandle: constructor")
{
    const std::string test_file = "test_raii.tmp";
    createTestFile(test_file);

    lab4::resource_core::FileHandle fh(test_file);

    REQUIRE(fh.isOpen() == true);
    REQUIRE(fh.getPath() == test_file);
    REQUIRE(fh.getFile() != nullptr);

    removeTestFile(test_file);
}
TEST_CASE("FileHandle: close ")
{
    const std::string test_file = "test_close.tmp";
    createTestFile(test_file);

    lab4::resource_core::FileHandle fh(test_file);
    REQUIRE(fh.isOpen() == true);

    fh.close();

    REQUIRE(fh.isOpen() == false);
    REQUIRE(fh.getFile() == nullptr);
    REQUIRE(fh.getPath().empty() == true);

    removeTestFile(test_file);
}
TEST_CASE("FileHandle: auto-close ")
{
    const std::string test_file = "test_auto.tmp";
    createTestFile(test_file);

    {
        lab4::resource_core::FileHandle fh(test_file);
        REQUIRE(fh.isOpen() == true);
    }//деструктор

    // После выхода из поля объект уничтожен
    lab4::resource_core::FileHandle fh2(test_file);
    REQUIRE(fh2.isOpen() == true);

    removeTestFile(test_file);
}
TEST_CASE("FileHandle: open")
{
    const std::string test_file = "test_open.tmp";
    createTestFile(test_file);

    lab4::resource_core::FileHandle fh;
    REQUIRE(fh.isOpen() == false);

    fh.open(test_file);

    REQUIRE(fh.isOpen() == true);
    REQUIRE(fh.getPath() == test_file);

    removeTestFile(test_file);
}
TEST_CASE("FileHandle: reopen changes path")
{
    const std::string file1 = "test_a.tmp";
    const std::string file2 = "test_b.tmp";
    createTestFile(file1);
    createTestFile(file2);

    lab4::resource_core::FileHandle fh(file1);
    REQUIRE(fh.getPath() == file1);

    fh.open(file2);
    REQUIRE(fh.getPath() == file2);

    removeTestFile(file1);
    removeTestFile(file2);
}
TEST_CASE("FileHandle: open fail")
{
    lab4::resource_core::FileHandle fh;

    REQUIRE_THROWS_AS(
        fh.open("nonexistent_12345.txt"),
        lab4::resource_core::ResourceError
    );
    REQUIRE(fh.isOpen() == false);
}
TEST_CASE("FileHandle: move constructor")
{
    const std::string test_file = "test_move.tmp";
    createTestFile(test_file);

    lab4::resource_core::FileHandle fh1(test_file);
    REQUIRE(fh1.isOpen() == true);

    lab4::resource_core::FileHandle fh2(std::move(fh1));

    REQUIRE(fh1.isOpen() == false);
    REQUIRE(fh1.getPath().empty() == true);
    REQUIRE(fh2.isOpen() == true);
    REQUIRE(fh2.getPath() == test_file);

    removeTestFile(test_file);
}
TEST_CASE("ResourceManager: caching new resource")
{
    const std::string test_file = "test_mgr_new.tmp";
    createTestFile(test_file);

    lab4::resource_core::ResourceManager manager;

    auto fh = manager.caching(test_file);

    REQUIRE(fh->isOpen() == true);
    REQUIRE(manager.sizeCache() == 1);

    removeTestFile(test_file);
}
TEST_CASE("ResourceManager: cachimg same reuses")
{
    const std::string test_file = "test_mgr_reuse.tmp";
    createTestFile(test_file);

    lab4::resource_core::ResourceManager manager;

    auto fh1 = manager.caching(test_file);
    auto fh2 = manager.caching(test_file);

    REQUIRE(fh1.get() == fh2.get());
    REQUIRE(manager.sizeCache() == 1);

    removeTestFile(test_file);
}
TEST_CASE("ResourceManager: caching 2 files")
{
    const std::string file1 = "test_mgr_a.tmp";
    const std::string file2 = "test_mgr_b.tmp";
    createTestFile(file1);
    createTestFile(file2);

    lab4::resource_core::ResourceManager manager;

    auto fh1 = manager.caching(file1);
    auto fh2 = manager.caching(file2);

    REQUIRE(fh1.get() != fh2.get());
    REQUIRE(manager.sizeCache() == 2);

    removeTestFile(file1);
    removeTestFile(file2);
}
TEST_CASE("ResourceManager: out of scope weak_ptr")
{
    const std::string test_file = "test_mgr_weak.tmp";
    createTestFile(test_file);

    lab4::resource_core::ResourceManager manager;

    {
        auto fh = manager.caching(test_file);
        REQUIRE((*fh).isOpen() == true);

    }

    REQUIRE(manager.sizeCache() == 1);

    auto fh2 = manager.caching(test_file);
    REQUIRE(fh2 != nullptr);
    REQUIRE((*fh2).isOpen() == true);

    removeTestFile(test_file);
}
TEST_CASE("ResourceManager: deleteDead ")
{
    const std::string test_file = "test_mgr_delete.tmp";
    createTestFile(test_file);

    lab4::resource_core::ResourceManager manager;

    {
        auto fh = manager.caching(test_file);
    }

    REQUIRE(manager.sizeCache() == 1);
    manager.deleteDead();
    REQUIRE(manager.sizeCache() == 0);

    removeTestFile(test_file);
}
TEST_CASE("ResourceManager: clearCache")
{
    const std::string test_file1 = "test_mgr_clear1.tmp";
    const std::string test_file2 = "test_mgr_clear2.tmp";
    createTestFile(test_file1);
    createTestFile(test_file2);
    lab4::resource_core::ResourceManager manager;

    manager.caching(test_file1);
    manager.caching(test_file2);
    REQUIRE(manager.sizeCache() == 2);

    manager.clearCache();
    REQUIRE(manager.sizeCache() == 0);

    removeTestFile(test_file1);
    removeTestFile(test_file2);
}
