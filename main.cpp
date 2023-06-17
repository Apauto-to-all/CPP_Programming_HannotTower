#include<iostream>
#include "hnt.h"
using namespace std;

void HNT_menu::hand_h() {
        string hen(60, '*');
        for (i = 0; i < 5; i++) {
            if (i == 2) cout << "\t\t\t汉诺塔游戏v4.2.8" << endl;
            else cout << endl;
        }
        cout << hen << endl;
}

int main() {
    system("chcp 65001"); // 控制台UTF-8编码
    system("cls");
    cout << "\033[?25l"; // 隐藏光标
    while (1) {
        HNT* h1 = new HNT();
        h1->play();
        delete h1;
        system("cls");
        Sleep(200);
    }
    return 0;
}
