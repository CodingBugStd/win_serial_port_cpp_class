#include <iostream>
#include <windows.h>
#include "SerialPort.h"
#include <vector>

static SerialPort* port1;        //硬件串口
static SerialPort* port2;      //透传串口

static int port1Handler(SerialPort::SerialPortEvent evt)
{
    if( evt.code == SerialPort::RECEIVE )
    {
        uint8_t buf[64];
        int cnt = evt.instance->read( buf , sizeof(buf) );
        std::cout<<"port1 recieve : "<<cnt<<" bytes."<<std::endl;
        
    }else if( evt.code == SerialPort::DISCONNECT )
    {
        std::cout<<"port "<<evt.instance->connectedSerialPortInfo().portName<<" disconnect"<<std::endl;
    }
    return 0;
}

static int port2Handler(SerialPort::SerialPortEvent evt)
{
    if( evt.code == SerialPort::RECEIVE )
    {
        uint8_t buf[64];
        int cnt = evt.instance->read( buf , sizeof(buf) );
        std::cout<<"port1 recieve : "<<cnt<<" bytes."<<std::endl;
    }else if( evt.code == SerialPort::DISCONNECT )
    {
        std::cout<<"port "<<evt.instance->connectedSerialPortInfo().portName<<" disconnect"<<std::endl;
    }
    return 0;
}

static int connect_port(void)
{
    if( port1->isConnected() )
    {
        port1->disconnect();
    }
    if( port2->isConnected() )
    {
        port2->disconnect();
    }

    auto serialList = SerialPort::getSerialPortList();
    std::cout<<"find "<<serialList.size()<<" serail port"<<std::endl;
    std::cout<<"----------port list---------"<<std::endl;
    for( auto it = serialList.begin() ; it != serialList.end() ; ++it )
    {
        std::cout<<"portName:"<<it->portName<<std::endl;
    }

    int portNumber;
    int portBound;

    std::cout<<"input hard serial port number:";
    std::cin>>portNumber;
    std::cout<<"input hard serial port bound:";
    std::cin>>portBound;

    port1->registerEventHandler( port1Handler , NULL );
    if( !port1->connect( portNumber , portBound ) )
    {
        std::cout<<portNumber<<" connect error."<<std::endl;
        return -1;
    }

    std::cout<<"input across serial port number:";
    std::cin>>portNumber;
    std::cout<<"input across serial port bound:";
    std::cin>>portBound;

    port2->registerEventHandler( port2Handler , NULL );
    if( !port2->connect( portNumber , portBound ) )
    {
        printf("connect error.");
        return -1;
    }

    return 0;
}

int main(void)
{
    port1 = new SerialPort;
    port2 = new SerialPort;

    while( connect_port() );

    while(1)
    {
        Sleep(100);
        port1->write( (uint8_t*)"HelloWorld!" , strlen("HelloWorld!") );
    }

    return 0;
}