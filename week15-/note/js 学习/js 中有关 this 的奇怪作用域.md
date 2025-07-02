---
title: js 学习
created: 2025-06-17
source: Cherry Studio
tags: 
---

### `this` 的核心概念

*   `this` 是一个**关键字**，它的值是一个**对象**。
*   `this` 的值在函数**执行时（运行时）**确定，而不是在函数定义时确定。
*   `this` 指向调用该函数的那个对象。它“指向”的是当前代码执行的上下文对象。

### `this` 的五种绑定规则（或确定方式）

理解 `this` 的关键在于掌握函数被调用的方式。不同的调用方式决定了 `this` 的值。

#### 1. 默认绑定 (Default Binding)

当函数独立调用，没有任何明确的调用者时，`this` 会被绑定到**全局对象**。

*   **浏览器环境中：** `window` 对象
*   **Node.js 环境中：** `global` 对象
*   **严格模式 (`'use strict'`) 下：** `this` 会被绑定到 `undefined`。这是为了防止意外地污染全局对象。

**示例：**

```javascript
function showThis() {
    console.log(this);
}

// 独立调用
showThis(); // 在浏览器中输出 window 对象，在 Node.js 中输出 global 对象 (非严格模式)

// 严格模式下的默认绑定
function showStrictThis() {
    'use strict';
    console.log(this);
}
showStrictThis(); // 输出 undefined (严格模式)

// 立即执行函数 (IIFE)
(function() {
    console.log(this); // 在浏览器中输出 window (非严格模式)
})();
```

**重要提示：** 即使是对象内部的方法，如果它被“提取”出来独立调用，也会应用默认绑定。这是回调函数问题中 `this` 丢失的根本原因。

```javascript
const obj = {
    name: "MyObject",
    method: function() {
        console.log(this.name);
    }
};

const extractedMethod = obj.method;
extractedMethod(); // 输出 undefined (非严格模式，因为this指向window，window.name通常是空字符串或undefined)
                   // TypeError (严格模式，因为this是undefined)
```

#### 2. 隐式绑定 (Implicit Binding)

当函数作为**对象的一个方法**被调用时，`this` 会被绑定到那个**调用它的对象**。

*   查找调用点：`this` 总是指向调用函数前面那个“点”之前的对象。

**示例：**

```javascript
const person = {
    name: "Alice",
    greet: function() {
        console.log("Hello, " + this.name);
    }
};

person.greet(); // `greet` 方法通过 `person` 对象调用，所以 `this` 指向 `person`。
               // 输出: "Hello, Alice"

const anotherPerson = {
    name: "Bob",
    greet: person.greet // 将 person.greet 赋值给 anotherPerson.greet
};
anotherPerson.greet(); // `greet` 方法通过 `anotherPerson` 对象调用，所以 `this` 指向 `anotherPerson`。
                      // 输出: "Hello, Bob"
```

**隐式丢失：**
当隐式绑定的函数作为回调函数传递时，会丢失 `this` 上下文，因为它不再通过对象来调用。这就是我们之前讨论的问题。

```javascript
function doSomething(cb) {
    cb(); // 这里 cb 是独立调用，所以会应用默认绑定
}

const obj = {
    value: 10,
    getValue: function() {
        console.log(this.value);
    }
};

doSomething(obj.getValue); // 输出 undefined (非严格模式) 或 TypeError (严格模式)
```

#### 3. 显式绑定 (Explicit Binding)

使用 `call()`, `apply()`, `bind()` 这三个函数方法可以强制指定 `this` 的值。

*   **`call(thisArg, arg1, arg2, ...)`：** 立即执行函数，第一个参数是 `this` 的绑定对象，后续参数是函数参数。
*   **`apply(thisArg, [argsArray])`：** 立即执行函数，第一个参数是 `this` 的绑定对象，第二个参数是包含所有函数参数的数组。
*   **`bind(thisArg, arg1, arg2, ...)`：** **不立即执行函数**，而是返回一个**新的函数**。这个新函数在被调用时，其 `this` 值会被永久绑定到 `bind()` 的第一个参数。

