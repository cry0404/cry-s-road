### 1. 链表 (Linked List)

链表是一种线性数据结构，但与数组不同，它不将元素存储在连续的内存位置。链表的每个元素都是一个独立的对象，称为“节点 (Node)”，每个节点包含数据和指向下一个节点的引用。

这是一个单向链表的实现：

Go

```
package main

import "fmt"

// Node 表示链表中的一个节点
type Node[T any] struct {
	Value T
	Next  *Node[T]
}

// LinkedList 表示单向链表
type LinkedList[T any] struct {
	head *Node[T]
	size int
}

// Append 在链表末尾添加一个新节点
func (ll *LinkedList[T]) Append(value T) {
	newNode := &Node[T]{Value: value}
	if ll.head == nil {
		ll.head = newNode
	} else {
		current := ll.head
		for current.Next != nil {
			current = current.Next
		}
		current.Next = newNode
	}
	ll.size++
}

// Insert 在指定位置插入一个新节点
func (ll *LinkedList[T]) Insert(index int, value T) error {
	if index < 0 || index > ll.size {
		return fmt.Errorf("index out of bounds")
	}
	newNode := &Node[T]{Value: value}
	if index == 0 {
		newNode.Next = ll.head
		ll.head = newNode
	} else {
		current := ll.head
		for i := 0; i < index-1; i++ {
			current = current.Next
		}
		newNode.Next = current.Next
		current.Next = newNode
	}
	ll.size++
	return nil
}

// Remove 删除指定位置的节点
func (ll *LinkedList[T]) Remove(index int) error {
	if index < 0 || index >= ll.size {
		return fmt.Errorf("index out of bounds")
	}
	if index == 0 {
		ll.head = ll.head.Next
	} else {
		current := ll.head
		for i := 0; i < index-1; i++ {
			current = current.Next
		}
		current.Next = current.Next.Next
	}
	ll.size--
	return nil
}

// Print 显示链表中的所有元素
func (ll *LinkedList[T]) Print() {
	current := ll.head
	for current != nil {
		fmt.Printf("%v -> ", current.Value)
		current = current.Next
	}
	fmt.Println("nil")
}

func main() {
	// 使用示例
	list := &LinkedList[int]{}
	list.Append(10)
	list.Append(20)
	list.Append(30)
	list.Print() // 输出: 10 -> 20 -> 30 -> nil

	list.Insert(1, 15)
	list.Print() // 输出: 10 -> 15 -> 20 -> 30 -> nil

	err := list.Remove(2)
	if err != nil {
		fmt.Println(err)
	}
	list.Print() // 输出: 10 -> 15 -> 30 -> nil
}
```

### 2. 栈 (Stack)

栈是一种后进先出 (LIFO - Last In, First Out) 的数据结构。在 Go 中，可以很方便地使用切片 (slice) 来实现。

Go

```
package main

import (
	"fmt"
)

// Stack 是一个后进先出的栈
type Stack[T any] struct {
	elements []T
}

// NewStack 创建一个新的栈
func NewStack[T any]() *Stack[T] {
	return &Stack[T]{elements: make([]T, 0)}
}

// Push 将元素压入栈顶
func (s *Stack[T]) Push(element T) {
	s.elements = append(s.elements, element)
}

// Pop 从栈顶弹出一个元素
func (s *Stack[T]) Pop() (T, error) {
	if s.IsEmpty() {
		var zero T
		return zero, fmt.Errorf("stack is empty")
	}
	index := len(s.elements) - 1
	element := s.elements[index]
	s.elements = s.elements[:index]
	return element, nil
}

// Peek 查看栈顶元素但不弹出
func (s *Stack[T]) Peek() (T, error) {
	if s.IsEmpty() {
		var zero T
		return zero, fmt.Errorf("stack is empty")
	}
	return s.elements[len(s.elements)-1], nil
}

// IsEmpty 检查栈是否为空
func (s *Stack[T]) IsEmpty() bool {
	return len(s.elements) == 0
}

// Size 返回栈的大小
func (s *Stack[T]) Size() int {
	return len(s.elements)
}

func main() {
	// 使用示例
	stack := NewStack[string]()
	stack.Push("A")
	stack.Push("B")
	stack.Push("C")

	fmt.Printf("Stack size: %d\n", stack.Size()) // 输出: Stack size: 3

	top, _ := stack.Peek()
	fmt.Printf("Top element: %s\n", top) // 输出: Top element: C

	val, _ := stack.Pop()
	fmt.Printf("Popped: %s\n", val) // 输出: Popped: C

	val, _ = stack.Pop()
	fmt.Printf("Popped: %s\n", val) // 输出: Popped: B

	fmt.Printf("Is empty: %t\n", stack.IsEmpty()) // 输出: Is empty: false
}
```

