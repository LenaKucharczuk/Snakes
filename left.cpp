#include <windows.h>
#include <iostream>
#include <list>


const int WIDTH = 10;
const int HEIGHT = 10;
const int START_POSITION_Y = 1;
const double START_POSITION_X_LEFT = 0.1;
const double START_POSITION_X_RIGHT = 8.9;
const double DIM = 0.4;
const double SNAKE_WIDTH = 0.2;
const char* L1_STR = "L1";
const char* L2_STR = "L2";

const int UP = 2;
const int DOWN = 4;
const int LEFT = 1;
const int RIGHT = 3;

char szClassName[] = "Snakes";
HWND anotherWindow;
int msgID = RegisterWindowMessage("CHANGE_SCREEN_MSG") ;
int connectMsgId = RegisterWindowMessage("Snakes-connect");
int connectedMsgId = RegisterWindowMessage("Snakes-connected");
bool connected = false;
UINT_PTR timerPtr;

typedef struct Node
{
    double x;
    double y;
} Node;

typedef struct Snake
{
    double dy;
    Node start;
    double width;
    std::list<Node> body;	// pierwsze współrzędne to głowa
    HWND hwnd;
} Snake;

Snake snake;
Snake babySnakes[10];
int direction = DOWN;
boolean playing = FALSE;

void InitializeSnake(Snake *);
void PaintSnake(HDC*, RECT, BOOL visibility = TRUE);
int CountOfPixels(double);
void changeDirection(int newDirection);
boolean move(HWND hwnd, int messageID, RECT rect);

int LEAVE_1 = RegisterWindowMessage(L1_STR);
int LEAVE_2 = RegisterWindowMessage(L2_STR);
int leave1count = 0;
int leave2count = 0;

LONG WINAPI WndProc1(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    InitializeSnake(&snake);

    WNDCLASS wc1;
    HWND hwnd1;
    MSG msg;
    wc1.style = 0; //styl klasy
    wc1.lpfnWndProc = (WNDPROC)WndProc1; 	//adres procedury obsługi zdarzeń
    wc1.cbClsExtra = 0; 	//liczba bajtów obszaru danych klasy
    wc1.cbWndExtra = 0; 	//liczba bajtów obszaru danych okna
    wc1.hInstance = hInstance; 	//uchwyt do instancji aplikacji
    wc1.hIcon = LoadIcon(NULL, IDI_APPLICATION); //uchwyt do ikony
    wc1.hCursor = LoadCursor(NULL, IDC_ARROW); //uchwyt do kursora
    wc1.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //kolor tła
    wc1.lpszMenuName = NULL; 	//nazwa menu
    wc1.lpszClassName = szClassName;//(LPCWSTR)szAppName;	//nazwa klasy
    RegisterClass(&wc1);

    hwnd1 = CreateWindow(
            szClassName,	//nazwa WNDCLASS
            szClassName,	//tytuł okna
            WS_OVERLAPPEDWINDOW, //styl okna
            200, 120,	//pozycja
            400, 500,	//rozmiar
            HWND_DESKTOP,	//uchwyt do okna rodzica
            NULL,	//uchwyt do menu
            hInstance,	//uchwyt do instancji aplikacji
            NULL); //dane (tworzenie okna)
    ShowWindow(hwnd1, nCmdShow);
    UpdateWindow(hwnd1);


    //pętla obsługi wiadomości
    while (GetMessage(&msg, NULL, 0, 0)) //pobranie wiadomości z kolejki
    {
        TranslateMessage(&msg);		//przekształcenie wiadomości (klawiatura)
        DispatchMessage(&msg);		//przekazanie wiad. do proc. obsługi zdarzeń
    }
    return msg.wParam;
}

