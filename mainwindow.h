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
    TreePathItem(const FolderNode& node, TreePathItem* parent)
        : QTreeWidgetItem(parent)
        , node_{node}
    {}

    TreePathItem(const FolderNode& node, QTreeWidget* parent)
        : QTreeWidgetItem(parent)
        , node_{node}
    {}

    const FolderNode& node() const {return node_;}

private:
    const FolderNode& node_;
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
    TreePathItem* addTreePathItem(const FolderNode& node, TreePathItem* parent = nullptr);

    QFileIconProvider icons_;
    QTreeWidget* treePath_;
    QTreeWidget* treeFileList_;
    QStatusBar* statusBar_;
    QLineEdit* editAddress_;

    FolderNode rootNode_;
};
#endif // MAINWINDOW_H
