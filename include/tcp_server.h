//
// Created by 20132 on 2022/4/17.
//

#ifndef XZMJX_TCP_SERVER_H
#define XZMJX_TCP_SERVER_H
#include "noncopyable.h"
#include "socket.h"
#include "iomanager.h"
#include <memory>
#include <vector>
namespace xzmjx{
class TcpServer:public std::enable_shared_from_this<TcpServer>,Noncopyable{
public:
    typedef std::shared_ptr<TcpServer> ptr;

    /**
     * @brief
     * @param worker
     * @param io_worker
     * @param accept_worker
     */
    TcpServer(IOManager* worker = IOManager::Self(),
              IOManager* io_worker = IOManager::Self(),
              IOManager* accept_worker = IOManager::Self());
    virtual ~TcpServer();
    /**
     * @brief 绑定地址
     * @param addr
     * @return 是否绑定成功
     */
    virtual bool bind(Address::ptr addr);

    /**
     * @brief 绑定地址数组
     * @param addrs 需要绑定的地址数组
     * @param fails 绑定失败的地址数组
     * @return 是否有绑定失败的地址
     */
    virtual bool bind(const std::vector<Address::ptr>& addrs,
                      std::vector<Address::ptr>& fails);

    /**
     * @brief 启动服务
     * @return 是否成功启动
     */
    virtual bool start();

    /**
     * @brief 停止服务
     * @return 是否成功停止
     */
    virtual void stop();

    /**
     * @brief 设置接受超时时间
     * @param v
     */
    void setRecvTimeout(uint64_t v){m_recv_timeout = v;}
    uint64_t getRecvTimeout() const { return m_recv_timeout;}

    /**
     * @brief 设置服务器名称
     * @param v
     */
    void setName(const std::string& v) { m_name = v;}
    std::string getName() const { return m_name;}

    /**
     * @brief 服务是否停止
     * @return
     */
    bool isStop() const{ return m_is_stop;}

    std::vector<Socket::ptr> getSocks() const{ return m_listen_socks;}
protected:
    /**
     * @brief 处理客户连接
     * @param client
     */
    virtual void handleClient(Socket::ptr client);

    /**
     * @brief 启动接受连接
     * @param sock
     */
    virtual void startAccept(Socket::ptr sock);
private:
    std::vector<Socket::ptr> m_listen_socks; ///监听的套接字数组
    IOManager* m_worker;                    ///
    IOManager* m_io_worker;                 ///处理新连接的调度器
    IOManager* m_accept_worker;             ///接受连接请求的调度器
    uint64_t m_recv_timeout;                 ///接收超时时间
    std::string m_name;                      ///服务器名称
    std::string m_type = "tcp";              ///服务器类型
    bool m_is_stop;                          ///是否停止
};
}///namespace xzmjx

#endif //XZMJX_TCP_SERVER_H
