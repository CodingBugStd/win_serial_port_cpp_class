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

    SerialPort serialPort;
    serialPort.registerEventHandler( serial_evt_handler , NULL );
    serialPort.connect(8 , 921600);

    while(1)
    {
        serialPort.write( (uint8_t*)t , strlen(t) );
        Sleep(1000);
    }
    
    return 0;
}


// #include <Windows.h>
// #include <iostream>

// int main() {
//     // 1. 打开串口
//     HANDLE hSerial = CreateFile( "COM8", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
//     if (hSerial == INVALID_HANDLE_VALUE) {
//         std::cerr << "Failed to open serial port." << std::endl;
//         return 1;
//     }

//     // 2. 配置串口
//     DCB dcbSerialParams = {0};
//     dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
//     if (!GetCommState(hSerial, &dcbSerialParams)) {
//         std::cerr << "Error in GetCommState." << std::endl;
//         CloseHandle(hSerial);
//         return 1;
//     }

//     dcbSerialParams.BaudRate = 921600;  // 设置波特率为9600
//     dcbSerialParams.ByteSize = 8;         // 数据位为8
//     dcbSerialParams.StopBits = ONESTOPBIT; // 1个停止位
//     dcbSerialParams.Parity = NOPARITY;    // 无奇偶校验

//     if (!SetCommState(hSerial, &dcbSerialParams)) {
//         std::cerr << "Error in SetCommState." << std::endl;
//         CloseHandle(hSerial);
//         return 1;
//     }

//     COMMTIMEOUTS timeouts = {0};
//     timeouts.ReadIntervalTimeout = 5;             // 读取超时时间（以毫秒为单位） -> 单个字符接收完毕后的超时时间
//     timeouts.ReadTotalTimeoutConstant = 5;        // 读取总超时时间（以毫秒为单位）
//     timeouts.ReadTotalTimeoutMultiplier = 1;      // 读取总超时时间乘数
//     timeouts.WriteTotalTimeoutConstant = 5;    // 写入总超时时间（以毫秒为单位）
//     timeouts.WriteTotalTimeoutMultiplier = 1;     // 写入总超时时间乘数

//     if (!SetCommTimeouts(hSerial, &timeouts)) {
//         std::cerr << "Failed to set serial port timeouts." << std::endl;
//         CloseHandle(hSerial);
//         return false;
//     }

//     while(1)
//     {
//         char readBuffer[256];
//         DWORD bytesRead;
//         if (!ReadFile(hSerial, readBuffer, sizeof(readBuffer), &bytesRead, NULL)) {
//             std::cerr << "Error in ReadFile." << std::endl;
//             CloseHandle(hSerial);
//             return 1;
//         }
//         if( bytesRead != 0 )    std::cout << readBuffer ;
//         Sleep(1);
//     }

//     // // 3. 读取数据
//     // char readBuffer[256];
//     // DWORD bytesRead;
//     // if (!ReadFile(hSerial, readBuffer, sizeof(readBuffer), &bytesRead, NULL)) {
//     //     std::cerr << "Error in ReadFile." << std::endl;
//     //     CloseHandle(hSerial);
//     //     return 1;
//     // }

//     // // 输出读取到的数据
//     // std::cout << "Read " << bytesRead << " bytes: " << readBuffer << std::endl;

//     // // 4. 写入数据
//     // const char* writeBuffer = "Hello, serial port!";
//     // DWORD bytesWritten;
//     // if (!WriteFile(hSerial, writeBuffer, strlen(writeBuffer), &bytesWritten, NULL)) {
//     //     std::cerr << "Error in WriteFile." << std::endl;
//     //     CloseHandle(hSerial);
//     //     return 1;
//     // }

//     // std::cout << "Written " << bytesWritten << " bytes: " << writeBuffer << std::endl;

//     // 5. 关闭串口
//     CloseHandle(hSerial);

//     return 0;
// }
