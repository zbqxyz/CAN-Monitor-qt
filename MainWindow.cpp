#include <QSerialPortInfo>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSortFilterProxyModel>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CanTree/CanTreeModel.h"
#include "CanTree/HeaderTreeNode.h"
#include "CanAdapter/CanAdapter.h"
#include "CanAdapter/CanAdapterFactory.h"
#include "WidgetUtils/MenuOpenKeeper.h"
#include "Commanders/CommanderDialog.h"
#include <QDirIterator>
#include <QDebug>
#include "Trace/TraceWindow.h"
#include "Simulator/TritiumSimulatorWindow.h"
#include "Cansole/Cansole.h"
#include "Cansole/cansoleIdDialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    MenuOpenKeeper * menuOpenKepper = new MenuOpenKeeper(this);
    ui->menuEnable_Columns->installEventFilter(menuOpenKepper);

    m_model = new CanTreeModel();
    if(useProxyModel)
    {
        m_proxyModel = new QSortFilterProxyModel(this);
        m_proxyModel->setSourceModel(m_model);
        m_proxyModel->setSortRole(Qt::UserRole);
        m_proxyModel->setDynamicSortFilter(false);
        ui->treeView->setModel(m_proxyModel);
    }else{
        ui->treeView->setModel(m_model);
    }

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    ui->treeView->setColumnWidth(0,150);
    ui->treeView->setColumnWidth(2,50);
    ui->treeView->setColumnWidth(3,70);
    ui->treeView->setColumnWidth(4,70);
    ui->treeView->setColumnWidth(5,200);
    ui->treeView->setColumnWidth(6,220);
    ui->treeView->header()->setSectionsClickable(true);
    connect(ui->treeView->header(), SIGNAL(sectionClicked(int)), this, SLOT(headerSectionClicked(int)));

    QString adapterName = QSettings().value("main/CanAdapter").toString();
    ui->canAdapterComboBox->addItems(CanAdapterFactory::getAdapterNames());
    ui->canAdapterComboBox->setCurrentText(adapterName);

    QSettings settings;
    QString path = settings.value("main/lastTreeFile").toString();
    if(path != "")
        loadTree(path);

    m_guiCanHandle = m_canHub.getNewHandle();

    connect(ui->transmitWidget, SIGNAL(onTransmit(can_message_t)), this, SLOT(canTransmit(can_message_t)));
    connect(m_guiCanHandle, SIGNAL(received(can_message_t)), m_model, SLOT(inputMessage(can_message_t)));

    populateCommanders();
}

MainWindow::~MainWindow()
{
    //delete m_commanderDialog;
    delete m_canAdapter;
    delete ui;
    delete m_model;
}

void MainWindow::onCustomContextMenu(const QPoint &point)
{

    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    m_contextMenuContext.clickedIndex = ui->treeView->indexAt(point);

    QMenu contextMenu(this);
    contextMenu.addAction(ui->actionAdd_Group);
    if(!selectedIndexes.empty())
        contextMenu.addAction(ui->actionDeleteTreeNodes);

    contextMenu.exec(ui->treeView->mapToGlobal(point));
}


void MainWindow::on_actionAdd_Group_triggered()
{
    QModelIndex index;
    if(m_proxyModel)
        index = m_proxyModel->mapToSource(m_contextMenuContext.clickedIndex);
    else
        index = m_contextMenuContext.clickedIndex;
    m_model->addNode(index, new HeaderTreeNode("New Group"));
}

void MainWindow::on_actionDeleteTreeNodes_triggered()
{
    const QModelIndexList &originalList = ui->treeView->selectionModel()->selectedIndexes();
    QModelIndexList list;
    if(m_proxyModel){
        foreach (auto index, originalList)
            list.append(m_proxyModel->mapToSource(index));
    }
    else
        list = originalList;

    m_model->deleteBranches(list);
}

