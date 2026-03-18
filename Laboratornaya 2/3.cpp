/*
    Эта программа делает то же самое что 2.cpp, но умножает матрицу на матрицу,
    а не на вектор. Это намного тяжелее - при размере 1600 нужно выполнить
    миллиарды умножений, поэтому разница между способами особенно заметна.
    
    Serial - обычное умножение тремя вложенными циклами, один поток.
    Самое медленное - при размере 1600 занимает около 25 секунд.
    
    Rows (по строкам) - несколько потоков делят строки результирующей матрицы между собой.
    Простой способ, даёт хорошее ускорение.
    
    Columns (по столбцам) - меняем порядок циклов чтобы данные читались подряд из памяти.
    Процессор любит читать память подряд - поэтому этот способ быстрее чем Rows.
    
    Blocks (блочный) - матрица делится на маленькие квадратные блоки 64х64.
    Каждый блок целиком помещается в быструю память процессора и не вытесняется
    пока мы его считаем. Из-за этого самый быстрый способ - при размере 1600
    работает в 12 раз быстрее чем Serial.
    
    Размеры для экспериментов: 128, 256, 512, 800, 1024, 1600.
    Время записывается в файл results_mm.txt, графики строятся в performance_mm.xlsx.
*/
#include <omp.h>        // OpenMP — параллельные директивы (#pragma omp)
#include <cstdio>       // printf, scanf, fopen, fprintf
#include <cstdlib>      // _aligned_malloc, _aligned_free
#include <windows.h>    // SetConsoleOutputCP, CreateProcessW
#include <string>       // std::wstring

// Выделяет память под матрицу N*N с выравниванием на 64 байта (размер кэш-линии)
// inline — тело функции вставляется в место вызова вместо обычного прыжка
// static — функция видна только в этом файле
static inline double* alloc_mat(int N) {
    return (double*)_aligned_malloc((size_t)N * N * sizeof(double), 64); // (size_t) — чтобы N*N не переполнилось при больших N
}
static inline void free_mat(double* p) { _aligned_free(p); } // освобождает память, выделенную _aligned_malloc

// Заполняет матрицы A и B детерминированными значениями
static void fill_mats(double* A, double* B, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i*N + j] = (double)((i + 1) % 97) * 0.01 + (double)((j + 3) % 89) * 0.001; // % — остаток от деления, не даёт числам быть слишком большими
            B[i*N + j] = (double)((i + 5) % 83) * 0.02 + (double)((j + 7) % 71) * 0.002; // разные коэффициенты чтобы A и B отличались
        }
    }
}

// Обнуляет матрицу C параллельно
static void zero_mat(double* C, int N) {
    const size_t NN = (size_t)N * N;          // общее кол-во элементов
    #pragma omp parallel for schedule(static)  // делим итерации между потоками равными кусками
    for (ptrdiff_t i = 0; i < (ptrdiff_t)NN; i++) C[i] = 0.0; // ptrdiff_t — знаковый тип достаточного размера для индексов
}

// Записывает время в файл и выводит на экран
static void write_time_ms(const char* method, int N, double ms, bool writeToFile) {
    if (writeToFile) {
        FILE* f = fopen("results_mm.txt", "a");                          // "a" — дозапись в конец файла
        if (f) { fprintf(f, "N=%d, %s=%.3f ms\n", N, method, ms); fclose(f); }
    }
    printf("N=%d, %s: %.3f ms\n", N, method, ms); // вывод на экран всегда
}

// Последовательное умножение матриц (три вложенных цикла i-j-k)
static void SerialMM(double* A, double* B, double* C, int N, bool check) {
    zero_mat(C, N);                  // обнуляем результат перед умножением
    double t0 = omp_get_wtime();     // засекаем время начала
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double sum = 0.0;            // локальная переменная для накопления суммы
            for (int k = 0; k < N; k++) {
                sum += A[i*N + k] * B[k*N + j]; // скалярное произведение строки A на столбец B
            }
            C[i*N + j] = sum;            // записываем результат в C[i][j]
        }
    }
    double t1 = omp_get_wtime();                             // засекаем время конца
    write_time_ms("Serial", N, (t1 - t0) * 1000.0, check);  // *1000 — переводим секунды в мс
}

// Параллельное умножение: разложение по строкам (каждый поток считает свои строки i)
static void ParallelMM_Rows(double* A, double* B, double* C, int N) {
    zero_mat(C, N);
    double t0 = omp_get_wtime();

    #pragma omp parallel for  // делим цикл по i между потоками, каждый пишет в свою строку C — нет конфликтов
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            for (int k = 0; k < N; k++) sum += A[i*N + k] * B[k*N + j];
            C[i*N + j] = sum;
        }
    }

    double t1 = omp_get_wtime();
    write_time_ms("Rows_Parallel", N, (t1 - t0) * 1000.0, true);
}

// Параллельное умножение: порядок циклов i-k-j (лучше для кэша чем i-j-k)
static void ParallelMM_Columns(double* A, double* B, double* C, int N) {
    zero_mat(C, N);
    double t0 = omp_get_wtime();

    #pragma omp parallel for  // параллелим внешний цикл по i
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < N; k++) {
            double aik = A[i*N + k];      // вытаскиваем A[i][k] один раз, а не N раз в цикле j
            for (int j = 0; j < N; j++) {
                C[i*N + j] += aik * B[k*N + j]; // умножаем на всю строку B[k] — подряд в памяти, эффективно для кэша
            }
        }
    }

    double t1 = omp_get_wtime();
    write_time_ms("Columns_Parallel", N, (t1 - t0) * 1000.0, true);
}

