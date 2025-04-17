---


---

# cs144-lab2:

## 简介：

lab2 的目的是继续完善 tcp 的细节，我们之前在 lab1 中实现的部分已经模拟了 ack 的逻辑，例如接收窗口什么的，但是某一些细节部分被忽略了，例如上一节的序列号和确认号——由于 TCP 序列号是 32 位无符号整数，最大值是 4294967295。当序列号达到这个最大值后，会“回绕”到 0，继续递增。这种现象称为**序列号回绕**（Sequence Number Wraparound）。

- 在低速网络中，序列号回绕可能需要很长时间才会发生。例如，传输 4GB 数据（2³² 字节）在 1Mbps 的网络上需要大约 8 小时。
- 但在现代高速网络中（如 10Gbps 或更高），序列号回绕可能在几秒钟内发生。例如，在 10Gbps 的网络上，传输 4GB 数据只需约 3.2 秒。

现代 tcp 中引入了 Tsval 和 Tsecr 等段来区分，这个可以在 wireshark 里抓的包看到。

从而我们可以基于此计算出对应的是否是发生了回绕的序列号。

![现代 tcp 的区分方式](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/wireshark.png)

类似的检测我们需要在  wrapping 中实现，将三种序列号的关系对应起来，用文档里的话来说的话，**就是将序列号转化为绝对序列号，绝对序列号始终从零开始并且不存在回绕，**而流索引则代表着流中每个字节的索引（在 Reassembler 中我们已经用到了），以输入 ‘cat‘ 的字节流为例：

![指导书的指导](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/instruction.png)

```cpp
#pragma once

#include <cstdint>

/*
 * Wrap32类型表示一个32位无符号整数，它：
 *    - 从任意的"零点"（初始值）开始，并且
 *    - 当它达到2^32 - 1时会绕回到零。
 */

class Wrap32
{
public:
  // 显式构造函数，接受一个32位原始值
  explicit Wrap32( uint32_t raw_value ) : raw_value_( raw_value ) {}

  /* 给定一个绝对序列号n和零点，构造一个Wrap32对象。 */
  static Wrap32 wrap( uint64_t n, Wrap32 zero_point );

  /*
   * unwrap方法返回一个绝对序列号，给定零点和一个"检查点"（另一个接近所需答案的绝对序列号），
   * 该序列号会映射到这个Wrap32。
   *
   * 可能有很多绝对序列号都会映射到同一个Wrap32。
   * unwrap方法应该返回最接近检查点的那一个。
   */
  uint64_t unwrap( Wrap32 zero_point, uint64_t checkpoint ) const;

  // 重载+运算符，返回原始值加上n的新Wrap32对象
  Wrap32 operator+( uint32_t n ) const { return Wrap32 { raw_value_ + n }; }
  
  // 重载==运算符，比较两个Wrap32对象的原始值是否相等
  bool operator==( const Wrap32& other ) const { return raw_value_ == other.raw_value_; }

protected:
  // 保护成员变量，存储32位原始值
  uint32_t raw_value_ {};
};
```

> 在绝对序列号和流索引之间进行转换足够简单 —— 只需加一或减一即可。不幸的是，在序列号和绝对序列号之间进行转换要困难一些，混淆两者可能会产生棘手的错误。为了系统地防止这些错误，我们将使用自定义类型： Wrap32 来表示序列号，并编写它与绝对序列号（用 uint64 t 表示）之间的转换。

### 具体实现：

这里的实现有点类似于初中时学绝对值的感受，先去除掉 zero_point 的影响，然后计算出 base 值，这里有一个巧妙的处理，利用位运算：

```cpp
uint64_t base = checkpoint & ~(0xFFFFFFFFULL)
```

这样可以非常高效的去除掉低 32 位，因为当 checkpoint 的序列号大于 2^32 后，每隔 2^32 就开始重新循环，所以我们需要找到最接近的当前周期。

比如：

- 如果 checkpoint = 4,294,970,000 (接近一个周期末尾)
- 清除低32位后得到 base = 0
- 这样我们可以构建当前周期、上一周期和下一周期的候选值