void MainWindow::headerSectionClicked(int index)
{
    ui->treeView->header()->setSortIndicatorShown(true);
    if(m_lastSortIndex != index)
    {
        m_nextSortMode = sortAscending;
        m_lastSortIndex = index;
    }

    switch(m_nextSortMode)
    {
    case sortNone:
        ui->treeView->sortByColumn(-1);
        m_nextSortMode = sortAscending;
        break;
    case sortAscending:
        ui->treeView->sortByColumn(index, Qt::AscendingOrder);
        m_nextSortMode = sortDescending;
        break;
    case sortDescending:
        ui->treeView->sortByColumn(index, Qt::DescendingOrder);
        m_nextSortMode = sortNone;
        break;
    }
}

void MainWindow::canTransmit(can_message_t cmsg)
{
    m_guiCanHandle->transmit(cmsg);

}

bool MainWindow::saveTreeInteractive()
{
    QString path = QSettings().value("main/lastTreeFile").toString();
    QString filename = QFileDialog::getSaveFileName(this,
                                           tr("Save Tree"), path,
                                           tr("Xml files (*.xml)"));

    if(filename == "")
        return false;

    QFile file(filename);
    if(file.open(QIODevice::WriteOnly))
    {
        QXmlStreamWriter xmlWriter(&file);
        xmlWriter.setAutoFormatting(true);
        m_model->writeTreeToXml(xmlWriter);
        file.close();
        m_model->isUserModified = false;
        QSettings().setValue("main/lastTreeFile", filename);
        return true;
    }else{
        QMessageBox::warning(0, tr("CAN Monitor"),
                             tr("The file\"") + filename + tr("\"could not be opened.\n") +
                             tr("The error message was: ") + file.errorString(),
                             QMessageBox::Ok);
        return false;
    }
}

void MainWindow::on_actionSave_Tree_triggered()
{
    saveTreeInteractive();
}

void MainWindow::loadTree(QString &filename)
{
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly)){
        QXmlStreamReader xmlReader(&file);
        m_model->readTreeFromXml(xmlReader);
        file.close();
        QSettings().setValue("main/lastTreeFile", filename);
    }else{
        QMessageBox::warning(0, tr("CAN Monitor"),
                             tr("The file\"") + filename + tr("\"could not be opened.\n") +
                             tr("The error message was: ") + file.errorString(),
                             QMessageBox::Ok);
    }
}

void MainWindow::on_actionLoad_Tree_triggered()
{
    QString path = QSettings().value("main/lastTreeFile").toString();
    QString filename = QFileDialog::getOpenFileName(this,
                                           tr("Load Tree"), path,
                                           tr("Xml files (*.xml)"));
    if(filename == "")
        return;

    loadTree(filename);
}

void MainWindow::closeEvent(QCloseEvent *event)
 {
    foreach (auto commander, m_openCommanders) {
        commander->closeEvent(event);
        if(!event->isAccepted())
            return;
    }

    if(!m_model->isUserModified){
        event->accept();
        return;
    }

    QMessageBox msgBox;
    msgBox.setText("The tree has been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Save:
            if (saveTreeInteractive())
                event->accept();
            else
                event->ignore();
            break;
        case QMessageBox::Discard:
            event->accept();
            break;
        case QMessageBox::Cancel:
            event->ignore();
            break;
    }
}

void  MainWindow::changeCanAdpapter(CanAdapter * ca)
{
    QBoxLayout * layout = ui->horizontalLayout;
    int layoutIndex = 2;

    if(m_canAdapter)
        delete m_canAdapter;
    if(m_canAdpterControlWidget)
    {
        layout->removeWidget(m_canAdpterControlWidget);
        delete m_canAdpterControlWidget;
    }
    m_canAdapter = ca;
    auto w = m_canAdapter->getControlWidget(ui->centralWidget);
    m_canAdpterControlWidget = w;
    if(w)
        layout->insertWidget(layoutIndex,w);

}

void MainWindow::on_canAdapterComboBox_currentTextChanged(const QString &adapterName)
{
    CanAdapter * ca = CanAdapterFactory::createAdapter(adapterName, m_canHub);
    if(ca) {
        changeCanAdpapter(ca);
        QSettings().setValue("main/CanAdapter", adapterName);
    }
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(0, tr("CAN Monitor 3000"),
                         tr("Version 0.5 alpha\n\n") +
                         tr("A versatile and simple CAN bus diagnostic tool by Peter Fuhrmann"),
                         QMessageBox::Ok);
}

