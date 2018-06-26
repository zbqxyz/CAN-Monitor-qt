#include "CommanderDialog.h"
#include "ui_CommanderDialog.h"
#include "ParameterTree/ParameterTreeModel.h"
#include "ParameterTree/ParameterNode.h"
#include "ButtonEditDialog.h"
#include <QMenu>
#include <QFileDialog>
#include <QSettings>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QDebug>



CommanderDialog::CommanderDialog(QWidget *parent, QString name) :
    QMainWindow(parent),
    ui(new Ui::CommanderDialog)
{
    ui->setupUi(this);
    m_model = new ParameterTreeModel();
    ui->treeView->setModel(m_model);

    if(name != "")
        m_name = name;
    else
        m_name = "New Commander";

    load();

    setObjectName(m_name);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onTreeViewContextMenu(const QPoint &)));

    ui->customButtonGroupbox->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customButtonGroupbox, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onButtonContextMenu(const QPoint &)));

}

CommanderDialog::~CommanderDialog()
{
    delete ui;
}

void CommanderDialog::onTreeViewContextMenu(const QPoint &point)
{

    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    m_TreeMenuContext.clickedIndex = ui->treeView->indexAt(point);

    QMenu contextMenu(this);
    contextMenu.addAction(ui->actionAddParameter);
    if(!selectedIndexes.empty())
        contextMenu.addAction(ui->actionDelete);

    contextMenu.exec(ui->treeView->mapToGlobal(point));
}

int CommanderDialog::getIndexOfButton(QWidget *w)
{

    for(int i=0; i< m_commanderButtons.length(); i++)
    {
        if(m_commanderButtons.at(i).button == w)
            return i;
    }
    return -1;
}

void CommanderDialog::on_actionAddParameter_triggered()
{
    m_model->addNode(m_TreeMenuContext.clickedIndex, new ParameterNode("New Parameter"));
}

void CommanderDialog::on_actionDelete_triggered()
{

}

void CommanderDialog::onButtonContextMenu(const QPoint &point)
{

    QWidget * clickedItem = ui->customButtonGroupbox->childAt(point);
    int index = getIndexOfButton(clickedItem);
    m_buttonMenuContext.clickedIndex = index;

    QMenu contextMenu(this);
    contextMenu.addAction(ui->actionAddButton);
    if(index >= 0) {
        contextMenu.addAction(ui->actionEditButton);
        contextMenu.addAction(ui->actionDeleteButton);
        if(index >= 1)contextMenu.addAction(ui->actionMoveButtonUp);
        if(index < m_commanderButtons.length()-1)contextMenu.addAction(ui->actionMoveButtonDown);
    }

    contextMenu.exec(ui->customButtonGroupbox->mapToGlobal(point));
}

void CommanderDialog::insertButton(int index, CommanderButtonData d)
{
    if(index < 0)
        index = m_commanderButtons.count();
    CommanderButton b;
    b.d = d;
    b.button = new QPushButton(b.d.text);
    m_commanderButtons.insert(index, b);
    ui->customButtonVerticalLayout->insertWidget(index, b.button);
}

void CommanderDialog::on_actionAddButton_triggered()
{
    auto dialog = new ButtonEditDialog();
    int res = dialog->exec();
    if(res == QDialog::Accepted){
        insertButton(m_buttonMenuContext.clickedIndex, dialog->dialogData);
    }
}

void CommanderDialog::deleteButton(int index)
{
    if(index < 0)
        return;
    auto b = m_commanderButtons.at(index);
    ui->customButtonVerticalLayout->removeWidget(b.button);
    delete b.button;
    m_commanderButtons.removeAt(index);
}

void CommanderDialog::on_actionDeleteButton_triggered()
{
    deleteButton(m_buttonMenuContext.clickedIndex);
}

void CommanderDialog::on_actionEditButton_triggered()
{
    auto dialog = new ButtonEditDialog();
    dialog->dialogData = m_commanderButtons.at(m_buttonMenuContext.clickedIndex).d;
    int res = dialog->exec();
    if(res == QDialog::Accepted){
        CommanderButton * b = &m_commanderButtons[m_buttonMenuContext.clickedIndex];
        b->d= dialog->dialogData;
        b->button->setText(b->d.text);
    }
}

void CommanderDialog::on_actionMoveButtonUp_triggered()
{
    int index = m_buttonMenuContext.clickedIndex;
    auto b = m_commanderButtons.at(index);
    deleteButton(index);
    insertButton(index-1, b.d);
}

void CommanderDialog::on_actionMoveButtonDown_triggered()
{
    int index = m_buttonMenuContext.clickedIndex;
    auto b = m_commanderButtons.at(index);
    deleteButton(index);
    insertButton(index+1, b.d);
}

void CommanderDialog::saveButtons(QXmlStreamWriter &writer)
{
    writer.writeStartElement("commanderButtons");
    foreach(auto b, m_commanderButtons)
    {
        writer.writeStartElement("Button");
        b.d.writeToXml(writer);
        writer.writeEndElement();
    }
    writer.writeEndElement();
}

void CommanderDialog::loadButtons(QXmlStreamReader &reader)
{
    if (!reader.readNextStartElement())
        return;
    if (reader.name() != "commanderButtons")
        return;
    while(reader.readNextStartElement())
    {
        if(reader.name() == "Button")
        {
            CommanderButtonData d;
            d.readFromXml(reader);
            insertButton(-1, d);
        }
    }
}

void CommanderDialog::load()
{
    QString path = QSettings().value("commanders/path").toString();
    path += "/" + m_name + ".xml";

    QFile file(path);
    if(file.open(QIODevice::ReadOnly)){
        QXmlStreamReader reader(&file);
        if(reader.readNextStartElement() && reader.name() == "Commander")
        {
            m_model->readTreeFromXml(reader);
            loadButtons(reader);
        }
        file.close();
    }
}

bool CommanderDialog::saveInteractive()
{
    QString path = QSettings().value("commanders/path").toString();
    path += "/" + m_name + ".xml";

    QString filename = QFileDialog::getSaveFileName(this,
                                           tr("Save Tree"), path,
                                           tr("Xml files (*.xml)"));

    if(filename == "")
        return false;

    QFile file(filename);
    if(file.open(QIODevice::WriteOnly))
    {
        QXmlStreamWriter writer(&file);
        writer.setAutoFormatting(true);
        writer.writeStartDocument();
        writer.writeStartElement("Commander");
        m_model->writeTreeToXml(writer);
        saveButtons(writer);
        writer.writeEndElement();
        writer.writeEndDocument();
        file.close();
        m_model->isUserModified = false;
        return true;
    }else{
        QMessageBox::warning(0, tr("CAN Monitor"),
                             tr("The file\"") + filename + tr("\"could not be opened.\n") +
                             tr("The error message was: ") + file.errorString(),
                             QMessageBox::Ok);
        return false;
    }

}

void CommanderDialog::on_actionSaveAs_triggered()
{
    saveInteractive();
}


