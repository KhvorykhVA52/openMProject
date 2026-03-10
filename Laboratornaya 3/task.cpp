// Лабораторная работа №3
// Решение СЛАУ методами Якоби и Зейделя с использованием OpenMP
#include <iostream>   // чтобы писать в консоль
#include <cmath>      // чтобы считать модуль числа
#include <omp.h>      // чтобы использовать несколько потоков
#include <cstdlib>    // чтобы генерировать случайные числа
#include <iomanip>    // чтобы красиво выводить числа
#include <fstream>    // чтобы писать в файл
#include <windows.h>  // чтобы русские буквы отображались

using namespace std;

// Мы угадываем число. после каждой попытки проверяем насколько наш ответ отличается от предыдущего,
 // если отличие маленькое - значит ответ почти не меняется и можно остановиться 
// maxdifference  проверяет насколько сильно изменилось решение за одну итерацию
double MaxDifference(double *X, double *X_last, int Dimension)
{
    double maxDiff = 0.0;          // пока максимальное отличие = 0
    for (int i = 0; i < Dimension; i++)  // перебираем все числа
     {
    double diff = fabs(X[i] - X_last[i]); // считаем разность (модуль чтобы не было минуса)
    if (diff > maxDiff)
        maxDiff = diff;        // запоминаем если это самое большое отличие
     }
    return maxDiff;                // возвращаем самое большое отличие
}

// Функция CopyArray просто копирует один массив в другой
void CopyArray(double *src, double *dst, int Dimension)
 //src указатель на массив откуда копируем
// dst указатель на массив куда копируем
// Dimension размер массива, чтобы функция знала сколько элементов копировать
{
    for (int i = 0; i < Dimension; i++)
        dst[i] = src[i]; // берём число из src и кладём в dst
}

// Функция SolveParallelYakoby — решает уравнение методом Якоби, но быстро (несколько потоков)
int SolveParallelYakoby(double *A, double *B, int Dimension)
{
    double *X      = new double[Dimension]; // создаём список для текущего ответа
    double *X_last = new double[Dimension]; // создаём список для предыдущего ответа

    // Выделяем памаять для начаольного приближение: x_i = b_i / a_ii
    for (int I = 0; I < Dimension; I++)
        X_last[I] = B[I] / A[I * Dimension + I];

    CopyArray(X_last, X, Dimension); // копируем начальное приближение в X

    const double Epsilon = 0.0001; // требуемая точность решения
    double maxDifference;          // норма разности между итерациями
    int IterationCounter = 0;      // счётчик итераций

    do
    {
        CopyArray(X, X_last, Dimension); // сохраняем X как предыдущее приближение

        // Параллельное вычисление новых значений вектора решения
        // Каждый поток обрабатывает свои строки матрицы независимо
        #pragma omp parallel for  // запускаем следующий цикл сразу в 8 потоков
        for (int I = 0; I < Dimension; I++) // каждый поток считает свои строки
        {
            X[I] = B[I]; // начинаем с правой части уравнения b_i
            for (int J = 0; J < Dimension; J++)
                if (I != J)
                    X[I] -= A[I * Dimension + J] * X_last[J]; // вычитаем a_ij * x_j (старые значения!)
            X[I] = X[I] / A[I * Dimension + I]; // делим на диагональный элемент
        }

        // Проверяем насколько изменилось решение за эту итерацию
        maxDifference = MaxDifference(X, X_last, Dimension);
        IterationCounter++;

    } while (maxDifference > Epsilon); // повторяем пока не достигнута точность

    delete[] X;      // освобождаем выделенную память
    delete[] X_last;
    return IterationCounter;
}

