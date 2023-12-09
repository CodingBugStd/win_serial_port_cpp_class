#include "CSerialPort/SerialPortWinBase.h"
#include "CSerialPort/SerialPortListener.h"
#include "CSerialPort/iutils.hpp"
#include "CSerialPort/ithread.hpp"
#include "CSerialPort/itimer.hpp"

#ifdef UNICODE
static wchar_t *CharToWChar(wchar_t *dest, const char *str)
{
    if (NULL == str)
    {
        return NULL;
    }

    int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0); // get char length
    MultiByteToWideChar(CP_ACP, 0, str, -1, dest, len);         // CP_UTF8

    return dest;
}
#endif

CSerialPortWinBase::CSerialPortWinBase()
    : m_baudRate(BaudRate9600)
    , m_parity(ParityNone)
    , m_dataBits(DataBits8)
    , m_stopbits(StopOne)
    , m_flowControl(FlowNone)
    , m_readBufferSize(4096)
    , m_handle(INVALID_HANDLE_VALUE)
    , m_monitorThread(INVALID_HANDLE_VALUE)
    , overlapMonitor()
    , m_overlapRead()
    , m_overlapWrite()
    , m_comConfigure()
    , m_comTimeout()
    , m_communicationMutex()
    , m_isThreadRunning(false)
    , p_buffer(new RingBuffer<char>(m_readBufferSize))
{
    IUtils::strncpy(m_portName, "", 1);

    overlapMonitor.Internal = 0;
    overlapMonitor.InternalHigh = 0;
    overlapMonitor.Offset = 0;
    overlapMonitor.OffsetHigh = 0;
    overlapMonitor.hEvent = CreateEvent(NULL, true, false, NULL);
}

CSerialPortWinBase::CSerialPortWinBase(const char *portName)
    : m_baudRate(BaudRate9600)
    , m_parity(ParityNone)
    , m_dataBits(DataBits8)
    , m_stopbits(StopOne)
    , m_flowControl(FlowNone)
    , m_readBufferSize(4096)
    , m_handle(INVALID_HANDLE_VALUE)
    , m_monitorThread(INVALID_HANDLE_VALUE)
    , overlapMonitor()
    , m_overlapRead()
    , m_overlapWrite()
    , m_comConfigure()
    , m_comTimeout()
    , m_communicationMutex()
    , m_isThreadRunning(false)
    , p_buffer(new RingBuffer<char>(m_readBufferSize))
{
    IUtils::strncpy(m_portName, portName, 256);

    overlapMonitor.Internal = 0;
    overlapMonitor.InternalHigh = 0;
    overlapMonitor.Offset = 0;
    overlapMonitor.OffsetHigh = 0;
    overlapMonitor.hEvent = CreateEvent(NULL, true, false, NULL);
}

CSerialPortWinBase::~CSerialPortWinBase()
{
    CloseHandle(overlapMonitor.hEvent);

    if (p_buffer)
    {
        delete p_buffer;
        p_buffer = NULL;
    }
}

void CSerialPortWinBase::init(const char *portName,
                              int baudRate /*= BaudRate::BaudRate9600*/,
                              Parity parity /*= Parity::ParityNone*/,
                              DataBits dataBits /*= DataBits::DataBits8*/,
                              StopBits stopbits /*= StopBits::StopOne*/,
                              FlowControl flowControl /*= FlowControl::FlowNone*/,
                              unsigned int readBufferSize /*= 4096*/)
{
    IUtils::strncpy(m_portName, portName, 256);
    m_baudRate = baudRate;
    m_parity = parity;
    m_dataBits = dataBits;
    m_stopbits = stopbits;
    m_flowControl = flowControl;
    m_readBufferSize = readBufferSize;

    if (p_buffer)
    {
        delete p_buffer;
        p_buffer = NULL;
    }
    p_buffer = new RingBuffer<char>(m_readBufferSize);
}

