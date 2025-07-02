---
title: go 学习
created: 2025-06-17
source: Cherry Studio
tags: 一次完整 http 请求
---
## Go 中实现 HTTP 请求并完整解析 JSON 响应

在 Go 中，我们通常使用标准库 `net/http` 来发起 HTTP 请求。对于 JSON 解析，我们则会用到 `encoding/json`。

### 核心步骤概览

1. **构建 HTTP 请求**：指定请求方法（GET, POST, PUT, DELETE 等）、URL、请求头（Header）和请求体（Body）。
2. **发送请求**：使用 `http.Client` 来执行请求。
3. **处理响应**：检查响应状态码，获取响应体。
4. **解析 JSON 响应体**：将 JSON 数据反序列化（unmarshal）为 Go 结构体。

### 示例：GET 请求并解析 JSON

假设我们要请求一个 API，它返回用户的 JSON 数据，例如：

```json
{
  "id": 1,
  "name": "Alice",
  "email": "alice@example.com",
  "isActive": true,
  "roles": ["admin", "user"]
}
```

#### 1. 定义 Go 结构体来匹配 JSON 结构

这是 JSON 解析的关键一步。你需要定义一个 Go 结构体，其字段类型和名称与 JSON 中的键值对相对应。

```go
package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"time" // 用于 http.Client 的超时设置
)

// User 定义了从 JSON 响应中解析用户信息的结构体
// 注意：字段名首字母必须大写才能被 json 包访问到（导出字段）
// `json:"..."` 是结构体标签，用于指定 JSON 键名与结构体字段的映射关系
// 如果 JSON 键名与 Go 结构体字段名完全一致（且 Go 字段名首字母大写），则可以省略标签
// 但为了健壮性和代码可读性，通常建议加上标签。
type User struct {
	ID       int      `json:"id"`
	Name     string   `json:"name"`
	Email    string   `json:"email"`
	IsActive bool     `json:"isActive"` // 注意 JSON 键是 camelCase, Go 字段是 PascalCase
	Roles    []string `json:"roles"`
}

// 假设我们有一个列表的响应，比如 /users
type UsersResponse struct {
    Users []User `json:"users"`
    Total int    `json:"total"`
}

func main() {
	// ... (后面会填充代码)
}
```

**知识普及：结构体标签 (Struct Tags)**

*   **作用**：结构体标签是 Go 语言的一个强大特性，它允许你在结构体字段定义旁边附加元数据。这些元数据可以通过反射在运行时被程序读取。
*   **`json:"..."` 标签**：`encoding/json` 包广泛使用这个标签来控制 JSON 编码（Marshal）和解码（Unmarshal）的行为。
    *   `json:"id"`：表示该 Go 字段对应的 JSON 键是 `id`。
    *   `json:"-"`：表示该字段在 JSON 编码/解码时被忽略。
    *   `json:",omitempty"`：表示如果该字段是其类型的零值（例如，int 的 0，string 的空字符串，bool 的 false，slice/map 的 nil），则在 JSON 编码时该字段被省略。
*   **反射 (Reflection)**：Go 的 `reflect` 包提供了在运行时检查和修改程序结构的能力，结构体标签就是反射的一个常见应用场景。在云原生领域，许多框架和工具（如 ORM、配置解析、RPC 框架）都大量依赖反射来动态处理数据结构。

#### 2. 发送 GET 请求并解析

```go
// ... (User 结构体定义)

func main() {
	apiURL := "https://jsonplaceholder.typicode.com/users/1" // 一个模拟的 API
	// 实际项目中，这个URL应该配置化，而不是硬编码

	// 1. 创建 http.Client 实例
	// http.DefaultClient 是一个默认的 Client 实例，通常不建议直接使用，
	// 因为它没有超时配置，可能导致连接挂起。
	// 推荐创建自定义的 Client，并设置超时。
	client := &http.Client{
		Timeout: 10 * time.Second, // 设置请求超时时间为 10 秒
	}

	// 2. 发送 GET 请求
	resp, err := client.Get(apiURL)
	if err != nil {
		// 网络错误、DNS 解析失败、连接超时等
		log.Fatalf("Error making HTTP request: %v", err)
	}
	// 确保响应体被关闭，释放资源。
	// defer 关键字确保函数执行结束后执行这行代码，即使出现错误也会执行。
	defer resp.Body.Close()

	// 3. 检查 HTTP 响应状态码
	// HTTP 状态码 2xx 表示成功
	if resp.StatusCode != http.StatusOK {
		log.Fatalf("Received non-OK HTTP status: %d %s", resp.StatusCode, resp.Status)
	}

	// 4. 读取响应体
	// ioutil.ReadAll 将响应体全部读入内存。
	// 对于非常大的响应体，应该考虑流式处理（Streaming），避免内存耗尽。
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Fatalf("Error reading response body: %v", err)
	}

	// 5. 解析 JSON 响应体
	var user User // 创建一个 User 结构体实例，用于接收解析后的数据
	err = json.Unmarshal(body, &user) // json.Unmarshal 需要一个字节切片和目标结构体的指针
	if err != nil {
		log.Fatalf("Error unmarshaling JSON: %v", err)
	}

	// 6. 打印解析后的数据
	fmt.Printf("Successfully parsed user:\n")
	fmt.Printf("  ID: %d\n", user.ID)
	fmt.Printf("  Name: %s\n", user.Name)
	fmt.Printf("  Email: %s\n", user.Email)
	fmt.Printf("  IsActive: %t\n", user.IsActive)
	fmt.Printf("  Roles: %v\n", user.Roles)
}
```

