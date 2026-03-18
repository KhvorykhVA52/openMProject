/*
    Задание 4, Вариант 2:
    Вычислить: A*b^T + (b*C)^T

    A, C — матрицы m x m
    b    — вектор размера m

    Шаги:
    1. temp1 = A * b^T  (матрица на вектор → вектор)
    2. temp2 = b * C    (вектор на матрицу → вектор)
    3. result = temp1 + temp2  (сумма двух векторов)
*/

#include <iostream>
#include <omp.h>
#include <cmath>
#include <windows.h>
using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    cout << "Задание 4, Вариант 2\n";
    cout << "Формула: A*b^T + (b*C)^T\n";
    cout << "Количество потоков: 4\n\n";

    omp_set_num_threads(4);

    int m = 4;
    double A[] = { 1,2,3,4,
                   5,6,7,8,
                   9,1,2,3,
                   4,5,6,7 };
    double C[] = { 2,1,3,2,
                   4,3,1,2,
                   1,2,4,3,
                   3,1,2,4 };
    double b[] = { 1,2,3,4 };

    // Вывод исходных данных
    cout << "Матрица A:\n";
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) cout << A[i*m+j] << " ";
        cout << "\n";
    }
    cout << "\nМатрица C:\n";
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) cout << C[i*m+j] << " ";
        cout << "\n";
    }
    cout << "\nВектор b: ";
    for (int i = 0; i < m; i++) cout << b[i] << " ";
    cout << "\n\n";

    double temp1[4] = {}, temp2[4] = {};
    double result_s[4], result_p[4];

    // ---- ПОСЛЕДОВАТЕЛЬНЫЙ ----
    for (int i = 0; i < m; i++) {           // temp1 = A * b^T
        temp1[i] = 0;
        for (int k = 0; k < m; k++)
            temp1[i] += A[i*m+k] * b[k];
    }
    for (int j = 0; j < m; j++) {           // temp2 = b * C
        temp2[j] = 0;
        for (int k = 0; k < m; k++)
            temp2[j] += b[k] * C[k*m+j];
    }
    for (int i = 0; i < m; i++)             // result = temp1 + temp2
        result_s[i] = temp1[i] + temp2[i];

    cout << "Последовательный результат: ";
    for (int i = 0; i < m; i++) cout << result_s[i] << " ";
    cout << "\n";

    // ---- ПАРАЛЛЕЛЬНЫЙ ----
    #pragma omp parallel for
    for (int i = 0; i < m; i++) {           // temp1 = A * b^T
        temp1[i] = 0;
        for (int k = 0; k < m; k++)
            temp1[i] += A[i*m+k] * b[k];
    }
    #pragma omp parallel for
    for (int j = 0; j < m; j++) {           // temp2 = b * C
        temp2[j] = 0;
        for (int k = 0; k < m; k++)
            temp2[j] += b[k] * C[k*m+j];
    }
    #pragma omp parallel for
    for (int i = 0; i < m; i++)             // result = temp1 + temp2
        result_p[i] = temp1[i] + temp2[i];

    cout << "Параллельный результат:     ";
    for (int i = 0; i < m; i++) cout << result_p[i] << " ";
    cout << "\n";

    // Проверка
    bool ok = true;
    for (int i = 0; i < m; i++)
        if (fabs(result_s[i] - result_p[i]) > 0.0001) ok = false;
    cout << (ok ? "\nРезультаты совпадают!\n" : "\nОшибка!\n");

    system("pause");
    return 0;
}