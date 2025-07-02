---
title: ts 中的常用工具类型
created: 2025-06-24
source: Cherry Studio
tags: 
---
### 1. `Partial<T>`

*   **作用**: 创建一个新类型，其所有属性都变为可选的。
*   **签名**: `type Partial<T> = { [P in keyof T]?: T[P] };` (内部实现)
*   **适用范围**:
    *   当需要创建一个接受部分属性来更新对象时。例如，在函数参数中，允许用户只传递需要修改的属性来更新一个完整的对象。
    *   模拟“可选参数对象”的模式。
*   **示例**:
    ```typescript
    interface User {
      id: number;
      name: string;
      email: string;
    }

    // myUserUpdate 的类型是 { id?: number; name?: string; email?: string; }
    function updateUser(id: number, myUserUpdate: Partial<User>) {
      // ... logic to update user by id using myUserUpdate properties
      console.log(`Updating user ${id} with data:`, myUserUpdate);
    }

    updateUser(1, { name: "Alice" }); // 允许只传递 name
    updateUser(1, { name: "Bob", email: "bob@example.com" }); // 允许传递部分属性
    ```

---

### 2. `Required<T>`

*   **作用**: 创建一个新类型，其所有属性都变为必需的。这个类型主要用于移除 `?` 可选修饰符。
*   **签名**: `type Required<T> = { [P in keyof T]-?: T[P]; };` (内部实现，其中 `-?` 是移除可选性的操作符)
*   **适用范围**:
    *   当从一个可能包含可选属性的对象类型开始，但又需要确保所有属性都被提供时。
    *   用于强制某些操作（如数据校验、对象创建）必须包含所有字段。
*   **示例**:
    ```typescript
    interface Product {
      id: number;
      name: string;
      description?: string; // description 是可选的
    }

    // createProduct 的参数需要所有属性都必须提供
    function createProduct(productData: Required<Product>) {
      // ... logic to create a product
      console.log("Creating product:", productData);
    }

    const initialProduct: Product = { id: 1, name: "Gadget" };
    // createProduct(initialProduct); // 错误: Property 'description' is missing in type 'Product' but required in type 'Required<Product>'. （如果 Product.description 不是必需的）
    // 实际上，如果 Product.description 是可选的，Required<Product> 会使其变为必需的。

    const completeProduct: Required<Product> = { id: 2, name: "Widget", description: "A useful widget." };
    createProduct(completeProduct); // 正确
    ```

---

### 3. `Readonly<T>`

*   **作用**: 创建一个新类型，其所有属性都变为只读的。防止属性被修改。
*   **签名**: `type Readonly<T> = { readonly [P in keyof T]: T[P]; };` (内部实现)
*   **适用范围**:
    *   当希望一个对象或其一部分属性在创建后不可变时。
    *   用于构建不可变数据结构，例如在 Redux 或状态管理场景中。
    *   作为函数返回值的类型，表明返回的对象不应被修改。
*   **示例**:
    ```typescript
    interface Point {
      x: number;
      y: number;
    }

    // const origin: Readonly<Point> = { x: 0, y: 0 };
    // origin.x = 10; // 错误: Cannot assign to 'x' because it is a read-only property.

    function createImmutablePoint(x: number, y: number): Readonly<Point> {
      return { x: x, y: y };
    }

    const pnt = createImmutablePoint(1, 2);
    // pnt.x = 5; // 错误
    ```

---

### 4. `Record<K, T>`

*   **作用**: 使用联合类型 `K` 作为属性名，以 `T` 作为属性值，创建一个新的对象类型。
*   **签名**: `type Record<K extends keyof any, T> = { [P in K]: T };` (内部实现)
*   **适用范围**:
    *   动态创建具有已知属性集合的对象类型。
    *   当属性键是来自一个枚举（或一组字符串/数字字面量）时，非常有用。
    *   实现字典（Map）或查找表类型。
