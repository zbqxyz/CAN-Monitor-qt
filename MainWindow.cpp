#include <QSerialPortInfo>
#include <QFileDialog>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CanTree/TreeModel.h"
#include "CanTree/HeaderTreeNode.h"
#include "CanAdapter/CanAdapterLawicel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_model = new CanTreeModel();
    ui->treeView->setModel(m_model);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    ui->treeView->setColumnWidth(0,150);
    ui->treeView->setColumnWidth(5,200);
    ui->treeView->setColumnWidth(6,160);


    m_canAdapter = new CanAdapterLawicel();
    auto w = m_canAdapter->getControlWidget(ui->centralWidget);
    ui->verticalLayout->insertWidget(0,w);


    connect(&m_tickTimer, SIGNAL(timeout()), this, SLOT(tickTimerTimeout()));
    m_tickTimer.setInterval(20);
    m_tickTimer.start();

    connect(ui->transmitWidget, SIGNAL(onTransmit(can_message_t)), this, SLOT(onTransmit(can_message_t)));
}


void MainWindow::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = ui->treeView->indexAt(point);
    m_contextMenuContext.index = index;

    QMenu contextMenu(this);
    contextMenu.addAction(ui->actionAdd_Group);
    if(index.isValid())
        contextMenu.addAction(ui->actionDelete_Node);

    contextMenu.exec(ui->treeView->mapToGlobal(point));
}

MainWindow::~MainWindow()
{
    if(m_canAdapter)
        delete m_canAdapter;
    delete ui;
    if(m_model)
        delete m_model;
}

void MainWindow::on_actionAdd_Group_triggered()
{
    m_model->insertNode(m_contextMenuContext.index, -1, new HeaderTreeNode("New Group"));
}

void MainWindow::on_actionDelete_Node_triggered()
{
    m_model->deleteNode(m_contextMenuContext.index);
}

void MainWindow::tickTimerTimeout()
{
    can_message_t cmsg;
    while(m_canAdapter->receive(&cmsg)){
        m_model->inputMessage(&cmsg);
    }
}

void MainWindow::onTransmit(can_message_t cmsg)
{
    m_canAdapter->transmit(&cmsg);
}

void MainWindow::on_actionSave_Tree_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                           tr("Save Tree"), ".",
                                           tr("Xml files (*.xml)"));
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    m_model->writeTreeToXml(xmlWriter);
    file.close();
}

void MainWindow::on_actionLoad_Tree_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                           tr("Load Tree"), ".",
                                           tr("Xml files (*.xml)"));
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QXmlStreamReader xmlReader(&file);
    m_model->readTreeFromXml(xmlReader);
    file.close();
}
