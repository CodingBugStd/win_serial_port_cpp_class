#include <iostream>

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPort_version.h"
#include "CSerialPort/iutils.hpp"
#include "CSerialPort/itimer.hpp"

#ifdef I_OS_WIN
#include "CSerialPort/SerialPortWinBase.h"
#define CSERIALPORTBASE CSerialPortWinBase
#elif defined I_OS_UNIX
#include "CSerialPort/SerialPortUnixBase.h"
#define CSERIALPORTBASE CSerialPortUnixBase
#else
// Not support
#define CSERIALPORTBASE
#endif // I_OS_WIN

CSerialPort::CSerialPort()
    : p_serialPortBase(NULL)
{
    p_serialPortBase = new CSERIALPORTBASE();

    p_serialPortBase->setReadIntervalTimeout(0);
    p_serialPortBase->setMinByteReadNotify(1);
}

CSerialPort::CSerialPort(const char *portName)
    : p_serialPortBase(NULL)
{
    p_serialPortBase = new CSERIALPORTBASE(portName);

    p_serialPortBase->setReadIntervalTimeout(0);
    p_serialPortBase->setMinByteReadNotify(1);
}

CSerialPort::~CSerialPort()
{
    if (p_serialPortBase)
    {
        delete p_serialPortBase;
        p_serialPortBase = NULL;
    }
}

void CSerialPort::init(const char *portName,
                                int baudRate /*= BaudRate::BaudRate9600*/,
                                Parity parity /*= Parity::ParityNone*/,
                                DataBits dataBits /*= DataBits::DataBits8*/,
                                StopBits stopbits /*= StopBits::StopOne*/,
                                FlowControl flowControl /*= FlowControl::FlowNone*/,
                                unsigned int readBufferSize /*= 4096*/)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->init(portName, baudRate, parity, dataBits, stopbits, flowControl, readBufferSize);
    }
}

void CSerialPort::setOperateMode(OperateMode operateMode)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setOperateMode(operateMode);
    }
}

bool CSerialPort::open()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->openPort();
    }
    else
    {
        return false;
    }
}

void CSerialPort::close()
{
    if (p_serialPortBase)
    {
        p_serialPortBase->closePort();
    }
}

bool CSerialPort::isOpen()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->isOpen();
    }
    else
    {
        return false;
    }
}

int CSerialPort::connectReadEvent(CSerialPortListener *event)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->connectReadEvent(event);
    }
    else
    {
        return ErrorNullPointer;
    }
}

int CSerialPort::disconnectReadEvent()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->disconnectReadEvent();
    }
    else
    {
        return ErrorNullPointer;
    }
}

unsigned int CSerialPort::getReadBufferUsedLen() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getReadBufferUsedLen();
    }
    else
    {
        return ErrorNullPointer;
    }
}

int CSerialPort::readData(void *data, int size)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->readData(data, size);
    }
    else
    {
        return ErrorNullPointer;
    }
}

int CSerialPort::readAllData(void *data)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->readAllData(data);
    }
    else
    {
        return ErrorNullPointer;
    }
}

int CSerialPort::readLineData(void *data, int size)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->readLineData(data, size);
    }
    else
    {
        return ErrorNullPointer;
    }
}

int CSerialPort::writeData(const void *data, int size)
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->writeData(data, size);
    }
    else
    {
        return ErrorNullPointer;
    }
}

void CSerialPort::setDebugModel(bool isDebug)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setDebugModel(isDebug);
    }
}

void CSerialPort::setReadIntervalTimeout(unsigned int msecs)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setReadIntervalTimeout(msecs);
    }
}

bool CSerialPort::flushBuffers()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->flushBuffers();
    }
    else
    {
        return false;
    }
}

bool CSerialPort::flushReadBuffers()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->flushReadBuffers();
    }
    else
    {
        return false;
    }
}

bool CSerialPort::flushWriteBuffers()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->flushWriteBuffers();
    }
    else
    {
        return false;
    }
}
unsigned int CSerialPort::getReadIntervalTimeout()
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getReadIntervalTimeout();
    }
    else
    {
        return 0;
    }
}

void CSerialPort::setMinByteReadNotify(unsigned int minByteReadNotify)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setMinByteReadNotify(minByteReadNotify);
    }
}

int CSerialPort::getLastError() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getLastError();
    }
    else
    {
        // null error
        return /*SerialPortError::*/ ErrorNullPointer;
    }
}

const char *CSerialPort::getLastErrorMsg() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getLastErrorMsg();
    }
    else
    {
        return "";
    }
}

void CSerialPort::clearError()
{
    if (p_serialPortBase)
    {
        p_serialPortBase->clearError();
    }
}

void CSerialPort::setPortName(const char *portName)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setPortName(portName);
    }
}

const char *CSerialPort::getPortName() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getPortName();
    }
    else
    {
        return "";
    }
}

void CSerialPort::setBaudRate(int baudRate)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setBaudRate(baudRate);
    }
}

int CSerialPort::getBaudRate() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getBaudRate();
    }
    else
    {
        return ErrorNullPointer;
    }
}

void CSerialPort::setParity(Parity parity)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setParity(parity);
    }
}

Parity CSerialPort::getParity() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getParity();
    }
    else
    {
        // should retrun error
        return /*Parity::*/ ParityNone;
    }
}

void CSerialPort::setDataBits(DataBits dataBits)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setDataBits(dataBits);
    }
}

DataBits CSerialPort::getDataBits() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getDataBits();
    }
    else
    {
        // should retrun error
        return /*DataBits::*/ DataBits8;
    }
}

void CSerialPort::setStopBits(StopBits stopbits)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setStopBits(stopbits);
    }
}

StopBits CSerialPort::getStopBits() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getStopBits();
    }
    else
    {
        // should retrun error
        return /*StopBits::*/ StopOne;
    }
}

void CSerialPort::setFlowControl(FlowControl flowControl)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setFlowControl(flowControl);
    }
}

FlowControl CSerialPort::getFlowControl() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getFlowControl();
    }
    else
    {
        // should retrun error
        return /*FlowControl::*/ FlowNone;
    }
}

void CSerialPort::setReadBufferSize(unsigned int size)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setReadBufferSize(size);
    }
}

unsigned int CSerialPort::getReadBufferSize() const
{
    if (p_serialPortBase)
    {
        return p_serialPortBase->getReadBufferSize();
    }
    else
    {
        return ErrorNullPointer;
    }
}

void CSerialPort::setDtr(bool set /*= true*/)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setDtr(set);
    }
}

void CSerialPort::setRts(bool set /*= true*/)
{
    if (p_serialPortBase)
    {
        p_serialPortBase->setRts(set);
    }
}

const char *CSerialPort::getVersion()
{
    static char version[256];
    IUtils::strncpy(version, "https://github.com/itas109/CSerialPort - V", 256);
    return IUtils::strncat(version, CSERIALPORT_VERSION, 20);
}
