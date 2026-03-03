# N Language 初学者指南 (v1.0.0)

欢迎来到 **N Language**！这是一门现代、轻量级且富有表现力的脚本语言，旨在为开发者提供简洁的语法和强大的功能。

本指南将带你快速掌握 N 语言的基础语法和核心特性。

---

## 1. 快速开始

### 安装
1. 运行 `NLanguageSetup.exe` 安装包。
2. 安装程序会自动将 N 语言添加到你的系统环境变量 (PATH) 中。
3. 打开命令行 (CMD 或 PowerShell)，输入 `n`。如果你看到版本信息和作者署名 `Squ4sh000`，说明安装成功！

### 运行代码
创建一个名为 `hello.ns` 的文件，输入以下代码：
```ns
print("Hello, N Language!");
```
在命令行中运行：
```bash
n hello.ns
```

---

## 2. 基础语法

### 变量与常量
N 语言使用 `var` 声明变量，使用 `const` 声明常量。
```ns
var name = "N";      // 变量可以被重新赋值
const PI = 3.14;     // 常量一旦赋值不可修改
```

### 数据类型
N 语言支持以下基础类型：
- **数字 (Number)**: `123`, `3.14`
- **字符串 (String)**: `"Hello"`
- **布尔值 (Boolean)**: `true`, `false`
- **空值 (Null)**: `null`
- **数组 (Array)**: `[1, 2, 3]`

---

## 3. 控制流程

### 条件判断 (If-Else)
```ns
var score = 85;
if (score >= 90) {
    print("优秀");
} else if (score >= 60) {
    print("及格");
} else {
    print("不及格");
}
```

### 循环 (While & For)
传统的 `for` 循环：
```ns
for (var i = 0; i < 5; i = i + 1) {
    print(i);
}
```

---

## 4. N 语言特色特性

### 1. `for-in` 遍历
这是 N 语言推荐的集合遍历方式，支持数组和字符串。
```ns
var fruits = ["苹果", "香蕉", "橙子"];
for (fruit in fruits) {
    print("我喜欢吃: ", fruit);
}

for (char in "N-Lang") {
    print(char);
}
```

### 2. `?` 错误传递运算符 (核心亮点)
这是 N 语言处理“可能失败的操作”的优雅方式。如果一个函数返回 `null`，在调用后加上 `?` 会立即中断当前函数并将 `null` 向上传递。
```ns
fn get_data(success) {
    if (success) { return "数据内容"; }
    return null;
}

fn process() {
    var data = get_data(false)?; // 如果 get_data 返回 null，process 函数直接在此处返回 null
    print("这行代码不会被执行");
    return data;
}
```

---

## 5. 函数与类

### 函数 (Function)
使用 `fn` 关键字定义函数：
```ns
fn add(a, b) {
    return a + b;
}
print(add(5, 10));
```

### 类与对象 (Class & Instance)
N 语言支持基础的面向对象编程：
```ns
class Player {
    var name = "未知";
    var level = 1;

    fn info() {
        print("玩家: ", self.name, " 等级: ", self.level);
    }
}

var p = Player();
p.name = "Squ4sh";
p.info();
```

---

## 6. 原生数组操作

使用内置的 `push` 函数和 `len` 函数：
```ns
var list = [1, 2];
push(list, 3);          // 添加元素
print(len(list));       // 输出 3
print(list[0]);         // 下标访问
```

---

## 7. VSCode 扩展使用

1. 在 VSCode 扩展商店搜索并安装 **N Language Support**。
2. 插件会自动识别你安装的 `n.exe`。
3. 在任何 `.ns` 文件中，**右键点击编辑器** 或 **文件资源管理器**，选择 **"Run N Script"** 即可一键运行。
4. 插件支持自动保存和语法高亮。

---

**祝你在 N 语言的世界里编程愉快！**
**作者: Squ4sh000**
