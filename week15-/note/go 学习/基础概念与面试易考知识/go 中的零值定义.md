---
title: go 中的零值定义
created: 2025-06-25
source: Cherry Studio
tags: 
---
在 Go 语言中，每种类型都有一个与其对应的**零值（Zero Value）**。零值是变量在声明时未被显式初始化时自动获得的值。这个概念非常重要，因为它保证了程序的确定性，并且可以简化代码，因为你不需要为所有变量都提供一个初始值。

不同类型的零值如下：

1.  **布尔类型 (`bool`)**:
    *   零值是 `false`。
    *   当你声明一个 `bool` 变量而不赋值时，它默认为 `false`。

    ```go
    var isReady bool // isReady 的零值是 false
    ```

2.  **数值类型 (整型、浮点型)**:
    *   **整型 (如 `int`, `int8`, `int16`, `int32`, `int64`, `uint`, `uint8`, `uint16`, `uint32`, `uint64`, `uintptr`, `byte` (别名 `uint8`), `rune` (别名 `int32`))**: 零值是 `0`。
    *   **浮点型 (如 `float32`, `float64`)**: 零值是 `0.0`。
    *   **复数类型 (如 `complex64`, `complex128`)**: 零值是 `0+0i`。

    ```go
    var count int // count 的零值是 0
    var price float64 // price 的零值是 0.0
    var result complex128 // result 的零值是 (0+0i)
    ```

3.  **字符串类型 (`string`)**:
    *   零值是空字符串 `""`。

    ```go
    var message string // message 的零值是 ""
    ```

4.  **指针类型 (`*T`)**:
    *   零值是 `nil`。`nil` 是一个特殊的常量，它表示指针、切片、通道、接口、映射和函数类型值的“空”或“不存在”的状态。

    ```go
    type MyStruct struct {
        // ... fields ...
    }
    var ptr *MyStruct // ptr 的零值是 nil
    ```

5.  **切片类型 (`[]T`)**:
    *   零值是 `nil` 切片。一个 `nil` 切片没有底层数组，长度和容量都为 0。
    *   与空切片 (`make([]T, 0)`) 不同，`nil` 切片在某些操作（如 `len()`, `cap()`）上表现相同，但它们不是同一个值。

    ```go
    var data []int // data 的零值是 nil
    ```

6.  **数组类型 (`[N]T`)**:
    *   数组的零值是包含 N 个其元素类型零值的数组。数组的每个元素都会被初始化为其类型的零值。

    ```go
    var numbers [3]int // numbers 的零值是 [0, 0, 0]
    var flags [2]bool  // flags 的零值是 [false, false]
    ```

7.  **结构体类型 (`struct`)**:
    *   结构体的零值是所有字段都为其类型的零值的结构体。Go 会自动为结构体的每个字段初始化为该字段类型的零值。

    ```go
    type Person struct {
        Name string
        Age  int
    }
    var p Person // p 的零值是 {Name: "", Age: 0}
    ```

8.  **映射类型 (`map[K]V`)**:
    *   零值是 `nil` 映射。与 `nil` 指针类似，`nil` 映射是不可写的。尝试向 `nil` 映射写入会导致运行时恐慌（panic）。可以使用 `make(map[K]V)` 来创建一个可写的空映射。

    ```go
    var config map[string]string // config 的零值是 nil
    ```

9.  **通道类型 (`chan T`)**:
    *   零值是 `nil` 通道。与 `nil` 指针和 `nil` 映射类似，`nil` 通道不能进行发送或接收操作，这会导致 goroutine 阻塞。

    ```go
    var msgChan chan string // msgChan 的零值是 nil
    ```

10. **函数类型 (`func(...) ...`)**:
    *   零值是 `nil` 函数。 `nil` 函数指针不能被调用。

    ```go
    var handler func(int) bool // handler 的零值是 nil
    ```

**`nil` 的特殊性：**

*   `nil` 是一个通用的零值，适用于**引用类型**（指针、切片、通道、接口、映射、函数）和表示空状态。
*   对于`nil` 切片和`nil` 映射，虽然它们是零值，但在使用它们时需要特别小心。`len()` 和 `cap()` 可以安全用于 `nil` 切片，返回 0。但是，试图向 `nil` 映射写数据会引发 panic。

理解不同类型的零值对于编写健壮、可预测的 Go 程序至关重要。很多时候，你可以依赖默认的零值初始化，而无需显式地赋值。