void MainWindow::on_actionSort_Items_Live_triggered(bool checked)
{
    if(m_proxyModel)
        m_proxyModel->setDynamicSortFilter(checked);
}

void MainWindow::on_actionEnableID_triggered(bool checked)
{ ui->treeView->setColumnHidden(1,!checked); }

void MainWindow::on_actionEnableDLC_triggered(bool checked)
{ ui->treeView->setColumnHidden(2,!checked); }

void MainWindow::on_actionEnableCount_triggered(bool checked)
{ ui->treeView->setColumnHidden(3,!checked); }

void MainWindow::on_actionEnablePeriod_triggered(bool checked)
{ ui->treeView->setColumnHidden(4,!checked); }

void MainWindow::on_actionEnableRaw_Data_triggered(bool checked)
{ ui->treeView->setColumnHidden(5,!checked); }

void MainWindow::on_actionEnableDecoded_Data_triggered(bool checked)
{ ui->treeView->setColumnHidden(6,!checked); }

void MainWindow::on_actionEnableFormat_triggered(bool checked)
{ ui->treeView->setColumnHidden(7,!checked); }

void MainWindow::populateCommanders()
{
    ui->menuCommander->clear();
    QString dir = QSettings().value("commanders/path").toString();
    if(dir != "")
    {
        QDirIterator it(dir, QStringList() << "*.xml", QDir::Files);
        while (it.hasNext()){
            it.next();
            QAction *action = new QAction(it.fileInfo().baseName(), this);
            ui->menuCommander->addAction(action);
            connect(action, SIGNAL(triggered(bool)), this, SLOT(actionCommanderTriggered()));
        }
    }
    ui->menuCommander->addSeparator();
    ui->menuCommander->addAction(ui->actionSetCommanderDirectory);
    ui->menuCommander->addAction(ui->actionNewCommander);
}

void MainWindow::commanderWindowClosed(QObject* o)
{
    auto closedCommander = static_cast<CommanderDialog*>(o);
    m_openCommanders.removeOne(closedCommander);
    populateCommanders();
}

void MainWindow::openCommander(QString name)
{
    auto dlg = new CommanderDialog(this, &m_canHub, name);
    connect(dlg, SIGNAL(destroyed(QObject*)), this, SLOT(commanderWindowClosed(QObject*)));
    m_openCommanders.append(dlg);
    dlg->show();
}

void MainWindow::actionCommanderTriggered()
{
    openCommander(static_cast<QAction*>(sender())->text());
}

void MainWindow::on_actionSetCommanderDirectory_triggered()
{
    QString dir = QSettings().value("commanders/path").toString();
    QString newDir = QFileDialog::getExistingDirectory (this, "Select Commanders Directory", dir, 0);
    if(newDir != "")
    {
        QSettings().setValue("commanders/path", newDir);
        populateCommanders();
    }
}

void MainWindow::on_actionNewCommander_triggered()
{
    openCommander("");
}

void MainWindow::on_actionTrace_triggered()
{
    auto w = new TraceWindow(this, m_canHub);
    w->show();
}

void MainWindow::on_actionTritium_Simulator_triggered()
{
    auto w = new TritiumSimulatorWindow(this, m_canHub);
    w->show();
}

void MainWindow::on_actionOpen_Cansole_triggered()
{
    auto dialog = new CansoleIdDialog();
    bool ok;
    dialog->cansoleId = QSettings().value("cansole/lastId").toString().toInt(&ok, 16);
    if(!ok) dialog->cansoleId = 0x780;
    int res = dialog->exec();
    if(res == QDialog::Accepted){
        QSettings().setValue("cansole/lastId", QString().number(dialog->cansoleId, 16));
        new Cansole(this, m_canHub, dialog->cansoleId);
    }
}
