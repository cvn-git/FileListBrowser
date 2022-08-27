#include "foldernode.h"

#include <QDebug>

#include <fstream>
#include <array>


namespace {

std::vector<std::string> parseFields(const std::string& line, size_t numFields)
{
    std::vector<std::string> fields(numFields);
    assert(numFields >= 2);

    size_t idx = 0;
    for (size_t fieldIdx = 0; fieldIdx < (numFields - 1); fieldIdx++)
    {
        const auto idx1 = line.find(' ', idx);
        if (idx1 == std::string::npos)
        {
            throw std::invalid_argument("Invalid file list format");
        }
        const auto idx2 = line.find_first_not_of(' ', idx1);
        if (idx2 == std::string::npos)
        {
            throw std::invalid_argument("Invalid file list format");
        }

        fields[fieldIdx] = line.substr(idx, idx1 - idx);
        idx = idx2;
        //qDebug() << QString::fromStdString(fields[fieldIdx]);
    }
    fields.back() = line.substr(idx);

    return fields;
}

}   // anonymous namesapce


void FolderNode::reset()
{
    subDirectories_.clear();
    files_.clear();
}

FolderNode* FolderNode::addSubDirectory(const std::string& name)
{
    if (files_.find(name) != files_.end())
    {
        throw std::invalid_argument("FolderNode::addSubDirectory(): file already exists");
    }

    auto it = subDirectories_.find(name);
    if (it == subDirectories_.end())
    {
        auto child = new FolderNode(this, name);
        auto ret = subDirectories_.emplace(name, child);
        it = ret.first;
    }
    return it->second;
}

FolderNode* FolderNode::getSubDirectory(const std::string& name, bool autoAdd)
{
    auto it = subDirectories_.find(name);
    if (it == subDirectories_.end())
    {
        if (autoAdd)
        {
            return addSubDirectory(name);
        }
        else
        {
            throw std::invalid_argument("FolderNode::getSubDirectory(): folder does not exist");
        }
    }
    else
    {
        return it->second;
    }
}

void FolderNode::addFile(const std::string& name, const FileInfo& info)
{
    if (subDirectories_.find(name) != subDirectories_.end())
    {
        throw std::invalid_argument("FolderNode::addFile(): folder already exists");
    }

    files_[name] = info;
}

void FolderNode::loadFileList(const std::string& filePath, FolderNode* rootNode)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("FolderNode::loadFileList(): Cannot open file list");
    }

    std::string line;
    std::getline(file, line);
    if (line.size() < 2)
    {
        throw std::runtime_error("FolderNode::loadFileList(): Invalid file list format");
    }

    if ((line[0] == '.') && (line[1] == ':'))
    {
        loadLinuxFileList(filePath, rootNode);
    }
    else
    {
        loadWindowFileList(filePath, rootNode);
    }
}

void FolderNode::loadWindowFileList(const std::string& filePath, FolderNode* rootNode)
{
    rootNode->reset();
    FolderNode *node = nullptr;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("FolderNode::loadWindowFileList(): Cannot open file list");
    }

    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        //qDebug() << "Read line:" << QString::fromStdString(line);
        if (line.size() < 2)
        {
            continue;
        }

        // Remove \r at the end if necessary
        if (line.back() == '\r')
        {
            line = line.substr(0, line.size() - 1);
        }

        if (node == nullptr)
        {
            // Find the folder group
            const std::string token("Directory of ");
            auto idx = line.find(token);
            if (idx == std::string::npos)
            {
                continue;
            }
            const std::string pathName = line.substr(idx + token.size());
            //qDebug() << "Processing folder:" << QString::fromStdString(pathName);

            // Parse folder path
            idx = 0;
            std::vector<std::string> names;
            while (true)
            {
                const auto newIdx = pathName.find('\\', idx);
                if (newIdx == std::string::npos)
                {
                    names.push_back(pathName.substr(idx));
                    break;
                }
                else
                {
                    names.push_back(pathName.substr(idx, newIdx - idx));
                    idx = newIdx + 1;
                }
            }
            node = rootNode;
            for (const auto& name : names)
            {
                //qDebug() << QString::fromStdString(name);
                node = node->getSubDirectory(name, true);
            }
        }
        else
        {
            if ((line[0] < '0') || (line[0] > '9'))
            {
                node = nullptr;
                continue;
            }

            auto fields = parseFields(line, 5);
            const auto& name = fields.back();

            if (fields[3] == "<DIR>")
            {
                if ((name == ".") || (name == ".."))
                {
                    continue;
                }

                node->addSubDirectory(name);
                //qDebug() << "Add folder:" << QString::fromStdString(name);
            }
            else
            {
                FileInfo info;

                // Parse date/time
                const auto dateStr =  QString::fromStdString(fields[0] + " " + fields[1] + " " + fields[2]);
                info.dateTime = QDateTime::fromString(dateStr, "dd/MM/yyyy hh:mm AP");
                //qDebug() << dateStr;
                //qDebug() << "Date time:" << info.dateTime;

                // Parse file size
                auto& s = fields[3];
                s.erase(std::remove(s.begin(), s.end(), ','), s.end());
                info.fileSize = std::stoll(s);

                node->addFile(name, info);
                //qDebug() << "Add file:" << QString::fromStdString(name) << info.fileSize << "bytes";
            }

        }
    }
}

void FolderNode::loadLinuxFileList(const std::string& filePath, FolderNode* rootNode)
{
    rootNode->reset();
    FolderNode *node = nullptr;

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("FolderNode::loadLinuxFileList(): Cannot open file list");
    }

    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        //qDebug() << "Read line:" << QString::fromStdString(line);

        // Remove \r at the end if necessary
        if ((!line.empty()) && (line.back() == '\r'))
        {
            line = line.substr(0, line.size() - 1);
        }

        if (node == nullptr)
        {
            if (line.size() < 2)
            {
                continue;
            }

            // Find the folder group
            if ((line.front() != '.') || (line.back() != ':'))
            {
                continue;
            }

            // Parse folder path
            size_t idx = 0;
            const std::string pathName = line.substr(0, line.size() - 1);
            std::vector<std::string> names;
            while (true)
            {
                const auto newIdx = pathName.find('/', idx);
                if (newIdx == std::string::npos)
                {
                    names.push_back(pathName.substr(idx));
                    break;
                }
                else
                {
                    names.push_back(pathName.substr(idx, newIdx - idx));
                    idx = newIdx + 1;
                }
            }
            node = rootNode;
            for (const auto& name : names)
            {
                //qDebug() << QString::fromStdString(name);
                node = node->getSubDirectory(name, true);
            }
        }
        else
        {
            if (line.empty())
            {
                node = nullptr;
                continue;
            }

            if ((line[0] != 'd') && (line[0] != '-'))
            {
                continue;
            }

            const auto fields = parseFields(line, 9);
            const auto& name = fields.back();

            if (fields[0].front() == 'd')
            {
                node->addSubDirectory(name);
                //qDebug() << "Add folder:" << QString::fromStdString(name);
            }
            else
            {
                FileInfo info;

                // Parse date/time
                const auto dateStr =  QString::fromStdString(fields[5] + " " + fields[6].substr(0, 8));
                info.dateTime = QDateTime::fromString(dateStr, "yyyy-MM-dd hh:mm:ss");
                //qDebug() << dateStr;
                //qDebug() << "Date time:" << info.dateTime;

                // Parse file size
                info.fileSize = std::stoll(fields[4]);

                node->addFile(name, info);
                //qDebug() << "Add file:" << QString::fromStdString(name) << info.fileSize << "bytes";
            }
        }
    }
}
