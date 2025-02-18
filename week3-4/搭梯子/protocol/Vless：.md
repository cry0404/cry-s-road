# Vless：

vless 是 vmess 的轻量级协议，所以是明文的，但是会经过 tls 加密。

这里利用的 xtls + vless + 回落原理来搭建的 。

xtls 相较于 tls 有了选择加密。减少了二次加密的可能。

这里 trojan 只会对本机进行监听，也就是对服务器自身的回落做出响应，对公网 ip 是忽略的。

## 回落的做法：

![image-20250217230907783](C:/Users/15276/AppData/Roaming/Typora/typora-user-images/image-20250217230907783.png)

这里的回落是如果进入服务端的是 trojan 协议的话，那么被 vless 解析后就会代理给对应的回落端口，比如这里的 8388 就是指向 trojan 的，同样的，trojan 也有回落端口，代表解析到不是 trojan 协议的，就返回到 nginx 代理的 web 服务器上。