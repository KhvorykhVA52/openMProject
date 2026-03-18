/*
    Задание 1, Вариант 1:
    Дано число n. На i-ой нити вычислить n^i.
    Затем найти сумму всех полученных чисел.
*/

#include <iostream>
#include <omp.h>
#include <cmath>
#include <windows.h>
using namespace std;

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    int n = 3;
    int num_threads = 4;

    cout << "Вариант: 1\n";
    cout << "Число n = " << n << "\n";
    cout << "Количество нитей: " << num_threads << "\n\n";

    omp_set_num_threads(num_threads);

    double results[4] = {};  // каждая нить пишет в свою ячейку

    #pragma omp parallel
    {
        int i = omp_get_thread_num();
        results[i] = pow(n, i); // нить i считает n^i
    }

    // Выводим результаты каждой нити по порядку
    double sum = 0;
    for (int i = 0; i < num_threads; i++) {
        cout << "Нить " << i << ": " << n << "^" << i << " = " << results[i] << "\n";
        sum += results[i];
    }

    cout << "\nСумма всех n^i = " << sum << "\n";

    system("pause");
    return 0;
}