#include <omp.h>      // подключаем  OpenMP для параллельных функций
#include <stdio.h>    // подключаем ввод-вывод
int main() {          // главная функция
    printf("Processors: %d\n", omp_get_num_procs());   // сколько ядер у процессора
    
    // Параллельная область 
    #pragma omp parallel                // создаем команду потоков
    printf("Hello from thread %d\n", omp_get_thread_num());   // каждый поток печатает свой номер(начинается с 0)
    
    // Параллельные циклы с расписанием 
    #pragma omp parallel for schedule(static, 10)   // делим цикл на блоки по 10 итераций, раздаем потокам
    for (int i = 0; i < 20; i++)        // у нас 20 итераций, значит будет 2 блока, итерации (0-9, 10-19)
        printf("i=%d, thread=%d\n", i, omp_get_thread_num());   // печатаем итерацию и номер потока
    
    // Секции 
    #pragma omp parallel sections       // создаем потоки для параллельных секций
    {
        #pragma omp section             // первая секция
        printf("Section A\n");          // выполнится каким-то потоком
        #pragma omp section            // вторая секция  
        printf("Section B\n");         // выполнится другим потоком (могут выполняться одновременно)
    }
    
    // Редукция
    int sum = 0;                       // общая переменная
    #pragma omp parallel for reduction(+:sum)   // каждый поток считает свою часть, потом складываем
    for (int i = 1; i <= 100; i++)     // суммируем числа от 1 до 100
        sum += i;                      // каждый поток добавляет свои числа
    printf("Sum 1..100 = %d\n", sum);  // печатаем результат (1 + 100) × 100 / 2 = 101 × 50 = 5050)
    
    return 0;                         // конец программы
    
}