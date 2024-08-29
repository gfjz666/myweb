## 基于Linux的C++轻量级Web服务器

### 项目应用技术：
- 1.该项目总体框架采用的是单Reactor多线程模型，在主线程里面通过I/O多路复用监听多个文件描述符上的事件。主线程负责连接的建立和断开、把读写和逻辑处理函数加入线程池的任务队列，由线程池的子线程完成相应的读写操作，实现任务的高并发处理；
- 2.在最底层实现了自动增长的Buffer缓冲区；
- 3.在应用层方面实现心跳机制，通过定时器清理掉不活跃的连接以减少高并发场景下不必要的系统资源的占用（文件描述符的占用、维护TCP连接所需要的资源等）；
- 4.对于HTTP请求报文，采用分散读进行读取，使用有限状态机和正则表达式进行解析；并通过集中写和内存映射的方式对响应报文进行传输；
- 5.应用层实现了数据库功能，采用RAII机制实现连接池，可以实现基本的注册和登录功能；
- 6.加入了日志模块帮助工程项目开发和实现服务器的日常运行情况的记录；


### 相关知识：
- 1.RAII全称是“Resource Acquisition is Initialization”，直译过来是“资源获取即初始化”.
- 核心思想是将资源或者状态与对象的生命周期绑定，构造函数中申请分配资源，在析构函数中释放资源，智能指针是最好的例子
- 2.线程池,减少线程的销毁，线程同步涉及信号量、互斥变量、条件变量等


### 主要模块：
- 用vector封装char，实现能自动增长的缓冲区;
- 利用单例模式和阻塞队列实现异步日志,结合缓冲区存入文件；
- 实现线程池+数据库连接池，增强资源的利用率;
- 使用正则表达式+状态机解析HTTP报文,能处理静态请求发送报文;

## 运行环境
- C++11
- Linux
- MySQL

## 运行


## 性能测试


## 感谢
[@markparticle](https://github.com/markparticle/WebServer)<br>
[@qinguoyi](https://github.com/qinguoyi/TinyWebServer)