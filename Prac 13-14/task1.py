#!/usr/bin/env python3
from bcc import BPF
from time import sleep

# Загружаем и компилируем C-код из отдельного файла
b = BPF(src_file="task1.c")

print("Трассировка памяти...")
# Сбор статистики 30 секунд
try:
    sleep(30)
except KeyboardInterrupt:
    pass

# Считываем ранее записанные в структуры данные о выделении
# и освобождении памяти для процессов
print("\n%-16s %-8s %-10s %-10s %-10s" %
      ("COMM", "PID", "ALLOC_KB", "FREE_CNT", "ALLOC_CNT"))
for k, v in b["stats"].items():
    print("%-16s %-8d %-10d %-10d %-10d" %
          (k.comm.decode(), k.pid, v.alloc_bytes // 1024,
           v.free_cnt, v.alloc_cnt))