LRESULT CALLBACK WndProc1(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect;
    const WORD ID_TIMER = 1;
    SetTimer(hwnd, ID_TIMER, 50, NULL);
    GetClientRect(hwnd, &rect);

    //czeka na snake'a, nie dostaje go od razu
    if(message == msgID)
    {
        playing = true;
        //snake.dy = lParam;
        PaintSnake(&hdc, rect, true);
        timerPtr = SetTimer(hwnd, ID_TIMER, 50, NULL);
        if (timerPtr == 0)
        {
            MessageBox(hwnd, "Cannot create timer!", "Fatal error!", MB_ICONSTOP);
        }
    }
    else
        if(!connected && (HWND)wParam != hwnd && message == connectMsgId)
        {
            connected = true;
            anotherWindow = (HWND)wParam;
            PostMessageA(anotherWindow, connectedMsgId, (WPARAM)hwnd, 0);
            timerPtr = SetTimer(hwnd, ID_TIMER, 50, NULL);
            if (timerPtr == 0)
            {
                MessageBox(hwnd, "Cannot create timer!", "Fatal error!", MB_ICONSTOP);
            }
        }
        else
            if(message == connectedMsgId)
            {
                connected = true;
                anotherWindow = (HWND)wParam;
                timerPtr = SetTimer(hwnd, ID_TIMER, 50, NULL);
                if (timerPtr == 0)
                {
                    MessageBox(hwnd, "Cannot create timer!", "Fatal error!", MB_ICONSTOP);
                }
            }
            else
                switch (message)
                {
                case WM_CREATE:
                    if(!connected)
                    {
                        PostMessageA(HWND_BROADCAST, connectMsgId, (WPARAM)hwnd, 0);
                    }
                case WM_PAINT:
                    hdc = BeginPaint(hwnd, &ps);
                    if (playing)
                        PaintSnake(&hdc, rect, TRUE);
                    EndPaint(hwnd, &ps);
                    return 0;
                case WM_KEYDOWN:
                    hdc = GetDC(hwnd);
                    PaintSnake(&hdc, rect, FALSE);
                    switch (wParam)
                    {
                        case VK_LEFT:
                            changeDirection(LEFT);
                            break;
                        case VK_RIGHT:
                            changeDirection(RIGHT);
                            break;
                        case VK_DOWN:
                            changeDirection(DOWN);
                            break;
                        case VK_UP:
                            changeDirection(UP);
                            break;
                    }
                    move(anotherWindow, msgID, rect);
                    PaintSnake(&hdc, rect, TRUE);
                    ReleaseDC(hwnd, hdc);
                case WM_TIMER:
                    hdc = GetDC(hwnd);
                    PaintSnake(&hdc, rect, FALSE);
                    move(anotherWindow, msgID, rect);
                    if(playing)
                        PaintSnake(&hdc, rect, TRUE);
                    else
                        KillTimer(hwnd, timerPtr);
                    ReleaseDC(hwnd, hdc);
                    break;
                case WM_DESTROY:
                    PostQuitMessage(0); //wysłanie wiadomości WM_QUIT
                    return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void InitializeSnake(Snake * snake)
{
    double cm = START_POSITION_X_LEFT;
    snake->start.y = CountOfPixels(cm);
    snake->start.x = CountOfPixels(cm);
    snake->width = CountOfPixels(SNAKE_WIDTH);
    snake->dy = 0.0;
    double dy = SNAKE_WIDTH;

    for (int i = 0; i < 10; ++i)
    {
        double x = snake->start.x;
        double y = dy;
        Node n;
        n.x = x;
        n.y = y;
        (snake->body).push_front(n);
        dy += 8;
    }
}

int CountOfPixels(double cm)
{
    double tst = (double)GetDeviceCaps(GetDC(NULL), LOGPIXELSY);
    return (int)((double)(tst / 2.54)*cm);
}

void PaintSnake(HDC* hdc, RECT rect, BOOL visibility)
{
    HPEN hPen;
    HBRUSH hBrush;
    if (visibility)
    {
        hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
        hBrush = CreateSolidBrush(RGB(255, 0, 0));
    }
    else
    {
        hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
        hBrush = CreateSolidBrush(RGB(255, 255, 255));
    }
    SelectObject(*hdc, hPen);
    SelectObject(*hdc, hBrush);

    for (Node n: snake.body)
        Rectangle(*hdc, n.x, n.y + snake.dy, n.x + snake.width, n.y + snake.dy + snake.width);

    DeleteObject(hPen);
    DeleteObject(hBrush);
}

void changeDirection(int newDirection)
{
    direction = direction % 2 != newDirection % 2 ? newDirection : direction;
}

void stepOn(double x, double y)
{
    Node n;
    n.x = x;
    n.y = y;
    snake.body.push_front(n);
    snake.body.pop_back();
}

boolean move(HWND anotherWindow, int messageID, RECT rect)		// false- gra przegrana
{
    Node head = (Node)snake.body.front();		//biore wspolrzedne glowy
    int x = head.x;
    int y = head.y;
    switch (direction)
    {
        case UP:
            y -= 8;	// y rosna w dół tablicy
            break;
        case DOWN:
            y += 8;
            break;
        case LEFT:
            x -= 8;
            break;
        case RIGHT:
            x += 8;
            break;
    }

    if (x <= rect.left || y >= rect.bottom || y < rect.top)
    {
        return false;
    }
    else
        if (x >= rect.right)
        {
            playing=FALSE;
            PostMessage(anotherWindow, messageID, 0, y);
        }

    stepOn(x, y);
    return true;
}
