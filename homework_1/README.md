# 函數呼叫機制
## parse_program
parse_program 中定義了遇到 func add(a, b)，會先生成 FUNC_BEG add - -，標記函數的起點，接著解析參數，為每個參數生成 FORMAL a - - 與 FORMAL b - -，告訴 VM 這裡準備接收外部傳入的變數。
```
 if (cur_token.type == TK_FUNC) {
            next_token(); char f_name[32]; strcpy(f_name, cur_token.text);
            emit("FUNC_BEG", f_name, "-", "-");
            next_token(); next_token();
}
```
## 當程式遇到 factor 
假設遇到 result = add(10, 20)。
它會先計算括號內的參數，並依序生成 PARAM 10 - - 與 PARAM 20 - -，準備把參數推出去。
```
if (cur_token.type == TK_LPAREN) { // 處理函數呼叫 add(1, 2)
            next_token(); int count = 0;
            while (cur_token.type != TK_RPAREN) {
                char arg[32]; expression(arg);
                emit("PARAM", arg, "-", "-"); count++; // 參數傳遞指令
                if (cur_token.type == TK_COMMA) next_token(); 
            }
            next_token(); new_t(res); char c_str[12]; snprintf(c_str, sizeof(c_str), "%d", count);
            emit("CALL", name, c_str, res); // 跳轉執行指令
        }
```

## 當程式遇到「回傳值」(statement)
遇到 return a + b;，計算完結果後，會生成 RET_VAL t_res - -，告訴 VM 把這個結果送回給呼叫者。
```
else if (cur_token.type == TK_RETURN) {
        next_token(); char res[32]; expression(res);
        emit("RET_VAL", res, "-", "-");
        if (cur_token.type == TK_SEMICOLON) next_token();
}
```
## 虛擬機執行階段
VM 開始執行前，會先掃描所有的指令，把所有 FUNC_BEG 的位置（PC, Program Counter）記在 func_pc 陣列裡。這樣才知道呼叫函數時要跳轉到哪一行。
```
for (int i = 0; i < quad_count; i++) {
        if (strcmp(quads[i].op, "FUNC_BEG") == 0) {
            strcpy(func_names[f_count], quads[i].arg1); func_pc[f_count++] = i + 1;
        }
    }
```
當 VM 執行到 PARAM 10 與 PARAM 20 時，會把這些值暫存到一個公用的參數區 param_stack 中。
```
else if (strcmp(q.op, "PARAM") == 0) {
            param_stack[param_sp++] = get_var(q.arg1); // 暫存參數
}
```
## 進入函數 (CALL)
執行到 CALL add 2 t1 時
1. 開闢新空間： sp++，將堆疊指標加一，進入全新的區域變數環境。
2. 記住回程路： stack[sp].ret_pc = pc + 1，記下 Caller 執行完 CALL 之後該繼續執行的下一行位置。
3. 記住回傳變數： stack[sp].ret_var = "t1"，記下等一下函數結束後，要把答案寫回 Caller 的哪個變數裡。
4. 搬運參數： 將剛剛放在 param_stack 的 10 和 20，搬進當前 Frame 的 incoming_args 陣列中。
5. 跳轉執行： 將 pc (程式計數器) 設為 add 函數的起點 target_pc。
```
else if (strcmp(q.op, "CALL") == 0) {
            int p_count = atoi(q.arg2); int target_pc = -1;
            for (int i = 0; i < f_count; i++) 
                if (strcmp(func_names[i], q.arg1) == 0) target_pc = func_pc[i];
            
            sp++; // 開闢新的堆疊幀 (進入函數)
            stack[sp].count = 0; stack[sp].ret_pc = pc + 1;
            strcpy(stack[sp].ret_var, q.result);
            stack[sp].formal_idx = 0;
            // 從參數暫存區把值拿過來
            for(int i=0; i<p_count; i++) stack[sp].incoming_args[i] = param_stack[param_sp - p_count + i];
            param_sp -= p_count; pc = target_pc; continue;
        }
```

## 接收參數 (FORMAL)
跳進 add 函數後，開頭的指令是 FORMAL a 與 FORMAL b。
VM 會從當前 Frame 的 incoming_args 中，依序把 10 和 20 拿出來，並使用 set_var 建立當前環境的區域變數 a = 10 與 b = 20。
```
else if (strcmp(q.op, "FORMAL") == 0) {
            set_var(q.arg1, stack[sp].incoming_args[stack[sp].formal_idx++]);
        }
```
## 函數返回 (RET_VAL)
當執行到 RET_VAL t_res 時：
1. 取得結果： 算出準備要回傳的值（例如 30）。
2. 拿出剛剛記下的 ret_pc (要跳回哪裡) 以及 ret_var (要把值存進 Caller 的哪個變數，即 t1)。
3. 銷毀空間： sp--，堆疊指標減一，清空了被呼叫函數的堆疊幀，回到了 Caller 的環境。
4. 寫入結果與跳轉： 在 Caller 的環境中呼叫 set_var("t1", 30)，並將 pc 設回 ret_pc，繼續往下執行。
```
else if (strcmp(q.op, "RET_VAL") == 0) {
            int ret_val = get_var(q.arg1); int ret_address = stack[sp].ret_pc;
            char target_var[32]; strcpy(target_var, stack[sp].ret_var);
            sp--; // 銷毀當前堆疊幀 (回到 Caller)
            set_var(target_var, ret_val); // 將回傳值寫入 Caller 的變數空間
            pc = ret_address; continue;
        }
```