当然，位运算是我最后让 Claude 检查的时候它给我的建议，我本人只会傻傻的除法 : (，但是这也从侧面证明 ai 真的是很棒的老师。

参考代码如下：

```cpp
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  uint32_t offset = (n + zero_point.raw_value_) % (1ULL<<32);
  //debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  return Wrap32 { offset };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  //消除最开始的随机序列号引起的偏移量，有利于找到绝对序列号
  uint32_t offset = raw_value_ - zero_point.raw_value_;
  //** note 清除低32位，使用位运算方法
  uint64_t base = checkpoint & ~(0xFFFFFFFFULL);//其实这里我一开始用的除法，后面让 ai 检查代码时它推荐我用位运算
  //位运算在底层还是好用的，又把之前 datalab 的知识还回去了 ：(
  uint64_t now = base + offset;
  uint64_t next = now + (1ULL<<32);
  uint64_t pre = now - (1ULL<<32);

  uint64_t distance1 = now >= checkpoint? now - checkpoint : checkpoint - now;
  uint64_t distance2 = next >= checkpoint? next - checkpoint : checkpoint - next;
  uint64_t distance3 = pre >= checkpoint? pre - checkpoint : checkpoint - pre;

  if (distance1 <= distance2 && distance1 <= distance3) {
    return now;
  } else if (distance2 <= distance1 && distance2 <= distance3) {
    return next;
  } else {
    return pre;
  }
 // debug( "unimplemented unwrap( {}, {} ) called", zero_point.raw_value_, checkpoint );
  return {};
}
```

---

## TCPReceiver：

我们已经实现了序列号的封装和解封装逻辑，接下来就需要真正的去实现 TCPReceiver 了，指导书上给出的结构如下：

![依旧是指导总结](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/instruction2.png)

我们需要对发过来的 message 进行处理，先看一下 message 的结构：

```cpp
struct TCPSenderMessage
{
  Wrap32 seqno { 0 };

  bool SYN {};
  std::string payload {};
  bool FIN {};

  bool RST {};

  // How many sequence numbers does this segment use?
  size_t sequence_length() const { return SYN + payload.size() + FIN; }
};
```

基于 message 的结构我们可以思考我们需要对 Receiver 设计怎样的私有类。

我们需要明确 TCPReceiver 有几种状态：

- 回想一下 tcp 还没建立连接时，服务端会处于监听模式，这个时候我们的随机序列号还没有确定。
- 当收到 syn 后，还没收到 fin 字段前，都应该是接受字节流的模式
- 收到 fin，自然就关闭了。

所以我们可以这样设计：

```CPP
class TCPReceiver
{
public:
  // Construct with given Reassembler
  explicit TCPReceiver( Reassembler&& reassembler ) : 
    reassembler_( std::move( reassembler ) ),
    is_syn_received_( false ),
    is_fin_received_( false ),
    has_error_( false ),
    zero_point_( std::nullopt ) {}

  /*
   * The TCPReceiver receives TCPSenderMessages, inserting their payload into the Reassembler
   * at the correct stream index.
   */
  void receive( TCPSenderMessage message );

  // The TCPReceiver sends TCPReceiverMessages to the peer's TCPSender.
  TCPReceiverMessage send() const;

  // Access the output
  const Reassembler& reassembler() const { return reassembler_; }
  Reader& reader() { return reassembler_.reader(); }
  const Reader& reader() const { return reassembler_.reader(); }
  const Writer& writer() const { return reassembler_.writer(); }

private:
  Reassembler reassembler_;
  bool is_syn_received_; // 标记是否已收到 SYN
  bool is_fin_received_; // 判断是否收到 FIN
  bool has_error_; //检测是否出错了
  std::optional<Wrap32> zero_point_; 
  //学到的定义方式,没收到 syn 前是可选的
};
```

分别对于三种状态做处理，**这里唯一注意的点是流索引相较，绝对序列号要去除 syn 的影响。**

实现还是比较简单的，难点在于将前面封装好的东西，如何去调用，**所以我觉得这里最重要的是明确你在调用什么。**

所以在实现之前最好整理一下思路，我们目前为止究竟实现了些什么东西，它在 tcp 中又对应什么。

### 回忆：

我们最开始实现的是 ByteStream，表示底层的数据流，分别对应 Writer 和 Reader 两端，用于缓冲数据并且控制流，也就是最底层的字节流。

之后实现的 Reassembler 则是处理 tcp 乱序的关键，类似于 tcp 的缓冲接受区，保存未按序到达的数据，同时将数据流组织有序后，发送给应用层。

而 Wrap32 类则模拟了 tcp 序列号系统，处理序列号只有 32 bit 可能会发生回绕的问题，封装的 wrap 和 unwrap 则是为了解决绝对序列号问题。

而我们现在要实现的 TCPSenderMessage 和 TCPReceiverMessage 则分别是模拟 tcp 段和 tcp 的确认，通过模拟以下的消息结构来达到实现 tcp 协议栈中接收方的作用：

- **TCPSenderMessage**：模拟TCP段
  - `seqno`：序列号
  - `SYN`/`FIN`/`RST`：连接控制标志
  - `payload`：数据部分
  
- **TCPReceiverMessage**：模拟TCP确认
  - `ackno`：确认号，表示接收方期望的下一个字节
  - `window_size`：接收窗口大小，用于流控制
  - `RST`：重置连接标志

梳理清楚后，再看一看对应头文件中定义的方法就可以开始实现了。逻辑很简单，主要是如何去调用之前封装好的内容...

参考代码如下：

```cpp
#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  //*note 先按影响程度做判断,如果遇到 rst 应该重置连接
  if(message.RST){
    is_syn_received_ = false;
    is_fin_received_ = false;
    zero_point_ = std::nullopt;
    has_error_ = true;  // 设置错误标志
   //简单重置状态
   reassembler_.reader().set_error() ;  // 设置ByteStream错误
    return;
  }
  if(message.SYN){
    if(!is_syn_received_){//第一次收到才设置零点
      is_syn_received_ = true;
      zero_point_ = message.seqno;
    }
  }
  if(!is_syn_received_ || !zero_point_.has_value()){
    return; //丢弃所有没有 syn 值的包
  }
  uint64_t checkpoint = reassembler_.writer().bytes_pushed();
  uint64_t abs_seqno = message.seqno.unwrap(zero_point_.value(),checkpoint);
  
  uint64_t stream_index = abs_seqno;
  if(message.SYN){
    stream_index = 0;
  }
  else{
    stream_index = abs_seqno - 1;
  }

  if(message.FIN){
    is_fin_received_ = true;
  }
  if (!message.SYN && abs_seqno == 0) {
    // 忽略具有SYN序列号但不是SYN的无效数据包
    return;
  }
  reassembler_.insert(stream_index, message.payload, message.FIN);
  //debug( "unimplemented receive() called" );
  
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  TCPReceiverMessage response {};
  //有错的话直接重来
  if (has_error_ || reassembler_.reader().has_error()) {
    response.RST = true;
    response.ackno = nullopt;
    return response;
  }
  
 
  uint64_t window_size = reassembler_.writer().available_capacity();
  response.window_size = min(window_size, static_cast<uint64_t>(UINT16_MAX));

  // 如果连接未建立，不设置确认号
  if (!is_syn_received_ || !zero_point_.has_value()) {
    return response;
  }

  // 计算确认号
  uint64_t abs_ackno = reassembler_.writer().bytes_pushed() + 1;  // +1 是因为SYN
  
  // 如果流已结束且已完全接收，再加1（FIN占一个序列号）
  if (reassembler_.writer().is_closed()) {
    abs_ackno += 1;
  }
  
  // 转换为相对序列号
  response.ackno = zero_point_->wrap(abs_ackno, *zero_point_);
  
  return response;
  //debug( "unimplemented send() called" );
  
}
```

30 个测试点，懒得截长图了，就这样吧。

目前做下来，感觉对于调用封装好的能力有待加强（其实是隔一段时间做就忘了前面干了啥...)。