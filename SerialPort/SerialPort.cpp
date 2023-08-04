#include "SerialPort.h"
#include <tchar.h>
#include <iostream>
#include "SerialPortHelper.h"

std::vector<SerialPort::SerialPortInfo>    SerialPort::_serialPortInfoList;

SerialPort::SerialPort()
{
    _refreshSerialPortInfoList();
    if( _serialPortInfoList.size() == 0 ){
        _serialInfo.portName = "----";
        _serialInfo.driverName = "----";
    }else{
        _serialInfo = *_serialPortInfoList.begin();
    }

    memset( _receiveBuf , 0 , 2048 );
    _receiveLen = 0;
    _receiveBufLock = new std::mutex;
    _receiveBufLock->unlock();

    _eventHandler = NULL;
    _eventHandlerUserCtx = NULL;

    hSerial = INVALID_HANDLE_VALUE;
    dcbSerialParams = {0};
    _recieveThread = NULL;
    // _callbackThread = NULL;
}

// void SerialPort::_callbackThreadImpl()
// {
//     while(1)
//     {
//         _evtList.size() != 0;
//         _eventHandler(*_evtList.begin());
//         // _evtList.
//     }
// }

void SerialPort::_recieveThreadImpl()
{
    SerialPortEvent _evt;
    char readBuffer[256];
    // uint16_t readBufferUse = 0;
    // int forceCopyFlag = 0;  //多次尝试上锁未成功，直接阻塞式上锁标志位
    // int tryUnlockCnt = 0;

    _evt.instance = this;
    _evt.user_ctx = _eventHandlerUserCtx;

    while(1)
    {
        DWORD bytesRead;
        if (!ReadFile(hSerial, readBuffer , sizeof(readBuffer) , &bytesRead, NULL)) {
            std::cerr << "Error in ReadFile." << std::endl;
            CloseHandle(hSerial);
            hSerial = INVALID_HANDLE_VALUE;

            //异步回调
            if( _eventHandler != NULL )
            {
                _evt.code = DISCONNET;
                // _evtList.push_back( _evt );
                _eventHandler( _evt );
            }

            return;
        }

        //载入异步接收缓冲区
        if( bytesRead > 0 )
        {
                _receiveBufLock->lock();

                if( _receiveLen + bytesRead  < sizeof( _receiveBuf ) )
                {
                    memcpy( _receiveBuf + _receiveLen , readBuffer , bytesRead );
                    _receiveLen += bytesRead;
                }

                _receiveBufLock->unlock();
                
                //异步回调
                if( _eventHandler != NULL )
                {
                    _evt.code = RECEIVE;
                    // _evtList.push_back( _evt );
                    _eventHandler( _evt );
                }
        }

    }
}

SerialPort::~SerialPort()
{
    
}

void SerialPort::_refreshSerialPortInfoList()
{
    HKEY hKey;
    SerialPortInfo  info;
    LONG res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hKey);
    if (res != ERROR_SUCCESS) {
        _serialPortInfoList.clear();
        return;
    }

    DWORD maxValueNameSize, maxValueDataSize;
    res = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &maxValueNameSize, &maxValueDataSize, NULL, NULL);
    if (res != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        _serialPortInfoList.clear();
        return;
    }

    DWORD numValues;
    res = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &numValues, NULL, NULL, NULL, NULL);
    if (res != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        _serialPortInfoList.clear();
        return;
    }

    TCHAR valueName[256];
    BYTE valueData[256];
    DWORD valueNameSize, valueDataSize;

    for (DWORD i = 0; i < numValues; i++) {
        valueNameSize = sizeof(valueName);
        valueDataSize = sizeof(valueData);
        res = RegEnumValue(hKey, i, valueName, &valueNameSize, NULL, NULL, valueData, &valueDataSize);
        if (res == ERROR_SUCCESS) {
            valueData[valueDataSize] = '\0';
            std::wstring portName(reinterpret_cast<wchar_t*>(valueData));
            info.portName = (char*)portName.data();//Wstring2String(portName);
            _serialPortInfoList.push_back(info);
        }
    }

    RegCloseKey(hKey);
}

bool SerialPort::connect( std::string portName , uint32_t bound , uint8_t dataBit, uint8_t stopBit )
{
    _serialInfo.portName = portName;
    _serialCfg.bound = bound;
    _serialCfg.dataBit = dataBit;
    _serialCfg.stopBit = stopBit;
    return _connect();
}

