# wsl连接到vmware进行的csapp实验实践：

本打算直接以wsl进行一系列csapp实验，但经过l1uyun的建议后决定采用wslssh连接到vmware来进行csapp实验，这样方便快照备份的同时也减少了在vmware上切换界面的烦恼，何乐而不为。

首先需要注意的是，由于wsl的nat模式下并不支持localhost代理，大概率在下载完后会反复弹出来这样一句：

```bash
检测到 localhost 代理配置，但未镜像到 WSL。NAT 模式下的 WSL 不支持 localhost 代理。
```

但是这在这里并不是很重要，我们只需要wsl作为一个ssh的机器罢了。

具体内容请参考：[双系统的终极解决方案](https://xiashuo.xyz/posts/devops/linux/wsl/#%E7%BD%91%E7%BB%9C%E8%AE%BE%E7%BD%AE---%E9%87%8D%E7%82%B9)

## 之后就只需要解决ssh的问题：

### 在虚拟机中操作

1. **确认 SSH 服务安装并启动**：如果虚拟机中没有安装 SSH 服务，在虚拟机终端中输入以下命令进行安装2：

```plaintext
sudo apt update
sudo apt upgrade
sudo apt install openssh-server
```

安装完成后，通过`sudo service ssh status`检查 SSH 服务是否启动成功。

2. **检查防火墙设置**：如果虚拟机启用了防火墙，需要确保 SSH 服务的默认端口 22 是开放的。以 Ubuntu 为例，使用以下命令检查当前防火墙规则：

```plaintext
sudo ufw status
```

如果 22 端口被防火墙阻止，执行以下命令开放该端口：

```plaintext
sudo ufw allow 22
```

### 在 WSL 中操作

1. **安装 SSH 客户端**：如果 WSL 中没有安装 SSH 客户端，在 WSL 的终端中输入`sudo apt-get install openssh-client`进行安装2。
2. **获取 WSL 的 IP 地址**：在 WSL 中输入`ifconfig`或`ip addr`命令查看 WSL 的 IP 地址，确保 WSL 与虚拟机处于同一网络段1。
3. **连接虚拟机**：在 WSL 的终端中输入`ssh username@192.168.174.130`，其中`username`是虚拟机的用户名，然后按回车键，根据提示输入虚拟机的登录密码即可连接到虚拟机2。



## 挂载本地目录到虚拟机内：

在官网下载对应lab.tar文件后，将其复制到虚拟机内

```bash
cp -r /mnt/hgfs/lab /some/other/place #wsl中下载的ubuntu并不完全隔离
```

解压完成后就可以开始实验了