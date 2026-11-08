# АК-2. Лабораторна робота 5 — модулі ядра Linux (частина 2)

Проєкт лабораторної роботи 4 розділено на два модулі:

* **`hello1`** — експортує функцію `print_hello()` (`EXPORT_SYMBOL`), веде список
  `struct hello_event`. Перед і після друку `Hello, world!` (через `pr_info`)
  записує час `ktime_get()` у поля `start` і `end`. При вивантаженні друкує для
  кожного виклику час початку і скільки наносекунд зайняв друк, потім звільняє пам'ять.
* **`hello2`** — має параметр `count` і викликає `print_hello()` з `hello1`
  `count` разів. Зі списком не працює.

## Файли

| Файл           | Призначення                                                         |
|----------------|---------------------------------------------------------------------|
| `inc/hello1.h` | спільний заголовок: `struct hello_event`, прототип `print_hello()`  |
| `hello1.c`     | модуль, що експортує `print_hello()` і працює зі списком            |
| `hello2.c`     | модуль з параметром `count`, викликає `print_hello()`               |
| `Makefile`     | збирання обох модулів (kbuild), `make clean`, `make checkpatch`     |

Каталог `inc` додано до шляхів пошуку заголовків у kbuild-частині Makefile:

```make
ccflags-y := -I$(src)/inc
```

Тому в `*.c` достатньо написати `#include "hello1.h"` без шляху.

## Повідомлення

| Рівень     | Де      | Коли                                                     |
|------------|---------|----------------------------------------------------------|
| `pr_info`  | hello1  | `Hello, world!`, завантаження/вивантаження, час кожного друку |
| `pr_err`   | hello1  | не вдалося виділити пам'ять під елемент списку (`-ENOMEM`) |
| `pr_warn`  | hello2  | `count` = 0 або 5…10 — модуль завантажується              |
| `pr_err`   | hello2  | `count` > 10 — `init` повертає `-EINVAL`                  |
| `pr_info`  | hello2  | вивантаження                                              |

## Збирання (крос-компіляція під ARM)

```bash
export ARCH=arm
export CROSS_COMPILE="ccache arm-linux-gnueabihf-"
export KDIR=$HOME/repos/linux-stable

make              # -> hello1.ko, hello2.ko
make checkpatch
modinfo hello1.ko
modinfo hello2.ko   # depends: hello1
```

## Запуск у QEMU

Як у лабораторній роботі 4, тільки копіюються два модулі:

```bash
cp ~/repos/lab5/hello1.ko ~/repos/lab5/hello2.ko ~/repos/busybox/_install/root/
```

У QEMU:

```sh
cd /root

# V. hello2 без hello1 — помилка "Unknown symbol print_hello"
insmod hello2.ko
dmesg | tail -n 2

insmod hello1.ko
insmod hello2.ko                       # count=1
cat /sys/module/hello2/parameters/count
rmmod hello2

insmod hello2.ko count=0               # pr_warn
rmmod hello2
insmod hello2.ko count=3
rmmod hello2
insmod hello2.ko count=7               # pr_warn
cat /sys/module/hello2/parameters/count
rmmod hello2
insmod hello2.ko count=15              # pr_err, -EINVAL

rmmod hello1                           # час кожного друку
lsmod

dmesg | grep -E 'hello1|hello2'
```

`rmmod hello1`, поки завантажено `hello2`, не спрацює: `hello2` використовує
символ із `hello1` (`lsmod` показує `Used by: hello2`).

## Чому `hello2` не завантажується без `hello1`

`hello2.ko` містить невизначений символ `print_hello`. `insmod` нічого не знає про
залежності: він просто передає файл ядру, а ядро шукає символ у таблиці
експортованих символів уже завантажених модулів. `hello1` не завантажено, символу
немає, тому ядро відмовляє (`hello2: Unknown symbol print_hello (err -2)`), а
`insmod` друкує `unknown symbol in module`. `modprobe hello2` спершу сам
завантажив би `hello1` за залежністю з `modules.dep` (поле `depends: hello1` видно
в `modinfo hello2.ko`).
