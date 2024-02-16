#include <stdio.h>
#include <stdlib.h>
#include <termios.h>  // tcgetattr , tcsetattr
#include <unistd.h>   // usleep

// Определение размеров доски
#define MAXROWS 25
#define MAXCOLUMNS 80

// Определение текущего состояния игровой доски
int currentBoard[MAXROWS][MAXCOLUMNS];

// Структура для хранения настроек терминала
static struct termios stored_settings;

int aliveNeighbor(int board[MAXROWS][MAXCOLUMNS], int row,
                  int column);  // функция для подсчета живых соседей вокруг клетки
int aliveNeighborsInRow(int board[MAXROWS][MAXCOLUMNS], int testRow, int row,
                        int column);  // функция для подсчета живых соседей в конкретной строке
void printBoardCurrentState();  // ф-я выводы текущего состояния игровой доски
void setBoardNextState();  // ф-я установки следующего состояния игровой доски в соответствии с правилами
                           // Конвея
int speedTime(int *speed);         // ф-я установки скорости игры
void inputCase(int option);        // ф-я обработки ввода пользователя
void inputFromFile(char *string);  // ф-я чтения начальной конфигурации мз файла
void clrscr();                     // функция очистки экрана
void set_keypress(void);           // ф-я установки терминала в режим клавиш

int main() {
    char option[64] = "";  // для ввода пользователя ( какую из опций пользователь выберет)
    int sleepingTime;      // время ожидания (влияет на скорость)
    clrscr();
    speedTime(&sleepingTime);  // выбираем скорость
    clrscr();

    // Вывод начальной информации об игре и запрос выбора начальной конфигурации

    printf("если захочешь отрегулировать скорость, нажми + или - ; для выхода жми q\n");
    printf("Выбери число от 1 до 5:\n");

    // получение допустимого ввода польз-ля для выбора начальной конфигурации
    while (atoi(option) < 1 || atoi(option) > 5) {  // atoi - переделывает из str в int
        fgets(option, sizeof(option),
              stdin);  // считывает строку из поток ввода и сохраняет ее в массив символов
    }
    inputCase(atoi(option));  // Ввод из файла ( основан на выборе польз-ля)

    fd_set rfds;  // Настройка переменных для обработки ввода польз-ля
    struct timeval tv;  //  структураБ которая исп для задания временного интервала

    set_keypress();  // установка терминала в режим клавиш

    while (1) {
        int fval;  // переменная для хранения рез-та функции select()Б которая ожидает ввода от пользователя
        char c = '\0';  // нулевой символ, для хранения символа, считанного с клавиатуры(если он есть, мы
                        // ничего не ввели)
        clrscr();
        printBoardCurrentState();
        setBoardNextState();

        // Подготовка для ожидания ввода от польз-ля
        FD_ZERO(&rfds);  // сбрасывает все биты в 0, чтобы начать с чистого листа
        FD_SET(0, &rfds);  // отслеживания стандартного ввода
        tv.tv_sec = 0;     // установка таймера в секундах
        tv.tv_usec = 0;    //установка таймера в микросекундах

        // Ожидание ввода или истечения таймера
        fval = select(2, &rfds, NULL, NULL, &tv);

        if (fval) {           // проверка на ввод от поль-ля
            c = getc(stdin);  // считываем символ и сохраняем его в С
        }
        if ((c == 'q') || (c == 'Q')) break;
        if (c == '-')
            if (sleepingTime < 125000) sleepingTime = sleepingTime + 5000;
        if (c == '+')
            if (sleepingTime >= 25000) sleepingTime = sleepingTime - 5000;
        usleep(sleepingTime);  // приостановка выполнения при изменении скорости
    }
    return 0;
}

int speedTime(int *speed) {
    char opt[64];  // создаем массив символов; 64 - стандартная длина ввода
    printf("Давай выберем скорость, нажми число от 1 до 5\n");
    do {
        fgets(opt, sizeof(opt), stdin);  // получение ввода от польз-ля
        if (atoi(opt) == 1) {
            *speed = 1000 * 100;
            break;
        } else if (atoi(opt) == 2) {
            *speed = 1000 * 75;
            break;
        } else if (atoi(opt) == 3) {
            *speed = 1000 * 50;
            break;
        } else if (atoi(opt) == 4) {
            *speed = 1000 * 25;
            break;
        } else if (atoi(opt) == 5) {
            *speed = 1000 * 1;
            break;
        }
    } while (atoi(opt) < 1 ||
             atoi(opt) > 5);  // повторяем, пока ввод не будет соответсвовать диапазону от 1 до 5
    return *speed;
}

