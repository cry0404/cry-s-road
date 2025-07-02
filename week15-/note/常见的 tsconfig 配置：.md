# 更多关于 tsconfig.json

我们不会逐个探讨 [`tsconfig.json`](https://www.typescriptlang.org/tsconfig/) 文件中的每一个选项...那才是文档的作用（它就像非常非常非常长的文档）。但至少让我们涵盖一些最常见的部分。从最重要到最不重要的编译器选项（在我看来）：

- [`lib`](https://www.typescriptlang.org/tsconfig/#lib)：将 `dom` 和 `dom.iterable`（注意：小写）添加到库列表中，以允许所有浏览器 API，如果你在编写前端代码的话。
- [`strict`](https://www.typescriptlang.org/tsconfig/#strict)：如果 `true`，则启用所有严格类型检查选项。我强烈建议新项目使用它。如果你在迁移现有的 JS 项目，可能需要将其关闭。
- [`skipLibCheck`](https://www.typescriptlang.org/tsconfig/#skipLibCheck): 如果 `true`，将跳过所有声明文件（这意味着它不会尝试检查你的无限大的 `node_modules` 文件夹）。这将大幅加快编译时间。
- [`verbatimModuleSyntax`](https://www.typescriptlang.org/tsconfig/#verbatimModuleSyntax): 如果 `true`，简化导入和导出类型的怪异行为，基本上它强制你使用 `import type` 语法。我推荐它。
- [`esModuleInterop`](https://www.typescriptlang.org/tsconfig/#esModuleInterop): 如果 `true`，允许你使用 `import` 语法与 CommonJS 模块。如果你需要与 CommonJS（Node）代码一起工作，这非常有用。
- [`moduleDetection`](https://www.typescriptlang.org/tsconfig/#moduleDetection): 如果设置为 `force`，将把所有内容都视为模块，这是任何新项目所需的行为。
- [`noUncheckedIndexedAccess`](https://www.typescriptlang.org/tsconfig/#noUncheckedIndexedAccess): 如果 `true`，将 `undefined` 添加到任何索引访问的类型中，这可以防止一些运行时错误。我推荐它。