/*
    Эта программа умножает матрицу на вектор двумя способами и сравнивает их скорость.
    
    Способ 1 (Serial) - обычное умножение. Один поток процессора считает всё сам,
    строка за строкой. Медленно, зато просто.
    
    Способ 2 (Parallel) - то же умножение, но строки делятся между несколькими потоками.
    Поток 0 считает одни строки, поток 1 другие - работают одновременно.
    
    На маленьких матрицах (размер например,  10) параллельный даже медленнее - процессор
    тратит время на создание потоков, а работы мало. На больших матрицах
    параллельный выигрывает в несколько раз.
    
    Результат: время обоих способов выводится на экран и сохраняется в файл
*/
#include <omp.h>      // библиотека OpenMP для параллельного программирования
#include <cstdio>     // printf, scanf, fopen, fprintf, fclose
#include <cmath>      // математические функции (sqrt и др.)
#include <windows.h>  // SetConsoleOutputCP, system("pause")

// Инициализация: ввод размера, выделение памяти, заполнение данными
void ProcessInit (double* &pMatrix, double* &pVector, double* &pResult, int &Size) { //double* указатель, т.e адрес в памяти где лежат числа типа double
    // & — ссылка на указатель, чтобы функция могла изменить сам указатель
    printf("Введите размер матрицы: "); // запрос размера у пользователя
    scanf("%d", &Size);                 // считываем размер в переменную Size
    
    pMatrix = new double[Size * Size];  // выделяем память под матрицу N×N
    pVector = new double[Size];         // выделяем память под вектор длиной N
    pResult = new double[Size];         // выделяем память под вектор результата
    
    for (int i = 0; i < Size * Size; i++) pMatrix[i] = 0.0; // заполняем матрицу нулями
    for (int i = 0; i < Size; i++) {
        pMatrix[i * Size + i] = 1.0; // ставим 1 на главной диагонали (единичная матрица)
        pVector[i] = 1.0;            // заполняем вектор единицами
        pResult[i] = 0.0;            // обнуляем вектор результата
    }
    
    printf("Размер: %d\n", Size); // выводим подтверждение введённого размера
}

// Последовательное умножение матрицы на вектор
void SerialProduct (double* pMatrix, double* pVector, double* pResult, int Size) {
    double start = omp_get_wtime();       // запоминаем время начала
    for (int i = 0; i < Size; i++) {      // цикл по строкам матрицы i — номер строки матрицы
        pResult[i] = 0.0;                 // обнуляем i-й элемент результата 
        for (int j = 0; j < Size; j++) {  // цикл по столбцам (элементам строки)  j — номер столбцa
            pResult[i] += pMatrix[i * Size + j] * pVector[j]; // скалярное произведение строки на вектор
        }
    }
    double end = omp_get_wtime();              // запоминаем время окончания =  сколько секунд ушло на умножение
    FILE* file = fopen("results.txt", "a");    // файл куда записываются результаты 
    if (file) { // проверка что файл открыл успешно
        fprintf(file, "Size=%d, Serial=%.6f сек\n", Size, end - start); // пишем результат в файл  %d — целое число,  %.6f — дробное с 6 знаками после запятой 

        fclose(file);                          // закрываем файл
    }
    printf("Последовательное: время = %.6f сек\n", end - start); // выводим время на экран
}

// Параллельное умножение матрицы на вектор (разложение по строкам)
void ParallelProduct (double* pMatrix, double* pVector, double* pResult, int Size) {
    double start = omp_get_wtime(); // запоминаем время начала
    int i, j;                       // объявляем переменные цикла заранее (требование OpenMP)
    #pragma omp parallel for private(j) // распределяем итерации по i между потоками; j — своя у каждого
    for (i=0; i< Size; i++)             // каждый поток обрабатывает свои строки
        { for (j=0; j< Size; j++) {     // цикл по элементам строки (у каждого потока своя j)
            pResult[i] += pMatrix[i*Size+j]*pVector[j]; // накапливаем сумму строки × вектор
        } 
    }
    double end = omp_get_wtime();           // запоминаем время окончания
    FILE* file = fopen("results.txt", "a"); // открываем файл в режиме дозаписи
    if (file) {
        fprintf(file, "Size=%d, Parallel=%.6f сек\n", Size, end - start); // пишем в файл
        fclose(file);                       // закрываем файл
    }
    printf("Параллельное: время = %.6f сек\n", end - start); // выводим время на экран
}

// Освобождение памяти
void ProcessTerminate (double* pMatrix, double* pVector, double* pResult, int Size) {
    delete[] pMatrix; // освобождаем память матрицы
    delete[] pVector; // освобождаем память вектора
    delete[] pResult; // освобождаем память результата
}

int main () {
    SetConsoleOutputCP(CP_UTF8); // устанавливаем кодировку вывода консоли UTF-8 (для кириллицы)
    SetConsoleCP(CP_UTF8);       // устанавливаем кодировку ввода консоли UTF-8
    double* pMatrix; // указатель на матрицу
    double* pVector; // указатель на вектор
    double* pResult; // указатель на вектор результата
    int Size;        // размер матрицы

    ProcessInit (pMatrix, pVector, pResult, Size);    // инициализация данных
    SerialProduct (pMatrix, pVector, pResult, Size);  // последовательное умножение
    ParallelProduct (pMatrix, pVector, pResult, Size);// параллельное умножение
    ProcessTerminate(pMatrix, pVector, pResult, Size);// освобождение памяти

    system("pause"); // ждём нажатия клавиши перед закрытием консоли
    return 0;        // завершаем программу
}