// Параллельное блочное умножение: матрица делится на блоки BS*BS
static void ParallelMM_Blocks(double* A, double* B, double* C, int N, int BS) {
    zero_mat(C, N);
    double t0 = omp_get_wtime();

    #pragma omp parallel for collapse(2) schedule(static) // collapse(2) — склеивает два цикла (i0 и j0) в один большой, больше задач для потоков
    for (int i0 = 0; i0 < N; i0 += BS) {       // i0 — начало блока по строкам
        for (int j0 = 0; j0 < N; j0 += BS) {   // j0 — начало блока по столбцам

            const int i_max = (i0 + BS < N) ? (i0 + BS) : N; // правая граница блока по i (тернарный оператор: условие ? если_true : если_false)
            const int j_max = (j0 + BS < N) ? (j0 + BS) : N; // правая граница блока по j

            for (int k0 = 0; k0 < N; k0 += BS) {              // k0 — начало блока по общему измерению
                const int k_max = (k0 + BS < N) ? (k0 + BS) : N;

                for (int i = i0; i < i_max; i++) {
                    double* Ci = C + (size_t)i * N;         // указатель на строку i матрицы C (быстрее чем C[i*N+j] каждый раз)
                    const double* Ai = A + (size_t)i * N;   // указатель на строку i матрицы A

                    for (int k = k0; k < k_max; k++) {
                        const double aik = Ai[k];            // кэшируем A[i][k] — используется N раз в цикле j
                        const double* Bk = B + (size_t)k * N; // указатель на строку k матрицы B

                        for (int j = j0; j < j_max; j++) {
                            Ci[j] += aik * Bk[j]; // C[i][j] += A[i][k] * B[k][j]
                        }
                    }
                }
            }
        }
    }

    double t1 = omp_get_wtime();
    write_time_ms("Blocks_Parallel", N, (t1 - t0) * 1000.0, true);
}

// Вычисляет контрольную сумму по диагонали (проверка что результат не NaN/мусор)
static double checksum(const double* C, int N) {
    double s = 0.0;
    for (int i = 0; i < N*N; i += (N + 1)) s += C[i]; // шаг N+1 — это элементы главной диагонали: C[0][0], C[1][1], ...
    return s;
}

// Запускает внешний процесс mm_to_xlsx.exe для построения графиков
static void run_converter() {
    std::wstring cmd = L"mm_to_xlsx.exe";       // L"..." — строка в Unicode (wide string)
    STARTUPINFOW si{}; si.cb = sizeof(si);      // структура с параметрами запуска процесса, {} — обнуляем
    PROCESS_INFORMATION pi{};                   // структура куда Windows запишет дескриптор нового процесса

    std::wstring cmdline = cmd;                 // CreateProcessW требует изменяемый буфер
    if (CreateProcessW(nullptr, cmdline.data(),
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {

        WaitForSingleObject(pi.hProcess, INFINITE); // ждём пока конвертер завершится

        CloseHandle(pi.hThread);   // освобождаем дескриптор потока
        CloseHandle(pi.hProcess);  // освобождаем дескриптор процесса
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8); // консоль выводит UTF-8 (кириллица не крякозябры)
    SetConsoleCP(CP_UTF8);       // консоль принимает ввод в UTF-8

    const int threads = 8;           // количество потоков OpenMP
    omp_set_num_threads(threads);    // устанавливаем число потоков
    printf("Потоков: %d\n", threads);

    const int Ns[] = {128, 256, 512, 800, 1024, 1600};              // массив размеров матриц для экспериментов
    const int numN = (int)(sizeof(Ns) / sizeof(Ns[0]));             // автоподсчёт количества элементов: размер массива / размер элемента

    for (int t = 0; t < numN; t++) {
        int N = Ns[t];
        printf("\n===== N=%d =====\n", N);

        double* A = alloc_mat(N);  // выделяем память под матрицу A (N*N)
        double* B = alloc_mat(N);  // выделяем память под матрицу B
        double* C = alloc_mat(N);  // выделяем память под результат C
        if (!A || !B || !C) { printf("Не хватило памяти на N=%d\n", N); return 1; } // проверка что память выделилась

        fill_mats(A, B, N); // заполняем A и B значениями

        SerialMM(A, B, C, N, false);              // прогрев кэша — первый запуск a
        printf("Checksum: %.6f\n", checksum(C, N)); // проверяем что результат корректный

        SerialMM(A, B, C, N, true);       // повторный замер — уже пишем в файл
        ParallelMM_Rows(A, B, C, N);      // параллельное по строкам
        ParallelMM_Columns(A, B, C, N);   // параллельное порядок i-k-j
        
        int BS = 64;                       // размер блока 64x64 (оптимально под кэш L1/L2)
        ParallelMM_Blocks(A, B, C, N, BS); // параллельное блочное

        free_mat(A); free_mat(B); free_mat(C); // освобождаем память
    }

    run_converter(); // запускаем построение графиков

    system("pause"); // ждём нажатия клавиши перед закрытием
    return 0;
}