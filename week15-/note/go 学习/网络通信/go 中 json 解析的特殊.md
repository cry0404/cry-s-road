# JSON 复习 

结构1 

```go
type parameters struct {
    Name string `json:"name"`
    Age int `json:"age"`
    School struct {
        Name string `json:"name"`
        Location string `json:"location"`
    } `json:"school"`
}
```

结构 1 中 school 嵌套的键将会被表示为 school.location 和 school.name

结构2

```go
type parameters struct {
    name string `json:"name"`
    Age int `json:"age"`
}
```

结构 2 中 name 将不会被解析成键，因为没有大写

结构 3

结构 3 中什么都不会被解析，因为没有构成反射

```go
type parameters struct {
    Name string
    Age int
}
```