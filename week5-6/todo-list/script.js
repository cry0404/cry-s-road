// 选择 HTML 元素
const taskInput = document.getElementById("input");
const addTaskBtn = document.getElementById("addTask"); // 统一变量名
const taskList = document.getElementById("taskList");

// 任务数组
let tasks = JSON.parse(localStorage.getItem("tasks")) || [];

// 渲染任务
function renderTasks() {
    taskList.innerHTML = ""; // 清空任务列表

    tasks.forEach((task, index) => {
        const li = document.createElement("li");

        li.innerHTML = `
            <span>${task}</span>
            <button onclick="deleteTask(${index})">删除</button>
        `;

        taskList.appendChild(li);
    });
}

// 添加任务
addTaskBtn.addEventListener("click", () => {
    const task = taskInput.value.trim();
    if (task) {
        tasks.push(task);
        localStorage.setItem("tasks", JSON.stringify(tasks)); // 存入本地存储
        taskInput.value = ""; // 清空输入框
        renderTasks();
    }
});

// 删除任务
function deleteTask(index) {
    tasks.splice(index, 1);
    localStorage.setItem("tasks", JSON.stringify(tasks));
    renderTasks();
}

// 初始化渲染
renderTasks();
