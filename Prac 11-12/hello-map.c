// Карта для подсчёта вызовов по UID
BPF_HASH(counter_table);

// Логика для openat
int hello_openat(void *ctx) {
   u64 uid;
   u64 counter = 0;
   u64 *p;

   uid = bpf_get_current_uid_gid() & 0xFFFFFFFF; // Маска, отсекающая GID — получаем UID
   p = counter_table.lookup(&uid);                // чтение из карты
   if (p != 0) {
      counter = *p;
   }
   counter++;
   counter_table.update(&uid, &counter);          // запись в карту обновлённого счётчика
   return 0;
}

// Логика для write
int hello_write(void *ctx) {
   u64 uid;
   u64 counter = 0;
   u64 *p;

   uid = bpf_get_current_uid_gid() & 0xFFFFFFFF;
   p = counter_table.lookup(&uid);
   if (p != 0) {
      counter = *p;
   }
   counter += 10; // Предположим, write стоит «дороже», добавляем 10, чтобы отличить
   counter_table.update(&uid, &counter);
   return 0;
}