### 3. 队列 (Queue)

队列是一种先进先出 (FIFO - First In, First Out) 的数据结构。同样，也可以用 Go 的切片来实现。

Go

```
package main

import (
	"fmt"
)

// Queue 是一个先进先出的队列
type Queue[T any] struct {
	elements []T
}

// NewQueue 创建一个新队列
func NewQueue[T any]() *Queue[T] {
	return &Queue[T]{elements: make([]T, 0)}
}

// Enqueue 将元素加入队尾
func (q *Queue[T]) Enqueue(element T) {
	q.elements = append(q.elements, element)
}

// Dequeue 从队首移除一个元素
func (q *Queue[T]) Dequeue() (T, error) {
	if q.IsEmpty() {
		var zero T
		return zero, fmt.Errorf("queue is empty")
	}
	element := q.elements[0]
	q.elements = q.elements[1:]
	return element, nil
}

// Peek 查看队首元素但不移除
func (q *Queue[T]) Peek() (T, error) {
	if q.IsEmpty() {
		var zero T
		return zero, fmt.Errorf("queue is empty")
	}
	return q.elements[0], nil
}

// IsEmpty 检查队列是否为空
func (q *Queue[T]) IsEmpty() bool {
	return len(q.elements) == 0
}

// Size 返回队列的大小
func (q *Queue[T]) Size() int {
	return len(q.elements)
}

func main() {
	// 使用示例
	queue := NewQueue[int]()
	queue.Enqueue(1)
	queue.Enqueue(2)
	queue.Enqueue(3)

	fmt.Printf("Queue size: %d\n", queue.Size()) // 输出: Queue size: 3

	front, _ := queue.Peek()
	fmt.Printf("Front element: %d\n", front) // 输出: Front element: 1

	val, _ := queue.Dequeue()
	fmt.Printf("Dequeued: %d\n", val) // 输出: Dequeued: 1

	val, _ = queue.Dequeue()
	fmt.Printf("Dequeued: %d\n", val) // 输出: Dequeued: 2

	fmt.Printf("Is empty: %t\n", queue.IsEmpty()) // 输出: Is empty: false
}
```

### 4. 集合 (Set)

集合是一个存储唯一元素且无序的数据结构。在 Go 中，通常使用 `map` 来模拟集合的行为，`key` 存储元素，`value` 存储一个空的 `struct{}` 以节省空间。

Go

```
package main

import "fmt"

// Set 是一个存储唯一元素的集合
type Set[T comparable] struct {
	items map[T]struct{}
}

// NewSet 创建一个新集合
func NewSet[T comparable]() *Set[T] {
	return &Set[T]{items: make(map[T]struct{})}
}

// Add 向集合中添加一个元素
func (s *Set[T]) Add(element T) {
	s.items[element] = struct{}{}
}

// Remove 从集合中移除一个元素
func (s *Set[T]) Remove(element T) {
	delete(s.items, element)
}

// Contains 检查集合中是否存在某个元素
func (s *Set[T]) Contains(element T) bool {
	_, exists := s.items[element]
	return exists
}

// Elements 返回集合中所有的元素（以切片形式）
func (s *Set[T]) Elements() []T {
	elements := make([]T, 0, len(s.items))
	for item := range s.items {
		elements = append(elements, item)
	}
	return elements
}

// Size 返回集合的大小
func (s *Set[T]) Size() int {
	return len(s.items)
}

func main() {
	// 使用示例
	set := NewSet[string]()
	set.Add("apple")
	set.Add("banana")
	set.Add("apple") // 重复添加，不会有效果

	fmt.Printf("Set size: %d\n", set.Size()) // 输出: Set size: 2
	fmt.Printf("Contains 'banana': %t\n", set.Contains("banana")) // 输出: Contains 'banana': true
	fmt.Printf("Contains 'orange': %t\n", set.Contains("orange")) // 输出: Contains 'orange': false

	set.Remove("apple")
	fmt.Println("Elements:", set.Elements()) // 输出: Elements: [banana] (顺序可能不同)
}
```

### 5. 哈希表 / 映射 (Hash Table / Map)

