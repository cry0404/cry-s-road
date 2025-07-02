---
title: docker大学习：1、容器的底层
date: 2025-06-13 12:00:00
categories: docker
tags: 学习
---

## 简介：

​	虽然已经享受了很久容器化技术带来的便捷，但其实并没有系统性地学习过 docker 的底层原理，恰逢苹果最近也推出了更适合于 macos 下的 container 。就借此机会，打算以本文来记述我的 docker 大学习。仅供参考，可能还会有很多错误，大概会不断修订完善。

---

## 从 cgroup 和 namespace 说起：

> A container is a standard unit of software that packages up code and all  its dependencies so the application runs quickly and reliably from one  computing environment to another.        												-- [Docker](https://www.docker.com/resources/what-container/)
>

### 虚拟机与容器：

​	一般来讲，启动一个虚拟机总是比启动一个容器要慢的多的（这里就不要拿国内拉取镜像再启动的速度来对比啦...)。

​	这源于两者的隔离机制不同，或者说两者的抽象层实现方式不同。

#### 虚拟机：

​	虚拟机一般使用 hypervisor 模拟出完整的硬件层，所以每个虚拟机内部有完整的操作系统，有自己的内核，文件系统以及用户空间。

![how vms work](https://storage.googleapis.com/qvault-webapp-dynamic-assets/course_assets/QS4HGNG.png)

#### 容器：

​	**容器 (Container)** 的隔离机制则是基于 **宿主机的操作系统内核** 实现的：

![how container works](https://storage.googleapis.com/qvault-webapp-dynamic-assets/course_assets/fmZG1Zd.png)

容器不是模拟硬件，也不是运行一个完整的 Guest OS。它利用宿主机的 Linux 内核提供的两个核心功能来实现隔离和资源管理：

1. **Namespace (命名空间):** 提供隔离性。
2. **cgroup (Control Groups - 控制组):** 提供资源限制和管理。

------

##### 1. Namespace (命名空间)

**作用：** 隔离进程的视图。它让容器内的进程感觉自己拥有独立的系统资源，但实际上这些资源是宿主机共享的，只是通过 Namespace 进行了隔离。**简单来讲，namespace 限制了容器的世界，使得容器像井底之蛙一样，只能通过有限的井口看外面的世界。**

**如何工作：** 当一个进程被创建时，它可以被分配到不同的命名空间中。一旦进入某个命名空间，它就只能看到该命名空间内的资源。

**默认情况下，子进程会复制父进程的 namespace，我们可以用 `lsns` 来查看宿主机上的 namespace，然后将其与一个容器的 namespace 做对比。**

**常见的6种 Namespace 类型：**

- **Mount Namespace (MNT):** 隔离文件系统挂载点。容器有自己的文件系统视图，就像拥有独立的根目录一样。宿主机的 `/` 目录在容器内是不可见的，容器看到的 `/` 是其独立的容器镜像内容加上可能的挂载卷。
- **PID Namespace (PID):** 隔离进程 ID。容器内的进程有一个独立的进程树，其内部的第一个进程 PID 通常是 1。容器外部的进程在容器内是不可见的，反之亦然。宿主机上的 PID 1 进程在容器内看不到，容器内的 PID 1 进程在宿主机上可能对应一个不同的 PID。
- **Network Namespace (NET):** 隔离网络接口、IP 地址、路由表、端口等。每个容器可以有自己独立的网络栈，拥有自己的 IP 地址和网络配置。
- **User Namespace (USER):** 隔离用户和组 ID。容器内的 `root` 用户在宿主机上可能只是一个普通用户，这增加了安全性，即使容器内的 `root` 权限被攻陷，攻击者也无法直接获取宿主机的 `root` 权限。
- **UTS Namespace (UTS):** 隔离主机名和域名。容器可以有自己的主机名，即使宿主机的主机名不同。
- **IPC Namespace (IPC):** 隔离进程间通信 (IPC) 资源，如信号量、消息队列和共享内存。保证容器间的 IPC 不会相互干扰。

##### 2. cgroup (Control Groups - 控制组)

**作用：** 限制、测量和隔离一组进程的系统资源使用，如 CPU、内存、硬盘 I/O 和网络带宽。**cgroup 则是限制了容器可以使用的资源，类似于财政管理。**

**如何工作：** cgroup 将进程组织成一个层级结构。在该层级结构的节点上，可以设置各种资源限制和管理策略。属于同一个 cgroup 的进程会共享该 cgroup 设置的资源限制。

**cgroup 提供的子系统 (Subsystems/Controllers):**

- **cpu:** 限制 CPU 的使用时间（例如，分配一定的 CPU 份额或规定最大使用率）。
- **cpuacct:** 统计 cgroup 中进程的 CPU 使用时间。
- **memory:** 限制内存的使用量。当达到限制时，系统可能会触发 OOM (Out of Memory) 机制。
- **blkio:** 限制块设备 (如硬盘) 的 I/O 速率。
- **net_cls / net_prio:** 标记网络数据包，以便通过流量控制工具来限制网络带宽。
- **devices:** 控制哪些进程可以访问哪些设备文件。
- **pids:** 限制 cgroup 中可以创建的进程数量。

---

#### 实际的运行：

​	用 `docker run -it --rm --name b1 busybox`启动一个简单的 busybox ，我们可以用 `docker inspect｜grep -i pid` 来查看有关的进程。

![macos 上](https://picture.cry4o4n0tfound.cn/2025/06/2a794bb9ee1c79c05d8c769ddde5c99a.jpg)

由于 macOS 是基于 bsd 的，所以 docker 其实是运行在本地的一个轻量级 linux 虚拟机上，无法像 linux 一样直接寻找到对应的宿主机中的容器进程。

![在 linux 虚拟机上](https://picture.cry4o4n0tfound.cn/2025/06/e0acad9d1f9946c803b58405b3544cab.jpg)

而在 linux 上一路溯源，我们能发现容器中的 sh 这个进程归根结底还是由 1 也就是 

​	`root           1       0  0 00:44 ?        00:00:03 /sbin/init`

 启动的，而在大多数，包括我的这个试验机 ubuntu 上面，一般都是启动我们的 `systemd`。

借此我们更深入了解到容器是进程这一概念。

继续看 namespace 之间的对比，对于容器本身：

​	![容器](https://picture.cry4o4n0tfound.cn/2025/06/ab653aa24c02986d2cf59c60a32c0471.jpg)

再看宿主机，可以很明显的对比出，除了类似于 user 和 time 这种 namespace 外，pid 以及 cgroup 等的 ns 是不同的

​	![宿主机](https://picture.cry4o4n0tfound.cn/2025/06/199d04695cdd53d689d4728378a622d5.jpg)

除此之外，我们可以通过 Cgroup 来限制容器使用的资源，包括内存，cpu 的份额等，修改 cgroup 目录下  `cpu_cfs_period_us(调度周期)` 、 `cpu_shares(cpu 分配比例)` 来限制 dokcer 的值。

当然，docker 本身启动时也提供了对应的参数来给予限制，例如：

限制 cpu 的份额：

```bash
docker run -it --name b1 --cpu-quota 200000 busybox 
```

限制内存的份额：

所以可以把某些不怎么活跃的容器限制到 swap 空间内，但需要注意的是，**超过默认设置的 memory 值执行的默认命令是 kil 当前的容器，需要人为的设定超出的逻辑。**

```bash
docker run -it --name b1 --memory 6m --memory-swap 10m busybox
```

---

## docker 的文件：

​	使用 `docker info` ，可以看到 docker 使用的文件系统是 `Storage Driver: overlay2` , 这是 docker 可以构建镜像的关键助力之一，它与 Dockerfile 的书写息息相关。

​	overlay2 和 AUFS 类似，它将所有目录称之为层（layer），overlay2 的目录是镜像和容器分层的基础，而把这些层统一展现到同一的目录下的过程称为联合挂载（union mount）。overlay2 把目录的下一层叫作lowerdir，上一层叫作upperdir，联合挂载后的结果叫作merged。总体来说，overlay2 是这样储存文件的：overlay2将镜像层和容器层都放在单独的目录，并且有唯一 ID，每一层仅存储发生变化的文件，最终使用联合挂载技术将容器层和镜像层的所有文件统一挂载到容器中，使得容器中看到完整的系统文件。

> overlay2 文件系统最多支持 128 个层数叠加，也就是说你的 Dockerfile 最多只能写 128 行

### **分层与挂载的核心机制**

overlay2 的运作依赖**镜像层**与**容器层**的分工协作。镜像层（`lowerdir`）是只读的静态数据，每一层对应 Docker 镜像构建中的一个步骤（如基础镜像、软件安装、配置追加等），存储在 `/var/lib/docker/overlay2` 下，以哈希 ID 为唯一标识。容器启动时，会在镜像层顶部动态生成一个**可写层**（`upperdir`），所有运行时文件增删改操作均在此层记录。两者的内容通过联合挂载（Union Mount）技术合并为统一视图（`merged`），使得容器内的进程如同操作一个完整的文件系统。

------

### **写时复制与文件操作细节**

当容器尝试修改镜像层中的文件时，overlay2 触发**写时复制（CoW）**机制（操作系统的知识终于印照到现实了！)：原始文件从镜像层复制到容器层后完成修改，镜像层内容保持不变。这种设计既保护了镜像的不可变性，又实现了多容器共享同一镜像时的资源节省。

读取文件时，overlay2 按“自上而下”的路径查找：先在容器层（`upperdir`）检索文件的最新版本，若不存在则逐级向镜像层（`lowerdir`）回溯。删除操作通过生成以 `.wh.` 为前缀的**白化文件**标记，覆盖镜像层中的对应文件，逻辑上使该文件在 `merged` 视图中“消失”。需要注意的是，重命名等原子操作可能跨越层级，导致上下层文件共存，实际场景中需结合应用逻辑规避潜在问题。

---

## 容器与镜像以及容器文件系统：

​	简单来讲，镜像(image)是指静态的文件，docker images看到的结果。容器(container)是docker run 命令执行镜像中预先设定好的程序而生成的进程，docker ps 看到的结果。以操作系统的知识来描述的话：**二者之间的关系类似于程序和进程。**

![层的对应](/Volumes/repositories/workspace/writing/image//image-20250622221828819.png)

这里的 init 层主要提供一些映射的关系，主要是针对 resolv（也就是 dns ）、hostname、hosts 做相关的挂载映射，保证与宿主机的一致。

也正是因为 init 层是个性化的针对每个宿主机的内容，所以在 commit 打包一个新镜像是不会j将 init 层打包进去的。

![image-20250622224816819](/Volumes/repositories/workspace/writing/image//image-20250622224816819.png)