**知识普及：`http.Client` 与超时**

*   **`http.Client`**：是 Go 中进行 HTTP 请求的核心组件。它可以配置代理、TLS 设置、Cookie Jar、重定向策略以及最重要的**超时**。
*   **超时 (Timeout)**：在分布式系统中至关重要。
    *   **连接超时 (Connect Timeout)**：客户端尝试与服务器建立 TCP 连接的最长时间。
    *   **读写超时 (Read/Write Timeout)**：在连接建立后，从服务器读取数据或向服务器写入数据的最长时间。
    *   **整个请求超时 (Overall Timeout)**：`http.Client.Timeout` 设置的是从请求开始到响应体完全读取完毕的**总时间限制**。
*   **为什么重要？**：在微服务架构中，一个服务可能依赖多个下游服务。如果没有超时，一个慢响应或无响应的下游服务可能导致上游服务长时间阻塞，甚至耗尽连接池资源，最终导致整个系统雪崩。超时是**弹性设计**的关键一环。

### 示例：POST 请求并发送 JSON 数据

POST 请求通常用于向服务器发送数据，例如创建新资源。

```go
// ... (User 结构体定义)

func main() {
    // ... (其他 GET 请求代码，或者单独放在一个函数里)

	// 假设我们要创建一个新用户
	apiURL := "https://jsonplaceholder.typicode.com/users" // 模拟创建用户的 API

	newUser := User{
		Name:     "Bob",
		Email:    "bob@example.com",
		IsActive: true,
		Roles:    []string{"editor"},
	}

	// 1. 将 Go 结构体转换为 JSON 字节切片
	// json.Marshal 将 Go 结构体序列化为 JSON 格式的字节切片
	jsonData, err := json.Marshal(newUser)
	if err != nil {
		log.Fatalf("Error marshaling new user to JSON: %v", err)
	}

	// 2. 创建一个 io.Reader 实例作为请求体
	// bytes.NewBuffer 是一个方便的实现 io.Reader 的方法，将字节切片包装成可读对象
	reqBody := bytes.NewBuffer(jsonData)

	client := &http.Client{
		Timeout: 10 * time.Second,
	}

	// 3. 发送 POST 请求
	// http.Post 简化了 POST 请求，但它没有提供设置请求头（如 Content-Type）的参数
	// 更灵活的方式是使用 http.NewRequest 和 client.Do
	// resp, err := client.Post(apiURL, "application/json", reqBody) // 这也是一个选择

	// 推荐使用 http.NewRequest 构造请求，因为它更灵活
	req, err := http.NewRequest("POST", apiURL, reqBody)
	if err != nil {
		log.Fatalf("Error creating POST request: %v", err)
	}

	// 4. 设置请求头 Content-Type
	// 告知服务器请求体是 JSON 格式
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Accept", "application/json") // 告知服务器我们期望接收 JSON 响应

	resp, err := client.Do(req) // 使用 client.Do() 执行请求
	if err != nil {
		log.Fatalf("Error making POST request: %v", err)
	}
	defer resp.Body.Close()

	// 5. 检查响应状态码 (201 Created 是创建成功的常见状态码)
	if resp.StatusCode != http.StatusCreated && resp.StatusCode != http.StatusOK {
		bodyBytes, _ := ioutil.ReadAll(resp.Body) // 读取错误响应体以便调试
		log.Fatalf("Received non-OK/Created HTTP status for POST: %d %s, Body: %s", resp.StatusCode, resp.Status, bodyBytes)
	}

	// 6. 读取并解析响应体（如果 POST 请求返回了新创建的资源信息）
	// 大多数 POST 请求成功后会返回新创建的资源（带 ID 等）
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		log.Fatalf("Error reading POST response body: %v", err)
	}

	var createdUser User
	err = json.Unmarshal(body, &createdUser)
	if err != nil {
		log.Fatalf("Error unmarshaling POST JSON response: %v", err)
	}

	fmt.Printf("\nSuccessfully created user:\n")
	fmt.Printf("  ID: %d\n", createdUser.ID)
	fmt.Printf("  Name: %s\n", createdUser.Name)
	// 实际的 jsonplaceholder 不会返回 ID，所以这里 ID 可能是 0 或其他默认值
	// 真正的 API 会返回一个包含新 ID 的 JSON
}
```