bool CSerialPortWinBase::openPort()
{
    IAutoLock lock(p_mutex);

    LOG_INFO("portName: %s, baudRate: %d, dataBit: %d, parity: %d, stopBit: %d, flowControl: %d, mode: %s, readBufferSize:%u(%u), readIntervalTimeoutMS: %u, minByteReadNotify: %u",
             m_portName, m_baudRate, m_dataBits, m_parity, m_stopbits, m_flowControl, m_operateMode == AsynchronousOperate ? "async" : "sync", m_readBufferSize,
             p_buffer->getBufferSize(), m_readIntervalTimeoutMS, m_minByteReadNotify);

    bool bRet = false;

    TCHAR *tcPortName = NULL;
    char portName[256] = "\\\\.\\";                    // support COM10 above \\\\.\\COM10
    IUtils::strncat(portName, m_portName, 6); // COM254

#ifdef UNICODE
    wchar_t wstr[256];
    tcPortName = CharToWChar(wstr, portName);
#else
    tcPortName = portName;
#endif
    unsigned long configSize = sizeof(COMMCONFIG);
    m_comConfigure.dwSize = configSize;

    DWORD dwFlagsAndAttributes = 0;
    if (m_operateMode == /*OperateMode::*/ AsynchronousOperate)
    {
        dwFlagsAndAttributes += FILE_FLAG_OVERLAPPED;
    }

    if (!isOpen())
    {
        // get a handle to the port
        m_handle = CreateFile(tcPortName,                   // communication port string (COMX)
                              GENERIC_READ | GENERIC_WRITE, // read/write types
                              0,                            // comm devices must be opened with exclusive access
                              NULL,                         // no security attributes
                              OPEN_EXISTING,                // comm devices must use OPEN_EXISTING
                              dwFlagsAndAttributes,         // Async I/O or sync I/O
                              NULL);

        if (m_handle != INVALID_HANDLE_VALUE)
        {
            // get default parameter
            GetCommConfig(m_handle, &m_comConfigure, &configSize);
            GetCommState(m_handle, &(m_comConfigure.dcb));

            // set parameter
            m_comConfigure.dcb.BaudRate = m_baudRate;
            m_comConfigure.dcb.ByteSize = m_dataBits;
            m_comConfigure.dcb.Parity = m_parity;
            m_comConfigure.dcb.StopBits = m_stopbits;
            // m_comConfigure.dcb.fDtrControl;
            // m_comConfigure.dcb.fRtsControl;

            m_comConfigure.dcb.fBinary = true;
            m_comConfigure.dcb.fInX = false;
            m_comConfigure.dcb.fOutX = false;
            m_comConfigure.dcb.fAbortOnError = false;
            m_comConfigure.dcb.fNull = false;

            // setBaudRate(m_baudRate);
            // setDataBits(m_dataBits);
            // setStopBits(m_stopbits);
            // setParity(m_parity);

            setFlowControl(m_flowControl); // @todo

            if (SetCommConfig(m_handle, &m_comConfigure, configSize))
            {
                // @todo
                // Discards all characters from the output or input buffer of a specified communications resource. It
                // can also terminate pending read or write operations on the resource.
                PurgeComm(m_handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);

                // init event driven approach
                if (m_operateMode == /*OperateMode::*/ AsynchronousOperate)
                {
                    m_comTimeout.ReadIntervalTimeout = MAXDWORD;
                    m_comTimeout.ReadTotalTimeoutMultiplier = 0;
                    m_comTimeout.ReadTotalTimeoutConstant = 0;
                    m_comTimeout.WriteTotalTimeoutMultiplier = 0;
                    m_comTimeout.WriteTotalTimeoutConstant = 0;
                    SetCommTimeouts(m_handle, &m_comTimeout);

                    // set comm event
                    // only need receive event
                    if (SetCommMask(m_handle, EV_RXCHAR))
                    {
                        m_isThreadRunning = true;
                        bRet = startThreadMonitor();

                        if (!bRet)
                        {
                            m_isThreadRunning = false;
                            m_lastError = /*SerialPortError::*/ ErrorInner;
                        }
                    }
                    else
                    {
                        // Failed to set Comm Mask
                        bRet = false;
                        m_lastError = /*SerialPortError::*/ ErrorInvalidParam;
                    }
                }
                else
                {
                    m_comTimeout.ReadIntervalTimeout = MAXDWORD;
                    m_comTimeout.ReadTotalTimeoutMultiplier = 0;
                    m_comTimeout.ReadTotalTimeoutConstant = 0;
                    m_comTimeout.WriteTotalTimeoutMultiplier = 100;
                    m_comTimeout.WriteTotalTimeoutConstant = 500;
                    SetCommTimeouts(m_handle, &m_comTimeout);

                    bRet = true;
                }
            }
            else
            {
                // set com configure error
                bRet = false;
                m_lastError = /*SerialPortError::*/ ErrorInvalidParam;
            }
        }
        else
        {
            // 串口打开失败，增加提示信息
            switch (GetLastError())
            {
                // 串口不存在
                case ERROR_FILE_NOT_FOUND:
                {
                    m_lastError = /*SerialPortError::*/ ErrorNotExist;

                    break;
                }
                    // 串口拒绝访问
                case ERROR_ACCESS_DENIED:
                {
                    m_lastError = /*SerialPortError::*/ ErrorAccessDenied;

                    break;
                }
                default:
                    m_lastError = /*SerialPortError::*/ ErrorOpenFailed;
                    break;
            }
        }
    }
    else
    {
        bRet = false;
        m_lastError = /*SerialPortError::*/ ErrorOpenFailed;
    }

    if (!bRet)
    {
        closePort();
    }

    LOG_INFO("open %s. code: %d, message: %s", m_portName, getLastError(), getLastErrorMsg());

    return bRet;
}

