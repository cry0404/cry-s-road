[context 实现原理解读](https://mp.weixin.qq.com/s?__biz=MzkxMjQzMjA0OQ==&mid=2247483677&idx=1&sn=d1c0e52b1fd31932867ec9b1d00f4ec2)
Context 经常是方法中的首个参数，初学时会感到困惑（比如我）
它的主要作用是实现并发协调以及对 goroutine 的生命周期控制
## Context 包中的 Context 定义

```
type Context interface {
	Deadline() (deadline time.Time, ok bool)    
    Done() <-chan struct{}   //作为一个信号发射的方式 
    Err() error    
    Value(key any) any
}
```
- • Deadline：返回 context 的过期时间；
    
- • Done：返回 context 中的 channel；
    
- • Err：返回错误；
    
- • Value：返回 context 中的对应 key 的值.

Context. Background ()

![[Pasted image 20250701121314.png]]