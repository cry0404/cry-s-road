---

---

##   简介：

完整项目地址：https://github.com/cry0404/minnow

指导书地址：https://cs144.github.io/

lab6 的目的是实现一个路由器，主要是实现其中的最长前缀匹配原则。

![最长匹配](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/router.png)

看到最长前缀匹配我的第一反应是大一下数据结构写过一个查单词的 trie 前缀字典树。但是指导书上给的是——“实现大约在 30-60 行代码”，于是本着能不麻烦就不麻烦的原则（反正这个又不需要效率），并且对于这个小型路由表来说，说不定效率要高一点（，毕竟 trie 在这种情况下可能会有许多空节点，内存效率并不是很高。所以最后就用了线性遍历路由表。

### 注意点：

#### 1. TTL 

**TTL (Time To Live)** 是 IP 头部中的一个关键字段，主要用于防止数据包在网络中无限循环。

#### 实现细节
```cpp
// TTL检查和递减
if (datagram.header.ttl <= 1) {
  // 如果TTL为1或0，丢弃数据报
  continue;
}

// 递减TTL
datagram.header.ttl--;
```

#### 注意事项
- TTL 减少必须先进行边界检查 (≤1)，防止溢出
- 递减 TTL 后必须重新计算 IP 头校验和（不然检测点 36 会疯狂报错)

#### 2. IP 头校验和

IP 协议使用头部校验和来验证数据完整性。当修改头部任何字段（如 TTL）时，必须重新计算校验和。

#### 实现细节
```cpp
// 递减TTL
datagram.header.ttl--;

// 重新计算IP头部校验和
datagram.header.compute_checksum();
```

#### 计算过程
1. 将校验和字段置零
2. 将 IP 头部视为 16 位整数序列
3. 对所有 16 位值求和
4. 将溢出位回卷（加到低位上）
5. 对结果取反码

### 参考代码:

[router,cc](https://github.com/cry0404/minnow/blob/lab7/src/router.cc)

[router.hh](https://github.com/cry0404/minnow/blob/lab7/src/router.hh)

## 测试结果：

![test-lab6](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/test1.png)

## lab 7 ：

lab 7 只是连结了我们之前所做的所有结果，然后来进行实际网路通信。但是需要有别的“实验伙伴”，由于我没有，也懒得多开一台机器来模拟，于是只简单做了个全局测试。

![test-lab7](https://picture.cry4o4n0tfound.cn/crywebsite/2025/04/test2.png)

就这样先完结吧，大概弄懂了百分之60左右，受益最大的地方在于框架设计的代码思路，以及封装的逻辑，cpp 的进步倒是一般般。打算之后再看一看这些框架代码的设计思路，不过最近估计是没有时间了...

完结撒花(oﾟvﾟ)ノ
