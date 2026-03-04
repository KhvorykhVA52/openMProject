/*
    Эта программа проверяет три разных способа ускорить умножение матрицы на вектор
    с помощью нескольких потоков процессора.
    
    Способ A (по строкам) - матрица делится на горизонтальные полосы.
    Каждый поток получает свои строки и считает их независимо.
    Никто никому не мешает - поэтому этот способ самый быстрый.
    
    Способ B (по столбцам) - для каждой строки отдельно запускаем несколько потоков
    которые суммируют элементы этой строки. Проблема в том что запуск потоков
    происходит для каждой строки заново - при размере 1000 это 1000 запусков.
    Из-за этого способ самый медленный.
    
    Способ C (блочный) - матрица делится на прямоугольные блоки.
    Каждый поток получает свой блок и работает с ним независимо.
    Блок небольшой и помещается в быструю память процессора (кэш) -
    поэтому данные читаются быстро и способ работает хорошо.
    
    Программу запускают несколько раз с разными размерами матриц.
    Всё время записывается в файл results_1.txt и переносится в Excel для графиков.
*/
#include <omp.h>      // библиотека OpenMP для параллельного программирования
#include <cstdio>     // printf, scanf, fopen, fprintf, fclose
#include <cmath>      // sqrt()
#include <windows.h>  // SetConsoleOutputCP, system("pause")
#include <string>     // работа со строками

// Инициализация: запрос размера, выделение памяти, заполнение данными
void ProcessInit (double* &pMatrix, double* &pVector, double* &pResult, int &Size) {
    printf("Введите размер матрицы: ");   // вывод приглашения
    scanf("%d", &Size);                   // чтение размера с клавиатуры
    
    pMatrix = new double[Size * Size];    // выделение памяти под матрицу Size×Size
    pVector = new double[Size];           // выделение памяти под вектор
    pResult = new double[Size];           // выделение памяти под вектор результата
    
    for (int i = 0; i < Size * Size; i++) pMatrix[i] = 0.0;  // обнуляем всю матрицу
    for (int i = 0; i < Size; i++) {
        pMatrix[i * Size + i] = 1.0;  // ставим 1 на главной диагонали (единичная матрица)
        pVector[i] = 1.0;             // заполняем вектор единицами
        pResult[i] = 0.0;             // обнуляем вектор результата
    }
    
    printf("Размер: %d\n", Size);  // подтверждение введённого размера
}

// Последовательное умножение матрицы на вектор
void SerialProduct (double* pMatrix, double* pVector, double* pResult, int Size) {
    double start = omp_get_wtime();          // засекаем время начала
    for (int i = 0; i < Size; i++) {
        pResult[i] = 0.0;                    // обнуляем i-й элемент результата
        for (int j = 0; j < Size; j++) {
            pResult[i] += pMatrix[i * Size + j] * pVector[j];  // скалярное произведение i-й строки на вектор
        }
    }
    double end = omp_get_wtime();            // засекаем время конца
    FILE* file = fopen("results_1.txt", "a");  // открываем файл в режиме дозаписи
    if (file) {
        fprintf(file, "Size=%d, Serial=%.6f сек\n", Size, end - start);  // пишем результат в файл
        fclose(file);                        // закрываем файл
    }
    printf("Последовательное: время = %.6f сек\n", end - start);  // выводим время на экран
}

// Параллельное умножение: разложение по строкам (каждый поток считает свои строки)
void ParallelProduct_A (double* pMatrix, double* pVector, double* pResult, int Size) {
    double start = omp_get_wtime();  // засекаем время начала
    int i, j;
    #pragma omp parallel for private (j)  // параллелим внешний цикл, j — своя переменная у каждого потока
    for (i=0; i< Size; i++) 
        { for (j=0; j< Size; j++) { 
            pResult[i] += pMatrix[i*Size+j]*pVector[j];  // каждый поток считает свои строки результата
        } 
    }
    double end = omp_get_wtime();                    // засекаем время конца
    FILE* file = fopen("results_1.txt", "a");        // открываем файл в режиме дозаписи
    if (file) {
        fprintf(file, "Size=%d, Rows_Parallel=%.6f сек\n", Size, end - start);  // пишем в файл
        fclose(file);
    }
    printf("A: %.6f сек\n", end - start);  // выводим время на экран
}

