#include "mainwindow.h"

#include <QMenuBar>
#include <QDebug>
#include <QVBoxLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QToolBar>
#include <QPushButton>
#include <QSettings>
#include <QApplication>

#include <functional>
#include <filesystem>


namespace {
class WaitCursor
{
public:
    WaitCursor()
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }

    ~WaitCursor()
    {
        QApplication::restoreOverrideCursor();
    }
};

}   // anonymous namespace


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(icons_.icon(QFileIconProvider::Folder));

    // Menu
    auto action = new QAction("Open", this);
    menuBar()->addMenu("&File")->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::openFile);

    // Status bar
    auto toolbar = new QToolBar();
    addToolBar(Qt::BottomToolBarArea, toolbar);
    statusBar_ = new QStatusBar();
    toolbar->addWidget(statusBar_);

    // Main widget
    auto widgetMain = new QWidget();
    setCentralWidget(widgetMain);
    auto layoutMain = new QVBoxLayout(widgetMain);

    editAddress_ = new QLineEdit();
    editAddress_->setReadOnly(true);
    layoutMain->addWidget(editAddress_);

    // Tree views
    auto splitter = new QSplitter();
    layoutMain->addWidget(splitter);

    treePath_ = new QTreeWidget();
    treePath_->setColumnCount(1);
    treePath_->setHeaderHidden(true);
    connect(treePath_, &QTreeWidget::currentItemChanged, this, &MainWindow::openPath);
    splitter->addWidget(treePath_);

    treeFileList_ = new QTreeWidget();
    treeFileList_->setColumnCount(4);
    treeFileList_->setHeaderLabels(QStringList{tr("Name"), tr("Type"), tr("Size"), tr("Date modified")});
    treeFileList_->setSortingEnabled(true);
    treeFileList_->sortItems(0, Qt::SortOrder::AscendingOrder);
    splitter->addWidget(treeFileList_);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
}

MainWindow::~MainWindow()
{
}

void MainWindow::openFile()
{
    auto settings = QSettings();
    const auto dir = settings.value("default_dir", QString()).toString();
    auto filePath = QFileDialog::getOpenFileName(this, tr("Select file list"), dir);
    if (filePath.isEmpty())
    {
        return;
    }
    qDebug() << "To open file:" << filePath;

    settings.setValue("default_dir", QFileInfo(filePath).dir().absolutePath());

    try
    {
        WaitCursor waitCursor;
        statusBar_->showMessage(tr("Reading file list from ") + filePath);
        statusBar_->repaint();

        FolderNode::loadWindowFileList(filePath.toStdString(), rootNode_);

        resetRootPath();
    }
    catch (std::exception& e)
    {
        qDebug() << "EXCEPTION:" << e.what();
    }
}

void MainWindow::openPath(QTreeWidgetItem* item)
{
    // Clear old contents
    treeFileList_->clear();

    // Process address
    auto& node = dynamic_cast<TreePathItem&>(*item).node();

    // List files
    QList<QTreeWidgetItem *> items;
    for (const auto& pair : node.getFiles())
    {
        const auto& info = pair.second;

        auto item = new QTreeWidgetItem();
        item->setText(0, QString::fromStdString(pair.first));
        item->setText(1, QString::fromStdString(std::filesystem::path(pair.first).extension().string()));
        item->setData(2, Qt::DisplayRole, info.fileSize);
        item->setData(3, Qt::DisplayRole, info.dateTime);
        item->setIcon(0, icons_.icon(QFileIconProvider::File));
        items.append(item);
    }
    treeFileList_->insertTopLevelItems(0, items);

    // Display
    std::string pathName;
    auto nodePtr = &node;
    while (nodePtr->parent() != nullptr)
    {
        pathName = nodePtr->name() + "/" + pathName;
        nodePtr = nodePtr->parent();
    }
    editAddress_->setText(QString::fromStdString(pathName));

    statusBar_->showMessage(QString::number(items.size()) + " item(s)");
}

void MainWindow::resetRootPath()
{
    // Clear old contents
    treePath_->clear();
    treeFileList_->clear();

    // Find starting node
    const FolderNode* node = &rootNode_;
    while ((node->getSubDirectories().size() == 1) && (node->getFiles().empty()))
    {
        auto it = node->getSubDirectories().begin();
        node = &it->second;
    }

    // Add starting node
    auto item = addTreePathItem(*node);

    treePath_->setCurrentItem(item);
}

TreePathItem* MainWindow::addTreePathItem(const FolderNode& node, TreePathItem* parent)
{
    TreePathItem* item = nullptr;

    // Create this item
    if (parent == nullptr)
    {
        item = new TreePathItem(node, treePath_);
    }
    else
    {
        item = new TreePathItem(node, parent);
        parent->addChild(item);
    }
    item->setText(0, QString::fromStdString(node.name()));
    item->setIcon(0, icons_.icon(QFileIconProvider::Folder));

    // Create child items
    for (const auto& p : node.getSubDirectories())
    {
        addTreePathItem(p.second, item);
    }

    return item;
}