// функция для выбора файла
void inputCase(int option) {
    switch (option) {
        case 1:
            inputFromFile("case1.txt");
            break;
        case 2:
            inputFromFile("case2.txt");
            break;
        case 3:
            inputFromFile("case3.txt");
            break;
        case 4:
            inputFromFile("case4.txt");
            break;
        case 5:
            inputFromFile("case5.txt");
            break;
    }
}
//Функция для загрузки начального распределения на доску из файлв
void inputFromFile(char *string) {
    FILE *f;                 // объявление указателя на файл
    f = fopen(string, "r");  // открываем файл для чтения
    // Выделение памяти под массив строк
    char **str = (char **)malloc(MAXCOLUMNS * MAXROWS * sizeof(char) + MAXROWS * sizeof(char *));
    char *ptr = (char *)(str + MAXROWS);  // указатель на определенную строку в памяти
    int n = 0;                            // перемеc c. C. нная для подсчета строк
    while (!feof(f)) {  // чтение строк из файла и сохранение их в массив (пока не конец файла)
        str[n] = ptr + MAXCOLUMNS * n;  // задаем адреса для строк внутри выделенной памяти
        fgets(str[n], MAXCOLUMNS, f);  // чтение строки из файла
        n++;
    }

    // преобразование символов в цифры и заполнение текущей доски
    for (int i = 0; i < MAXROWS; i++) {
        for (int j = 0; j < MAXCOLUMNS; j++) {
            if (str[i][j] == 'O') {
                currentBoard[i][j] = 1;
            } else {
                currentBoard[i][j] = 0;
            }
        }
    }
    // вывод загруженных строк на экран
    for (int i = 0; i < n - 1; i++) {
        puts(str[i]);  //вывод на экран
    }
    free(str);
    fclose(f);
}

void clrscr() { system("clear"); }

// подсчет живых соседей для данной клетки
int aliveNeighbor(int board[MAXROWS][MAXCOLUMNS], int row, int column) {
    int aliveNeighbors = 0;
    int testRow = 0;

    // проверка  нахождения текущей клетки на верхней строке
    if (row == 0) {
        testRow = MAXROWS - 1;  // если даБ устанавливаем testrow на нижнюю границу
    } else {
        testRow = row - 1;  // в противном случае - на строку выше
    }
    // подсчет живых соседей в верхней строке
    aliveNeighbors += aliveNeighborsInRow(board, testRow, row, column);

    testRow = row;  // устанавливаем testRow на текущую строку

    aliveNeighbors +=
        aliveNeighborsInRow(board, testRow, row, column);  // подсчет живых соседей в текущ строке
    // проверка на нахождение клетки на последней строке
    if (row == MAXROWS - 1) {
        testRow = 0;  // если да - устанавливаем на верхнюю строку
    } else {
        testRow = row + 1;  // нет - на строку ниже
    }

    aliveNeighbors += aliveNeighborsInRow(board, testRow, row, column);  // подсчет соседей в нижней строке

    return aliveNeighbors;  // возвращаем число соседей
}

// подсчет живых соседей в соседнем столбце
int aliveNeighborsInRow(int board[MAXROWS][MAXCOLUMNS], int testRow, int row, int column) {
    int aliveNeighbors = 0;
    int testColumn = 0;  // тестируем соседние столбцы

    if (column == 0) {  // проверка нахождения кледки в крайнем левом столбце
        testColumn = MAXCOLUMNS - 1;  // если да - то testColumn устанавливаем на крайний правый столбец
    } else {
        testColumn = column - 1;  // если нет - то берем столбец слева
    }

    aliveNeighbors += board[testRow][testColumn];  // осуществляем подсчет живых соседей

    testColumn = column;  // назначаем testColumn на текущий столбец
    // если условие выполняется, добавляем живых соседей в текущем столбце
    if (testRow != row) {
        aliveNeighbors += board[testRow][testColumn];
    }
    // проверка нахождения кледки в крайнем правом столбце
    if (column == MAXCOLUMNS - 1) {
        testColumn = 0;  // если да - то testColumn устанавливаем на крайний левый  столбец
    } else {
        testColumn = column + 1;  // если нет - то берем столбец справа
    }

    aliveNeighbors += board[testRow][testColumn];  // считаем соседей в соседнем столбце

    return aliveNeighbors;
}

// выводим текущее состояние доски в консоль
void printBoardCurrentState() {
    for (int i = 0; i < MAXROWS; i++) {
        for (int j = 0; j < MAXCOLUMNS; j++) {
            if (currentBoard[i][j]) {  // проверка является ли текущая клетка живой
                printf("0");
            } else {
                printf(" ");
            }
        }

        printf("\n");
    }
}

// установка следующего состояния доски
void setBoardNextState() {
    int nextBoard[MAXROWS][MAXCOLUMNS];  // создаем новую доску для следующего состояния

    for (int i = 0; i < MAXROWS; i++) {
        for (int j = 0; j < MAXCOLUMNS; j++) {
            int neighbors = 0;

            neighbors = aliveNeighbor(currentBoard, i, j);  // считаем соседей для текущ клетки

            if ((currentBoard[i][j] && neighbors == 2) || neighbors == 3) {
                nextBoard[i][j] = 1;
            } else {
                nextBoard[i][j] = 0;
            }
        }
    }
    // меняем старые значения клетов на новые
    for (int i = 0; i < MAXROWS; i++) {
        for (int j = 0; j < MAXCOLUMNS; j++) {
            currentBoard[i][j] = nextBoard[i][j];
        }
    }
}
// установка режима нажатия клавиш без ожидания Enter
void set_keypress(void) {
    struct termios new_settings;  // структура для новых настроек терминала
    tcgetattr(0, &stored_settings);  // получаем текущие настройки терминала и сохраняем их в stored_settings
    new_settings = stored_settings;  // копируем текущие настройки для внесения изменений
    new_settings.c_lflag &=
        (~ICANON & ~ECHO);  //отключаем введение по Enter  и отоброжение символов при вводе
    new_settings.c_cc[VTIME] = 0;  // время ожидания 0
    new_settings.c_cc[VMIN] = 1;  // чтение начинается, если введен хоть один символ
    tcsetattr(0, TCSANOW, &new_settings);  // применение новых настроек терминала (НЕМЕДЛЕННО)
}
