#include <iostream>
#include <windows.h>
#include "SerialPort.h"
#include <vector>

static int serial_evt_handler(SerialPort::SerialPortEvent evt)
{
    char buf[128];
    int cnt;
    cnt = evt.instance->read(  (uint8_t*)buf , sizeof(buf) - 1  );
    buf[ cnt ] = '\0';
    std::cout<<buf;
    return 0;
}

int main()
{
    std::vector<SerialPort::SerialPortInfo> _list = SerialPort::getSerialPortList();
    char* t = "HelloWorld!\r\n";

    for( const auto& portInfo : _list )
    {
        std::cout<<portInfo.portName<<std::endl;
    }

    SerialPort* serialPort = new SerialPort;
    serialPort->registerEventHandler( serial_evt_handler , NULL );
    serialPort->connect(29 , 921600);

    while(1)
    {
        static uint16_t timeout = 0;
        timeout++;
        serialPort->write( (uint8_t*)t , strlen(t) );
        Sleep(1000);
    }

    while(1);

    return 0;
}
