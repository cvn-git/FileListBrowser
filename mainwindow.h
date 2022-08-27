#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "foldernode.h"

#include <QMainWindow>
#include <QTreeWidget>
#include <QStatusBar>
#include <QFileIconProvider>
#include <QLineEdit>


class TreePathItem : public QTreeWidgetItem
{
public:
    TreePathItem(FolderNode* node, TreePathItem* parent)
        : QTreeWidgetItem(parent)
        , node_{node}
    {}

    TreePathItem(FolderNode* node, QTreeWidget* parent)
        : QTreeWidgetItem(parent)
        , node_{node}
    {}

    FolderNode* node() const {return node_;}

private:
    FolderNode* node_;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void openFile();
    void openPath(QTreeWidgetItem* item);
    void resetRootPath();
    TreePathItem* addTreePathItem(FolderNode* node, TreePathItem* parent = nullptr);

    QFileIconProvider icons_;
    QTreeWidget* treePath_;
    QTreeWidget* treeFileList_;
    QStatusBar* statusBar_;
    QLineEdit* editAddress_;

    FolderNode* rootNode_{nullptr};
};
#endif // MAINWINDOW_H
