#include <iostream>
#include <windows.h>
#include "SerialPort.h"
#include <vector>

int main()
{
    std::vector<SerialPort::SerialPortInfo> _list = SerialPort::getSerialPortList();

    for( const auto& portInfo : _list )
    {
        std::cout<<portInfo.portName<<std::endl;
    }

    while(1);
    
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

//     // 3. 读取数据
//     char readBuffer[256];
//     DWORD bytesRead;
//     if (!ReadFile(hSerial, readBuffer, sizeof(readBuffer), &bytesRead, NULL)) {
//         std::cerr << "Error in ReadFile." << std::endl;
//         CloseHandle(hSerial);
//         return 1;
//     }

//     // 输出读取到的数据
//     std::cout << "Read " << bytesRead << " bytes: " << readBuffer << std::endl;

//     // 4. 写入数据
//     const char* writeBuffer = "Hello, serial port!";
//     DWORD bytesWritten;
//     if (!WriteFile(hSerial, writeBuffer, strlen(writeBuffer), &bytesWritten, NULL)) {
//         std::cerr << "Error in WriteFile." << std::endl;
//         CloseHandle(hSerial);
//         return 1;
//     }

//     std::cout << "Written " << bytesWritten << " bytes: " << writeBuffer << std::endl;

//     // 5. 关闭串口
//     CloseHandle(hSerial);

//     return 0;
// }
