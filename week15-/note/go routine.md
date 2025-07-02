从这串代码中看 go routine
```
package main

func concurrentFib(n int) []int {
	result := make([]int, 0)
	//创建对应的结果数组
	ch := make(chan int, n+1)
	go fibonacci(n, ch)
	for item := range ch{
		result = append(result, item)
	}
	return result
}

// don't touch below this line

func fibonacci(n int, ch chan int) {
	x, y := 0, 1
	for i := 0; i < n; i++ {
		ch <- x
		x, y = y, x+y
	}
	close(ch)
}
```
这里只启动了一个 go fibonacci，这个单独的 go routine 负责计算，而另外的则顺利运行下面的 for 循环，负责接收另外一个 go routine 传输过来的值，非常优雅