// Метод Якоби — последовательный (один поток)
// Нужен для сравнения скорости с параллельной версией
int SolveSerialYakoby(double *A, double *B, int Dimension)
{
    double *X      = new double[Dimension]; // текущий вектор решения
    double *X_last = new double[Dimension]; // вектор с предыдущей итерации

    // Начальное приближение
    for (int I = 0; I < Dimension; I++)
        X_last[I] = B[I] / A[I * Dimension + I];

    CopyArray(X_last, X, Dimension);

    const double Epsilon = 0.0001;
    double maxDifference;
    int IterationCounter = 0;

    do
    {
        CopyArray(X, X_last, Dimension);

        // Последовательное вычисление — без #pragma omp
        for (int I = 0; I < Dimension; I++)
        {
            X[I] = B[I];
            for (int J = 0; J < Dimension; J++)
                if (I != J)
                    X[I] -= A[I * Dimension + J] * X_last[J];
            X[I] = X[I] / A[I * Dimension + I];
        }

        maxDifference = MaxDifference(X, X_last, Dimension);
        IterationCounter++;

    } while (maxDifference > Epsilon);

    delete[] X;
    delete[] X_last;
    return IterationCounter;
}

// Метод Зейделя — параллельный
// Отличие от Якоби: сразу использует новые значения X — поэтому сходится быстрее
// Внешний цикл последовательный (каждый x зависит от предыдущего)
// Параллелится только внутренний цикл суммирования
int SolveParallelZeidel(const double* A, const double* B, int Dimension)
{
    double* X      = new double[Dimension]; // вектор решения
    double* X_last = new double[Dimension]; // не используется в вычислениях, только для памяти

    const double Epsilon = 0.0001;

    // Начальное приближение
    for (int I = 0; I < Dimension; I++)
        X[I] = B[I] / A[I * Dimension + I];

    double maxDifference;
    int IterationCounter = 0;

    do {
        maxDifference = 0.0;

        // Внешний цикл — последовательный, т.к. каждое x_i зависит от предыдущих
        for (int I = 0; I < Dimension; I++)
        {
            double old = X[I];   // запоминаем старое значение для вычисления нормы
            double sum = 0.0;    // сумма произведений a_ij * x_j

            // Параллельное суммирование с редукцией:
            // каждый поток считает свою часть суммы, результаты складываются в sum
            #pragma omp parallel for reduction(+:sum)
            for (int J = 0; J < Dimension; J++)
                if (I != J)
                    sum += A[I * Dimension + J] * X[J]; // используем НОВЫЕ x_j (отличие от Якоби)

            X[I] = (B[I] - sum) / A[I * Dimension + I]; // новое значение x_i

            // Обновляем максимальное отклонение
            double d = fabs(X[I] - old);
            if (d > maxDifference) maxDifference = d;
        }
        IterationCounter++;

    } while (maxDifference > Epsilon);

    delete[] X;
    delete[] X_last;
    return IterationCounter;
}