void CSerialPortWinBase::closePort()
{
    // Finished
    if (isOpen())
    {
        stopThreadMonitor();

        if (m_handle != INVALID_HANDLE_VALUE)
        {
            // stop all event
            SetCommMask(m_handle, 0); // SetCommMask(m_handle,0) stop WaitCommEvent()

            // Discards all characters from the output or input buffer of a specified communications resource. It can
            // also terminate pending read or write operations on the resource.
            PurgeComm(m_handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);

            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
        }

        ResetEvent(overlapMonitor.hEvent);
    }
}

unsigned int __stdcall CSerialPortWinBase::commThreadMonitor(LPVOID pParam)
{
    // Cast the void pointer passed to the thread back to
    // a pointer of CSerialPortWinBase class
    CSerialPortWinBase *p_base = (CSerialPortWinBase *)pParam;

    int iRet = 0;

    DWORD dwError = 0;
    COMSTAT comstat;

    if (p_base)
    {
        DWORD eventMask = 0;

        HANDLE m_mainHandle = p_base->getMainHandle();
        OVERLAPPED m_overlapMonitor = p_base->getOverlapMonitor();

        for (; p_base->isThreadRunning();)
        {
            eventMask = 0;
            // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-waitcommevent
            if (!WaitCommEvent(m_mainHandle, &eventMask, &m_overlapMonitor))
            {
                if (ERROR_IO_PENDING == GetLastError())
                {
                    // method 1
                    WaitForSingleObject(m_overlapMonitor.hEvent, INFINITE);

                    // method 2
                    // DWORD numBytes;
                    // GetOverlappedResult(m_mainHandle, &m_overlapMonitor, &numBytes, TRUE);
                }
            }

            if (eventMask & EV_RXCHAR)
            {
                // solve 线程中循环的低效率问题
                ClearCommError(m_mainHandle, &dwError, &comstat);
                if (comstat.cbInQue >= p_base->getMinByteReadNotify()) // 设定字符数,默认为1
                {
                    char *data = NULL;
                    data = new char[comstat.cbInQue];
                    if (data)
                    {
                        if (p_base->p_buffer)
                        {
                            int len = p_base->readDataWin(data, comstat.cbInQue);
                            p_base->p_buffer->write(data, len);
#ifdef CSERIALPORT_DEBUG
                            char hexStr[201]; // 100*2 + 1
                            LOG_INFO("write buffer(usedLen %u). len: %d, hex(top100): %s", p_base->p_buffer->getUsedLen(), len,
                                     IUtils::charToHexStr(hexStr, data, len > 100 ? 100 : len));
#endif

                            if (p_base->p_readEvent)
                            {
                                unsigned int readIntervalTimeoutMS = p_base->getReadIntervalTimeout();
                                if (readIntervalTimeoutMS > 0)
                                {
                                    if (p_base->p_timer)
                                    {
                                        if (p_base->p_timer->isRunning())
                                        {
                                            p_base->p_timer->stop();
                                        }

                                        LOG_INFO("onReadEvent. portName: %s, readLen: %u", p_base->getPortName(), p_base->p_buffer->getUsedLen());
                                        p_base->p_timer->startOnce(readIntervalTimeoutMS, p_base->p_readEvent, &CSerialPortListener::onReadEvent, p_base->getPortName(),
                                                                   p_base->p_buffer->getUsedLen());
                                    }
                                }
                                else
                                {
                                    LOG_INFO("onReadEvent. portName: %s, readLen: %u", p_base->getPortName(), p_base->p_buffer->getUsedLen());
                                    p_base->p_readEvent->onReadEvent(p_base->getPortName(), p_base->p_buffer->getUsedLen());
                                }
                            }
                        }

                        delete[] data;
                        data = NULL;
                    }
                }
            }
        }
    }
    else
    {
        // point null
        iRet = 0;
    }

    return iRet;
}

