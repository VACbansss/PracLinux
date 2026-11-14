#!/usr/bin/env python3
from bcc import BPF
from time import sleep

# Загружаем и компилируем C-код из отдельного файла
b = BPF(src_file="task2.c")

print("Трассировка сети...")
# Сбор статистики 20 секунд
try:
    sleep(20)
except KeyboardInterrupt:
    pass

# Считываем значения с карт conns и tx_bytes и выводим в консоль
print("\n=== Соединения (выборка) ===")
for k, v in list(b["conns"].items())[:10]:
    print(f"{k.saddr:#x}:{k.sport} -> {k.daddr:#x}:{k.dport}")

print("\n=== Отправлено байт по ifindex ===")
for k, v in b["tx_bytes"].items():
    print(f"ifindex {k.value}: {v.value} байт")