*   **示例**:
    ```typescript
    // 假设我们有一个颜色名称的枚举
    enum ColorEnum {
      Red = "RED",
      Green = "GREEN",
      Blue = "BLUE",
    }

    // 使用 Record 创建一个 Record<ColorEnum, string> 类型，
    // 表示每个颜色都有一个对应的字符串描述
    // 它的类型是 { RED: string; GREEN: string; BLUE: string; }
    type ColorMap = Record<ColorEnum, string>;

    const colorDescriptions: ColorMap = {
      RED: "The color of passion",
      GREEN: "The color of nature",
      BLUE: "The color of the sky",
    };

    // colorDescriptions.YELLOW = "Something else"; // 错误, YELLOW 不是 ColorEnum 的一部分

    // 另一个例子：创建表示用户角色的映射
    type UserRoles = "admin" | "editor" | "viewer";
    type UserPermissions = Record<UserRoles, boolean>;

    const adminPermissions: UserPermissions = {
      admin: true,
      editor: true,
      viewer: true,
    };

    const viewerPermissions: UserPermissions = {
      admin: false,
      editor: false,
      viewer: true,
    };
    ```

---

### 5. `Pick<T, K>`

*   **作用**: 从类型 `T` 中选择一组属性 `K` (K 必须是 T 的属性键的子集)，创建一个新类型。
*   **签名**: `type Pick<T, K extends keyof T> = { [P in K]: T[P]; };` (内部实现)
*   **适用范围**:
    *   从一个大的接口或类型中提取出部分属性，用于创建更小、更集中的类型。
    *   在函数返回或参数中，只暴露或接收对象的部分成员。
*   **示例**:
    ```typescript
    interface Todo {
      id: number;
      title: string;
      description: string;
      completed: boolean;
    }

    // 创建一个只包含 id 和 title 的类型
    type TodoPreview = Pick<Todo, "id" | "title">;
    // TodoPreview 的类型是 { id: number; title: string; }

    const todoPreview: TodoPreview = {
      id: 1,
      title: "Learn TypeScript",
      // description: "abc", // 错误, description 不在 TodoPreview 中
      // completed: false,   // 错误, completed 不在 TodoPreview 中
    };

    function displayTodoTitle(todo: Pick<Todo, "title">) {
      console.log("Todo title:", todo.title);
    }

    displayTodoTitle({ title: "Finish this example" }); // 成功
    // displayTodoTitle({ id: 2, title: "Another todo" }); // 成功, 额外的属性会被忽略
    ```

---

### 6. `Omit<T, K>`

*   **作用**: 从类型 `T` 中移除一组属性 `K` (K 必须是 T 的属性键的子集)，创建一个新类型。
*   **签名**: `type Omit<T, K extends keyof T> = Pick<T, Exclude<keyof T, K>>;` (内部实现，利用了 `Pick` 和 `Exclude`)
*   **适用范围**:
    *   当需要一个对象类型，但不包含某些特定的属性时。
    *   例如，创建一个不包含敏感信息的返回类型（如密码字段）。
    *   从一个已有的类型中“减去”某些属性。
*   **示例**:
    ```typescript
    interface UserAuth {
      id: number;
      username: string;
      passwordHash: string; // 敏感信息
      email: string;
    }

    // 创建一个没有 passwordHash 的 UserProfile 类型
    type UserProfile = Omit<UserAuth, "passwordHash">;
    // UserProfile 的类型是 { id: number; username: string; email: string; }

    const profile: UserProfile = {
      id: 101,
      username: "johndoe",
      email: "john.doe@example.com",
      // passwordHash: "somehash", // 错误, passwordHash 被移除
    };

    function getUserPublicInfo(user: Omit<UserAuth, 'passwordHash' | 'email'>) {
        // 只暴露 id 和 username
        console.log(`User public ID: ${user.id}, Username: ${user.username}`);
    }
    getUserPublicInfo({ id: 102, username: "janedoe" });
    ```

---

### 7. `Exclude<T, U>`

*   **作用**: 从类型 `T` 中移除所有可以分配给类型 `U` 的成员，创建一个新的联合类型。
*   **签名**: `type Exclude<T, U> = T extends U ? never : T;` (内部实现，一个条件类型，利用了 distributive conditional types)
*   **适用范围**:
    *   当需要从一个联合类型中过滤掉某些特定的类型时。
    *   常用于构建更复杂的联合类型，或与 `Record`、`Omit` 等组合使用。