bool CSerialPortWinBase::isOpen()
{
    // Finished
    return m_handle != INVALID_HANDLE_VALUE;
}

unsigned int CSerialPortWinBase::getReadBufferUsedLen() const
{
    unsigned int usedLen = 0;

    if (m_operateMode == /*OperateMode::*/ AsynchronousOperate)
    {
        usedLen = p_buffer->getUsedLen();
    }
    else
    {
        DWORD dwError = 0;
        COMSTAT comstat;
        ClearCommError(m_handle, &dwError, &comstat);
        usedLen = comstat.cbInQue;
    }

    LOG_INFO("getReadBufferUsedLen: %u", usedLen);

    return usedLen;
}

int CSerialPortWinBase::readDataWin(void *data, int size)
{
    IAutoLock lock(p_mutex);

    DWORD numBytes = 0;

    if (isOpen())
    {
        if (m_operateMode == /*OperateMode::*/ AsynchronousOperate)
        {
            m_overlapRead.Internal = 0;
            m_overlapRead.InternalHigh = 0;
            m_overlapRead.Offset = 0;
            m_overlapRead.OffsetHigh = 0;
            m_overlapRead.hEvent = CreateEvent(NULL, true, false, NULL);

            if (!ReadFile(m_handle, (void *)data, (DWORD)size, &numBytes, &m_overlapRead))
            {
                if (ERROR_IO_PENDING == GetLastError())
                {
                    GetOverlappedResult(m_handle, &m_overlapRead, &numBytes, true);
                }
                else
                {
                    m_lastError = /*SerialPortError::*/ ErrorReadFailed;
                    numBytes = (DWORD)-1;
                }
            }

            CloseHandle(m_overlapRead.hEvent);
        }
        else
        {
            if (ReadFile(m_handle, (void *)data, (DWORD)size, &numBytes, NULL))
            {
            }
            else
            {
                m_lastError = /*SerialPortError::*/ ErrorReadFailed;
                numBytes = (DWORD)-1;
            }
        }
    }
    else
    {
        m_lastError = /*SerialPortError::*/ ErrorNotOpen;
        numBytes = (DWORD)-1;
    }

    return numBytes;
}

