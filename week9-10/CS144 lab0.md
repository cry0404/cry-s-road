# CS144  lab 0

cs144 的目的是带我们实现一个属于自己的 tcp/ip 协议栈。

由于我采用的是 minnow 版，所以环境配置就下载了一个 ubuntu24.04 的裸机，在本地的 vmware 中运行。只简单的配置了 ble.sh 。由于现在 ai 太好用了，根本没用到 doxygen 这种工具，直接让 ai 总结就行。

1. 

   ```bash
   git clone --recursive https://github.com/akinomyoga/ble.sh.git
   ```

2. 安装：

   ```bash
   cd ble.sh
   make install
   ```

3. 

   ```bash
   source ~/.local/share/blesh/ble.sh
   ```

lab 0 就是带你熟悉环境配置，分别带我们体验了 telnet 以及 netcat 、 traceroute 这些基础的工具。

## 获取网页:

在网页浏览器中，访问 http://cs144.keithw.org/hello 。然后在 telnet 中实现同样的过程。

![telnet](https://picture.cry4o4n0tfound.cn/crywebsite/2025/03/请求网页.png)

可以看到成功访问了。但需要注意的是，在这里**telnet 中的手速一定要快，不然连接会断掉。**（早在 thm 的房间里就被此深深折磨过，以及这里的 quit 是错误用法，但正确的我也忘了，文档上似乎有写，反正无关紧要）。

---



## 使用 netcat 与 telnet ：

非常简单的操作，按照流程来即可以了。这里的 netcat 就相当于我们访问的 web 服务器，而 telnet 则是充当了浏览器的作用，帮助我们去请求。

![netcat](https://picture.cry4o4n0tfound.cn/crywebsite/2025/03/netcat.png)

并且在这里连接就不会中断，因为你是在本地监听，所以手速稍微放慢一点也没关系啦。

---



## 使用操作系统流套接字编写网络程序：

前面的 git 推送没什么好说的。如果你对 git 操作不熟悉的话，那么我推荐你阅读这本[gitbook](https://beej.us/guide/bggit/)，或者多问问 ai 也是一个不错的选择。

cs144 对实验流程做了一些建议：

- 使用 [C++ 参考文档](https://en.cppreference.com) 作为资源。（我们建议避免使用 `cplusplus.com`，因为它可能已经过时。）  

- **永远不要** 使用 `malloc()` 或 `free()`。  

- **永远不要** 使用 `new` 或 `delete`。  

- 基本上不要使用原始指针 (`*`)，只在必要时使用“智能”指针（`unique_ptr` 或 `shared_ptr`）。（你在 CS144 课程中不会需要使用这些。）  

- 避免使用模板、线程、锁和虚函数。（你在 CS144 课程中不会需要使用这些。）  

- 避免使用 C 风格字符串（`char *str`）或字符串函数（`strlen()`、`strcpy()`）。这些容易出错。请使用 `std::string` 代替。  

- **永远不要** 使用 C 风格的类型转换（例如 `(FILE *)x`）。如果必须转换，请使用 C++ `static_cast`。（通常你在 CS144 课程中不会需要这个。）  

- **优先** 通过 `const` 引用传递函数参数（例如：`const Address & address`）。  

- **尽量** 将所有变量声明为 `const`，除非它们需要被修改。  

- **尽量** 将所有方法声明为 `const`，除非它们需要修改对象。  

- 避免全局变量，并尽可能缩小变量的作用域。  

- 在提交作业之前，运行以下命令：  
  - `cmake --build build --target tidy` 以获取改进 C++ 代码的建议。  
  
  - `cmake --build build --target format` 以保持代码格式一致。  
  
    

---



### 实现 webget：

在做 cs144 之前我的套接字编程基础约等于 0 ，所以在做之前恶补了一下。

在实现之前，我们需要阅读 util 中封装好的 Socket 类。

```c++
Socket (基类)
├── DatagramSocket
│   ├── UDPSocket 
│   ├── PacketSocket
│   └── LocalDatagramSocket
└── TCPSocket
    └── LocalStreamSocket

封装的函数:
- bind(): 绑定地址
- connect(): 连接到对端
- shutdown(): 关闭连接
- local_address(): 获取本地地址 
- peer_address(): 获取对端地址
```

同时查看对应封装的接口：

```c++
class Socket : public FileDescriptor {
    void bind(const Address& address);
    void connect(const Address& address);
    void listen(int backlog);
    // ...
};
```

主要是继承了 Socket 类的 TcpSocket 和 UdpSocket  需要仔细查看。

接下来开始编写。只要掌握了基本的读写应该就没问题。（就算有问题多问问 ai 就没问题了：)

```c++
void get_URL( const string& host, const string& path )
{
  cerr << "Function called: get_URL(" << host << ", " << path << ")\n";
  cerr << "Warning: get_URL() has not been implemented yet.\n";
  TCPSocket socket;
  Address address( host, "http");
  socket.connect( address );
  socket.write( "GET " + path + " HTTP/1.1\r\n");
  socket.write( "Host: " + host + "\r\n");
  socket.write( "Connection: close\r\n");
  socket.write( "\r\n");
  while(true){
    string buffer;
    socket.read( buffer );
    std::cout<<buffer;
    if(buffer.empty()){
      break;
    }
  }
  socket.shutdown(SHUT_RDWR);
  socket.close();
}
```

可以看到成功运行了。需要注意的是，**不要忘了 GET 和 Host 后面都有空格**（因为这个第一次还错了...)

![webget](https://picture.cry4o4n0tfound.cn/crywebsite/2025/03/webget.png)

---

## 实现内存可靠的字节流：

这里本质上就是实现一个队列而已，但是这个队列应该尽量做到效率高。

> 要明确：字节流是有限的，但在写入者结束输入并完成流之前，它可以几乎是任意长的。您的实现必须能够处理比容量大得多的流。容量限制了在特定时刻内存中保留的字节数（已写入但尚未读取的字节数），但并不限制流的长度。即使一个只有一字节容量的对象，也能携带长达数以千计的流，只要写入者每次只写入一个字节，并且读者在写入者被允许写入下一个字节之前读取每个字节。

在类中添加一些方便统计的变量:

```c++
protected:
  // Please add any additional state to the ByteStream here, and not to the Writer and Reader interfaces.
  uint64_t capacity_;
  bool error_ {};
  std:: string buffer_; //存储
  bool closed_ { false };//检查状态，相当于多封装一层
  uint64_t pushed_count_ {0};//统计已写的
  uint64_t popped_count_ {0};//统计弹出的
  bool is_stream_closed() const { return closed_; }//封装 close 的检查，便于 writer 和 reader 调用
};
```

具体的实现：

```c++
#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) 
    : capacity_( capacity ) 
    , error_( false )
    , buffer_()
    , closed_( false )
    , pushed_count_( 0 )
    , popped_count_( 0 ) 
{}

void Writer::push( string data )
{
  if ( is_closed() ) {
    return;
  }

  size_t available = available_capacity();
  if ( data.size() > available ) {
    data = data.substr( 0, available );
  }
  buffer_ += data;
  pushed_count_ += data.size();
  //(void)data; // Your code here.
}

void Writer::close()
{
  // Your code here.
  closed_ = true; // 关闭直接将刚才设计的成员变量关掉即可
  return;
}

bool Writer::is_closed() const
{
  return closed_; // Your code here.
}

uint64_t Writer::available_capacity() const
{

  return capacity_ - buffer_.size(); // Your code here.
}

uint64_t Writer::bytes_pushed() const
{

  return pushed_count_; // Your code here.
}

string_view Reader::peek() const
{
  string_view view( buffer_ );
  return view; // Your code here.
}

void Reader::pop( uint64_t len )
{
  len = min( len, buffer_.size() ); // 确保不超过当前缓冲区的大小
  buffer_ = buffer_.substr( len );
  popped_count_ += len;
  //(void)len; // Your code here.
  return;
}

bool Reader::is_finished() const
{
  return is_stream_closed() && buffer_.empty(); // Your code here.
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size(); // Your code here.
}

uint64_t Reader::bytes_popped() const
{
  return popped_count_; // Your code here.
}

```

由于本身封装已经很好了，基本上只是作一些简单的调用，主要是理解类中的成员变量（其实对于没怎么正经写过 oop 的人来说还挺新奇）。

尝试运行，可以看到运行的很成功，t_webget 很慢可能是因为我没有设置网络(?。

![lab0](https://picture.cry4o4n0tfound.cn/crywebsite/2025/03/lab0.png)

(终端设置代理后就降到了 1.6 s 左右，基本正常了，唉，网络...)

---

## 总结

写 cs144 对于我来说属于一个新奇的挑战，因为其实从来没有认认真真写过 oop，上一个还是图书管理系统......并且在套接字基础约等于 0 的情况下一开始看起来是很劝退的。支持我做下去的其实是 ai。最近一直有在思考，ai 时代下，该怎么去写代码。目前得出来的答案是：让自己懂大致的框架即可。当中低端的代码 ai 都可以轻松实现时，大体的架构更为重要，编程更接近搭积木了，我们更应该在乎的是如何去搭，而不是积木本身。

或者更简单一点的理由——做的过程中总能慢慢学到一些东西，更何况还有一个无比耐心的老师陪着你，所以只需要告诉自己：just do  it。

