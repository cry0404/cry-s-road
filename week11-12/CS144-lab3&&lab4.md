---

---

##  简介：

**非常建议做之前认真阅读指导书以及回顾之前写过些什么内容！！！**

代码的合并在指导书上说的很详细了，不再赘述。

lab3 是要让我们实现 TCP 发送的功能 ，而 minnow 版的 lab4 则是在现实世界中用 ping 来测量 RTT，相较于 sponge 版，它其实是将 sponge 的 lab3 和 lab4 做了一定程度的整合，以至于我做起来非常痛苦。

另外，学网络如果光写代码确实也缺乏了网络学习中的乐趣，没有实际解决网络问题的成就感，而 minnow 版的这个现实世界结合没啥意思，所以我十分推荐将 lab4 替换为以下链接里的抓包实战，做完一定会觉得分析网络是一件很有意思的事情。

[计算机网络实用技术：序章](https://www.kawabangga.com/posts/6097)

甚至我觉得抓包分析就应该作为计算机网络的期末考试，而不是去默写背诵一堆又臭又长的概念...

抑或是半期考试考什么书上 Alice 和 Bob 通信时监听中间人的名字是什么...

## lab-3:

做完抓包分析对整个网络有了更深理解后，我们再来看我们需要实现的内容。

到目前为止，我们已经实现了：

```
tcp 发送方 + 连接管理
```

现在只要实现发送方，并再做一定整合我们就可以完成 tcp 协议栈了。

```
客户端 = TCP发送方 + TCP接收方 + 连接管理
服务端 = TCP发送方 + TCP接收方 + 连接管理
```

## 实现:

磨了很久，中间 debug 不出来问了好几次 ai。里面的细节有些多，并且在本 lab 中的测试用例让我倒回去改了 lab1 中的代码（现在已经在对应 lab1 中修改了）。

这里前几个函数的实现思路可以参考 bytestream，例如这里的有多少 seuqnce_numbers_in_flight 或者 consecutive_retransmissions 其实都只是一个状态，就和 bytestream 中 is_connect 是一个道理。所以在私有成员中维护一个值返回即可。

```cpp
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  
  debug( "unimplemented sequence_numbers_in_flight() called" );
  return bytes_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  debug( "unimplemented consecutive_retransmissions() called" );
  return retransmissions_;
}

```

### 一些细节：

**这里面另外需要注意的点是，syn 与 fin 的处理， syn 设置后，装的数据里要为它留一个位置，同样的，如果 fin 设置 装不下也需要在下一个包里填上去。**

以及窗口的大小一定是从 1 开始的。

```cpp
uint64_t window_size = window_size_ > 0 ? window_size_ : 1;
```

重传状态记录，用队列是方便带时间标记，有点类似于写 bfs 算法题。

```cpp
outstanding_segments_.push({msg, 0});
```

以及只有在成功确认新段时才能重置 RTO，而不是在每次收到 ACK 时

```cpp
if (segments_acknowledged) {
  retransmissions_ = 0;
  RTO_ms_ = initial_RTO_ms_;
}
```

这里面还使用了回调函数：

```cpp
transmit(msg);
```

算是很有意思的处理（不过也有可能是因为我写的代码不多的原因）。

最重要的是！！！一定要在每个可能发送数据的地方都要检查流错误，测试用例里面有很多针对这个地方的点。

```cpp
if (msg.RST) {
  writer().set_error();
  return;
}
```

完整代码：

```cpp
#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// This function is for testing only; don't add extra state to support it.
//跟踪已经发送但未被确认的序列号数量
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return bytes_in_flight_;
}

//记录连续重传的数量
uint64_t TCPSender::consecutive_retransmissions() const
{

  return retransmissions_;
}
//实现数据的发送，先要考虑窗口大小等限制
void TCPSender::push( const TransmitFunction& transmit )
{
  //首先检查流是否有错
  if (reader().has_error()) {
    TCPSenderMessage msg;
    msg.seqno = isn_ + next_seqno_;
    msg.RST = true;
    transmit(msg);
    return;
  }
  uint64_t window_size = window_size_ > 0 ? window_size_ : 1;
  //如果有确认号的话就将其解包转换
  uint64_t abs_ackno = ackno_.has_value() ? ackno_.value().unwrap(isn_, next_seqno_) : 0;
  uint64_t window_end = abs_ackno + window_size;
  
  uint64_t available_window = window_end > next_seqno_ ? window_end - next_seqno_ : 0;

  // 如果最开始没有发送 syn 的话就先发送
  if (next_seqno_ == 0 && available_window > 0) {
    TCPSenderMessage msg;
    msg.seqno = isn_;
    msg.SYN = true;
    
    // 尝试添加数据负载 
    size_t payload_capacity = available_window > 1 ? available_window - 1 : 0; // SYN占用1个序列号
    payload_capacity = std::min(payload_capacity, static_cast<uint64_t>(TCPConfig::MAX_PAYLOAD_SIZE));
    
    if (payload_capacity > 0 && (reader().peek().size()>0)) {
      size_t read_size = std::min(payload_capacity, reader().peek().size());
      if (read_size > 0) {
        msg.payload = reader().peek().substr(0, read_size);
        reader().pop(read_size);
      }
    }
    
    // 检查是否应该在同一个包中发送 FIN
    if (reader().is_finished() && !fin_sent_ && 
        available_window > 1 + msg.payload.size()) { // 需要额外的窗口空间给FIN
        msg.FIN = true;
        fin_sent_ = true;
    }

    transmit(msg);

    size_t segment_size = 1 + msg.payload.size() + (msg.FIN ? 1 : 0); // SYN占1，payload占其大小，FIN占1
    outstanding_segments_.push({msg, 0});
    bytes_in_flight_ += segment_size;
    next_seqno_ += segment_size;
    
    if (!timer_running_) {
      timer_running_ = true;
      time_since_last_activity_ = 0;
      RTO_ms_ = initial_RTO_ms_;
    }
    
    available_window -= segment_size;
    
    // 如果已发送FIN，不再发送数据
    if (msg.FIN) {
      return;
    }
  }
  
  while(available_window > 0 ){
    if(reader().peek().size() == 0 && (!reader().is_finished() || fin_sent_)) {
      break;
    }
    TCPSenderMessage msg;
    msg.seqno = isn_ + next_seqno_;
    //这里需要去找 tcpconfig 中对于 tcp 数据段的要求
    size_t payload_size = std::min(available_window,static_cast<uint64_t>(TCPConfig::MAX_PAYLOAD_SIZE));
    payload_size = std::min(payload_size,reader().peek().size());

    if(payload_size > 0){
      msg.payload = reader().peek().substr(0,payload_size);
      reader().pop(payload_size);
    }

    if (reader().is_finished() && !fin_sent_ && available_window > msg.payload.size()) {
      msg.FIN = true;
      fin_sent_ = true;
    }

    if (msg.payload.empty() && !msg.SYN && !msg.FIN) {
      break;
    }
    
    // 发送消息
    transmit(msg);
    size_t segment_size = msg.payload.size() + (msg.FIN ? 1 : 0);
    outstanding_segments_.push({msg, 0});
    bytes_in_flight_ += segment_size;
    next_seqno_ += segment_size;
    
    // 启动计时器
    if (!timer_running_) {
      timer_running_ = true;
      time_since_last_activity_ = 0;
      RTO_ms_ = initial_RTO_ms_;
    }
    
    available_window -= segment_size;
    
    // 如果已发送FIN，不再发送数据
    if (msg.FIN) {
      break;
    }
  }
  return;
}
//创建不包含数据的 tcp 消息，也就是只有个头
TCPSenderMessage TCPSender::make_empty_message() const
{
  
  TCPSenderMessage msg;
  msg.seqno = isn_ + next_seqno_;
  if (reader().has_error()) {
    msg.RST = true;
  }
  return msg;
}

void TCPSender::receive(const TCPReceiverMessage& msg)
{
  // 检查是否收到RST标志
  if (msg.RST) {
    writer().set_error();
    return;
  }

  window_size_ = msg.window_size;

  if(msg.ackno.has_value()){
    uint64_t abs_ackno = msg.ackno.value().unwrap(isn_,next_seqno_);

    if(abs_ackno <= next_seqno_){
      ackno_ = msg.ackno;
    

    bool segments_acknowledged = false;
    //然后就是经典的队列遍历了
    while(!outstanding_segments_.empty()){
      const auto& segment = outstanding_segments_.front();
      uint64_t seg_seqno = segment.msg.seqno.unwrap(isn_,next_seqno_);
      uint64_t seg_end = seg_seqno + segment.msg.payload.size() + 
                          (segment.msg.SYN ? 1 : 0) + 
                          (segment.msg.FIN ? 1 : 0);
      if (seg_end <= abs_ackno) {
          // 完全确认此段
          bytes_in_flight_ -= (segment.msg.payload.size() + 
                             (segment.msg.SYN ? 1 : 0) + 
                             (segment.msg.FIN ? 1 : 0));
          
          outstanding_segments_.pop();
          segments_acknowledged = true;
        } else {
          break;
        }
      }
      
      // 如果确认了新的段，重置重传计数和RTO
      if (segments_acknowledged) {
        retransmissions_ = 0;
        RTO_ms_ = initial_RTO_ms_;
        
        // 重置计时器（如果还有未确认的段）
        if (!outstanding_segments_.empty()) {
          time_since_last_activity_ = 0;
        } else {
          timer_running_ = false;
        }                    
    }
  }
 }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  //同样的，发送消息前都应该检查流是否有错
  if (reader().has_error()) {
    TCPSenderMessage msg;
    msg.seqno = isn_ + next_seqno_;
    msg.RST = true;
    transmit(msg);
    return;
  }
  if (timer_running_) {
    time_since_last_activity_ += ms_since_last_tick;
    
    // 检查是否超时
    if (time_since_last_activity_ >= RTO_ms_ && !outstanding_segments_.empty()) {
      // 重传最早的未确认段
      transmit(outstanding_segments_.front().msg);
      
      // 窗口非零时增加连续重传计数并更新RTO
      if (window_size_ > 0) {
       retransmissions_++;
        RTO_ms_ *= 2;  // 指数退避
      }
      
      // 重置计时器
      time_since_last_activity_ = 0;
    }
  }
}
```

测试结果：

![很坑的测试用例](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/test.png)

## 应用：

按照指导书上的教程最后成功实现了互通。以及 webget 也测试成功了。还是很有趣的。这里可以好好体验一下抓包的<s>乐趣</s>。

![抓包终于结束了](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/通信.png)

## 总结：

建议好好阅读提供的框架代码， tcp_ipv4.cc 实现了一个基于 TCP/IPv4 的网络应用程序框架，用于创建可以在 TUN 虚拟网络接口上运行的 TCP 客户端和服务器。

这里原理和 clash 等代理软件有类似之处。不过 cpp 的那些特性实在太难看了，没心情继续看了 : (