int CSerialPortWinBase::readData(void *data, int size)
{
    IAutoLock lock(p_mutex);

    if (size <= 0)
    {
        return 0;
    }

    DWORD numBytes = 0;

    if (isOpen())
    {
        if (m_operateMode == /*OperateMode::*/ AsynchronousOperate)
        {
            numBytes = p_buffer->read((char *)data, size);
        }
        else
        {
            if (ReadFile(m_handle, data, (DWORD)size, &numBytes, NULL))
            {
            }
            else
            {
                m_lastError = /*SerialPortError::*/ ErrorReadFailed;
                numBytes = (DWORD)-1;
            }
        }
    }
    else
    {
        m_lastError = /*SerialPortError::*/ ErrorNotOpen;
        numBytes = (DWORD)-1;
    }

#ifdef CSERIALPORT_DEBUG
    char hexStr[201]; // 100*2 + 1
    LOG_INFO("read. len: %d, hex(top100): %s", numBytes, IUtils::charToHexStr(hexStr, (const char *)data, numBytes > 100 ? 100 : numBytes));
#endif

    return numBytes;
}

int CSerialPortWinBase::readAllData(void *data)
{
    return readData(data, getReadBufferUsedLen());
}

int CSerialPortWinBase::readLineData(void *data, int size)
{
    IAutoLock lock(p_mutex);

    DWORD numBytes = 0;

    if (isOpen())
    {
        m_lastError = /*SerialPortError::*/ ErrorNotImplemented;
        numBytes = (DWORD)-1;
    }
    else
    {
        m_lastError = /*SerialPortError::*/ ErrorNotOpen;
        numBytes = (DWORD)-1;
    }

    return numBytes;
}

int CSerialPortWinBase::writeData(const void *data, int size)
{
    IAutoLock lock(p_mutex);

    DWORD numBytes = 0;

    if (isOpen())
    {
        // @todo maybe mutile thread not need this
        // Discards all characters from the output or input buffer of a specified communications resource. It can also
        // terminate pending read or write operations on the resource.
        //::PurgeComm(m_handle, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_RXABORT);

#ifdef CSERIALPORT_DEBUG
        char hexStr[201]; // 100*2 + 1
        LOG_INFO("write. len: %d, hex(top100): %s", size, IUtils::charToHexStr(hexStr, (const char *)data, size > 100 ? 100 : size));
#endif

        if (m_operateMode == /*OperateMode::*/ AsynchronousOperate)
        {
            m_overlapWrite.Internal = 0;
            m_overlapWrite.InternalHigh = 0;
            m_overlapWrite.Offset = 0;
            m_overlapWrite.OffsetHigh = 0;
            m_overlapWrite.hEvent = CreateEvent(NULL, true, false, NULL);

            if (!WriteFile(m_handle, (void *)data, (DWORD)size, &numBytes, &m_overlapWrite))
            {
                if (ERROR_IO_PENDING == GetLastError())
                {
                    GetOverlappedResult(m_handle, &m_overlapWrite, &numBytes, true);
                }
                else
                {
                    m_lastError = /*SerialPortError::*/ ErrorWriteFailed;
                    numBytes = (DWORD)-1;
                }
            }

            CloseHandle(m_overlapWrite.hEvent);
        }
        else
        {
            if (WriteFile(m_handle, (void *)data, (DWORD)size, &numBytes, NULL))
            {
            }
            else
            {
                m_lastError = /*SerialPortError::*/ ErrorWriteFailed;
                numBytes = (DWORD)-1;
            }
        }
    }
    else
    {
        m_lastError = /*SerialPortError::*/ ErrorNotOpen;
        numBytes = (DWORD)-1;
    }

    return numBytes;
}

void CSerialPortWinBase::setDebugModel(bool isDebug)
{
    //@todo
}

void CSerialPortWinBase::setReadIntervalTimeout(unsigned int msecs)
{
    m_readIntervalTimeoutMS = msecs;
}

void CSerialPortWinBase::setMinByteReadNotify(unsigned int minByteReadNotify)
{
    m_minByteReadNotify = minByteReadNotify;
}

bool CSerialPortWinBase::flushBuffers()
{
    IAutoLock lock(p_mutex);

    if (isOpen())
    {
        return TRUE == FlushFileBuffers(m_handle);
    }
    else
    {
        return false;
    }
}

bool CSerialPortWinBase::flushReadBuffers()
{
    IAutoLock lock(p_mutex);

    if (isOpen())
    {
        return TRUE == PurgeComm(m_handle, PURGE_RXCLEAR);
    }
    else
    {
        return false;
    }
}

