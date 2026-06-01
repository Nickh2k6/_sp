1. 基本指令執行
ls -la
echo "Hello, World!"
pwd
2. 內建指令：cd、exit、history
cd /tmp
cd            # 回 HOME
history       # 顯示歷史紀錄
exit          # 離開 shell
3. 管線（Pipeline）
ls -la | grep ".c"
ps aux | sort -nrk 3 | head -5
cat shell.c | grep "fork\|pipe\|dup2" | wc -l
4. I/O 重導向（< > >>）
echo "hello world" > output.txt
cat < output.txt
echo "more data" >> output.txt
cat output.txt
5. 背景執行（&）
sleep 10 &
6. Tab 自動補全
ls  <按 Tab>       # 補全檔案名稱
ec <按 Tab>        # 補全指令為 echo
cd <按 Tab>        # 補全程式目錄
7. Shell 提示符號
進入後會看到類似 user@host:~/path$ 的彩色提示字串（含 ~ 縮寫）。
8. 訊號處理（Ctrl+C / Ctrl+Z）
sleep 30
# 按下 Ctrl+C — Shell 不會結束，只有子行程 sleep 被終止
# 按下 Ctrl+Z — Shell 不會暫停