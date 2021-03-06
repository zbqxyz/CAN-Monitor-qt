#-------------------------------------------------
#
# Project created by QtCreator 2018-06-09T14:02:49
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += network widgets
requires(qtConfig(udpsocket))


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CAN-Monitor-qt
TEMPLATE = app

SOURCES += main.cpp\
    MainWindow.cpp \
    Tree/TreeModel.cpp \
    Tree/TreeNode.cpp \
    CanTree/CanTreeModel.cpp \
    CanTree/HeaderTreeNode.cpp \
    CanTree/MessageTreeNode.cpp \
    CanAdapter/CanAdapter.cpp \
    CanAdapter/CanAdapterLawicel.cpp \
    CanAdapter/SlcanControlWidget.cpp \
    CanAdapter/ChinaControlWidget.cpp \
    CanAdapter/SerialPortComboBox.cpp \
    WidgetUtils/HexStringValidator.cpp \
    WidgetUtils/CustomLineEdit.cpp \
    lib-slcan/slcan.c \
    TransmitWidget.cpp \
    CanAdapter/CanAdapterLoopback.cpp \
    CanAdapter/CanAdapterFactory.cpp \
    CanAdapter/CanAdapterTesting.cpp \
    WidgetUtils/MenuOpenKeeper.cpp \
    CanAdapter/TestingControlWidget.cpp \
    Commanders/CommanderDialog.cpp \
    Tree/XmlTreeNode.cpp \
    CanTree/CanTreeNodeFactory.cpp \
    ParameterTree/ParameterTreeModel.cpp \
    ParameterTree/ParameterGroupNode.cpp \
    ParameterTree/ParameterNode.cpp \
    Commanders/ButtonEditDialog.cpp \
    ParameterTree/ParameterTreeNodeFactory.cpp \
    Commanders/CommanderPropertiesDialog.cpp \
    CanHub/CanHub.cpp \
    CanAdapter/PollingCanAdapter.cpp \
    CanAdapter/CanAdapterTritium.cpp \
    Tree/ComboBoxDelegate.cpp \
    Format/format.cpp \
    CanAdapter/TritiumControlWidget.cpp \
    Trace/TraceWindow.cpp \
    CanTable/CanTableModel.cpp \
    CanTable/CanTableNode.cpp \
    util/canMessageUtil.cpp \
    util/rangeParse.cpp \
    Simulator/TritiumSimulatorWindow.cpp \
    CanAdapter/CanAdapterChina.cpp \
    Cansole/Cansole.cpp \
    Cansole/CansoleIdDialog.cpp \
    util/DialogUtil.cpp

HEADERS  += MainWindow.h \
    Tree/TreeModel.h \
    Tree/TreeNode.h \
    CanTree/CanTreeModel.h \
    CanTree/HeaderTreeNode.h \
    CanTree/MessageTreeNode.h \
    CanAdapter/CanAdapter.h \
    CanAdapter/CanAdapterLawicel.h \
    CanAdapter/SlcanControlWidget.h \
    CanAdapter/ChinaControlWidget.h \
    CanAdapter/SerialPortComboBox.h \
    WidgetUtils/HexStringValidator.h \
    WidgetUtils/CustomLineEdit.h \
    lib-slcan/can_message.h \
    lib-slcan/slcan.h \
    TransmitWidget.h \
    CanAdapter/CanAdapterLoopback.h \
    CanAdapter/CanAdapterFactory.h \
    CanAdapter/CanAdapterTesting.h \
    WidgetUtils/MenuOpenKeeper.h \
    CanAdapter/TestingControlWidget.h \
    Commanders/CommanderDialog.h \
    Tree/XmlTreeNode.h \
    CanTree/CanTreeNode.h \
    Tree/XmlTreeNodeFactory.h \
    CanTree/CanTreeNodeFactory.h \
    ParameterTree/ParameterTreeModel.h \
    ParameterTree/ParameterGroupNode.h \
    ParameterTree/ParameterTreeNode.h \
    ParameterTree/ParameterNode.h \
    Commanders/ButtonEditDialog.h \
    ParameterTree/ParameterTreeNodeFactory.h \
    Commanders/CommanderPropertiesDialog.h \
    CanHub/CanHub.h \
    CanAdapter/PollingCanAdapter.h \
    CanAdapter/CanAdapterTritium.h \
    Tree/ComboBoxDelegate.h \
    Format/format.h \
    CanAdapter/TritiumControlWidget.h \
    Trace/TraceWindow.h \
    CanTable/CanTableModel.h \
    CanTable/CanTableNode.h \
    util/canMessageUtil.h \
    util/rangeParse.h \
    Simulator/TritiumSimulatorWindow.h \
    CanAdapter/CanAdapterChina.h \
    Cansole/Cansole.h \
    Cansole/CansoleIdDialog.h \
    util/DialogUtil.h



FORMS    += MainWindow.ui \
    CanAdapter/SlcanControlWidget.ui \
    CanAdapter/ChinaControlWidget.ui \
    TransmitWidget.ui \
    CanAdapter/TestingControlWidget.ui \
    Commanders/CommanderDialog.ui \
    Commanders/ButtonEditDialog.ui \
    Commanders/CommanderPropertiesDialog.ui \
    CanAdapter/TritiumControlWidget.ui \
    Trace/TraceWindow.ui \
    Simulator/TritiumSimulatorWindow.ui \
    Cansole/CansoleIdDialog.ui

RC_ICONS = canMonitorIcon.ico