bool CSerialPortWinBase::flushWriteBuffers()
{
    IAutoLock lock(p_mutex);

    if (isOpen())
    {
        return TRUE == PurgeComm(m_handle, PURGE_TXCLEAR);
    }
    else
    {
        return false;
    }
}

void CSerialPortWinBase::setPortName(const char *portName)
{
    // Windows : COM1
    // Linux : /dev/ttyS0
    IUtils::strncpy(m_portName, portName, 256);
}

const char *CSerialPortWinBase::getPortName() const
{
    return m_portName;
}

void CSerialPortWinBase::setBaudRate(int baudRate)
{
    IAutoLock lock(p_mutex);
    m_baudRate = baudRate;
    m_comConfigure.dcb.BaudRate = m_baudRate;
    SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
}

int CSerialPortWinBase::getBaudRate() const
{
    return m_baudRate;
}

void CSerialPortWinBase::setParity(Parity parity)
{
    IAutoLock lock(p_mutex);
    m_parity = parity;

    if (isOpen())
    {
        m_comConfigure.dcb.Parity = (unsigned char)parity;
        switch (parity)
        {
            case /*Parity::*/ ParityNone:
                m_comConfigure.dcb.fParity = FALSE;
                break;
            case /*Parity::*/ ParityOdd:
                m_comConfigure.dcb.fParity = TRUE;
                break;
            case /*Parity::*/ ParityEven:
                m_comConfigure.dcb.fParity = TRUE;
                break;
            case /*Parity::*/ ParitySpace:
                if (m_dataBits == /*DataBits::*/ DataBits8)
                {
                    // Space parity with 8 data bits is not supported by POSIX systems
                }
                m_comConfigure.dcb.fParity = TRUE;
                break;
            case /*Parity::*/ ParityMark:
                // Mark parity is not supported by POSIX systems
                m_comConfigure.dcb.fParity = TRUE;
                break;
        }
        SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
    }
}

Parity CSerialPortWinBase::getParity() const
{
    return m_parity;
}

void CSerialPortWinBase::setDataBits(DataBits dataBits)
{
    IAutoLock lock(p_mutex);
    m_dataBits = dataBits;

    if (isOpen())
    {
        switch (dataBits)
        {
            case /*DataBits::*/ DataBits5: // 5 data bits
                if (m_stopbits == /*StopBits::*/ StopTwo)
                {
                    // 5 Data bits cannot be used with 2 stop bits
                }
                else
                {
                    m_comConfigure.dcb.ByteSize = 5;
                    SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                }
                break;
            case /*DataBits::*/ DataBits6: // 6 data bits
                if (m_stopbits == /*StopBits::*/ StopOneAndHalf)
                {
                    // 6 Data bits cannot be used with 1.5 stop bits
                }
                else
                {
                    m_comConfigure.dcb.ByteSize = 6;
                    SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                }
                break;
            case /*DataBits::*/ DataBits7: // 7 data bits
                if (m_stopbits == /*StopBits::*/ StopOneAndHalf)
                {
                    // 7 Data bits cannot be used with 1.5 stop bits
                }
                else
                {
                    m_comConfigure.dcb.ByteSize = 7;
                    SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                }
                break;
            case /*DataBits::*/ DataBits8: // 8 data bits
                if (m_stopbits == /*StopBits::*/ StopOneAndHalf)
                {
                    // 8 Data bits cannot be used with 1.5 stop bits
                }
                else
                {
                    m_comConfigure.dcb.ByteSize = 8;
                    SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                }
                break;
        }
    }
}

DataBits CSerialPortWinBase::getDataBits() const
{
    return m_dataBits;
}