**示例：**

```javascript
function sayHello() {
    console.log("Hello, " + this.name);
}

const user1 = { name: "Carol" };
const user2 = { name: "David" };

// call()
sayHello.call(user1); // 输出: "Hello, Carol"
sayHello.call(user2); // 输出: "Hello, David"

// apply()
sayHello.apply(user1); // 输出: "Hello, Carol"

// bind()
const boundSayHello = sayHello.bind(user1);
boundSayHello(); // 输出: "Hello, Carol" (无论在哪里调用 boundSayHello，this 始终是 user1)

// bind 解决回调函数 this 丢失问题
const objWithMethod = {
    id: 123,
    logId: function() {
        console.log("ID:", this.id);
    }
};

setTimeout(objWithMethod.logId.bind(objWithMethod), 1000); // 1秒后输出: "ID: 123"
```

#### 4. `new` 绑定 (New Binding)

当使用 `new` 关键字调用函数时（作为构造函数），会发生以下四步：

1.  创建一个全新的空对象。
2.  新对象会被连接到该函数的原型链上。
3.  函数体内的 `this` 会被绑定到这个新创建的对象。
4.  如果函数没有返回其他对象，那么 `new` 表达式会返回这个新创建的对象。

**示例：**

```javascript
function Person(name) {
    this.name = name; // `this` 指向新创建的实例对象
    console.log("Inside Person constructor, this is:", this);
}

const alice = new Person("Alice"); // 输出: "Inside Person constructor, this is: Person { name: 'Alice' }"
console.log(alice.name); // 输出: "Alice"
```

#### 5. 箭头函数绑定 (Arrow Function Binding)

箭头函数（ES6+）没有自己的 `this` 绑定。它会**捕获其定义时所处的词法作用域的 `this` 值**。这意味着箭头函数的 `this` 取决于它被定义在哪个普通函数（或全局作用域）内部，而不是取决于它如何被调用。

*   **词法 `this`：** 箭头函数的 `this` 是静态的，一旦定义就确定了。

**示例：**

```javascript
const user = {
    name: "John",
    // 这是一个普通方法，它的 this 由调用方式决定
    regularMethod: function() {
        console.log("Regular method this:", this.name); // John

        // 内部的箭头函数，它的 this 捕获了 outerMethod 的 this (也就是 user 对象)
        setTimeout(() => {
            console.log("Arrow function this:", this.name); // John
        }, 1000);
    },
    // 这是一个箭头函数作为对象属性 (不推荐，因为它没有隐式绑定特性)
    arrowMethod: () => {
        console.log("Arrow method this:", this.name);
        // 在这种情况下，arrowMethod 的 this 会是全局对象 (window/global)
        // 因为它定义在全局作用域中，或者说，它的上一层词法作用域是全局作用域
    }
};

user.regularMethod();
// 1秒后输出: "Arrow function this: John"

user.arrowMethod(); // 输出: "Arrow method this: undefined" (因为 this 指向 window/global)

// 箭头函数解决回调函数 this 丢失问题
const objWithCallback = {
    value: 20,
    process: function() {
        // 这里的 this 是 objWithCallback
        setTimeout(() => {
            console.log("Processed value:", this.value); // this 仍然是 objWithCallback
        }, 1000);
    }
};
objWithCallback.process(); // 1秒后输出: "Processed value: 20"
```

### `this` 绑定规则的优先级

理解优先级很重要，当多个规则可能适用时，哪个规则会胜出？

1.  **`new` 绑定**：优先级最高。
2.  **显式绑定 (`call`, `apply`, `bind`)**：次高。其中 `bind` 创建的新函数其 `this` 绑定优先级高于 `call/apply` （因为 `bind` 一旦绑定就不能再通过 `call/apply` 改变）。
3.  **隐式绑定**：再次之。
4.  **默认绑定**：优先级最低。

**特殊情况：箭头函数不遵循上述规则，它有自己的词法 `this` 规则，且优先级最高。**