**知识普及：`http.NewRequest` vs `http.Get`/`http.Post`**

*   `http.Get(url string)` 和 `http.Post(url string, contentType string, body io.Reader)` 是 `http.Client` 的便捷方法。它们内部调用了 `http.NewRequest`。
*   **`http.NewRequest(method string, url string, body io.Reader)`**：这是构建请求的通用方法。
    *   **灵活性**：它允许你设置任何 HTTP 方法（GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD等）。
    *   **请求头 (Headers)**：你可以通过 `req.Header.Set()` 或 `req.Header.Add()` 设置任意请求头，这对于认证（Authorization）、内容类型（Content-Type）、自定义请求头等非常重要。
    *   **更细粒度的控制**：你还可以设置请求的上下文（Context）用于取消请求或传递值。

### 处理未知 JSON 结构或动态 JSON

有时你可能无法预先知道 JSON 的完整结构，或者 JSON 结构会根据某些条件动态变化。Go 的 `encoding/json` 包也提供了处理这种情况的方法。

#### 1. 使用 `map[string]interface{}`

`interface{}` 是 Go 的空接口，可以表示任何类型的值。`map[string]interface{}` 可以用来解析任意 JSON 对象。

```go
func parseUnknownJson() {
    jsonString := `{"name": "dynamic_data", "value": 123, "details": {"type": "info", "code": "A1"}}`

    var data map[string]interface{} // 使用 map[string]interface{}
    err := json.Unmarshal([]byte(jsonString), &data)
    if err != nil {
        log.Fatalf("Error unmarshaling unknown JSON: %v", err)
    }

    fmt.Printf("\nParsed unknown JSON (map):\n")
    fmt.Printf("Name: %s (type: %T)\n", data["name"], data["name"])
    fmt.Printf("Value: %v (type: %T)\n", data["value"], data["value"]) // JSON numbers are float64 in interface{}
    // 访问嵌套对象
    if details, ok := data["details"].(map[string]interface{}); ok {
        fmt.Printf("Details Type: %s\n", details["type"])
    }
}
```

**注意：**
*   当你使用 `map[string]interface{}` 解析 JSON 数字时，它们会被解析为 `float64` 类型。
*   你需要进行类型断言 (`value.(ExpectedType)`) 来安全地访问这些值，否则可能会发生运行时 panic。

#### 2. 使用 `json.RawMessage`

如果你想延迟解析某个字段，或者某个字段可能是多种类型，可以使用 `json.RawMessage`。它会将该字段的原始 JSON 字节保存下来，你可以稍后再解析它。

