#!/usr/bin/python3
from bcc import BPF

# Загружаем и компилируем C-код из отдельного файла
b = BPF(src_file="hello.c")

# Привязка к событию ядра: получаем имя syscall execve для текущей платформы
syscall = b.get_syscall_fnname("execve")

# Привязка функции hello() к событию: когда процесс вызывает execve,
# ядро выполнит hello()
b.attach_kprobe(event=syscall, fn_name="hello")

# Чтение вывода из trace_pipe в реальном времени
b.trace_print()