Go 语言内置了 `map` 类型，它就是哈希表的实现。它的使用非常直接，是 Go 语言的核心特性之一。

Go

```
package main

import "fmt"

func main() {
	// 声明并初始化一个 map
	// 键是 string 类型，值是 int 类型
	ages := make(map[string]int)

	// 添加键值对
	ages["Alice"] = 30
	ages["Bob"] = 25
	ages["Charlie"] = 35

	fmt.Println("Bob's age:", ages["Bob"]) // 输出: Bob's age: 25

	// 获取一个不存在的键会返回该值类型的零值
	fmt.Println("David's age:", ages["David"]) // 输出: David's age: 0

	// 使用 "comma ok" idiom 来检查键是否存在
	age, ok := ages["David"]
	if !ok {
		fmt.Println("David's age is not in the map.") // 输出: David's age is not in the map.
	} else {
		fmt.Println("David's age is", age)
	}
	
	// 删除一个键值对
	delete(ages, "Charlie")
	fmt.Println("Map after deletion:", ages) // 输出: Map after deletion: map[Alice:30 Bob:25]

	// 遍历 map (注意：遍历顺序是不保证的)
	for name, age := range ages {
		fmt.Printf("%s is %d years old.\n", name, age)
	}
}
```

### 6. 二叉搜索树 (Binary Search Tree)

二叉搜索树 (BST) 是一种特殊的二叉树，它满足以下性质：

- 任何节点的左子树中的所有键都小于该节点的键。
    
- 任何节点的右子树中的所有键都大于该节点的键。
    
- 左右子树也都是二叉搜索树。
    



```
package main

import "fmt"

// TreeNode 定义了树的节点
type TreeNode[T comparable] struct {
	Value T
	Left  *TreeNode[T]
	Right *TreeNode[T]
}

// BinarySearchTree 定义了二叉搜索树
type BinarySearchTree[T comparable] struct {
	Root *TreeNode[T]
    less func(a, b T) bool // 用于比较元素的函数
}

// NewBinarySearchTree 创建一个新的BST
func NewBinarySearchTree[T comparable](lessFunc func(a, b T) bool) *BinarySearchTree[T] {
    return &BinarySearchTree[T]{less: lessFunc}
}

// Insert 将一个值插入到树中
func (bst *BinarySearchTree[T]) Insert(value T) {
	bst.Root = bst.insertRec(bst.Root, value)
}

func (bst *BinarySearchTree[T]) insertRec(node *TreeNode[T], value T) *TreeNode[T] {
	if node == nil {
		return &TreeNode[T]{Value: value}
	}

	if bst.less(value, node.Value) {
		node.Left = bst.insertRec(node.Left, value)
	} else if bst.less(node.Value, value) {
		node.Right = bst.insertRec(node.Right, value)
	}
	// 如果值相等，则不插入（也可以根据需求更新节点）

	return node
}

// Search 查找一个值是否存在于树中
func (bst *BinarySearchTree[T]) Search(value T) bool {
	return bst.searchRec(bst.Root, value)
}

func (bst *BinarySearchTree[T]) searchRec(node *TreeNode[T], value T) bool {
	if node == nil {
		return false
	}
	if bst.less(value, node.Value) {
		return bst.searchRec(node.Left, value)
	}
	if bst.less(node.Value, value) {
		return bst.searchRec(node.Right, value)
	}
	return true // 值相等
}

// InOrderTraversal 中序遍历 (会按排序顺序输出)
func (bst *BinarySearchTree[T]) InOrderTraversal() {
	inOrderRec(bst.Root)
	fmt.Println()
}

func inOrderRec[T comparable](node *TreeNode[T]) {
	if node != nil {
		inOrderRec(node.Left)
		fmt.Printf("%v ", node.Value)
		inOrderRec(node.Right)
	}
}


func main() {
    // 使用示例
    lessFunc := func(a, b int) bool { return a < b }
	bst := NewBinarySearchTree[int](lessFunc)
	
	elements := []int{8, 3, 10, 1, 6, 14, 4, 7, 13}
	for _, el := range elements {
		bst.Insert(el)
	}

	fmt.Print("In-order traversal: ")
	bst.InOrderTraversal() // 输出: In-order traversal: 1 3 4 6 7 8 10 13 14 

	fmt.Printf("Search for 6: %t\n", bst.Search(6)) // 输出: Search for 6: true
	fmt.Printf("Search for 99: %t\n", bst.Search(99))// 输出: Search for 99: false
}
```