```go
// ResponseWithDynamicPayload 假设有一个包含动态 payload 的响应
type ResponseWithDynamicPayload struct {
	Status  string          `json:"status"`
	Payload json.RawMessage `json:"payload"` // Payload 可以是任何 JSON
}

// UserPayload 如果 payload 是用户类型
type UserPayload struct {
	Username string `json:"username"`
	Age      int    `json:"age"`
}

// ProductPayload 如果 payload 是产品类型
type ProductPayload struct {
	Name  string  `json:"name"`
	Price float64 `json:"price"`
}

func parseRawMessage() {
	jsonUser := `{"status": "success", "payload": {"username": "GoGopher", "age": 10}}`
	jsonProduct := `{"status": "success", "payload": {"name": "Laptop", "price": 1200.50}}`

	// 解析用户 payload
	var respUser ResponseWithDynamicPayload
	err := json.Unmarshal([]byte(jsonUser), &respUser)
	if err != nil {
		log.Fatalf("Error unmarshaling user raw message: %v", err)
	}

	fmt.Printf("\nParsed RawMessage (User):\n")
	fmt.Printf("Status: %s\n", respUser.Status)
	var user UserPayload
	err = json.Unmarshal(respUser.Payload, &user) // 再次解析 Payload
	if err != nil {
		log.Fatalf("Error unmarshaling user payload: %v", err)
	}
	fmt.Printf("Payload Username: %s, Age: %d\n", user.Username, user.Age)

	// 解析产品 payload
	var respProduct ResponseWithDynamicPayload
	err = json.Unmarshal([]byte(jsonProduct), &respProduct)
	if err != nil {
		log.Fatalf("Error unmarshaling product raw message: %v", err)
	}

	fmt.Printf("\nParsed RawMessage (Product):\n")
	fmt.Printf("Status: %s\n", respProduct.Status)
	var product ProductPayload
	err = json.Unmarshal(respProduct.Payload, &product) // 再次解析 Payload
	if err != nil {
		log.Fatalf("Error unmarshaling product payload: %v", err)
	}
	fmt.Printf("Payload Name: %s, Price: %.2f\n", product.Name, product.Price)
}
```

### 生产环境中的考量与云原生实践

在实际的后端和云原生开发中，需要考虑更多：

1.  **错误处理**：
    *   不仅仅是 `log.Fatalf`。在生产代码中，你应该返回错误，让调用者决定如何处理。例如，对于网络错误，可以进行重试；对于业务错误（HTTP 状态码 4xx），可以返回给前端。
    *   Go 的错误处理是显式的，你需要检查每一个可能返回错误的操作。
    *   可以使用 Go 的 `errors` 包和第三方库如 `pkg/errors` 来构建更丰富的错误上下文。

2.  **日志记录**：
    *   使用结构化日志（如 `zap` 或 `logrus`）记录请求和响应的关键信息，包括 URL、状态码、耗时、错误详情等。这对于调试和可观测性至关重要。
    *   在云原生环境中，日志通常会被收集到中心化的日志系统（如 ELK Stack, Grafana Loki），因此结构化日志更容易被解析和查询。

3.  **配置管理**：
    *   API URL、超时时间、认证凭据等都应该是可配置的，而不是硬编码。
    *   在云原生环境中，可以使用环境变量、Kubernetes ConfigMap/Secret 或配置管理工具（如 HashiCorp Vault）来管理配置。

4.  **可观测性 (Observability)**：
    *   **监控 (Monitoring)**：使用 Prometheus 等工具收集 HTTP 请求的指标，如请求量、成功率、错误率、响应时间等。
    *   **链路追踪 (Tracing)**：使用 OpenTelemetry/Jaeger/Zipkin 等工具追踪请求在微服务之间的调用链，帮助你理解请求流和定位性能瓶颈。

5.  **重试机制 (Retry)**：
    *   对于网络瞬时错误（如连接超时、DNS 解析失败、5xx 错误），可以考虑实现指数退避（Exponential Backoff）的重试策略。
    *   注意避免对幂等性（Idempotency）敏感的操作进行简单重试（例如，多次创建订单）。

6.  **连接池 (Connection Pooling)**：
    *   `http.Client` 内部默认使用 `http.Transport` 来管理连接池。
    *   对于高并发场景，确保 `http.Transport` 的 `MaxIdleConns`、`MaxIdleConnsPerHost` 和 `IdleConnTimeout` 设置合理，以避免频繁建立和关闭 TCP 连接，提高性能。
    *   `client := &http.Client{ Transport: &http.Transport{ MaxIdleConns: 100, MaxIdleConnsPerHost: 10, IdleConnTimeout: 90 * time.Second, }, Timeout: 10 * time.Second, }`

7.  **上下文 (Context)**：
    *   在微服务或协程中传递 `context.Context`，用于取消请求、设置截止时间、传递请求范围的值（如 trace ID）。
    *   `req, _ := http.NewRequestWithContext(ctx, "GET", apiURL, nil)`

8.  **API 网关 (API Gateway)**：
    *   在云原生架构中，通常会有一个 API 网关（如 Nginx, Envoy, Kong, Apigee）来统一处理认证、授权、限流、熔断、日志、指标收集等横切关注点，从而减轻后端服务的负担。

9.  **服务发现 (Service Discovery)**：
    *   在微服务中，你不会直接硬编码服务的 IP 地址或端口。你会使用服务发现机制（如 Kubernetes Service, Consul, Eureka）来动态查找服务实例。



