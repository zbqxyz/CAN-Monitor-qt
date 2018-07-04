#include "CanAdapterTritium.h"

#include "lib-slcan/slcan.h"
#include "SlcanControlWidget.h"
#include <QDebug>
#include <QCoreApplication>
#include <QTime>
#include "TritiumControlWidget.h"

// Implements the Tritium Ethernet to CAN bridge used for their
// (Solar Car) racing products, like e.g. the WS22 motor inverter
// Protocoll:
// https://tritium.com.au/includes/TRI82.007v4_Ethernet_Interface.pdf


CanAdapterTritium::CanAdapterTritium(CanHub &canHub)
    : m_groupAddress(QStringLiteral("239.255.60.60")), m_port(4876)
{
    m_canHandle = canHub.getNewHandle(CanHub::f_isCanAdapter);
    generateClientIdentifier();

    m_statusTimer.setSingleShot(true);
    connect(&m_statusTimer, SIGNAL(timeout()), this, SLOT(statusTimerTimeout()));
    m_statusTimer.start(2000);

    connect(m_canHandle, SIGNAL(received(can_message_t)), this, SLOT(transmit(can_message_t)));

    m_udpSocket.bind(QHostAddress::AnyIPv4, m_port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    m_udpSocket.joinMulticastGroup(m_groupAddress);
    connect(&m_udpSocket, SIGNAL(readyRead()), this, SLOT(processDatagrams()));
}

CanAdapterTritium::~CanAdapterTritium(){
    close();
    delete m_canHandle;
}

void CanAdapterTritium::generateClientIdentifier()
{
    qsrand(QTime::currentTime().msec());
    for(int i=0; i<7; i++)
        m_clientIdentifier[i] = qrand();
}

struct TritiumHeader
{
    uint8_t busNumber;
    char clientIdentifier[7];
};

struct TritiumMessage
{
    uint32_t id;
    bool H;
    bool S;
    bool R;
    bool E;
    uint8_t dlc;
    uint8_t data[8];
};

static bool decodeHeader(TritiumHeader &h, const char * data)
{
    if(memcmp(&data[1], "Tritiu", 6)) // check magic (first 6 bytes)
        return false;
    if((data[7] & 0xF0) != 0x60) // check magic (last nibble)
        return false;

    h.busNumber = data[7] & 0x0F;
    memcpy(h.clientIdentifier, &data[9], 7);

    return true;
}

static void encodeHeader(char * data, const char * clientIdentifier, uint8_t busNumber)
{
    memcpy(data, "\x00Tritiu\x60", 8);
    data[7] |= busNumber & 0x0f;
    data[8] = 0;
    memcpy(&data[9], clientIdentifier, 7);
}

static void decodeMessage(TritiumMessage &m, const char * data)
{
    m.id  = data[0] << 24;
    m.id |= data[1] << 16;
    m.id |= data[2] <<  8;
    m.id |= data[3] <<  0;
    m.H = data[4] & 0x80 ? 1:0;
    m.S = data[4] & 0x40 ? 1:0;
    m.R = data[4] & 0x02 ? 1:0;
    m.E = data[4] & 0x01 ? 1:0;
    m.dlc = data[5];
    memcpy(m.data, &data[6], 8);
}

static void encodeCanMessage(char * data, const can_message_t & cmsg)
{
    data[0] = cmsg.id >> 24;
    data[1] = cmsg.id >> 16;
    data[2] = cmsg.id >>  8;
    data[3] = cmsg.id >>  0;
    data[4]  = cmsg.IDE ? 0x01:0x00;
    data[4] |= cmsg.RTR ? 0x02:0x00;
    data[5] = cmsg.dlc;
    memcpy(&data[6], cmsg.data, cmsg.dlc);
}

void CanAdapterTritium::statusTimerTimeout()
{
    emit(updateStatus("Receiving no CAN Bridge on network"));
}

void CanAdapterTritium::processMessage(const TritiumMessage &message, const TritiumHeader &header, const QHostAddress &sourceAddress)
{
    if(message.H) // heartbeat
    {
        int baud = message.data[0] << 8;
        baud |= message.data[1];

        emit(updateStatus("CAN Bridge at " + sourceAddress.toString() +
                          "  Bus=" + QString().number(header.busNumber) +
                          "  Baud=" + QString().number(baud) + "k" ));
        m_statusTimer.start(2000);
    }
    else if(message.S) // setting
    {
    }
    else // CAN message
    {
        if(m_isOpen)
        {
            can_message_t cmsg;
            cmsg.IDE = message.E;
            cmsg.RTR = message.R;
            cmsg.id = message.id;
            cmsg.dlc = message.dlc;
            memcpy(cmsg.data, message.data, 8);
            m_canHandle->transmit(cmsg);
        }
    }
}

void CanAdapterTritium::processDatagrams()
{
    QByteArray data;

    while (m_udpSocket.hasPendingDatagrams()) {
        data.resize(int(m_udpSocket.pendingDatagramSize()));
        QHostAddress sourceAddress;
        m_udpSocket.readDatagram(data.data(), data.size(), &sourceAddress);
        // qDebug() << data.toHex();
        if(data.length() < 30) // minimum datagram length
            continue;

        const char * d = data.data();
        TritiumHeader header;
        if(!decodeHeader(header, d))
            continue;

        // don't feed on our own milk
        if(memcmp(header.clientIdentifier, m_clientIdentifier, sizeof(m_clientIdentifier)) == 0)
            continue;

        int offset = 16;
        int length = data.length() - 16;
        while (length >= 14) {
            TritiumMessage message;
            decodeMessage(message, &d[offset]);
            processMessage(message, header, sourceAddress);

            offset += 14;
            length -= 14;
        }
    }
}

void CanAdapterTritium::transmit(can_message_t cmsg)
{
    if(m_isOpen)
    {
        QByteArray data(30, 0);
        encodeHeader(data.data(), m_clientIdentifier, 13);
        encodeCanMessage(data.data()+16, cmsg);
        m_udpSocket.writeDatagram(data.data(), data.size(), m_groupAddress, m_port);
    }
}

QWidget * CanAdapterTritium::getControlWidget(QWidget *parent){
    auto w = new TritiumControlWidget(parent);
    connect(w, SIGNAL(openClicked()), this, SLOT(openClicked()));
    connect(w, SIGNAL(closeClicked()), this, SLOT(closeClicked()));
    connect(this, SIGNAL(updateStatus(QString)), w, SLOT(displayStatus(QString)));
    return w;
}

bool CanAdapterTritium::open()
{
    m_isOpen = true;
    return true;
}

void CanAdapterTritium::close()
{
    m_isOpen = false;
}

bool CanAdapterTritium::isOpen()
{
    return m_isOpen;
}

void CanAdapterTritium::openClicked(){
    open();
}

void CanAdapterTritium::closeClicked(){
    close();
}