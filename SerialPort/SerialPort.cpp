#include "SerialPort.h"
#include <windows.h>
#include <tchar.h>

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
    _eventHandler = NULL;
    _eventHandlerUserCtx = NULL;
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
            info.portName = (char*)portName.data();
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

}
    
bool SerialPort::_connect()
{
    
}

size_t SerialPort::receiveLen()
{

}

int SerialPort::read(uint8_t*buf , size_t size)
{

}

SerialPort::SerialConnectCfg SerialPort::connectedSerialPortCfg()
{

}

bool SerialPort::disconnect()
{
    return false;
}

bool SerialPort::isConnected()
{
    return false;
}

SerialPort::SerialPortInfo SerialPort::connectedSerialPortInfo()
{
    SerialPort::SerialPortInfo  info;
}

std::vector<SerialPort::SerialPortInfo> SerialPort::getSerialPortList()
{
    _refreshSerialPortInfoList();
    return _serialPortInfoList;
}