// Параллельное умножение: разложение по столбцам (reduction — каждый поток накапливает свою сумму)
void ParallelProduct_B (double* pMatrix, double* pVector, double* pResult, int Size) {
    double start = omp_get_wtime();  // засекаем время начала
    
    int i, j;
    double IterSum = 0;              // накопитель суммы для одной строки
    for (i=0; i< Size; i++) {
        IterSum = 0;                 // обнуляем сумму перед каждой строкой
        #pragma omp parallel for reduction (+:IterSum)  // каждый поток считает свою часть суммы, потом всё складывается
        for (j=0; j< Size; j++) IterSum += pMatrix[i*Size+j]*pVector[j];  // умножение элементов строки на вектор
        pResult[i] = IterSum;        // сохраняем итоговую сумму в результат
    }

    double end = omp_get_wtime();                   // засекаем время конца
    FILE* file = fopen("results_1.txt", "a");       // открываем файл в режиме дозаписи
    if (file) {
        fprintf(file, "Size=%d, Columns_Parallel=%.6f сек\n", Size, end - start);  // пишем в файл
        fclose(file);
    }
    printf("B: %.6f сек\n", end - start);  // выводим время на экран
}

// Параллельное умножение: блочное разбиение (каждый поток работает со своим блоком матрицы)
void ParallelProduct_C (double* pMatrix, double* pVector, double* pResult, int Size) {
    double start = omp_get_wtime();              // засекаем время начала
    int ThreadID;                                // номер текущего потока
    int GridThreadNum = 8;                       // количество потоков
    int GridSize = int(sqrt(double(GridThreadNum)));  // размер сетки потоков (sqrt(8) ≈ 2)
    int BlockSize = Size / GridThreadNum;        // размер блока на один поток
    omp_set_num_threads(GridThreadNum);          // устанавливаем количество потоков
    
    #pragma omp parallel private(ThreadID)       // каждый поток имеет свой ThreadID
    {
        int ThreadID = omp_get_thread_num();     // получаем номер текущего потока (0..7)
        double* pThreadResult = new double[Size]; // локальный массив результата для этого потока
        for (int i = 0; i < Size; i++) pThreadResult[i] = 0.0;  // обнуляем локальный результат
        
        int i_start = (int(ThreadID/GridSize))*BlockSize;  // начальная строка блока для этого потока
        int j_start = (ThreadID%GridSize)*BlockSize;       // начальный столбец блока для этого потока
        double IterResult;
        for (int i = 0; i < BlockSize; i++) {              // проходим по строкам своего блока
            IterResult = 0;                                 // обнуляем промежуточный результат
            for (int j = 0; j < BlockSize; j++) {          // проходим по столбцам своего блока
                IterResult += pMatrix[(i + j_start) * Size + j_start + j] * pVector[i + j_start]; // умножаем элемент блока на вектор
                pThreadResult[i + j_start] = IterResult;   // сохраняем в локальный результат
            }
        }
        
        #pragma omp critical                               // только один поток за раз входит в этот блок
        for (int i = 0; i < Size; i++) pResult[i] += pThreadResult[i];  // суммируем локальные результаты в общий
        
        delete[] pThreadResult;  // освобождаем локальный массив
    }
    double end = omp_get_wtime();                   // засекаем время конца
    FILE* file = fopen("results_1.txt", "a");       // открываем файл в режиме дозаписи
    if (file) {
        fprintf(file, "Size=%d, Blocks_Parallel=%.6f сек\n", Size, end - start);  // пишем в файл
        fclose(file);
    }
    printf("C: %.6f сек\n", end - start);  // выводим время на экран
}

// Освобождение памяти
void ProcessTerminate (double* pMatrix, double* pVector, double* pResult, int Size) {
    delete[] pMatrix;  // освобождаем память матрицы
    delete[] pVector;  // освобождаем память вектора
    delete[] pResult;  // освобождаем память вектора результата
}

int main () {
    SetConsoleOutputCP(CP_UTF8);  // устанавливаем кодировку вывода консоли UTF-8 (для русских букв)
    SetConsoleCP(CP_UTF8);        // устанавливаем кодировку ввода консоли UTF-8
    double* pMatrix;              // указатель на матрицу
    double* pVector;              // указатель на вектор
    double* pResult;              // указатель на вектор результата
    int Size;                     // размер матрицы и векторов

    ProcessInit (pMatrix, pVector, pResult, Size);     // инициализация: ввод размера, выделение памяти
    SerialProduct(pMatrix, pVector, pResult, Size);    // последовательное умножение
    ParallelProduct_A (pMatrix, pVector, pResult, Size); // параллельное: по строкам
    ParallelProduct_B (pMatrix, pVector, pResult, Size); // параллельное: по столбцам (reduction)
    ParallelProduct_C (pMatrix, pVector, pResult, Size); // параллельное: блочное разбиение
    ProcessTerminate(pMatrix, pVector, pResult, Size); // освобождение памяти

    system("pause");  // пауза перед закрытием консоли
    return 0;         // успешное завершение программы
}