// Решение СЛАУ методом Зейделя без параллелизации (последовательная версия)
// Принимает Epsilon и MaxIters для гибкой настройки точности и лимита итераций
// Метод Зейделя — последовательный
// Можно задать свою точность и максимальное число итераций
int SolveSerialZeidel(const double* A, const double* B, int Dimension,
                      double Epsilon, int MaxIters)
{
    double* X      = new double[Dimension];
    double* X_last = new double[Dimension];

    // Начальное приближение
    for (int I = 0; I < Dimension; I++)
        X[I] = B[I] / A[I * Dimension + I];

    double maxDifference;
    int IterationCounter = 0;

    do {
        CopyArray(X, X_last, Dimension); // сохраняем текущее решение
        maxDifference = 0.0;

        for (int I = 0; I < Dimension; I++)
        {
            double s = B[I];
            for (int J = 0; J < Dimension; J++)
                if (I != J)
                    s -= A[I * Dimension + J] * X[J]; // Зейдель: используем обновлённые X
            X[I] = s / A[I * Dimension + I];

            double d = fabs(X[I] - X_last[I]);
            if (d > maxDifference) maxDifference = d;
        }
        IterationCounter++;

        if (IterationCounter >= MaxIters) break; // принудительная остановка при превышении лимита

    } while (maxDifference > Epsilon);

    delete[] X;
    delete[] X_last;
    return IterationCounter;
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    omp_set_num_threads(8); // задаём количество потоков OpenMP = 8

    // Открываем файл для записи результатов экспериментов
    ofstream outFile("results.txt");
    if (!outFile.is_open())
    {
        cerr << u8"Ошибка открытия файла!" << endl;
        return 1;
    }

    // Размерности матриц для тестирования
    int testDimensions[] = {100, 500, 1000, 2000, 3000, 5000};
    int numTests = 6;

    for (int test = 0; test < numTests; test++)
    {
        int Dimension = testDimensions[test];
        cout << u8"\nТест с размерностью " << Dimension << endl;

        // Выделяем память под матрицу A (Dimension x Dimension) и вектор B
        double *A = new double[Dimension * Dimension];
        double *B = new double[Dimension];

        srand(42 + test); // инициализация генератора (42+test для воспроизводимости)

        // Заполняем матрицу A с диагональным доминированием
        // Это необходимое условие сходимости метода Якоби
        for (int i = 0; i < Dimension; i++) {
            for (int j = 0; j < Dimension; j++) {
                if (i == j)
                    A[i * Dimension + j] = rand() % 20 + 10;          // диагональ: 10..29 (большие)
                else
                    A[i * Dimension + j] = (rand() % 10) * (rand() % 2 ? 1 : -1); // остальные: -9..9
            }
        }

        // Заполняем вектор правой части случайными числами от -10 до 9
        for (int i = 0; i < Dimension; i++)
            B[i] = rand() % 20 - 10;

        double start, end;
        int iterParallel, iterSequential, iterZeidel;

        // Замер времени параллельного метода Якоби
        start = omp_get_wtime();
        iterParallel = SolveParallelYakoby(A, B, Dimension);
        end = omp_get_wtime();
        double parallelTime = (end - start) * 1000; // переводим секунды в миллисекунды

        // Замер времени последовательного метода Якоби
        start = omp_get_wtime();
        iterSequential = SolveSerialYakoby(A, B, Dimension);
        end = omp_get_wtime();
        double sequentialTime = (end - start) * 1000;

        // Замер времени параллельного метода Зейделя
        start = omp_get_wtime();
        iterZeidel = SolveParallelZeidel(A, B, Dimension);
        end = omp_get_wtime();
        double zeidelTime = (end - start) * 1000;

        // Вывод результатов в консоль
        cout << u8"Используется потоков: " << omp_get_max_threads() << endl;
        cout << u8"\nСравнение производительности" << endl;
        cout << fixed << setprecision(0);
        cout << u8"Время параллельного (Якоби) решения: "  << parallelTime   << u8" мс" << endl;
        cout << u8"Время последовательного решения: "       << sequentialTime << u8" мс" << endl;
        cout << u8"Время параллельного решения (Зейдель): " << zeidelTime     << u8" мс" << endl;
        cout << u8"Итераций Зейделя: " << iterZeidel   << endl;
        cout << u8"Итераций Якоби: "   << iterParallel << endl;
        cout << u8"Ускорение: " << fixed << setprecision(2)
             << sequentialTime / parallelTime << "x" << endl;

        // Запись результатов в файл в формате CSV через ";"
        outFile << fixed << setprecision(0) << noshowpoint;
        outFile << u8"Размерность: " << Dimension
                << u8"; Параллельное время (Якоби): "  << parallelTime
                << u8"; Время последовательное: "       << sequentialTime << ";\n"
                << u8"Параллельное время (Зейдель): "   << zeidelTime
                << fixed << setprecision(2)
                << u8"; Ускорение: " << sequentialTime / parallelTime << ";\n\n"
                << endl;

        // Освобождаем память после каждого теста
        delete[] A;
        delete[] B;
    }

    outFile.close(); // закрываем файл с результатами
    cout << u8"\nРезультаты сохранены в файл results.txt" << endl;

    return 0;
}