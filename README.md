# Курсовой проект по предмету "Дискретный анализ" (5 семестр)

Задание: Необходимо реализовать два известных метода сжатия данных для сжатия одного файла. Методы сжатия:
1. Кодирование по Хаффману
2. Алгоритм LZW

Формат запуска должен быть аналогичен формату запуска программы gzip. Должны поддерживаться следующие ключи: -c, -d, -k, -l, -r, -t, -1, -9. Должно поддерживаться указание символа дефиса в качестве стандартного ввода.

** Демострация работы программы: **

Тестирование на тексте произведения «Слово о полку Игореве»:
MacBook-Pro-MacBook:~ macbookpro$ ./archiver -9 input7.txt
MacBook-Pro-MacBook:~ macbookpro$ ./archiver -k -d input7.txt.z
MacBook-Pro-MacBook:~ macbookpro$ ./archiver -t input7.txt.z
The file is correct
MacBook-Pro-MacBook:~ macbookpro$ ./archiver -l input7.txt.z
Options:
Uncompressed file size: 39416
Compressed file size: 15377
Ratio: 61%
Uncompressed file name: input7.txt
MacBook-Pro-MacBook:~ macbookpro$
