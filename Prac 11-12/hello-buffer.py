#!/usr/bin/python3
from bcc import BPF

program = r"""
BPF_PERF_OUTPUT(output); //perf-буффер для отправки событий в пространство пользователя 
/*
* pid идентификатор процесса
* uid идентификатор пользователя
* command имя процесса
* message заполняется в зависимости от четности PID
*/
struct data_t {
   int pid;
   int uid;
   char command[16];
   char message[16];
};
//Вызываемая функция на kprobe
int hello(void *ctx) {
   struct data_t data = {}; // Создание структуры
   int pid = bpf_get_current_pid_tgid() >> 32; // получаем PID из bpf_get_current_pid_tgid с помощью смещения битов на 32

   data.pid = pid;
   data.uid = bpf_get_current_uid_gid() & 0xFFFFFFFF; // получение UID с помощью bpf_get_current_uid_gid и маски оставляющая младшие биты

   bpf_get_current_comm(&data.command, sizeof(data.command)); //Копирование имени текущего процесса

   // Определение чётности PID и заполнение сообщения
   if (pid % 2 == 0) {
       // Чётный PID
       __builtin_memcpy(data.message, "Even PID Hello", 15);
   } else {
       // Нечётный PID
       __builtin_memcpy(data.message, "Odd PID Hello", 14);
   }

   output.perf_submit(ctx, &data, sizeof(data));
   return 0;
}
"""

b = BPF(text=program)
syscall = b.get_syscall_fnname("execve")
b.attach_kprobe(event=syscall, fn_name="hello")
//Обработка событий
def print_event(cpu, data, size):
   data = b["output"].event(data)
   msg = data.message.decode('utf-8', errors='ignore').rstrip('\x00')
   print(f"{data.pid} {data.uid} {data.command.decode()} {msg}")

// Инициирует вызов функции print_event каждый раз когда в пространство пользователя (output) попадают события
b["output"].open_perf_buffer(print_event)
while True:
   b.perf_buffer_poll()
