---

---

##  简介：

lab 5 是我们迈向链路层的第一步，如指导书中给出的图所示：

![架构](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/architecture.png)

### tcp 的报文机制：

指导书上介绍了三种 tcp 的传输方式：

- TCP-in-UDP-in-IP。TCP 段可以承载在用户数据报的负载中。在正常（用户空间）设置中工作，这最容易实现：Linux 提供了一个接口（一个“数据报套接字”，UDPSocket），允许应用程序仅提供用户数据报的负载和目标地址，内核负责构建 UDP 头、IP 头和以太网头，然后将数据包发送到适当的下一跳。内核确保每个套接字都有一个独特的本地和远程地址以及端口号的组合，并且由于内核是写入 UDP 和 IP 头的，它可以保证不同应用程序之间的隔离。quic 协议就利用了类似的概念，但是实现不同 。

- TCP-in-IP. 在日常使用中，TCP 段几乎总是直接放置在互联网数据报中，IP 和 TCP 头部之间没有 UDP 头部。这就是人们所说的“TCP/IP”。这实现起来稍微有点困难。Linux 提供了一个名为 TUN 设备的接口，允许应用程序提供整个互联网数据报，内核负责其余部分（写入以太网头部，实际上通过物理以太网卡发送等）。但现在应用程序必须自己构建完整的 IP 头部，而不仅仅是有效载荷。clash 和 v2rayn 等代理软件的实现就是基于此，在应用程序部分实现了一部分操作系统网络协议栈的部分。例如 clash.meta 内核里面的 fake ip 等功能。

- TCP-in-IP-in-Ethernet. 在上述方法中，我们仍然依赖于 Linux 内核的部分网络栈。每次你的代码将 IP 数据报写入 TUN 设备时，Linux 都必须构建一个适当的链路层（以太网）帧，其中 IP 数据报作为有效载荷。这意味着 Linux 必须根据下一跳的 IP 地址确定下一跳的以太网目标地址。如果它不知道这个映射，Linux 会广播一个查询，询问“谁声明了以下 IP 地址？你的以太网地址是什么？”并等待响应。

  而第三个则是 lab5 中要完成的内容，相对来说层之间的隔离很好，有利于封装。

### arp 协议的内容：

我们将在 lab 5 中实现网络接口，以及对应有关的 arp 协议相关内容。在此之前可以来回顾一下 arp 的内容：

> ARP（Address Resolution Protocol，地址解析协议）是用于将网络层地址（如IP地址）解析为链路层地址（如MAC地址）的协议。它主要用于在局域网（LAN）中，通过已知的IP地址来获取对应的MAC地址，以便设备之间能够进行数据帧的传输。

arp 本质上只是在进行特殊的链路层封装，在这里我们尤其需要注意的是 arp 请求不能太过频繁,不然发送过多会淹没整个网络。除此之外，arp 中的 opcode 也是我们这次 lab 中需要注意的点：

- ARP 请求（opcode = 1） 是最常见的 ARP 操作类型。当一台设备需要知道某个 IP 地址对应的 MAC 地址时，它会广播发送一个 ARP 请求。这个请求会询问："谁有 IP 地址 X.X.X.X？请告诉你的 MAC 地址。"  

- ARP 应答（opcode = 2） 当设备收到 ARP 请求，并且发现请求的 IP 地址就是自己时，它会发送 ARP 应答。这个应答包含自己的 MAC 地址，直接回复给请求方。  

上面的两种就是 lab5 中定义的 OPCODE_REQUEST 和 OPCODE_BROADCAST。

而下面的两种 arp 请求已经基本被 dhcp 所取代了。

- RARP 请求（opcode = 3） RARP 是反向 ARP，现已较少使用。RARP 请求用于设备知道自己的 MAC 地址但不知道 IP 地址时，它会询问："我的 MAC 地址是 XX:XX:XX:XX:XX:XX，我的 IP 地址是什么？  

- RARP 应答（opcode = 4） 这是对 RARP 请求的响应，服务器会告诉客户端它应该使用的 IP 地址。 

对于 arp 的地址解析，我们需要维护两个表，一个表映射对应 mac 和 ip，有点类似于 NAT 的实现，除此之外，还需要维护一个缓存时间的表，以免狂发 arp 请求。实际上，dhcp 中也有类似的缓存时间表，用来分配 ip。

