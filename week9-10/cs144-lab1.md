# cs144-lab1:

## 配置环境：

记得做前将代码与之前 lab0 合并一下即可，至于 debug ，本地的所有测试都在 build 中，挨着测试即可。

由于握手环节需要 cmu 的内部网络，所以跳过， ip-raw.cc 我就没有去实现。

以下是 lab1 的实践内容：

> 作为实验作业的一部分，你正在实现一个 TCP 接收器：该模块接收数据报并将其转换为可靠的字节流，以便应用程序可以从套接字中读取——就像你的 webget 程序在检查点 0 中从 web 服务器读取字节流一样。
>
> TCP 发送器将其字节流划分为短段（每个段不超过约 1,460 字节）以便它们可以放入数据报中。但是，网络可能会重新排序这些数据报，或者丢弃它们，或者多次投递它们。接收器必须将这些段重新组装成它们最初开始的连续字节流。
>
> 在这个实验室中，您将编写负责此重组的数据结构：一个重组器。它将接收子字符串，由字节字符串组成，以及该字符串中第一个字节的索引，该索引位于更大的流中。流中的每个字节都有自己的唯一索引，从零开始向上计数。一旦重组器知道流中的下一个字节，它就会将其写入 ByteStream 的写入端——与您在检查点 0 中实现的相同的 ByteStream。重组器的“客户”可以从同一 ByteStream 的读取端读取。

其实我们就是在这里是在实现 tcp 的重组部分，需要联系上一个 lab0 中的 byte_stream 内容。在此之前，或许回顾一下 tcp 的报文结构有助于我们实现 lab1。

