# 並行程式設計入門：執行緒、同步與經典問題

> **初學者指南 · 從理論到實作**
>
> 本書以 C 語言與 POSIX Threads (pthreads) 為教學平台，
> 透過實例逐步引導讀者理解多執行緒程式設計的核心概念。

---

## 目錄

- [第一章：執行緒（Thread）](#第一章執行緒thread)
- [第二章：競爭條件（Race Condition）](#第二章競爭條件race-condition)
- [第三章：互斥鎖（Mutex）](#第三章互斥鎖mutex)
- [第四章：死結（Deadlock）](#第四章死結deadlock)
- [第五章：銀行存提款模擬](#第五章銀行存提款模擬)
- [第六章：生產者消費者問題](#第六章生產者消費者問題)
- [第七章：哲學家用餐問題](#第七章哲學家用餐問題)
- [第八章：總結](#第八章總結)
- [附錄 A：編譯與執行](#附錄-a編譯與執行)

---

## 第一章：執行緒（Thread）

### 1.1 什麼是執行緒？

**執行緒（Thread）** 是作業系統能夠進行運算排程的最小單位。一個程式（行程, Process）可以包含一個或多個執行緒，這些執行緒共享同一個記憶體空間，但各自擁有獨立的執行堆疊與暫存器狀態。

想像一個廚房：
- **行程（Process）** 就像整個廚房，有自己的爐具、冰箱、廚具
- **執行緒（Thread）** 就像在廚房裡工作的廚師，他們共用同一個廚房和食材

### 1.2 為什麼需要使用執行緒？

1. **提高效能**：多核心 CPU 可以同時執行多個執行緒
2. **改善回應性**：UI 執行緒不會被耗時操作阻塞
3. **資源共享**：同一行程內的執行緒共享記憶體，溝通成本低

### 1.3 POSIX Threads 基本操作

在 C 語言中，我們使用 `<pthread.h>` 來操作執行緒：

```c
#include <pthread.h>
#include <stdio.h>

void* my_task(void* arg) {
    int* num = (int*)arg;
    printf("Thread received: %d\n", *num);
    return NULL;
}

int main() {
    pthread_t thread;
    int value = 42;

    // 建立執行緒
    pthread_create(&thread, NULL, my_task, &value);
    // 等待執行緒結束
    pthread_join(thread, NULL);

    return 0;
}
```

**編譯方式**：
```bash
gcc -pthread -o program program.c
```

### 1.4 關鍵 API

| 函式 | 功能 |
|------|------|
| `pthread_create()` | 建立新執行緒 |
| `pthread_join()` | 等待指定執行緒結束 |
| `pthread_exit()` | 結束當前執行緒 |
| `pthread_self()` | 取得當前執行緒 ID |

---

## 第二章：競爭條件（Race Condition）

### 2.1 什麼是競爭條件？

**競爭條件（Race Condition）** 是指多個執行緒同時存取共享資料，且執行結果取決於執行緒的執行順序（即「誰先跑到」），導致程式行為不可預測的情況。

### 2.2 經典範例：銀行結餘錯誤

假設兩個執行緒同時對 `balance = 0` 執行以下操作：

```
執行緒 A（存款）： balance++   → 期望結果 balance = 1
執行緒 B（提款）： balance--   → 期望結果 balance = -1
```

如果交替執行，可能產生正確結果，但問題出在 `balance++` 不是一個**原子操作**：

```c
// balance++ 在 CPU 層級實際上被拆成三步：
1. LOAD  balance, R1    // 從記憶體讀取 balance 到暫存器
2. ADD   R1, 1, R1      // 暫存器加 1
3. STORE R1, balance    // 寫回記憶體
```

當兩個執行緒交錯執行時：

| 時間 | 執行緒 A | 執行緒 B | balance 值 |
|------|----------|----------|-----------|
| T1   | LOAD → R1=0 |          | 0 |
| T2   |          | LOAD → R1=0 | 0 |
| T3   | ADD → R1=1 |          | 0 |
| T4   |          | ADD → R1=-1 | 0 |
| T5   | STORE → balance=1 | | **1** |
| T6   |          | STORE → balance=-1 | **-1** |

最終結果是 -1，但我們做了一次存款和一次提款，正確結果應該是 0！一個操作就「遺失」了。這就是**競爭條件**造成的資料不一致。

### 2.3 競爭條件發生的三個條件

1. **多個執行緒存取共享資源**
2. **至少一個執行緒在修改（寫入）該資源**
3. **存取沒有被同步機制保護**

### 2.4 為什麼競爭條件難以除錯？

- **非確定性（Non-deterministic）**：每次執行結果可能不同
- **難以重現**：依賴執行緒排程的時機
- **被認為「幾乎不會發生」**：但在高併發系統中幾乎必然發生

---

## 第三章：互斥鎖（Mutex）

### 3.1 什麼是互斥鎖？

**互斥鎖（Mutex, Mutual Exclusion）** 是一種同步機制，確保同一時間只有一個執行緒能進入**臨界區段（Critical Section）**——也就是存取共享資源的程式碼區塊。

形象比喻：**廁所的門鎖**——一次只能一個人使用，其他人必須排隊等候。

### 3.2 Mutex 基本用法

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;  // 靜態初始化

// 進入臨界區段前上鎖
pthread_mutex_lock(&lock);

// 臨界區段：存取共享資源
balance++;

// 離開臨界區段時解鎖
pthread_mutex_unlock(&lock);
```

### 3.3 Mutex 的 API

| 函式 | 功能 |
|------|------|
| `pthread_mutex_init()` | 初始化 mutex |
| `pthread_mutex_lock()` | 上鎖（若已被鎖住則阻塞等待） |
| `pthread_mutex_trylock()` | 嘗試上鎖（不阻塞） |
| `pthread_mutex_unlock()` | 解鎖 |
| `pthread_mutex_destroy()` | 銷毀 mutex |

### 3.4 使用 Mutex 修正競爭條件

修正第二章的銀行範例：

```c
int balance = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* deposit(void* arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);
        balance++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* withdraw(void* arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);
        balance--;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}
```

有了 mutex 保護，`balance++` 和 `balance--` 變成原子操作，不會交錯執行，保證結果正確。

### 3.5 使用 Mutex 的注意事項

1. **保持臨界區段越小越好**：只在必要時才上鎖
2. **避免在鎖內執行阻塞操作**：如 I/O、sleep
3. **務必配對 Lock/Unlock**：遺漏解鎖會導致死結
4. **鎖的粒度要適當**：太粗（鎖太大範圍）降低效能；太細增加複雜度

---

## 第四章：死結（Deadlock）

### 4.1 什麼是死結？

**死結（Deadlock）** 是指兩個或多個執行緒互相等待對方釋放資源，導致所有執行緒都無法繼續執行的情況。

**經典比喻**：十字路口的四個方向都有車，每台車都在等待右邊的車先走——結果全部卡住。

### 4.2 死結的四個必要條件（Coffman 條件）

| 條件 | 說明 |
|------|------|
| **互斥（Mutual Exclusion）** | 資源一次只能被一個執行緒使用 |
| **持有並等待（Hold and Wait）** | 執行緒持有至少一個資源，同時等待其他資源 |
| **不可搶佔（No Preemption）** | 資源不能被強制從持有者手中搶走 |
| **循環等待（Circular Wait）** | 存在一組執行緒 {T1, T2, ..., Tn}，T1 等待 T2 的資源，T2 等待 T3 的資源，...，Tn 等待 T1 的資源 |

### 4.3 死結範例

```c
// 執行緒 A：先鎖 chopstick[0]，再鎖 chopstick[1]
pthread_mutex_lock(&chopstick[0]);
pthread_mutex_lock(&chopstick[1]);

// 執行緒 B：先鎖 chopstick[1]，再鎖 chopstick[0]
pthread_mutex_lock(&chopstick[1]);
pthread_mutex_lock(&chopstick[0]);
```

如果 A 鎖住 0 的同時、B 鎖住了 1，則 A 等 1、B 等 0，形成**循環等待** → 死結！

### 4.4 避免死結的策略

1. **破壞「持有並等待」**：要求執行緒一次取得所有需要的資源
2. **破壞「不可搶佔」**：若無法取得所有資源，則釋放已持有的資源
3. **破壞「循環等待」**：規定資源取得順序（資源階層化, Resource Hierarchy）

最常見的策略是 **資源階層化**：為所有資源編號，並要求執行緒必須按編號順序取得資源。

```c
// 資源階層化解決方案
void* philosopher(void* arg) {
    int id = (int)arg;
    int left = id;           // 左筷子編號
    int right = (id+1) % 5;  // 右筷子編號

    while (1) {
        think();
        // 不論左右，總是先拿編號小的筷子
        if (left < right) {
            lock(left);
            lock(right);
        } else {
            lock(right);
            lock(left);
        }
        eat();
        unlock(left);
        unlock(right);
    }
}
```

---

## 第五章：銀行存提款模擬

### 5.1 問題描述

模擬銀行帳戶的存款與提款操作：
- 一個帳戶，初始餘額為 0
- 同一個帳戶同時進行 100,000 次存款（+1）和 100,000 次提款（-1）
- 最終正確餘額應為 0

### 5.2 程式架構

檔案位置：`code/bank/bank.c`

```
main()
  ├── test_without_lock()    ← 不使用 mutex（展示競爭條件）
  └── test_with_lock()       ← 使用 mutex（正確同步）
```

### 5.3 核心實作

**無鎖版本（有競爭條件）**：
```c
void* deposit_without_lock(void* arg) {
    for (int i = 0; i < 100000; i++) {
        balance++;  // 非原子操作！
    }
}
void* withdraw_without_lock(void* arg) {
    for (int i = 0; i < 100000; i++) {
        balance--;  // 非原子操作！
    }
}
```

**有鎖版本（安全）**：
```c
void* deposit_with_lock(void* arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);
        balance++;
        pthread_mutex_unlock(&mutex);
    }
}
void* withdraw_with_lock(void* arg) {
    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);
        balance--;
        pthread_mutex_unlock(&mutex);
    }
}
```

### 5.4 執行結果

```text
=== Without mutex (race condition) ===
Final balance: -14532 (expected: 0)
Result: WRONG - race condition caused data loss!

=== With mutex (safe synchronization) ===
Final balance: 0 (expected: 0)
Result: CORRECT
```

### 5.5 觀察與討論

- 每次執行「無鎖版本」的結果都可能不同
- 這證明了 `balance++` / `balance--` 不是原子操作
- 200,000 次操作中只要有**一次**交錯發生，結果就會錯誤
- mutex 雖然讓程式變慢了一些，但保證了正確性

---

## 第六章：生產者消費者問題

### 6.1 問題描述

**生產者消費者問題（Producer-Consumer Problem）**，也稱為**有界緩衝區問題（Bounded Buffer Problem）**，是一個經典的同步問題：

- **生產者（Producer）**：產生資料並放入緩衝區
- **消費者（Consumer）**：從緩衝區取出資料進行處理
- **緩衝區（Buffer）**：有容量上限的共享佇列

需要解決的問題：
1. 當緩衝區滿時，生產者不能繼續放入（必須等待）
2. 當緩衝區空時，消費者不能繼續取出（必須等待）
3. 同時間只能有一個執行緒操作緩衝區

### 6.2 程式架構

檔案位置：`code/producer_consumer/producer_consumer.c`

```
main()
  ├── 啟動 2 個生產者執行緒
  ├── 啟動 2 個消費者執行緒
  └── 等待所有執行緒結束
```

### 6.3 使用 Condition Variable 的生產者消費者

**條件變數（Condition Variable）** 允許執行緒在特定條件下等待，並由其他執行緒喚醒。

我們使用：
- **一個 mutex**：保護緩衝區的存取
- **`not_full` 條件變數**：通知生產者緩衝區有空位了
- **`not_empty` 條件變數**：通知消費者緩衝區有資料了

```c
// 共享資料
int buffer[BUFFER_SIZE];     // 環形緩衝區
int count = 0;               // 目前資料數量
int in = 0, out = 0;         // 寫入／讀取位置

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
```

**生產者流程**：
```c
void* producer(void* arg) {
    // 生產一個項目
    pthread_mutex_lock(&mutex);
    while (buffer 已滿) {
        pthread_cond_wait(&not_full, &mutex);  // 等待，同時釋放 mutex
    }
    放入資料到 buffer;
    pthread_cond_signal(&not_empty);  // 通知消費者
    pthread_mutex_unlock(&mutex);
}
```

**消費者流程**：
```c
void* consumer(void* arg) {
    pthread_mutex_lock(&mutex);
    while (buffer 已空) {
        pthread_cond_wait(&not_empty, &mutex);  // 等待，同時釋放 mutex
    }
    從 buffer 取出資料;
    pthread_cond_signal(&not_full);  // 通知生產者
    pthread_mutex_unlock(&mutex);
}
```

### 6.4 為什麼用 `while` 而不是 `if` 檢查條件？

使用 `while` 而非 `if` 是因為**虛假喚醒（Spurious Wakeup）**——即使沒有執行緒呼叫 `pthread_cond_signal()`，等待中的執行緒也可能被喚醒。使用 `while` 迴圈確保條件真正滿足後才繼續執行。

### 6.5 Condition Variable 的 API

| 函式 | 功能 |
|------|------|
| `pthread_cond_wait(cond, mutex)` | 等待條件成立（原子性地解鎖並等待） |
| `pthread_cond_signal(cond)` | 喚醒一個等待該條件的執行緒 |
| `pthread_cond_broadcast(cond)` | 喚醒所有等待該條件的執行緒 |
| `pthread_cond_init()` | 初始化條件變數 |
| `pthread_cond_destroy()` | 銷毀條件變數 |

### 6.6 執行結果範例

```text
Producer 1 produced: 1000  [buffer: 1/5]
Producer 2 produced: 2000  [buffer: 2/5]
Consumer 1 consumed: 1000  [buffer: 1/5]
Producer 1 produced: 1001  [buffer: 2/5]
...
  Producer 2: buffer full, waiting...
Consumer 2 consumed: 2001  [buffer: 4/5]
  Producer 2: buffer full, waiting...  → buffer 有空間了，繼續生產

All items produced and consumed successfully!
```

---

## 第七章：哲學家用餐問題

### 7.1 問題描述

**哲學家用餐問題（Dining Philosophers Problem）** 由 Dijkstra 提出，是死結的經典範例。

**情境設定**：
- 5 位哲學家圍坐在圓桌前
- 每位哲學家前面有一盤義大利麵
- 每兩位哲學家之間有一根筷子（共 5 根）
- 哲學家交替進行「思考」和「用餐」
- 用餐時需要同時拿起**左右兩根**筷子

**問題**：如果每位哲學家都先拿起左邊的筷子，再等右邊的筷子——當 5 位同時拿起左筷時，所有人都在等右筷，形成**死結**！

```
     [P0]──0──[P1]
      /             \
     4               1
     |               |
    [P4]            [P2]
      \             /
       3──[P3]──2

    Pn: 哲學家 n
    n: 筷子 n
```

### 7.2 程式架構

檔案位置：`code/dining_philosophers/dining_philosophers.c`

```c
pthread_mutex_t chopsticks[5];  // 每根筷子是一個 mutex

void* philosopher(void* arg) {
    int id, left, right;
    // left = id, right = (id + 1) % 5

    while (running) {
        think();
        lock(left);   // 拿起左筷
        lock(right);  // 拿起右筷
        eat();
        unlock(left);  // 放下左筷
        unlock(right); // 放下右筷
    }
}
```

### 7.3 死結版本

當所有哲學家同時拿起左筷：

| 時間 | P0 | P1 | P2 | P3 | P4 |
|------|----|----|----|----|----|
| T1   | 拿起筷0 | 拿起筷1 | 拿起筷2 | 拿起筷3 | 拿起筷4 |
| T2   | 等筷1 ❌ | 等筷2 ❌ | 等筷3 ❌ | 等筷4 ❌ | 等筷0 ❌ |

**所有人都在等別人釋放筷子，沒有人能用餐 → 死結！**

### 7.4 解決方案：資源階層化（Asymmetric Pickup）

解決方法很優雅：**讓最後一位哲學家用相反順序拿筷子**。

```c
// 資源階層化：按筷子編號順序拿取
if (use_fix && id == LAST_PHILOSOPHER) {
    lock(right);  // 最後一位先拿右筷
    lock(left);
} else {
    lock(left);   // 其他人先拿左筷
    lock(right);
}
```

為什麼這能解決死結？

因為這打破了**循環等待**條件：現在 P4 先拿筷 0 而非筷 4。筷 0 被 P0 持有時，P4 會等待，而 P0 用完會釋放筷 0。這樣就不可能形成完整的循環等待鏈了。

### 7.5 執行結果

```text
========================================
  Dining Philosophers Problem
  5 philosophers, 5 chopsticks
========================================

=== Without deadlock fix (prone to deadlock!) ===

Philosopher 0 is THINKING...
Philosopher 1 is THINKING...
Philosopher 2 is THINKING...
Philosopher 3 is THINKING...
Philosopher 4 is THINKING...
  Philosopher 0 picked up chopsticks [0(←), 1(→)]
Philosopher 0 is EATING...  (cycle 1)
  Philosopher 1 picked up chopsticks [1(←), 2(→)]
Philosopher 1 is EATING...  (cycle 1)
...

----------------------------------------

=== With deadlock fix (asymmetric pickup) ===

Philosopher 4 picked up chopsticks [0(→), 4(←)]  ← 注意：順序相反！
...
```

---

## 第八章：總結

### 8.1 核心概念回顧

| 概念 | 一句話總結 |
|------|-----------|
| **執行緒 (Thread)** | 行程內的輕量級執行單元，共享記憶體空間 |
| **競爭條件 (Race Condition)** | 多執行緒未經同步地存取共享資料導致的非預期結果 |
| **互斥鎖 (Mutex)** | 確保同一時間只有一個執行緒進入臨界區段的機制 |
| **條件變數 (Condition Variable)** | 讓執行緒在特定條件下等待並由其他執行緒喚醒 |
| **死結 (Deadlock)** | 多執行緒互相等待對方釋放資源，全部無法繼續執行 |
| **資源階層化** | 按固定順序取得資源以破壞循環等待，避免死結 |

### 8.2 三程式的對應關係

| 程式 | 使用的同步機制 | 解決的問題 |
|------|---------------|-----------|
| 銀行存提款 | Mutex | 競爭條件 |
| 生產者消費者 | Mutex + Condition Variable | 有界緩衝區同步 |
| 哲學家用餐 | Mutex + 資源階層化 | 死結 |

### 8.3 多執行緒程式設計原則

1. **最小化共享資源**：盡量減少執行緒間的資料共享
2. **保持臨界區段簡短**：鎖的範圍越小，效能越好
3. **避免嵌套鎖**：若無法避免，確保所有執行緒以相同順序上鎖
4. **使用 Condition Variable 處理等待**：不要用 busy-wait（忙等）
5. **優先使用高階抽象**：如 C++ 的 `std::lock_guard`、Python 的 `with lock:`
6. **測試、測試、再測試**：競爭條件和死結在測試中可能不會每次都出現

### 8.4 延伸學習

- **信號量（Semaphore）**：另一種經典同步機制
- **讀寫鎖（Read-Write Lock）**：區分讀取和寫入的鎖
- **樂觀鎖（Optimistic Locking）**：假設衝突很少發生
- **無鎖資料結構（Lock-Free Data Structures）**：不使用鎖的執行緒安全資料結構
- **Actor Model**：透過訊息傳遞而非共享狀態來避免競爭條件
- **事務記憶體（Transactional Memory）**：讓共享記憶體操作像資料庫事務一樣

---

## 附錄 A：編譯與執行

### A.1 編譯所有程式

從 `code/` 目錄執行：

```bash
cd code
make
```

### A.2 執行程式

```bash
./bank_sim              # 銀行存提款模擬
./producer_consumer_sim # 生產者消費者問題
./dining_sim            # 哲學家用餐問題
```

### A.3 清理編譯檔案

```bash
make clean
```

### A.4 單獨編譯特定程式

```bash
# 銀行存提款
gcc -Wall -Wextra -pthread -o bank_sim bank/bank.c

# 生產者消費者
gcc -Wall -Wextra -pthread -o producer_consumer_sim producer_consumer/producer_consumer.c

# 哲學家用餐
gcc -Wall -Wextra -pthread -o dining_sim dining_philosophers/dining_philosophers.c
```

---

> **本書使用 C 語言與 POSIX Threads 教學，所有程式碼均在 Linux 環境下測試通過。**
>
> 建議讀者親自編譯、執行並修改程式碼，以加深對並行程式設計概念的理解。