*   **示例**:
    ```typescript
    type Primitive = string | number | boolean;

    // 从 Primitive 联合类型中移除 string，得到一个只包含 number 和 boolean 的新类型
    type NonStringPrimitive = Exclude<Primitive, string>;
    // NonStringPrimitive 的类型是 string | number | boolean

    type MyNumbers = 1 | 2 | 3 | 4 | 5;
    type OddAndEven = Exclude<MyNumbers, 2 | 4>;
    // OddAndEven 的类型是 1 | 3 | 5

    type Status = "pending" | "processing" | "success" | "failed";
    type FinishedStatus = Exclude<Status, "pending" | "processing">;
    // FinishedStatus 的类型是 "success" | "failed"
    ```

---

### 8. `Extract<T, U>`

*   **作用**: 从类型 `T` 中提取所有可以分配给类型 `U` 的成员，创建一个新的联合类型。这是 `Exclude` 的反向操作。
*   **签名**: `type Extract<T, U> = T extends U ? T : never;` (内部实现，一个条件类型)
*   **适用范围**:
    *   当需要从一个联合类型中只保留某些特定类型时。
    *   经常与 `Exclude` 配对使用，或用于精炼联合类型。
*   **示例**:
    ```typescript
    type Primitive = string | number | boolean;

    // 从 Primitive 联合类型中提取出可以分配给 string 的成员
    type OnlyStringPrimitive = Extract<Primitive, string>;
    // OnlyStringPrimitive 的类型是 string

    type MyNumbers = 1 | 2 | 3 | 4 | 5;
    type EvenNumbers = Extract<MyNumbers, 2 | 4 | 6>;
    // EvenNumbers 的类型是 2 | 4

    type Status = "pending" | "processing" | "success" | "failed";
    type InProgressStatus = Extract<Status, "pending" | "processing">;
    // InProgressStatus 的类型是 "pending" | "processing"
    ```

---

### 9. `NonNullable<T>`

*   **作用**: 从类型 `T` 中移除 `null` 和 `undefined` 类型，创建一个新的类型。
*   **签名**: `type NonNullable<T> = T extends null | undefined ? never : T;` (内部实现)
*   **适用范围**:
    *   当需要确保一个类型不包含 `null` 或 `undefined`，以避免潜在的运行时错误时。
    *   在函数返回值、属性声明等地方使用，以加强类型安全性。
*   **示例**:
    ```typescript
    type MaybeValue = string | null | undefined;

    // myString 的类型是 string
    type MyString = NonNullable<MaybeValue>;

    const str: MyString = "hello";
    // const strNull: MyString = null; // 错误
    // const strUndefined: MyString = undefined; // 错误

    function processNonNull(value: NonNullable<MaybeValue>) {
      console.log(value.toUpperCase()); // 安全地调用方法
    }

    processNonNull("world");
    // processNonNull(null); // 错误
    // processNonNull(undefined); // 错误
    ```

---

### 10. `Parameters<T>`

*   **作用**: 提取函数类型 `T` 的所有参数类型组成的元组类型。
*   **签名**: `type Parameters<T extends (...args: any[]) => any> = T extends (...args: infer P) => any ? P : never;` (内部实现，利用了 `infer` 关键字)
*   **适用范围**:
    *   当需要获取函数的参数类型，例如为了创建一个接受相同参数的新函数，或者用来进行类型检查。
    *   在编写高阶函数、柯里化函数、装饰器时非常有用。
*   **示例**:
    ```typescript
    function greet(name: string, age: number): string {
      return `Hello, ${name}! You are ${age} years old.`;
    }

    // GetParams 的类型是 [name: string, age: number] (一个元组类型)
    type GetParams = Parameters<typeof greet>;

    // 示例：创建一个新函数，接收与 greet 相同的参数
    function callAndLog(fn: (...args: GetParams) => string, ...args: GetParams): void {
      const result = fn(...args);
      console.log("Function called with:", args, "Result:", result);
    }

    callAndLog(greet, "Alice", 30);
    ```
    `Parameters<T>` 的结果是一个**元组类型**（Tuple Type），它代表了有序的参数列表。

---

### 11. `ConstructorParameters<T>`

*   **作用**: 提取构造函数类型 `T` 的所有参数类型组成的元组类型。
*   **签名**: `type ConstructorParameters<T extends abstract new (...args: any) => any> = T extends abstract new (...args: infer P) => any ? P : never;` (内部实现)
*   **适用范围**:
    *   与 `Parameters` 类似，但专门用于处理类的构造函数。
    *   当需要为类的构造函数创建类型别名或进行参数推断时。
