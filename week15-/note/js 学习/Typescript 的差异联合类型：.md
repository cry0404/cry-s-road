两个原始类型的联合，比如 `string | number`，非常简单：它要么是字符串，要么是数字。对象类型也是如此，但要知道你正在处理哪种类型可能会有些棘手。这就是"差异属性"（或"标签"）派上用场的地方。它只是一个告诉你正在处理哪种类型的属性，并使使用条件逻辑来处理每种类型变得容易。它的特别之处在于它只能有一个值。

具有区分属性的对象的联合被称为 [“区分联合”](https://www.typescriptlang.org/docs/handbook/typescript-in-5-minutes-func.html#discriminated-unions) 或“标记联合”。

Map 与 Set 都没有 length 属性，是由 size 来作为计数的

```typescript
type MultipleChoiceLesson = {
  kind: "multiple-choice"; // Discriminant property
  question: string;
  studentAnswer: string;
  correctAnswer: string;
};

type CodingLesson = {
  kind: "coding"; // Discriminant property
  studentCode: string;
  solutionCode: string;
};

type Lesson = MultipleChoiceLesson | CodingLesson;

function isCorrect(lesson: Lesson): boolean {
  switch (lesson.kind) {
    case "multiple-choice":
      return lesson.studentAnswer === lesson.correctAnswer;
    case "coding":
      return lesson.studentCode === lesson.solutionCode;
  }
}
```

联合类型在需要考虑另一种新形状时非常实用，因为 TypeScript 可以确保你处理所有可能的情况。如果我们做出这些更改：

```typescript
type TrueFalseLesson = {
  kind: "true-false"; // Discriminant property
  question: string;
  studentAnswer: boolean;
  correctAnswer: boolean;
};

type Lesson = MultipleChoiceLesson | CodingLesson | TrueFalseLesson;
```

TypeScript 将抛出错误： `Function lacks ending return statement and return type does not include 'undefined'` 。这提醒我们需要添加一个第三种情况：

```ts
function isCorrect(lesson: Lesson): boolean {
  switch (lesson.kind) {
    case "multiple-choice":
      return lesson.studentAnswer === lesson.correctAnswer;
    case "coding":
      return lesson.studentCode === lesson.solutionCode;
    case "true-false":
      return lesson.studentAnswer === lesson.correctAnswer;
  }
}
```
---
## 关于 map
# Maps

TypeScript（显然）也内置了[映射](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Map) ，它是键值对的集合。你可以使用类型参数 `<K, V>` 指定键和值的类型。

```typescript
// A Map with string keys and number values
const podracerSpeeds = new Map<string, number>();

podracerSpeeds.set("Anakin Skywalker", 947);
podracerSpeeds.set("Sebulba", 941);

podracerSpeeds.set("R2-D2", true);
// Error: Argument of type 'true' is not assignable to parameter of type 'number'

podracerSpeeds.set(420, 69);
// Error: Argument of type 'number' is not assignable to parameter of type 'string'
```

Map 是一种"集合类"对象，因此它使用 `size` 属性而不是 `length`。

```typescript
console.log(podracerSpeeds.size);
// 1
```

如何轻松遍历一个 Map：

```typescript
for (const [racer, speed] of podracerSpeeds) {
  console.log(`${racer} raced at ${speed} speed`);
}
// Anakin raced at 947 speed
// Sebulba raced at 941 speed
```

以下是 Map 最重要的几个方法，[`get`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Map/get)、[`delete`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Map/delete) 和 [`has`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Map/has)。

```typescript
console.log(podracerSpeeds.get("Sebulba"));
// 941

console.log(podracerSpeeds.has("Sebulba"));
// true

podracerSpeeds.delete("Sebulba");
console.log(podracerSpeeds.get("Sebulba"));
// undefined
```
---
## 动态签名的概念

# 动态键

有时，你无法提前知道对象的所有属性名称。例如，假设你正在构建一个客户管理系统，其中员工可以向客户记录中添加自定义键/值对：

```
- favoriteColor: "blue"
- favoriteFood: "pizza"
- favoriteAnimal: "cat"
- etc
```

你无法提前知道用户将添加什么，但你仍然希望在你的程序中建模数据。

你可以使用索引签名来定义动态键 

```typescript
type UserMetrics = {
  [key: string]: number;
};
```

这种类型表示“这个对象可以有任意数量的属性，只要键是字符串且值是数字。”

```ts
const metrics: UserMetrics = {
  wordsPerMinute: 50,
  errors: 2,
  timeOnPage: 120,
};

metrics["refreshRate"] = 60; // OK
metrics["theme"] = "dark"; // Error: Type 'string' is not assignable to type 'number'
```
---
## 类型的整体层次结构图：
![整体结构图](https://storage.googleapis.com/qvault-webapp-dynamic-assets/course_assets/X3YIzJS.png)