![tcp报文结构](https://picture.cry4o4n0tfound.cn/crywebsite/2025/03/tcp.png)

而这里面与我们要实现的 Reassembler 相关联最大的两个字段就是

**序列号 (Sequence Number)**

- 位置：TCP 头部的前 32 位。
- 作用：序列号标识了数据段在整个数据流中的位置。发送端为每个数据段分配一个序列号，接收端根据序列号对数据进行排序和重组。如果数据包乱序到达，接收端会利用序列号重新排列它们。
- 重组相关性：这是重组的核心字段，确保数据按顺序拼接。

**确认号 (Acknowledgment Number)**

- 位置：TCP 头部的第 33-64 位。
- 作用：确认号表示接收端期望接收的下一个字节的序列号。它用于确认已经成功接收的数据，帮助发送端知道哪些数据需要重传。
- 重组相关性：通过确认号，接收端可以间接通知发送端是否存在丢失的数据包，从而辅助重组过程。

如果对于 tcp 不够熟悉，可以阅读这一篇 [blog](https://juanha.github.io/2018/05/05/tcp/)，或者看一看黑书的 tcp 部分也不错。

而我们则需要根据序列号与确认号来构建 Reassembler。

---

## Reassembler:

因为我们的数据包有着对应的编号，也就是模拟我们之前提到的序列号，我们可以使用有序键值对类似 map 或者 set 来实现，我采用的是 map。除此之外，我们需要维护一个 next-index（这里的 next _index 即可理解为对应的确认号）来作为重叠以及是否丢包等的判断依据。

**构造之前一定先阅读 byte_stream_helper ，明白 byte_stream 是如何调用内部派生的 writer 和 reader 接口的。理解 output_.writer() 和 output_.reader() 其实是转换出了对应的派生类。避免了多重继承复杂性，非常优雅，但是对于 c艹 掌握不好的人来说是一种痛苦...**

```cpp
#include "byte_stream.hh"

class Reassembler
{
public:
  // 构建Reassembler以写入给定的ByteStream。
  explicit Reassembler( ByteStream&& output ) : 
  output_( std::move( output ) ),
  pending_data(),
  next_index(0),
  eof_index(0),
  eof_rec(false){}

  /*
   * 插入一个新的子字符串以重新组装到ByteStream中。
   *   `first_index`: 子字符串的第一个字节的索引
   *   `data`: 子字符串本身
   *   `is_last_substring`: 此子字符串表示流的结束
   *   `output`: 对Writer的可变引用
   *
   * Reassembler的任务是将索引的子字符串（可能是乱序或重叠的）重新组装回原始的ByteStream。
   * 一旦Reassembler知道流中的下一个字节，它应该立即将其写入输出。
   *
   * 如果Reassembler了解到字节适合流的可用容量但还不能写入（因为前面的字节仍然未知），
   * 它应该将它们存储在内部，直到填补了空缺。
   *
   * Reassembler应该丢弃任何超出流可用容量的字节
   * （即即使前面的空缺被填补也无法写入的字节）。
   *
   * Reassembler在写入最后一个字节后应关闭流。
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring );

  // Reassembler本身存储了多少字节？
  // 此函数仅用于测试；不要为了支持它而添加额外的状态。
  uint64_t count_bytes_pending() const;

  // 访问输出流读取器
  Reader& reader() { return output_.reader(); }
  const Reader& reader() const { return output_.reader(); }

  // 访问输出流写入器，但仅为const（不能从外部写入）
  const Writer& writer() const { return output_.writer(); }

private:
  ByteStream output_;
  std::map<uint64_t, std::string>     pending_data;// 存储数据的映射
  uint64_t next_index = 0; // 下一个索引
  uint64_t eof_index = 0;
  bool eof_rec = false;
};

```

对于重组器的设置，我们需要考虑到以下因素。

- **乱序**: 缓存并排序。
- **丢失**: 请求重传。
- **重复**: 丢弃冗余。
- **重叠**: 裁剪多余。
- **大小异常**: 截断或丢弃。
- **中断**: 停止重组。

不过在 lab1 中我们不用考虑这么全面，只需要考虑其中的一部分。

而在这里我们需要考虑的是5个值构建出来的区域：

```
uint64_t first_index;
uint64_t len = data.size();
uint64_t next_index;
uint64_t count = count_bytes_pending();
uint64_t available = output_.writer().available_capacity();
```

### 实现：

分析实验指导书上给出的示意图：

![存储结构](https://picture.cry4o4n0tfound.cn/crywebsite/2025/03/storage.png)

于是整个 lab1 中象征存储的部分其实有三个，因为我们在 Reassembler 中并没有涉及到 pop 相关的操作：

```cpp
//两个依据 bytestream 中定义构建出来的数据
total_capacity = output_.writer().available_capacity() + output_.reader().bytes_buffered();

available_space = output_.writer().available_capacity();

//以及我们定义的 pending_data 用来存储还没 push 到 writer 中的值
pending_data
```

所以对于我们需要满足以上的所有逻辑，比如对于 available_space ，其实模拟的是我们的接收窗口 rwnd:

```cpp
 // 如果缓冲区已满，不存储任何未来数据，直接返回
  if (available_space == 0) {
    return; 
 }
```

按照道理来说，我们应该可以存储在 pending_data 中，但实际上，我们需要模仿 tcp 的流量控制，所以不能存储。

> ### TCP流量控制原理
>
> 在TCP协议中，接收窗口(receive window)是一个关键概念：
>
> - 接收窗口大小表示接收方当前愿意接收的数据量
> - 当接收窗口为0时，发送方必须停止发送数据
> - 只有当接收方处理了一些数据并增加了窗口大小，发送方才能继续发送

同样的，我们也需要遵循存储限制：

```cpp
if (new_index > next_index && new_index - next_index + len > total_capacity) {
    if (new_index - next_index >= total_capacity) {
      return;  // 确实超出范围才返回
}
```

### 细节：

>  整个流中第一个字节的索引是多少？Zero.

指导书上简化了我们对于重组的处理，由于第一个传来的是0，而我们默认一定会将其推送出去。比较明显的是，next_index 应该在我们能连续处理数据时才更新。

之后的动作均是检查存储空间是否足够，足够便推送，不够就丢弃的思维来写，需要仔细考虑的部分就是重叠该如何处理，如果有很多重叠数据的话可能会造成很多内存浪费。

完整代码如下:

```cpp
#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring)
{
  debug("unimplemented insert({}, {}, {}) called", first_index, data, is_last_substring);
  
  // note: 一定先判断发过来的字节流类型，以及是否是最后一串字节流
  if (data.empty()) {
    if (is_last_substring) {
      eof_rec = true;
      eof_index = first_index;
      
      if (next_index >= eof_index) {
        output_.writer().close();
      }
    }
    return;
  }

  uint64_t len = data.size();
  
  // 处理EOF标记
  if (is_last_substring) {
    eof_rec = true;
    eof_index = first_index + len;
    
    if (next_index >= eof_index) {
      output_.writer().close();
    }
  }

  // 获取可处理的容量限制
  uint64_t total_capacity = output_.writer().available_capacity() + output_.reader().bytes_buffered();
  uint64_t available_space = output_.writer().available_capacity();
  
  // 如果缓冲区已满，不存储任何未来数据，直接返回
  if (available_space == 0) {
    return; 
  }
  /* note:数据重叠处理是最复杂的部分，因为都避免重复状态，所以都细致思考*/
  // 处理数据重叠部分 - 先处理与前面数据的重叠
  uint64_t new_index = first_index;
  
  // 如果起始位置在已处理数据之前，调整起始位置
  if (first_index < next_index) {
    // 完全重叠，直接丢弃
    if (first_index + len <= next_index) {
      return;
    }
    // 部分重叠，截取未处理部分
    uint64_t offset = next_index - first_index;
    data = data.substr(offset);
    new_index = next_index;
    len = data.size();
  }
  
  // 检查重叠情况 - 与pending_data中已有数据比较
  auto upper_bound = pending_data.upper_bound(new_index);
  if (upper_bound != pending_data.begin()) {
    auto prev = upper_bound;
    --prev;
    
    // 检查与前一个片段的重叠
    if (prev->first <= new_index && new_index < prev->first + prev->second.size()) {
      // 有重叠，调整起始位置
      uint64_t overlap = prev->first + prev->second.size() - new_index;
      if (overlap >= len) {
        // 完全被包含，丢弃
        return;
      }
      data = data.substr(overlap);
      new_index += overlap;
      len = data.size();
    }
  }
  
  // 处理与后续片段的重叠
  auto it = pending_data.lower_bound(new_index);
  while (it != pending_data.end() && new_index + len > it->first) {
    if (it->first < new_index + len) {
      if (new_index + len >= it->first + it->second.size()) {
        // 当前数据完全覆盖了这个片段
        pending_data.erase(it++);
      } else {
        // 部分重叠，截断当前数据
        len = it->first - new_index;
        data = data.substr(0, len);
        break;
      }
    } else {
      break;
    }
  }
  
  // 检查是否超出总容量限制
  if (new_index > next_index && new_index - next_index + len > total_capacity) {
    if (new_index - next_index >= total_capacity) {
      return;  // 确实超出范围才返回
    }
    uint64_t available = total_capacity - (new_index - next_index);
    data = data.substr(0, available);
    len = data.size();
  }

  // 处理数据写入
  if (new_index == next_index) {
    // 检查可用容量
    if (available_space >= len) {
      // 有足够容量，直接写入
      output_.writer().push(data);
      next_index += len;
    } else {
      // 容量不足，只写入能写入的部分，剩余部分丢弃
      string write_data = data.substr(0, available_space);
      output_.writer().push(write_data);
      next_index += available_space;
      // 不存储剩余部分到pending_data
    }
    
    // 查找并处理后续连续片段
    bool found_next = true;
    while (!pending_data.empty() && found_next && output_.writer().available_capacity() > 0) {
      found_next = false;
      auto next_it = pending_data.find(next_index);
      if (next_it != pending_data.end()) {
        // 检查可用容量
        uint64_t avail = output_.writer().available_capacity();
        string pending_data_str = next_it->second;
        
        if (avail >= pending_data_str.size()) {
          // 能全部写入
          output_.writer().push(pending_data_str);
          next_index += pending_data_str.size();
          pending_data.erase(next_it);
          found_next = true;
        } else if (avail > 0) {
          // 只能部分写入，剩余部分丢弃
          string write_part = pending_data_str.substr(0, avail);
          output_.writer().push(write_part);
          next_index += avail;
          pending_data.erase(next_it);
          // 不存储剩余部分
          break; 
        } else {
          break;
        }
      }
    }
  } else if (!data.empty() && new_index > next_index) {
    // 确保new_index与next_index之间的距离加上数据长度不超过总容量
    if (new_index - next_index + len <= total_capacity && available_space > 0) {
      // 存入pending_data
      pending_data[new_index] = data;
    }
    // 否则丢弃数据
  }
  
  // 检查是否需要关闭流
  if (eof_rec && next_index >= eof_index) {
    output_.writer().close();
  }
  return;
}

uint64_t Reassembler::count_bytes_pending() const
{
  debug("unimplemented count_bytes_pending() called");
  uint64_t count = 0;
  for (const auto& pair : pending_data) {
    count += pair.second.size();
  }
  return count;
}
```



### 测试结果:

可以看到测试还算不错。不过由于没有做模块化处理，可能会显得有些赘余，俗称💩山。

![测试结果](https://picture.cry4o4n0tfound.cn/crywebsite/2025/03/test1.png)

