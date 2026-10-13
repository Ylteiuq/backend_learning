#include <cstdio>
#include <stdexcept>
#include <string>
#include <iostream>
#include <utility>

class FileWriter
{
public:
    explicit FileWriter(const char *path);
    FileWriter(FileWriter &&other) noexcept;
    ~FileWriter() noexcept;

    FileWriter(const FileWriter &) = delete;
    FileWriter &operator=(const FileWriter &) = delete;
    FileWriter &operator=(FileWriter&& other) noexcept;

    void write(const std::string &text);
    void write(const char* text);
    bool isOpen() const noexcept;

private:
    std::FILE *file_{nullptr};
};

FileWriter::FileWriter(const char *path)
{
    file_ = std::fopen(path, "w");
    if (!isOpen())
    {
        throw std::runtime_error("failed to open file");
    }
    std::puts("Open file successfully\n");
}

FileWriter::FileWriter(FileWriter &&other) noexcept
    : file_(other.file_)
{
    other.file_ = nullptr;
}

FileWriter& FileWriter::operator=(FileWriter&& other) noexcept
{
    if(this == &other)
    {
        return *this;
    }
    if(file_ != nullptr)
    {
        std::fclose(file_);
    }
    file_ = other.file_;
    other.file_ = nullptr;
    return *this;
}

FileWriter::~FileWriter() noexcept
{
    if (file_)
    {
        if (!std::fclose(file_))
        {
            std::puts("Close file successfully\n");
        }
    }
}

void FileWriter::write(const std::string &text)
{
    if (!isOpen())
    {
        throw std::logic_error("file is not open");
    }
    const auto status = std::fputs(text.c_str(), file_);
    if (status == EOF)
    {
        throw std::runtime_error("fputs ending");
    }
}

void FileWriter::write(const char* text)
{
    if(!isOpen())
    {
        throw std::logic_error("file is not open");
    }
    const auto status = std::fputs(text, file_);
    if(status == EOF)
    {
        throw std::runtime_error("fputs ending");
    }
}

bool FileWriter::isOpen() const noexcept
{
    return file_ != nullptr;
}

void normalCase()
{
    const char *path = "build/raii_normal.txt";
    FileWriter fileWriter(path);

    const std::string input1 = "start\n";
    const std::string input2 = "finish\n";

    fileWriter.write(input1);
    fileWriter.write(input2);
}

void earlyReturnCase()
{
    const char *path = "build/raii_early.txt";
    FileWriter filewriter(path);

    const std::string input = "start\n";

    filewriter.write(input);
    return;
}

void exceptionCase()
{
    const char *path = "build/raii_exception.txt";
    FileWriter fileWriter(path);

    const std::string input = "start\n";

    fileWriter.write(input);

    throw std::runtime_error("simulated failure");
}

void movecase()
{
    const char *path = "build/raii_move.txt";
    FileWriter filewriter(path);
    const std::string input = "start\n";
    filewriter.write(input);

    {
        FileWriter filewritercopy(std::move(filewriter));
        std::cout << filewritercopy.isOpen() << filewriter.isOpen();
        filewritercopy.write(input);
    }
    try
    {
        filewriter.write(input);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void moveAssignmentCase()
{
    const char* sourcepath = "build/raii_assign_source.txt";
    FileWriter source(sourcepath);
    source.write("source start\n");
    const char* targetpath = "build/raii_assign_target.txt";
    FileWriter target(targetpath);
    target.write("target start\n");
    std::cout << source.isOpen() << '\n' << target.isOpen();
    target.write("after assignment\n");
}

int main()
{
    normalCase();
    earlyReturnCase();
    try
    {
        exceptionCase();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    movecase();
    moveAssignmentCase();
}
