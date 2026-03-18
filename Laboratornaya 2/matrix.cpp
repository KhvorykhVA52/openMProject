
#include <cstdio>
#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);     // для вывода текста
SetConsoleCP(CP_UTF8);           // для ввода текста
    printf("УМНОЖЕНИЕ ДВУХ МАТРИЦ\n");

    int rows1, cols1;
    int rows2, cols2;

    printf("Введите размеры ПЕРВОЙ матрицы (строки и столбцы через пробел): ");
    scanf("%d %d", &rows1, &cols1);

    printf("Введите размеры ВТОРОЙ матрицы (строки и столбцы через пробел): ");
    scanf("%d %d", &rows2, &cols2);

    printf("Первая матрица: %d x %d\n", rows1, cols1);
    printf("Вторая матрица: %d x %d\n", rows2, cols2);

    if (cols1 == rows2) {
        // Создаем матрицы
        double* A = new double[rows1 * cols1];
        double* B = new double[rows2 * cols2];
        double* C = new double[rows1 * cols2];

        // Заполняем матрицу A (чередуем 0 и 1)
        for (int i = 0; i < rows1; i++) {
            for (int j = 0; j < cols1; j++) {
                A[i * cols1 + j] = (i + j) % 2;
            }
        }

        // Заполняем матрицу B (чередуем 0 и 1 по другому)
        for (int i = 0; i < rows2; i++) {
            for (int j = 0; j < cols2; j++) {
                B[i * cols2 + j] = (i * j) % 2;
            }
        }

        // Обнуляем матрицу C
        for (int i = 0; i < rows1 * cols2; i++) {
            C[i] = 0;
        }

        // ВЫЧИСЛЯЕМ УМНОЖЕНИЕ: C = A × B
        for (int i = 0; i < rows1; i++) {
            for (int k = 0; k < cols1; k++) {
                double aik = A[i * cols1 + k];
                for (int j = 0; j < cols2; j++) {
                    C[i * cols2 + j] += aik * B[k * cols2 + j];
                }
            }
        }

        // Выводим матрицу A
        printf("\nМатрица A (%d x %d):\n", rows1, cols1);
        for (int i = 0; i < rows1; i++) {
            printf("  ");
            for (int j = 0; j < cols1; j++) {
                printf("%.0f ", A[i * cols1 + j]);
            }
            printf("\n");
        }

        // Выводим матрицу B
        printf("\nМатрица B (%d x %d):\n", rows2, cols2);
        for (int i = 0; i < rows2; i++) {
            printf("  ");
            for (int j = 0; j < cols2; j++) {
                printf("%.0f ", B[i * cols2 + j]);
            }
            printf("\n");
        }

        // Выводим результат - матрицу C
        printf("\nРЕЗУЛЬТАТ: Матрица C\n", rows1, cols2);
        for (int i = 0; i < rows1; i++) {
            printf("  ");
            for (int j = 0; j < cols2; j++) {
                printf("%.0f ", C[i * cols2 + j]);
            }
            printf("\n");
        }

        // Освобождаем память
        delete[] A;
        delete[] B;
        delete[] C;
    }
    else {
        printf("\nУМНОЖЕНИЕ НЕВОЗМОЖНО!\n");
        printf("  Количество столбцов первой матрицы (%d) != количеству строк второй матрицы (%d)\n", cols1, rows2);
    }

    system("pause");
    return 0;
}