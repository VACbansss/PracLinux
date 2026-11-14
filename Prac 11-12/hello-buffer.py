#!/usr/bin/python3
from bcc import BPF

# Загружаем и компилируем C-код из отдельного файла
b = BPF(src_file="hello-buffer.c")

syscall = b.get_syscall_fnname("execve")
b.attach_kprobe(event=syscall, fn_name="hello")

# Обработка событий
def print_event(cpu, data, size):
   data = b["output"].event(data)
   msg = data.message.decode('utf-8', errors='ignore').rstrip('\x00')
   print(f"{data.pid} {data.uid} {data.command.decode()} {msg}")

# Инициирует вызов функции print_event каждый раз, когда в пространство
# пользователя (output) попадают события
b["output"].open_perf_buffer(print_event)
while True:
   b.perf_buffer_poll()