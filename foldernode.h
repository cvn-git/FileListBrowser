#ifndef FOLDER_NODE_H
#define FOLDER_NODE_H

#include <QDateTime>

#include <map>
#include <unordered_map>
#include <string>


struct FileInfo
{
    QDateTime   dateTime;
    size_t      fileSize{0};
};


class FolderNode
{
public:
    FolderNode(const std::string& name = {}, const FolderNode* parent = nullptr)
        : name_{name}
        , parent_{parent}
    {}

    FolderNode(const FolderNode& other) = delete;
    FolderNode& operator=(const FolderNode& other) = delete;

    void reset();
    FolderNode& addSubDirectory(const std::string& name);
    FolderNode& getSubDirectory(const std::string& name, bool autoAdd = false);
    void addFile(const std::string& name, const FileInfo& info);

    const std::map<std::string, FolderNode>& getSubDirectories() const {return subDirectories_;}
    const std::unordered_map<std::string, FileInfo>& getFiles() const {return files_;}
    const std::string& name() const {return name_;}
    const FolderNode* parent() const {return parent_;}

    static void loadFileList(const std::string& filePath, FolderNode& rootNode);
    static void loadWindowFileList(const std::string& filePath, FolderNode& rootNode);
    static void loadLinuxFileList(const std::string& filePath, FolderNode& rootNode);

private:
    const std::string name_;
    const FolderNode* const parent_;
    std::map<std::string, FolderNode> subDirectories_;
    std::unordered_map<std::string, FileInfo> files_;
};

#endif  // FOLDER_NODE_H