*   **示例**:
    ```typescript
    class Person {
      constructor(public name: string, public age: number) {}
    }

    // GetConstructorParams 的类型是 [name: string, age: number]
    type GetConstructorParams = ConstructorParameters<typeof Person>;

    // 示例：创建一个函数，可以接受任何 Person 构造函数参数，然后实例化一个 Person
    function createPersonInstance(
      ctor: abstract new (...args: GetConstructorParams) => Person,
      ...args: GetConstructorParams
    ): Person {
      return new ctor(...args);
    }

    const personInstance = createPersonInstance(Person, "Bob", 25);
    console.log(personInstance.name); // "Bob"
    ```

---

### 12. `ReturnType<T>`

*   **作用**: 提取函数类型 `T` 的返回值类型。
*   **签名**: `type ReturnType<T extends (...args: any[]) => any> = T extends (...args: any[]) => infer R ? R : never;` (内部实现，利用了 `infer` 关键字)
*   **适用范围**:
    *   当需要获取函数的返回类型，以便在其他地方使用该类型时。
    *   例如，为函数的返回值创建类型别名，或者为更高阶函数定义的返回类型。
*   **示例**:
    ```typescript
    function getGreeting(name: string): string {
      return `Hello, ${name}`;
    }

    // GreetingMsg 的类型是 string
    type GreetingMsg = ReturnType<typeof getGreeting>;

    function createMessage(transformer: (msg: GreetingMsg) => string) {
      const originalMsg = getGreeting("World");
      return transformer(originalMsg);
    }

    const finalMessage = createMessage(msg => msg.toUpperCase());
    console.log(finalMessage); // "HELLO, WORLD"
    ```

---

### 13. `InstanceType<T>`

*   **作用**: 获取一个构造函数类型 `T` 的实例类型。
*   **签名**: `type InstanceType<T extends abstract new (...args: any) => any> = T extends abstract new (...args: any) => infer R ? R : any;` (内部实现)
*   **适用范围**:
    *   当需要知道一个类实例化后对象的类型是什么时。
    *   常用于泛型类或函数，以引用类的实例类型。
*   **示例**:
    ```typescript
    class Car {
      constructor(public brand: string) {}
      drive(): void {
        console.log(`${this.brand} is driving.`);
      }
    }

    // CarInstance 的类型是 Car 的实例类型
    type CarInstance = InstanceType<typeof Car>;

    function maintainCar(car: CarInstance) {
      console.log(`Maintaining ${car.brand}...`);
      car.drive();
    }

    const myCar: CarInstance = new Car("Toyota");
    maintainCar(myCar);
    ```

---

### 14. `ThisParameterType<T>`

*   **作用**: 提取函数类型 `T` 的 `this` 参数的类型。如果函数没有显式声明 `this` 参数，则返回 `unknown`。
*   **签名**: `type ThisParameterType<T> = T extends (this: infer U, ...args: any[]) => any ? U : unknown;`
*   **适用范围**:
    *   在处理具有明确绑定的 `this` 上下文的函数时非常有用。
    *   例如，在为类方法创建类型签名或在函数重载时处理不同的 `this` 上下文。
*   **示例**:
    ```typescript
    function showName(this: { name: string }, age: number) {
      console.log(`${this.name} is ${age} years old.`);
    }

    // PersonContext 的类型是 { name: string }
    type PersonContext = ThisParameterType<typeof showName>;

    // 示例：为一个函数绑定正确的 this 上下文
    function callWithExplicitThis(
      fn: (this: PersonContext, age: number) => void,
      context: PersonContext,
      arg: number
    ) {
      fn.call(context, arg);
    }

    const person = { name: "Alice" };
    callWithExplicitThis(showName, person, 25); // 输出: "Alice is 25 years old."

    // 另一个例子，如果函数没有显式的 this 参数
    function regularFunction(a: number, b: number) {
      return a + b;
    }

    // NoThisContext 的类型是 unknown
    type NoThisContext = ThisParameterType<typeof regularFunction>;
    ```

---

### 15. `OmitThisParameter<T>`

