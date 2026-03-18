/*
    Задание 3, Вариант 1:
    Подсчёт определённого интеграла методом левых и правых прямоугольников.

    Метод левых прямоугольников:  высота = f(левый край отрезка)
    Метод правых прямоугольников: высота = f(правый край отрезка)
    Площадь каждого прямоугольника = высота * ширина (h)
*/

#include <iostream>
#include <omp.h>
#include <cmath>
#include <windows.h>
using namespace std;

double f(double x) { return x * x; } // f(x) = x²

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    cout << "Вариант: 1\n";
    cout << "Метод левых и правых прямоугольников\n";
    cout << "Количество потоков: 4\n\n";

    omp_set_num_threads(4);

    double a = 0, b = 10;
    int n = 10000000;
    double h = (b - a) / n;
    double exact = (b*b*b - a*a*a) / 3.0; // точное значение = 333.333

    cout << "f(x) = x^2\n";
    cout << "Интервал: [" << a << ", " << b << "]\n";
    cout << "Разбиений: " << n << "\n";
    cout << "Точное значение: " << exact << "\n\n";

    double t0 = omp_get_wtime();
    double sumL = 0;
    #pragma omp parallel for reduction(+:sumL)
    for (int i = 0; i < n; i++)
        sumL += f(a + i * h);       // левая граница: x = a + i*h
    double resL = sumL * h;
    double timeL = omp_get_wtime() - t0;

    cout << "Левые прямоугольники:\n";
    cout << "  Результат:   " << resL << "\n";
    cout << "  Погрешность: " << fabs(resL - exact) << "\n";
    cout << "  Время:       " << timeL << " сек\n\n";

    t0 = omp_get_wtime();
    double sumR = 0;
    #pragma omp parallel for reduction(+:sumR)
    for (int i = 1; i <= n; i++)
        sumR += f(a + i * h);       // правая граница: x = a + i*h
    double resR = sumR * h;
    double timeR = omp_get_wtime() - t0;

    cout << "Правые прямоугольники:\n";
    cout << "  Результат:   " << resR << "\n";
    cout << "  Погрешность: " << fabs(resR - exact) << "\n";
    cout << "  Время:       " << timeR << " сек\n";

    system("pause");
    return 0;
}