#include "io_context.h"
#include "awaiters.h"
#include <cstdio>
#include <iostream>

/**
 * @brief 从socket中接收数据并发送回socket
 *
 * 从指定的socket接收数据，并将接收到的数据重新发送回该socket。
 * 如果接收到的数据长度为0或负数，或者发送失败，则返回false。
 * 如果发送成功，则打印发送的数据长度，并返回true。
 *
 * @param socket 要操作的socket对象
 * @return bool 发送成功返回true，否则返回false
 */
task<bool> inside_loop(Socket& socket) {

    char buffer[1024] = {0};
    ssize_t recv_len = co_await socket.recv(buffer, sizeof(buffer));
    ssize_t send_len = 0;
    while(send_len < recv_len) {
        auto res = co_await socket.send(buffer + send_len, recv_len - send_len);
        if(res <= 0) {
            co_return false;
        }
        send_len += res;
    }

    std::cout << "Done send"<<send_len<<std::endl;
    if (recv_len <= 0)
    {
        co_return false;
    }
    printf("%s", buffer);
    co_return true;
}

/*
    普通协程，不用await，直接resume()即可
*/
task<> echo_socket(std::shared_ptr<Socket> socket) {
    for(;;) {
        std::cout<<"BEGIN"<<std::endl;
        bool b = co_await inside_loop(*socket);
        if(!b)
        {
            break;
        }
        std::cout<<"END"<<std::endl;
    }
}

task<> accept(Socket& listen) {
    for(;;) {
        // 获取客户端连接
        auto socket = co_await listen.accept();
        auto t = echo_socket(socket);
        // resume()后会执行到echo_socket的第一个co_await
        t.resume();
    }
}

int main() {
    IoContext io;
    // 创建了socket，bind,listen
    Socket listen{"10009", io};
    auto t = accept(listen);
    t.resume();

    io.run();
}