同时 lab 中用了很多封装程度很高的结构，parser，arp_message，internetdatagram 等结构，读起来很麻烦，如果只是简单的调用封装好的接口的话确实很容易就能写完，但是事后完全不知道发生了什么...

所以建议边写边去看调用接口的背后到底是怎么写的，然后就又能见识到一些高级<s>可恶</s>的 c++ 特性。

## 实现：

```cpp
#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  const uint32_t next_hop_ip = next_hop.ipv4_numeric();
  const auto it = arp_table_.find(next_hop_ip);
  //先在对应的 arp_table 中查找，如果查找到直接发送
  if (it != arp_table_.end() && it->second.expiry_time > current_time_) {
    EthernetFrame frame;
    frame.header.dst = it->second.eth_addr;
    frame.header.src = ethernet_address_;
    frame.header.type = EthernetHeader::TYPE_IPv4;
    //这里需要查看 util 中的 parser.hh 中定义的序列，相当于是封装了 ip 层到链路层帧的转换这一过程
    Serializer serializer;
    dgram.serialize(serializer);
    frame.payload = serializer.finish();

    transmit(frame);
  } else {
    if (arp_request_time_.find(next_hop_ip) == arp_request_time_.end() || 
        current_time_ - arp_request_time_[next_hop_ip] >= ARP_REQUEST_TIMEOUT_) {
        
        ARPMessage arp_request;

        //这里对应的是发送出去的 opcode
        arp_request.opcode = ARPMessage::OPCODE_REQUEST;
        arp_request.sender_ethernet_address = ethernet_address_;
        arp_request.sender_ip_address = ip_address_.ipv4_numeric();
        arp_request.target_ethernet_address = {};
        arp_request.target_ip_address = next_hop_ip;
        //根据 ip 中的信息构建链路帧
        EthernetFrame frame;
        frame.header.dst = ETHERNET_BROADCAST;
        frame.header.src = ethernet_address_;
        //确定是 arp 还是 ipv4
        frame.header.type = EthernetHeader::TYPE_ARP;

        Serializer serializer;
        arp_request.serialize(serializer);
        frame.payload = serializer.finish();

        transmit(frame);
        arp_request_time_[next_hop_ip] = current_time_;
    }

    // 存储待发送的数据报及其添加时间
    pending_dgram_info_[next_hop_ip].emplace(current_time_, dgram);
  }
}


void NetworkInterface::recv_frame( EthernetFrame frame )
{
  //如果不是发给本地的包或者不是广播包的话直接丢弃
  if (frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST) {
    return;
  }

  //如果是 ipv4 包则直接进行处理
  if (frame.header.type == EthernetHeader::TYPE_IPv4) {
    //这里需要阅读 parser.hh 的逻辑
    Parser parser{std::move(frame.payload)};
    InternetDatagram dgram;
    dgram.parse(parser);

    if (!parser.has_error()) {
      datagrams_received_.push(std::move(dgram));
    }
  } else if (frame.header.type == EthernetHeader::TYPE_ARP) {
    Parser parser{std::move(frame.payload)};
    ARPMessage arp_msg;
    arp_msg.parse(parser);

    if (parser.has_error() || !arp_msg.supported()) {
      return;
    }
    //处理对应是 arp 请求的，需要先解析
    const uint32_t sender_ip = arp_msg.sender_ip_address;
    const EthernetAddress sender_mac = arp_msg.sender_ethernet_address;

    arp_table_[sender_ip] = {sender_mac, current_time_ + ARP_TIMEOUT_};
    arp_request_time_.erase(sender_ip);
    
    if (arp_msg.opcode == ARPMessage::OPCODE_REQUEST &&
        arp_msg.target_ip_address == ip_address_.ipv4_numeric()) {
        ARPMessage arp_reply;
        arp_reply.opcode = ARPMessage::OPCODE_REPLY;
        arp_reply.sender_ethernet_address = ethernet_address_;
        arp_reply.sender_ip_address = ip_address_.ipv4_numeric();
        arp_reply.target_ethernet_address = sender_mac;
        arp_reply.target_ip_address = sender_ip;

        EthernetFrame reply_frame;
        reply_frame.header.dst = sender_mac;
        reply_frame.header.src = ethernet_address_;
        reply_frame.header.type = EthernetHeader::TYPE_ARP;

        Serializer serializer;
        arp_reply.serialize(serializer);
        reply_frame.payload = serializer.finish();

        transmit(reply_frame);
    }

    // 检查是否有等待发送到此 IP 的数据报
    auto pending_it = pending_dgram_info_.find(sender_ip);
    if (pending_it != pending_dgram_info_.end() && !pending_it->second.empty()) {
      auto& pending_queue = pending_it->second;
      
      // 只发送未过期的数据报（添加时间 + ARP_REQUEST_TIMEOUT_ > current_time_）
      while (!pending_queue.empty()) {
        auto [timestamp, dgram] = pending_queue.front();
        pending_queue.pop();
        
        // 检查数据报是否过期（超过 ARP_REQUEST_TIMEOUT_ 未收到响应）
        if (current_time_ - timestamp < ARP_REQUEST_TIMEOUT_) {
          
          EthernetFrame data_frame;
          data_frame.header.dst = sender_mac;
          data_frame.header.src = ethernet_address_;
          data_frame.header.type = EthernetHeader::TYPE_IPv4;
          
          Serializer serializer;
          dgram.serialize(serializer);
          data_frame.payload = serializer.finish();
          
          transmit(data_frame);
        }
        // 过期的数据报会被丢弃（什么都不做，只从队列中移除）
      }
      
      // 移除空队列
      pending_dgram_info_.erase(sender_ip);
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  current_time_ += ms_since_last_tick;

  // 处理 ARP 表项过期
  for (auto it = arp_table_.begin(); it != arp_table_.end(); ) {
    if (current_time_ >= it->second.expiry_time) {
      it = arp_table_.erase(it);
    } else {
      ++it;
    }
  }

  // 处理 ARP 请求超时和重发
  for (auto it = arp_request_time_.begin(); it != arp_request_time_.end(); ) {
    const uint32_t next_hop_ip = it->first;
    const uint64_t last_request_time = it->second;
    
    // 如果自上次请求以来已超过 ARP_REQUEST_TIMEOUT_
    if (current_time_ - last_request_time >= ARP_REQUEST_TIMEOUT_) {
      // 检查是否有尚未过期的待发送数据报
      bool has_valid_pending_datagrams = false;
      auto pending_it = pending_dgram_info_.find(next_hop_ip);
      
      if (pending_it != pending_dgram_info_.end()) {
        // 创建一个临时队列来保存未过期的数据报
        std::queue<std::pair<uint64_t, InternetDatagram>> valid_datagrams;
        
        // 检查所有待发送的数据报
        while (!pending_it->second.empty()) {
          auto [timestamp, dgram] = pending_it->second.front();
          pending_it->second.pop();
          
          // 如果数据报未过期，保留它
          if (current_time_ - timestamp < ARP_REQUEST_TIMEOUT_) {
            valid_datagrams.push({timestamp, std::move(dgram)});
            has_valid_pending_datagrams = true;
          }
          // 过期的数据报会被丢弃
        }
        
        // 用未过期的数据报替换原队列
        pending_it->second = std::move(valid_datagrams);
        
        // 如果没有有效的数据报，移除整个条目，相当于去掉过期了
        if (pending_it->second.empty()) {
          pending_dgram_info_.erase(pending_it);
        }
      }
      
      // 如果仍有未过期的数据报，重新发送 ARP 请求
      if (has_valid_pending_datagrams) {
        ARPMessage arp_request;
        arp_request.opcode = ARPMessage::OPCODE_REQUEST;
        arp_request.sender_ethernet_address = ethernet_address_;
        arp_request.sender_ip_address = ip_address_.ipv4_numeric();
        arp_request.target_ethernet_address = {};
        arp_request.target_ip_address = next_hop_ip;
        
        EthernetFrame frame;
        frame.header.dst = ETHERNET_BROADCAST;
        frame.header.src = ethernet_address_;
        frame.header.type = EthernetHeader::TYPE_ARP;
        
        Serializer serializer;
        arp_request.serialize(serializer);
        frame.payload = serializer.finish();
        
        transmit(frame);
        
        // 更新请求时间
        it->second = current_time_;
        ++it;
      } else {
        // 如果没有未过期的数据报，移除 ARP 请求条目
        it = arp_request_time_.erase(it);
      }
    } else {
      ++it;
    }
  }
}

```

测试结果：

![测试结果](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/test5.png)
