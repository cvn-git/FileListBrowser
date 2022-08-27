#ifndef FOLDER_NODE_H
#define FOLDER_NODE_H

#include <QDateTime>
#include <QObject>

#include <map>
#include <unordered_map>
#include <string>


struct FileInfo
{
    QDateTime   dateTime;
    size_t      fileSize{0};
};


class FolderNode : public QObject
{
public:
    FolderNode(QObject* parent = nullptr, const std::string& name = {})
        : QObject(parent)
        , name_{name}
    {}

    void reset();
    FolderNode* addSubDirectory(const std::string& name);
    FolderNode* getSubDirectory(const std::string& name, bool autoAdd = false);
    void addFile(const std::string& name, const FileInfo& info);

    const std::map<std::string, FolderNode*>& getSubDirectories() const {return subDirectories_;}
    const std::unordered_map<std::string, FileInfo>& getFiles() const {return files_;}
    const std::string& name() const {return name_;}

    static void loadFileList(const std::string& filePath, FolderNode* rootNode);
    static void loadWindowFileList(const std::string& filePath, FolderNode* rootNode);
    static void loadLinuxFileList(const std::string& filePath, FolderNode* rootNode);

private:
    const std::string name_;
    std::map<std::string, FolderNode*> subDirectories_;
    std::unordered_map<std::string, FileInfo> files_;
};

#endif  // FOLDER_NODE_H