void CSerialPortWinBase::setStopBits(StopBits stopbits)
{
    IAutoLock lock(p_mutex);
    m_stopbits = stopbits;

    if (isOpen())
    {
        switch (m_stopbits)
        {
            case /*StopBits::*/ StopOne: // 1 stop bit
                m_comConfigure.dcb.StopBits = ONESTOPBIT;
                SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                break;
            case /*StopBits::*/ StopOneAndHalf: // 1.5 stop bit - This is only for the Windows platform
                if (m_dataBits == /*DataBits::*/ DataBits5)
                {
                    //	1.5 stop bits can only be used with 5 data bits
                }
                else
                {
                    m_comConfigure.dcb.StopBits = ONE5STOPBITS;
                    SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                }
                break;

                /*two stop bits*/
            case /*StopBits::*/ StopTwo: // 2 stop bit
                if (m_dataBits == /*DataBits::*/ DataBits5)
                {
                    // 2 stop bits cannot be used with 5 data bits
                }
                else
                {
                    m_comConfigure.dcb.StopBits = TWOSTOPBITS;
                    SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                }
                break;
        }
    }
}

StopBits CSerialPortWinBase::getStopBits() const
{
    return m_stopbits;
}

void CSerialPortWinBase::setFlowControl(FlowControl flowControl)
{
    IAutoLock lock(p_mutex);

    m_flowControl = flowControl;

    if (isOpen())
    {
        switch (m_flowControl)
        {
            case /*FlowControl::*/ FlowNone: // No flow control

                m_comConfigure.dcb.fOutxCtsFlow = FALSE;
                m_comConfigure.dcb.fRtsControl = RTS_CONTROL_DISABLE;
                m_comConfigure.dcb.fInX = FALSE;
                m_comConfigure.dcb.fOutX = FALSE;
                SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                break;

            case /*FlowControl::*/ FlowSoftware: // Software(XON / XOFF) flow control
                m_comConfigure.dcb.fOutxCtsFlow = FALSE;
                m_comConfigure.dcb.fRtsControl = RTS_CONTROL_DISABLE;
                m_comConfigure.dcb.fInX = TRUE;
                m_comConfigure.dcb.fOutX = TRUE;
                SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                break;

            case /*FlowControl::*/ FlowHardware: // Hardware(RTS / CTS) flow control
                m_comConfigure.dcb.fOutxCtsFlow = TRUE;
                m_comConfigure.dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
                m_comConfigure.dcb.fInX = FALSE;
                m_comConfigure.dcb.fOutX = FALSE;
                SetCommConfig(m_handle, &m_comConfigure, sizeof(COMMCONFIG));
                break;
        }
    }
}

FlowControl CSerialPortWinBase::getFlowControl() const
{
    return m_flowControl;
}

void CSerialPortWinBase::setReadBufferSize(unsigned int size)
{
    IAutoLock lock(p_mutex);
    if (isOpen())
    {
        m_readBufferSize = size;
    }
}

unsigned int CSerialPortWinBase::getReadBufferSize() const
{
    return m_readBufferSize;
}

void CSerialPortWinBase::setDtr(bool set /*= true*/)
{
    IAutoLock lock(p_mutex);
    if (isOpen())
    {
        EscapeCommFunction(m_handle, set ? SETDTR : CLRDTR);
    }
}

void CSerialPortWinBase::setRts(bool set /*= true*/)
{
    IAutoLock lock(p_mutex);
    if (isOpen())
    {
        EscapeCommFunction(m_handle, set ? SETRTS : CLRRTS);
    }
}

OVERLAPPED CSerialPortWinBase::getOverlapMonitor()
{
    // Finished
    return overlapMonitor;
}

HANDLE CSerialPortWinBase::getMainHandle()
{
    // Finished
    return m_handle;
}

bool CSerialPortWinBase::isThreadRunning()
{
    return m_isThreadRunning;
}

bool CSerialPortWinBase::startThreadMonitor()
{
    // Finished

    // start event thread monitor
    bool bRet = false;
    if (0 == i_thread_create(&m_monitorThread, NULL, commThreadMonitor, (LPVOID)this))
    {
        bRet = true;
    }
    else
    {
        bRet = false;
    }

    return bRet;
}

bool CSerialPortWinBase::stopThreadMonitor()
{
    // Finished

    SetCommMask(m_monitorThread, 0);
    m_isThreadRunning = false;
    //_endthreadex(0);//not recommend

    return true;
}
