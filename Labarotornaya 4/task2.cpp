/*
    Задание 2, Вариант 1:
    В матрице A(m,n) найдите наименьший и наибольший элементы.
*/

#include <iostream>
#include <omp.h>
#include <windows.h>
using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    int rows = 4, cols = 5;
    double A[4][5] = {
        { 3,  7,  2, 11,  5},
        {-4,  8,  0,  6, -2},
        { 9,  1, 14,  3,  7},
        { 5, -8,  4,  2, 10}
    };

    cout << "Вариант: 1\n";
    cout << "Количество потоков: 4\n\n";

    cout << "Матрица " << rows << "x" << cols << ":\n";
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            cout << A[i][j] << "\t";
        cout << "\n";
    }

    omp_set_num_threads(4);

    double minVal =  1e18;
    double maxVal = -1e18;

    // Каждый поток ищет min/max в своей части
    // reduction — в конце берётся общий min и max
    #pragma omp parallel for reduction(min:minVal) reduction(max:maxVal)
    for (int i = 0; i < rows * cols; i++) {
        double val = A[i / cols][i % cols];
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    cout << "\nНаименьший элемент: " << minVal << "\n";
    cout << "Наибольший элемент: " << maxVal << "\n";

    system("pause");
    return 0;
}