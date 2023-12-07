#pragma once

#include <vector>
#include <stdint.h>
#include <string>
#include <thread>
#include <mutex>
#include <windows.h>

//todo
//1、接收线程的终止与回收
//2、析构器

//异步event函数目前线程不安全
//注册回调尽量在connect前

class SerialPort{
public:
    SerialPort();
    virtual ~SerialPort();

    typedef struct{
        std::string     portName;           //端口号
        std::string     driverName;         //驱动信息
    }SerialPortInfo;

    typedef struct{
        uint32_t bound;
        uint8_t  dataBit;
        uint8_t  stopBit;
    }SerialConnectCfg;

    typedef enum{
        RECEIVE = 0,
        CONNECT = 1,
        DISCONNECT = 2
    }SerialPortEventCode;

    typedef struct{
        SerialPortEventCode code;
        SerialPort*         instance;
        void*               user_ctx;
    }SerialPortEvent;

    bool connect( std::string portName , uint32_t bound = 115200 , uint8_t dataBit = 8 , uint8_t stopBit = 1 );
    bool connect( std::string portName , SerialConnectCfg& cfg);
    bool connect( int portNumber , uint32_t bound = 115200 , uint8_t dataBit = 8 , uint8_t stopBit = 1 );
    bool connect( int portNumber , SerialConnectCfg& cfg);
    bool connect( SerialPortInfo& info , uint32_t bound = 115200 , uint8_t dataBit = 8 , uint8_t stopBit = 1 );
    bool connect( SerialPortInfo& info , SerialConnectCfg& cfg );
    bool disconnect();
    bool isConnected();
    size_t receiveLen();
    SerialPortInfo      connectedSerialPortInfo();
    SerialConnectCfg    connectedSerialPortCfg();

    /**
     * @brief 向串口写入(同步，内部带锁)
     * 
     * @param dat 数据
     * @param size 大小
     * @return true 成功
     * @return false 失败
     */
    bool write(uint8_t*dat , size_t size);
    /**
     * @brief 从串口读取(同步，内部带锁，直接从接收缓存中获取)
     * 
     * @param buf 缓存区
     * @param size 读取的大小
     * @return int 读取到的数量
     */
    int  read(uint8_t*buf , size_t size);

    void registerEventHandler( int (*handler)(SerialPortEvent) , void* ctx )
    {
        _eventHandler = handler;
        _eventHandlerUserCtx = ctx;
    }

    void cancelEventHandler()
    {
        _eventHandler = NULL;
    }

    static std::vector<SerialPortInfo> getSerialPortList();

private:
    SerialPortInfo      _serialInfo;            //当前操作的串口信息
    SerialConnectCfg    _serialCfg;             //当前操作的串口的配置
    int (*_eventHandler)(SerialPortEvent);      //用户注册的串口事件回调函数
    void*   _eventHandlerUserCtx;               //用户需要传入事件回调的上下文
    uint8_t _receiveBuf[2048];                  //接收缓存
    size_t  _receiveLen;                        //当前接收缓存未被读取的长度
    std::mutex*  _receiveBufLock;               //接收缓存锁
    bool _connect();                            //根据_serialInfo将实例连接到指定串口,创建接收线程
    void _callback(SerialPortEvent& evt);       //用户事件回调函数实现

    HANDLE _hSerial;                        //win api  串口句柄
    std::mutex* _hSerialReadLock;           //_hSerial读锁
    std::mutex* _hSerialWriteLock;          //_hSerial写锁
    DCB _dcbSerialParams;                   //用于设置win api的一些参数
    std::thread* _recieveThread;            //接收线程
    void _recieveThreadImpl();

    static void _refreshSerialPortInfoList();
    static std::vector<SerialPortInfo>    _serialPortInfoList;
};

