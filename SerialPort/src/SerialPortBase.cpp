#include "CSerialPort/SerialPortBase.h"
#include "CSerialPort/ithread.hpp"
#include "CSerialPort/itimer.hpp"

CSerialPortBase::CSerialPortBase()
    : m_lastError(0)
    , m_operateMode(AsynchronousOperate)
    , m_readIntervalTimeoutMS(50)
    , m_minByteReadNotify(1)
    , p_mutex(NULL)
    , p_readEvent(NULL)
    , p_timer(NULL)
{
    p_mutex = new IMutex();
    p_timer = new ITimer<CSerialPortListener>();
}

CSerialPortBase::CSerialPortBase(const char *portName)
    : m_lastError(0)
    , m_operateMode(AsynchronousOperate)
    , m_readIntervalTimeoutMS(50)
    , m_minByteReadNotify(1)
    , p_mutex(NULL)
    , p_readEvent(NULL)
    , p_timer(NULL)

{
    p_mutex = new IMutex();
    p_timer = new ITimer<CSerialPortListener>();
}

CSerialPortBase::~CSerialPortBase()
{
    if (p_mutex)
    {
        delete p_mutex;
        p_mutex = NULL;
    }

    if (p_timer)
    {
        delete p_timer;
        p_timer = NULL;
    }
}

void CSerialPortBase::setOperateMode(OperateMode operateMode)
{
    m_operateMode = operateMode;
}

unsigned int CSerialPortBase::getReadIntervalTimeout()
{
    return m_readIntervalTimeoutMS;
}

unsigned int CSerialPortBase::getMinByteReadNotify()
{
    return m_minByteReadNotify;
}

int CSerialPortBase::getLastError() const
{
    return m_lastError;
}

const char *CSerialPortBase::getLastErrorMsg() const
{
    switch (m_lastError)
    {
        case ErrorUnknown:
            return "unknown error";
        case ErrorOK:
            return "success";
        case ErrorFail:
            return "general error";
        case ErrorNotImplemented:
            return "not implemented error";
        case ErrorInner:
            return "innet error";
        case ErrorNullPointer:
            return "null pointer error";
        case ErrorInvalidParam:
            return "invalid param error";
        case ErrorAccessDenied:
            return "access denied error";
        case ErrorOutOfMemory:
            return "out of memory error";
        case ErrorTimeout:
            return "timeout error";
        case ErrorNotInit:
            return "not init error";
        case ErrorInitFailed:
            return "init failed error";
        case ErrorAlreadyExist:
            return "already exist error";
        case ErrorNotExist:
            return "not exist error";
        case ErrorAlreadyOpen:
            return "already open error";
        case ErrorNotOpen:
            return "not open error";
        case ErrorOpenFailed:
            return "open failed error";
        case ErrorCloseFailed:
            return "close failed error";
        case ErrorWriteFailed:
            return "write failed error";
        case ErrorReadFailed:
            return "read failed error";
        default:
            return "undefined error code";
            break;
    }
}

void CSerialPortBase::clearError()
{
    m_lastError = ErrorOK;
}

int CSerialPortBase::connectReadEvent(CSerialPortListener *event)
{
    if (event)
    {
        p_readEvent = event;
        return ErrorOK;
    }
    else
    {
        return ErrorInvalidParam;
    }
}

int CSerialPortBase::disconnectReadEvent()
{
    p_readEvent = NULL;
    return ErrorOK;
}