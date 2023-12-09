#include <iostream>
#include <windows.h>
#include <vector>

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"


static CSerialPort* port1 = NULL;
static CSerialPort* port2 = NULL;

int main(void)
{
    auto portList = CSerialPortInfo::availablePortInfos();
    for( auto it = portList.begin() ; it != portList.end() ; ++it )
    {
        std::cout<<"port name : "<<it->portName<<std::endl;
        std::cout<<"port desc : "<<it->description<<std::endl;
    }

    port1 = new CSerialPort;
    port2 = new CSerialPort;

    port1->init( "COM1" , 115200 );
    port1->setOperateMode(AsynchronousOperate);
    port1->setReadBufferSize(512);
    port1->open();

    port2->init( "COM3" , 115200 );
    port2->setOperateMode(AsynchronousOperate);
    port2->setReadBufferSize(512);
    port2->open();

    while(1)
    {
        int cnt;
        uint8_t buf[512];

        cnt = port1->readAllData( buf );
        if( cnt > 0 )
        {
            port2->writeData(buf,cnt);
        }

        cnt = port2->readAllData( buf );
        if( cnt > 0 )
        {
            port1->writeData(buf,cnt);
        }
    }

    return 0;
}