bool SerialPort::connect( std::string portName , SerialConnectCfg& cfg)
{
    _serialInfo.portName = portName;
    _serialCfg = cfg;
    return _connect();
}

bool SerialPort::connect( int portNumber , uint32_t bound , uint8_t dataBit , uint8_t stopBit )
{
    char buf[32] = "COM";
    sprintf( buf+3 , "%d" , portNumber );
    _serialInfo.portName = buf;
    _serialCfg.bound = bound;
    _serialCfg.dataBit = dataBit;
    _serialCfg.stopBit = stopBit;
    return _connect();
}

bool SerialPort::connect( int portNumber , SerialConnectCfg& cfg)
{
    char buf[32] = "COM";
    sprintf( buf+3 , "%d" , portNumber );
    _serialInfo.portName = buf;
    _serialCfg = cfg;
    return _connect();
}

bool SerialPort::connect( SerialPort::SerialPortInfo& info , uint32_t bound , uint8_t dataBit , uint8_t stopBit)
{
    _serialInfo = info;
    _serialCfg.bound = bound;
    _serialCfg.dataBit = dataBit;
    _serialCfg.stopBit = stopBit;
    return _connect();
}

bool SerialPort::connect( SerialPortInfo& info , SerialConnectCfg& cfg )
{
    _serialInfo = info;
    _serialCfg = cfg;
    return _connect();
}

bool SerialPort::write(uint8_t*dat , size_t size)
{
    if( !isConnected() )
    {   
        return false;
    }
    DWORD bytesWritten;
    if (!WriteFile(hSerial, dat, size , &bytesWritten, NULL)) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
        return false;
    }
    return true;
}
    
bool SerialPort::_connect()
{
    hSerial = CreateFile( (LPCSTR)_serialInfo.portName.c_str() , GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open serial port." << std::endl;
        return false;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error in GetCommState." << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    dcbSerialParams.BaudRate = _serialCfg.bound;  // 设置波特率
    dcbSerialParams.ByteSize = _serialCfg.dataBit;         // 数据位
    dcbSerialParams.StopBits = _serialCfg.stopBit - 1 ; //见 ONESTOPBIT 宏
    dcbSerialParams.Parity = NOPARITY;    // 无奇偶校验

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error in SetCommState." << std::endl;
        CloseHandle(hSerial);
        hSerial == INVALID_HANDLE_VALUE;
        return false;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 5;             // 读取超时时间（以毫秒为单位） -> 单个字符接收完毕后的超时时间
    timeouts.ReadTotalTimeoutConstant = 5;        // 读取总超时时间（以毫秒为单位）
    timeouts.ReadTotalTimeoutMultiplier = 1;      // 读取总超时时间乘数
    timeouts.WriteTotalTimeoutConstant = 5;    // 写入总超时时间（以毫秒为单位）
    timeouts.WriteTotalTimeoutMultiplier = 1;     // 写入总超时时间乘数

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Failed to set serial port timeouts." << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    //建立异步接收、回调函数线程
    _recieveThread = new std::thread(&SerialPort::_recieveThreadImpl , this );

    return true;
}

size_t SerialPort::receiveLen()
{
    return _receiveLen;
}

int SerialPort::read(uint8_t*buf , size_t size)
{
    if( !isConnected() )
    {
        return -1;
    }
    int cnt = size > _receiveLen ? _receiveLen : size ;
    _receiveBufLock->lock();
    memcpy( buf , _receiveBuf , cnt );
    memmove( _receiveBuf , _receiveBuf+cnt , _receiveLen - cnt );
    _receiveLen -= cnt;
    _receiveBufLock->unlock();

    return cnt;
}

SerialPort::SerialConnectCfg SerialPort::connectedSerialPortCfg()
{
    return _serialCfg;
}

bool SerialPort::disconnect()
{
    return false;
}

bool SerialPort::isConnected()
{
    return hSerial == INVALID_HANDLE_VALUE ? false : true ;
}

SerialPort::SerialPortInfo SerialPort::connectedSerialPortInfo()
{
    return _serialInfo;
}

std::vector<SerialPort::SerialPortInfo> SerialPort::getSerialPortList()
{
    _refreshSerialPortInfoList();
    return _serialPortInfoList;
}

