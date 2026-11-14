#!/usr/bin/python3
from bcc import BPF
from time import sleep

# Загружаем и компилируем C-код из отдельного файла
b = BPF(src_file="hello-map.c")

# Привязка к событиям ядра
syscall_openat = b.get_syscall_fnname("openat")  # открытие или создание файла
syscall_write = b.get_syscall_fnname("write")    # запись в открытый файл

# Привязка функций hello_openat и hello_write к соответствующим kprobe
b.attach_kprobe(event=syscall_openat, fn_name="hello_openat")
b.attach_kprobe(event=syscall_write, fn_name="hello_write")

print("Tracing... Hit Ctrl-C to end.")
# Чтение данных из карты
while True:
    sleep(2)
    s = ""
    for k, v in b["counter_table"].items():
        s += f"ID {k.value}: {v.value}\t"
    print(s)