*   **作用**: 移除函数类型 `T` 的 `this` 参数，返回一个没有 `this` 参数的函数类型。
*   **签名**: `type OmitThisParameter<T> = T extends (this: infer ThisType, ...args: infer Args) => infer ReturnType ? (...args: Args) => ReturnType : T;`
*   **适用范围**:
    *   当一个函数签名包含了 `this` 参数（作为第一个参数，但不是普通参数），而你又想得到不带这个 `this` 参数的函数签名时。
    *   常用于对已知包含 `this` 参数的函数进行类型上的“清理”或转换。
*   **示例**:
    ```typescript
    interface MyObject {
      value: number;
      increment: (this: MyObject, amount: number) => number;
    }

    // InnerFunction 的类型为 (amount: number) => number
    type InnerFunction = OmitThisParameter<MyObject["increment"]>;

    function processIncrement(handler: InnerFunction) {
      console.log("Processing increment function...");
      const result = handler(5); // here handler is just a regular function
      console.log("Result:", result);
    }

    const obj: MyObject = {
      value: 10,
      increment: function(this: MyObject, amount: number): number {
        this.value += amount;
        return this.value;
      },
    };

    // 注意：调用 InnerFunction 时，内部可能仍依赖于上下文的绑定，
    // 但类型本身已经移除了 this 参数的声明。
    // 如果直接调用 obj.increment，还是需要正确的 this 上下文。
    // 但如果用于传递给一个不关心 this 的函数，这是有用的。
    // processIncrement(obj.increment); // 这会产生一个类型错误，因为 obj.increment 需要 this 上下文
                                       // 但 InnerFunction 类型本身是对的。
                                       // 更常见的用法是传递一个匿名函数或绑定后的函数
    processIncrement(function (amount: number) {
         // 即使在这里没有 this，类型检查器也通过了
         // 但如果 handler 内部尝试访问 this 会是什么结果取决于实际执行环境
         return amount * 2;
    });
    ```

---

### 16. `ThisType<T>`

*   **作用**: 允许在对象字面量中，为方法中的 `this` 明确指定类型。它通常用在 `this` 参数声明被省略的对象字面量上。
*   **签名**: `type ThisType<T> = T;` (其作用体现在类型检查的上下文)
*   **适用范围**:
    *   为一个对象字面量（或一个由对象字面量构成的函数/方法）创建上下文，使得该对象字面量中的 `this` 指向指定的类型 `T`。
    *   非常适合用于编写类的实例方法，但又想用函数式声明来定义这些方法。
*   **示例**:
    ```typescript
    // 假设我们有一个对象，我们希望其内部方法能够访问 'counter' 属性
    interface CounterObject {
      counter: number;
      increment: () => void;
      double: () => number;
    }

    // 使用 WithThisType 来声明对象字面量中的 this 上下文是 CounterObject
    const counterImpl = {
      // this 现在明确是 CounterObject 类型的
      increment() {
        this.counter++;
      },
      double() {
        return this.counter * 2;
      },
      counter: 0
    };

    // 使用 ThisType 包装对象方法体，明确 'this' 的类型
    const myCounter: CounterObject = {
      counter: 0,
      increment: function() { // TypeScript 可以智能地推断出这里的 this
        this.counter++;
      },
      double: function() {   // 但是，用 ThisType 可以更明确
        return this.counter * 2;
      }
    };

    //更典型地使用 ThisType 进行匿名对象定义：
    const example = Object.assign({
        // 在这个 ReturnType<typeof Object.assign> 的上下文中，this 是指被创建的对象。
        // 在 object literal 内部，this 指代的是这个 object literal 本身。
        // TypeScript 默认不知道这个隐式的 this 指代什么，所以我们需要用 ThisType 来指定。
        // { counter: 0 } 是其类型, 即具有 counter 属性的对象
    }, {
        counter: 0,
        increment() {
            this.counter++;
        },
        double() {
            return this.counter * 2;
        }
    } as ThisType<{ counter: number }>) // 编译器知道 this 是 { counter: number }

    example.increment();
    example.increment();
    console.log(example.counter); // 2
    console.log(example.double()); // 4
    ```
    `ThisType<T>` 主要用于需要显式定义对象字面量方法中 `this` 的指向，尤其是在不使用类的情况下。


