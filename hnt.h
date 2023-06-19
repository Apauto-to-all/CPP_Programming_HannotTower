#include <iostream>
#include <string>
#include <io.h>
#include <fcntl.h>
#include <Windows.h>
#include <conio.h>
#include <functional>
#include <fstream>
#include <ctime>
#include <sstream>
//哈希表
#include< unordered_map>
//播放音频
#include <mmsystem.h>
#include<dsound.h>
#pragma comment(lib, "WINMM.LIB")
//保留3位小数
#include <iomanip>
//产生随机字符串
#include <random>
//处理json文件
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std;


// 汉诺塔游戏/菜单
class HNT_menu {
protected:
    int i, j, k;
protected: //登入
    string wrong = ""; // 用于输出错误
    static string username; //储存用户名
    string password = ""; //储存密码
    int sf_dr = 0; //是否登入判断，默认为0还没登入
    int yk = 0; //游客模式，开启为1
    // 需要更新的数据
    int dr_xz = 1; //登入选择模式，默认先选择第一个选项
    int cg = 0;//闯关模式，开启为1，关闭为0正常模式
    int game_se = 1; // 音效，默认开启

protected: //菜单
    int cd_xz = 1; //菜单选择模式，默认先选择第一个选项
    bool isKeyPressed[256] = { false }; // 记录按键是否被按下
    int choose_ks = 0; //是否选择“开始游戏”
    int choose_sz = 0; //设置
    int choose_tc = 0; //退出
    int tc_sf = 1; //选中退出的“是”与“否”
    int cd_out = 0; // 是否退出菜单
    string sf_se = "on"; 

protected://音效大全
    //音效，上下移动
    inline void se_choice() {
        PlaySound(TEXT("se/choice.wav"), NULL, SND_ASYNC);
    }
    //音效，选中
    inline void se_xuan() {
        PlaySound(L"se/xuan.wav", NULL, SND_ASYNC);
    }
    //音效，返回
    inline void se_tui() {
        PlaySound(TEXT("se/tui.wav"), NULL, SND_ASYNC);
    }
    //音效，左右移动
    inline void se_dian() {
        PlaySound(TEXT("se/dian.wav"), NULL, SND_ASYNC);
    }
    //音效，放
    inline void se_fang() {
        PlaySound(TEXT("se/fang.wav"), NULL, SND_ASYNC);
    }
    //音效，赢
    inline void se_win() {
        PlaySound(TEXT("se/win.wav"), NULL, SND_ASYNC);
    }
    //音乐播放
    HMMIO hFile;
    MMCKINFO ckRIFF = { 0 }, ckData = { 0 };
    BYTE* pData = new BYTE[ckData.cksize];
    DWORD bytesRead = mmioRead(hFile, reinterpret_cast<HPSTR>(pData), ckData.cksize);
    HWAVEOUT hWaveOut = nullptr;
    WAVEHDR waveHdr = { 0 };
    WAVEFORMATEX wavFormat;
    //背景音乐
    void playLoop(LPCTSTR filePath)
    {
        // 打开WAV文件
        hFile = mmioOpen(const_cast<LPTSTR>(filePath), nullptr, MMIO_ALLOCBUF | MMIO_READ);
        if (!hFile)
        {
            // 打开文件失败
            return;
        }
        // 获取WAV文件头信息
        ckRIFF = { 0 };
        ckData = { 0 };
        ckRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
        if (mmioDescend(hFile, &ckRIFF, nullptr, MMIO_FINDRIFF) != MMSYSERR_NOERROR)
        {
            // 找不到RIFF块
            mmioClose(hFile, 0);
            return;
        }
        ckData.ckid = mmioFOURCC('d', 'a', 't', 'a');
        if (mmioDescend(hFile, &ckData, &ckRIFF, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
        {
            // 找不到数据块
            mmioClose(hFile, 0);
            return;
        }
        // 创建音频播放设备
        wavFormat = { 0 };
        wavFormat.wFormatTag = WAVE_FORMAT_PCM;
        wavFormat.nChannels = 2;
        wavFormat.nSamplesPerSec = 48000;
        wavFormat.wBitsPerSample = 16;
        wavFormat.nBlockAlign = 4;
        wavFormat.nAvgBytesPerSec = wavFormat.nSamplesPerSec * wavFormat.nBlockAlign;

        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wavFormat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
        {
            // 创建音频播放设备失败
            mmioClose(hFile, 0);
            return;
        }
        // 播放音频数据
        pData = new BYTE[ckData.cksize];
        bytesRead = mmioRead(hFile, reinterpret_cast<HPSTR>(pData), ckData.cksize);
        if (bytesRead != ckData.cksize)
        {
            // 读取数据失败
            delete[] pData;
            mmioClose(hFile, 0);
            waveOutClose(hWaveOut);
            return;
        }
        waveHdr.lpData = reinterpret_cast<LPSTR>(pData);
        waveHdr.dwBufferLength = ckData.cksize;
        waveHdr.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
        waveHdr.dwLoops = INFINITE;

        if (waveOutPrepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        {
            // 准备缓冲区失败
            delete[] pData;
            mmioClose(hFile, 0);
            waveOutClose(hWaveOut);
            return;
        }

        if (waveOutWrite(hWaveOut, &waveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        {
            // 播放音频数据失败
            delete[] pData;
            mmioClose(hFile, 0);
            waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR));
            waveOutClose(hWaveOut);
            return;
        }
    }
    // 清除bgm内存
    void cleanup_bgm()
    {
        // 停止播放
        waveOutReset(hWaveOut);

        // 卸载缓冲区
        if (waveOutUnprepareHeader(hWaveOut, &waveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        {
            // 卸载缓冲区失败，需要手动释放
            delete[] reinterpret_cast<BYTE*>(waveHdr.lpData);
        }

        // 关闭音频设备
        if (waveOutClose(hWaveOut) != MMSYSERR_NOERROR)
        {
            // 关闭音频设备失败，需要手动释放
            delete[] reinterpret_cast<BYTE*>(waveHdr.lpData);
        }
        hWaveOut = nullptr;

        // 关闭WAV文件
        mmioClose(hFile, 0);
        hFile = nullptr;

        // 释放动态分配的内存
        delete[] reinterpret_cast<BYTE*>(waveHdr.lpData);
    }
    // 更新bgm
    inline void update_yx() {
        if (sf_dr == 1) {
            ifstream ifs("jr.json");
            json u_jr;
            ifs >> u_jr;
            ifs.close();
            game_se = u_jr.at(username).at("sz").at(0);
            if (game_se == 1) sf_se = "on";
            else sf_se = "off";
        }
    }

private: //登入
    string user_name = ""; // 临时用户名
    void dr_choice(unsigned short key) {
        switch (key)
        {
        case VK_UP:
        case 'W':
            if (!isKeyPressed[key]) {
                if (dr_xz != 1) {
                    dr_xz--;
                    se_choice();
                    wrong = "";
                }
                else wrong = "无法向上移动\a";
                dr_show();
                isKeyPressed[key] = true;
            }
            break;
        case VK_DOWN:
        case 'S':
            if (!isKeyPressed[key]) {
                if (dr_xz != 3) {
                    dr_xz++;
                    se_choice();
                    wrong = "";
                }
                else wrong = "无法向下移动\a";
                dr_show();
                isKeyPressed[key] = true;
            }
            break;
        case VK_RETURN:
            if (!isKeyPressed[key]) {
                se_xuan();
                cin.clear();
                cin.ignore(1024, '\n');
                system("cls");
                if (dr_xz == 1) login();
                else if (dr_xz == 2) sign_up();
                else {
                    username = password = "";
                    yk = 1;
                }
                isKeyPressed[key] = true;
            }
            break;
        default:
            break;
        }
    }
    //局部刷新，实现选项移动
    void dr_show() {
        if (dr_xz == 1) {
            cout << "\033[1;34m\033[10;H\t\t->\t已有账户？登入\033[0m"; //亮蓝色
            cout << "\033[13;H\033[2K\t\t\t没有账户？注册";
            cout << "\033[16;H\033[2K\t\t\t游客使用\033[0m";
        }
        else if (dr_xz == 2) {
            cout << "\033[10;H\033[2K\t\t\t已有账户？登入\033[0m";
            cout << "\033[1;34m\033[13;H\t\t->\t没有账户？注册\033[0m";
            cout << "\033[16;H\033[2K\t\t\t游客使用\033[0m";
        }
        else {
            cout << "\033[10;H\033[2K\t\t\t已有账户？登入\033[0m";
            cout << "\033[13;H\033[2K\t\t\t没有账户？注册\033[0m";
            cout << "\033[1;34m\033[16;H\t\t->\t游客使用\033[0m";
        }
        if (wrong != "") {
            cout << "\033[1;33m\033[20;H" << wrong;
        }
        else cout << "\033[20;H\033[2K";
        cout << "\033[u";
    }
    //有账户登入
    void login() {
        cout << "\033[?25h";
        hand_h();
        cout << "友情提示：请勿输入中文，如果不想登入，输入“b”退出" << "\n";
        while (1) {
            cout << "\n\t\t请输入账户名:";
            getline(cin, username);
            if (username == "b") {
                sf_dr = 1;
                cout << "\033[?25l";
                return;
            }
            if (username.empty()) {
                cout << "\n输入为空，请重新输入" << endl;
                continue;
            }
            cout << "\n\t\t输入密码：";
            getline(cin, password);
            if (password == "b") {
                sf_dr = 1;
                cout << "\033[?25l";
                return;
            }
            //验证账户和密码的正确性
            ifstream ifs("up.json");
            json js_j = json::parse(ifs);
            ifs.close();
            if (js_j.count(username) != 0 && js_j[username] == password) cout << "登入成功" << endl;
            else {
                cout << "\n\t\t账户名或密码出现问题，请重新登入。\n\t\t";
                continue;
            }
            Sleep(500);
            system("cls");
            sf_dr = 1;
            break;
        }
        cout << "\033[?25l";
        update_yx();
    }
    //无账户注册
    void sign_up() {
        cout << "\033[?25h";
        string choice;
        hand_h();
        cout << "友情提示：请勿输入中文，如果不想注册，输入“b”退出" << endl;
        while (1) {
            cout << "\n\t\t请输入想注册的账户名:";
            getline(cin, user_name);
            if (user_name.empty()) {
                cout << "\n输入为空，请重新输入" << endl;
                continue;
            }
            if (user_name == "b") {
                sf_dr = 1;
                cout << "\033[?25l";
                return;
            }
            // 检测账户名是否正确
            ifstream ifs("up.json");
            json js_j = json::parse(ifs);
            ifs.close();
            if (js_j.count(user_name) != 0) {
                cout << "\t\t已有人注册过该账户名，请重新输入" << endl;
                continue;
            }
            cout << "\n\t\t您输入的账户名是：" << user_name << endl;
            cout << "\t\t请再次确认（回车确认）";
            getline(cin, choice);
            if (choice.empty()) break;
            else {
                continue;
            }
        }
        while (1) {
            cout << "\n\t\t请输入密码：";
            getline(cin, password);
            if (password.empty()) {
                cout << "\n输入为空，请重新输入" << endl;
                continue;
            }
            if (password == "b") {
                sf_dr = 1;
                cout << "\033[?25l";
                return;
            }
            cout << "\n\t\t您输入的密码是：" << password << endl;
            cout << "\t\t请再次确认（回车确认）";
            getline(cin, choice);
            if (choice == "") break;
            else {
                continue;
            }
        }
        cout << "\033[?25l";
        username = user_name;
        json jup;
        // 从文件读取 JSON 数据
        ifstream file_in_up("up.json");
        file_in_up >> jup;
        // 添加新属性到 jup 对象中
        jup[username] = password;
        // 将更新后的 JSON 数据写入文件
        ofstream file_out_up("up.json");
        file_out_up << jup.dump(4) << endl;
        json jjr;
        // 从文件读取 JSON 数据
        ifstream file_in_jr("jr.json");
        file_in_jr >> jjr;
        // 创建名为 username 的新对象,向 jjr[username] 对象中添加多个键值对
        jjr[username]["1"] = { 0, 0.0, "" }; //难度1：通过次数，最高得分，最高得分创下时间
        jjr[username]["2"] = { 0, 0.0, "" };
        jjr[username]["3"] = { 0, 0.0, "" };
        jjr[username]["4"] = { 0, 0.0, "" };
        jjr[username]["5"] = { 0, 0.0, "" };
        jjr[username]["6"] = { 0, 0.0, "" };
        jjr[username]["7"] = { 0, 0.0, "" };
        jjr[username]["8"] = { 0, 0.0, "" };
        jjr[username]["fire"] = { 0, 0.0, "" }; //征服之路：达到的最高得分时的最高难度，最高得分，最高得分创下时间
        jjr[username]["sz"] = { 1, }; //储存设置，是否开启音效，1为开启
        // 将更新后的 JSON 数据写入文件
        ofstream file_out_jr("jr.json");
        file_out_jr << jjr.dump(4) << endl;
        file_out_jr.close();
        cout << "注册成功，现登入中……" << endl;
        Sleep(500);
        system("cls");
        sf_dr = 1; //显示已经登入
    }
    // 登入界面
    void hnt_dr() {
        while (1) {
            system("cls");
            string hen(60, '*');
            // 检测文件是否存在
            ifstream ifs11("up.json");
            ifstream ifs22("jr.json");
            if (!ifs11.good() && !ifs22.good()) { // 文件不存在
                json j = json({});
                // 创建第一个文件
                ofstream ofs1("up.json");
                ofs1 << j << endl;
                ofs1.close();
                // 创建第二个文件
                ofstream ofs2("jr.json");
                ofs2 << j << endl;
                ofs2.close();
            }
            hand_h();
            cout << "\t请选择：\n\n" << endl;
            cout << "\t\t\t已有账户？登入\n\n\n\t\t\t没有账户？注册\n\n" << endl;
            cout << "\t\t\t游客使用\n\n" << endl;
            cout << hen << endl;
            for (i = 0; i < 5; i++) cout << endl;
            cout << "\033[s";//储存光标位置
            Sleep(500);
            dr_show();
            while (1) {
                for (i = 0; i < 256; i++) {
                    if (GetAsyncKeyState(i) & 0x8000) dr_choice(i);
                    else isKeyPressed[i] = false; // 按键松开后重置状态
                }
                if (sf_dr == 1 || yk == 1) {
                    dr_xz = 1;
                    break;
                }
            }
            if (username == "b" || user_name == "b" || password == "b") {
                user_name = username = password = "";
                sf_dr = 0;
            }
            else break;
        }
    }

private: //菜单
    // 菜单主体
    void menu_choice(unsigned short key) {
        switch (key)
        {
        case VK_UP:
        case 'W':
            if (!isKeyPressed[key]) {
                if (cd_xz != 1) {
                    cd_xz--;
                    if (game_se == 1) se_choice();
                    wrong = "";
                }
                else wrong = "无法向上移动\a";
                menu_show();
                isKeyPressed[key] = true;
            }
            break;
        case VK_DOWN:
        case 'S':
            if (!isKeyPressed[key]) {
                if (choose_sz == 1) {
                    if (cd_xz != 5) {
                        cd_xz++;
                        if (game_se == 1) se_choice();
                        wrong = "";
                    }
                    else wrong = "无法向下移动\a";
                }
                else if (choose_tc == 1 || choose_ks == 1) {
                    if (cd_xz != 2) {
                        cd_xz++;
                        if (game_se == 1) se_choice();
                        wrong = "";
                    }
                    else wrong = "无法向下移动\a";
                }
                else {
                    if (cd_xz != 3) {
                        cd_xz++;
                        if (game_se == 1) se_choice();
                        wrong = "";
                    }
                    else wrong = "无法向下移动\a";
                }
                menu_show();
                isKeyPressed[key] = true;
            }
            break;
        case VK_RETURN:
            if (!isKeyPressed[key]) {
                if (game_se == 1) se_xuan();
                cin.clear();
                cin.ignore(1024, '\n');
                sz_xz();
                all_cd_show();
                isKeyPressed[key] = true;
            }
            break;
        case VK_BACK:
            if (!isKeyPressed[key]) {
                if (game_se == 1) se_tui();
                system("cls");
                if (choose_ks == 0 && choose_sz == 0 && choose_tc == 0) {
                    wrong = "无法再返回\a";
                }
                else {
                    choose_ks = 0;
                    choose_sz = 0;
                    choose_tc = 0;
                    cd_xz = 1;
                    wrong = "";
                }
                all_cd_show();
                isKeyPressed[key] = true;
            }
            break;
        }
    }
    string generateRandomString() {
        const string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        const int len = 4;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, chars.length() - 1);
        string str(len, '*');
        do {
            for (int i = 0; i < len; i++) {
                str[i] = chars[dis(gen)];
            }
        } while (str.find_first_of("0123456789") == string::npos ||
            str.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz") == string::npos);
        return str;
    }
    //设置选项
    void sz_xz() {
        system("cls");
        if (choose_ks == 0 && choose_sz == 0 && choose_tc == 0) {
            if (cd_xz == 1) choose_ks = 1;
            else if (cd_xz == 2) {
                choose_sz = 1;
                cd_xz = 1;
            }
            else {
                choose_tc = 1;
                cd_xz = 1;
            }
        }
        else {
            if (choose_ks == 1) { //开始游戏选项
                if (cd_xz == 1) cd_out = 1;
                else {
                    cd_out = 1;
                    cg = 1;
                }
            }
            if (choose_sz == 1) { // 设置选项
                if (cd_xz == 1) { // 开关音效
                    game_se = (game_se + 1) % 2;
                    if (game_se == 1) sf_se = "on";
                    else sf_se = "off";
                    if (sf_dr == 1) {
                        ifstream ifs("jr.json");
                        json u_jr;
                        ifs >> u_jr;
                        ifs.close();
                        u_jr[username]["sz"][0] = game_se;
                        ofstream ofs("jr.json");
                        ofs << u_jr.dump(4) << endl;
                        ofs.close();
                    }
                }
                else if (cd_xz == 2) { // 查看个人战绩
                    jr_show();
                }
                else if (cd_xz == 3) {
                    zf_road();
                }
                else if (cd_xz == 4) { //登入或退出登入
                    if (sf_dr == 1) {
                        username = password = "";
                        cout << "已登出" << endl;
                        system("pause");
                        sf_dr = 0;
                    }
                    else {
                        yk = 0;
                        hnt_dr();
                    }
                }
                else if (cd_xz == 5) {//注销账户
                    zs_zh();
                }
            }
            if (choose_tc == 1 && cd_xz == 1) exit(0); //退出选项
            else {
                choose_tc = 0;
                cd_xz = 1;
            }
        }
    }
    //注销账户
    void zs_zh() {
        system("cls");
        hand_h();
        if (sf_dr == 1) {
            cout << "\033[?25h";
            cout << "\n\t\t是否注销账号——" << username << endl;
            cout << "\t\t注意：此操作会永久删除该账户。" << endl;
            string scdh = generateRandomString(), sc = "";
            cout << "\n\t\t删除代号：" << scdh << endl;
            cout << "\n\t\t如果你不想注销，输入“b”退出" << endl;
            cout << "\n\t\t如果你确定删除该账户，请输入删除代号：";
            getline(cin, sc);
            while (sc != scdh) {
                if (sc == "b") return;
                cout << "\t\t！！！输入错误！！！\n" << endl;
                cout << "\t\t请输入正确的删除代号：";
                getline(cin, sc);
            }
            ifstream ifs1("up.json");
            ifstream ifs2("jr.json");
            json j1, j2;
            ifs1 >> j1;
            ifs2 >> j2;
            // 删除 JSON 对象中的某个键对
            j1.erase(username);
            j2.erase(username);
            // 将修改后的 JSON 数据保存到文件中
            ofstream ofs1("up.json");
            ofstream ofs2("jr.json");
            ofs1 << j1.dump(4) << endl;
            ofs2 << j2.dump(4) << endl;
            cout << "\t\t已永久删除" << endl;
            sf_dr = 0;
            username = password = "";
            yk = 1;
            system("pause");
            cout << "\033[?25l";
        }
        else {
            cout << "\n未登入账户，无法注销\n" << endl;
            system("pause");
        }
    }
    inline void all_cd_show() {
        system("cls");
        cd_sx();
        Sleep(200);
        menu_show();
    }
    // 局部更新，实现选项移动
    void menu_show() {
        if (choose_ks == 1) {
            if (cd_xz == 1) {
                cout << "\033[8;H\033[2K\033[1;34m\t\t->\t正常模式\033[0m"; //亮蓝色
                cout << "\033[11;H\033[2K\t\t\t征服之路\033[0m";
                cout << "\033[15;H\033[2K\033[1;30m“正常模式”——自主选择难度\033[0m";
            }
            else if (cd_xz == 2) {
                cout << "\033[8;H\033[2K\t\t\t正常模式\033[0m";
                cout << "\033[11;H\033[2K\033[1;34m\t\t->\t征服之路\033[0m";
                cout << "\033[15;H\033[2K\033[1;31m“征服之路”————从易到难，不断进阶，最终见证得分之巅！\033[0m";
            }
        }
        if (choose_sz == 1) {
            cout << "\033[8;H\033[2K\t\t\t开关音效：" << sf_se << "\033[0m"; //亮蓝色
            cout << "\033[11;H\033[2K\t\t\t查看个人战绩\033[0m";
            cout << "\033[14;H\033[2K\t\t\t“征服之路”排行榜\n\n";
            if (sf_dr == 1) cout << "\033[17;H\033[2K\t\t\t退出登入\033[0m";
            else cout << "\033[17;H\033[2K\t\t\t登入账户\033[0m";
            cout << "\033[20;H\033[2K\t\t\t注销账户\033[0m";
            if (cd_xz == 1) {
                cout << "\033[8;H\033[1;34m\033[2K\t\t->\t开关音效："<<sf_se<<"\033[0m";
            }
            else if (cd_xz == 2) {
                cout << "\033[11;H\033[1;34m\033[2K\t\t->\t查看个人战绩\033[0m";
            }
            else if (cd_xz == 3) {
                cout << "\033[14;H\033[1;34m\033[2K\t\t->\t“征服之路”排行榜\n\n";
            }
            else if (cd_xz == 4) {
                if (sf_dr == 1) cout << "\033[17;H\033[1;34m\033[2K\t\t->\t退出登入\033[0m";
                else cout << "\033[17;H\033[1;34m\033[2K\t\t->\t登入账户\033[0m";
            }
            else if (cd_xz == 5) {
                cout << "\033[20;H\033[1;34m\033[2K\t\t->\t注销账户\033[0m";
            }
        }
        if (choose_tc == 1) {
            if (cd_xz == 1) {
                cout << "\033[8;H\033[1;34m\t\t->\t是\033[0m"; //亮蓝色
                cout << "\033[11;H\033[2K\t\t\t否\033[0m";
            }
            else if (cd_xz == 2) {
                cout << "\033[8;H\033[2K\t\t\t是\033[0m";
                cout << "\033[11;H\033[1;34m\t\t->\t否\033[0m";
            }
        }
        if (choose_ks == 0 && choose_sz == 0 && choose_tc == 0) {
            cout << "\033[8;H\033[2K\t\t\t开始游戏\033[0m";
            cout << "\033[11;H\033[2K\t\t\t设置\033[0m";
            cout << "\033[14;H\033[2K\t\t\t退出\033[0m";
            if (cd_xz == 1) {
                cout << "\033[8;H\033[2K\033[1;34m\t\t->\t开始游戏\033[0m";
            }
            else if (cd_xz == 2) {
                cout << "\033[11;H\033[2K\033[1;34m\t\t->\t设置\033[0m";
            }
            else if (cd_xz == 3) {
                cout << "\033[14;H\033[2K\033[1;34m\t\t->\t退出\033[0m";
            }
        }
        if (!(choose_sz == 1))cout << "\033[1;33m\033[17;H\033[2K" << wrong;
        else cout << "\033[1;33m\033[23;H\033[2K" << wrong;
        cout << "\033[u" << endl;
    }
    // 整体刷新，进入不同选项界面
    void cd_sx() {
        string hen(60, '*');
        if (choose_ks == 0 && choose_sz == 0 && choose_tc == 0) {
            hand_h();
            if (sf_dr == 1) cout << username << ",你好！" << endl;
            else cout << "游客，你好！" << endl;
            cout << "\t\t\t开始游戏\n\n" << endl;
            cout << "\t\t\t设置\n\n" << endl;
            cout << "\t\t\t退出\n" << endl;
        }
        else {
            if (choose_ks == 1) {
                hand_h();
                cout << "\n\t\t\t正常模式\n\n" << endl;
                cout << "\t\t\t征服之路\n" << endl;
            }
            else if (choose_sz == 1) {
                hand_h();
                if (sf_dr == 1) cout << username << ",你好！" << endl;
                else cout << "游客，你好！" << endl;
                cout << "\t\t\t开关音效：" << sf_se << "\n\n" << endl;
                cout << "\t\t\t查看个人战绩\n\n" << endl;
                cout << "\t\t\t“征服之路”排行榜\n\n" << endl;
                if(sf_dr == 1) cout << "\t\t\t退出登入\n\n" << endl;
                else cout << "\t\t\t登入账户\n\n" << endl;
                cout << "\t\t\t注销账户\n" << endl;
            }
            else {
                hand_h();
                cout << "\n\t\t\t是\n\n" << endl;
                cout << "\t\t\t否\n" << endl;
            }
        }
        cout << hen << endl;
        for (i = 0; i < 5; i++) cout << endl;
        cout << "\033[s";//储存光标位置
    }
    //显示个人战绩
    void jr_show() {
        int all_times;
        double max_score;
        string time_m;
        system("cls");
        if (username.empty()) cout << "抱歉，不为游客提供战绩显示服务" << endl;
        else {
            cout << "\n" << username << ",你好！下面是你的战绩：" << endl;
            ifstream ifs("jr.json");
            json s_jr;
            ifs >> s_jr;
            ifs.close();
            cout << "\n\033[1;30m“正常模式”战绩：\033[0m" << endl;
            for (i = 0; i < 8; i++) {
                all_times = s_jr.at(username).at(to_string(i + 1)).at(0);
                max_score = s_jr.at(username).at(to_string(i + 1)).at(1);
                time_m = s_jr.at(username).at(to_string(i + 1)).at(2);
                cout << "\n" << "难度" << i + 1 << "\t通过次数：" << setw(3) << all_times;
                cout << "\t最高得分：" << setw(6) << fixed << setprecision(3) << max_score;
                cout << "\t创下时间：" << time_m << endl;
            }
            double max_cg_df = s_jr.at(username).at("fire").at(1);
            cout << "\n\033[1;31m“征服之路”战绩：\033[0m" << endl;
            cout << "\n最高难度：" << s_jr.at(username).at("fire").at(0);
            cout << "\t最高得分：" << fixed << setprecision(3) << max_cg_df;
            cout << "\t创下时间：" << s_jr.at(username).at("fire").at(2) << endl;
        }
        cout << endl;
        system("pause");
    }
    //显示“征服之路”的排行榜
    void zf_road() {
        system("cls");
        ifstream ifs("jr.json");
        json j;
        ifs >> j;
        ifs.close();
        //使用哈希表
        unordered_map<string, double> zf_pm; //用于储存“征服之路”前十的账户名和得分
        i = 0;
        // 使用keys()函数获取json文件中的所有键
        for (auto& e : j.items()) {
            // 向哈希表中插入元素
            zf_pm.insert({ e.key(), j.at(e.key()).at("fire").at(1) });
        }
        vector< pair< string, double>> vec(zf_pm.begin(), zf_pm.end());
        // 自定义一个比较函数，使得 sort 函数按照值从大到小排序
        auto cmp = [](const pair< string, double>& a, const pair< string, double>& b) {
            return a.second > b.second;
        };
        // 使用 sort 函数对 vector 进行排序
        sort(vec.begin(), vec.end(), cmp);
        // 输出排序后的结果
        cout << "\n\033[1;31m“征服之路”前十排行榜：\033[0m" << endl;
        int k = 1, dr_pm = 0;
        for (auto& p : vec) {
            if (sf_dr == 1)if (username == p.first) dr_pm = k;
            if (k == 1) cout << "\033[1;33m"; // 金色
            else if (k == 2)cout << "\033[38;2;255;165;0m"; // 橙色
            else if (k == 3)cout << "\033[33m"; // 黄色
            cout << "\n第" << k++ << "名：\t" << p.first;
            cout << "\t总得分为： " << fixed << setprecision(3) << p.second;
            cout << "\t创下时间：" << j.at(p.first).at("fire").at(2) << endl;
            cout << "\033[0m"; // 恢复默认颜色
            if (k == 10) break;
        }
        for (; k <= 10;) {
            cout << "\n第" << k++ << "名：暂无数据" << endl;
        }
        if (sf_dr == 1 && dr_pm != 0) cout << "\n\n\033[1;33m" << username << ",恭喜你！你是：第" << dr_pm << "名\033[0m" << endl;
        cout << endl;
        system("pause");
    }
    //显示菜单
    void cd_cd() {
        string hen(60, '*');
        hand_h();
        if (sf_dr == 1) cout << username << ",你好！" << endl;
        else cout << "游客，你好！" << endl;
        cout << "\t\t\t开始游戏\n\n" << endl;
        cout << "\t\t\t设置\n\n" << endl;
        cout << "\t\t\t退出\n" << endl;
        cout << hen << endl;
        for (i = 0; i < 5; i++) cout << endl;
        cout << "\033[s";//储存光标位置
        Sleep(200);
        menu_show();
        while (1) {
            for (i = 0; i < 256; i++) {
                if (GetAsyncKeyState(i) && 0x8000) menu_choice(i);
                else isKeyPressed[i] = false; // 按键松开后重置状态
            }
            if (cd_out != 0) break;
        }
    }
public:
    inline void hand_h();
    HNT_menu() {
        if (username != "") sf_dr = 1;
        update_yx();
        playLoop(TEXT("se/bgm.wav"));
        cd_cd();
    }
};

//汉诺塔游戏/游戏主体
class HNT : public HNT_menu {
private:
    int xz = 1; // 塔的层数，默认为1
    int n1 = 10, n2 = 18; // n1为高度，n2为宽度
    string h[9], hu[9]; // 用于储存塔的形状
    int kb = 0; // 储存拿取塔的数字（0~xz）
    int xz1; //用于交换的中间数字
    string txts1 = "说明：ad左右移动，w拿取，s放下，t显示/关闭说明，p重新开始，u难度上升，j难度下降，esc退出到主菜单";
    string txts2 = "说明：exe停止征服，直接计分";
    string txts = "";
    string wrong = ""; // 用于输出错误
    DWORD start_time = GetTickCount();  // 记录开始时间

    // 需要更新的数据
    double cg_df = 0.0; //闯关总得分
    double times = 0.0; //时间
    int count = 0; //计数
    bool isKeyPressed[256] = { false }; // 记录按键是否被按下
    int win = 0; //判断是否赢了，如果赢了win=1
    int a[3][10]; // 储存所有塔的位置
    int txt = 1; //是否显示说明，0为不显示，1为显示，可由t控制
    int xzk = 0; //准备需要移动的塔，0为不选择，1~3为选择最上面的它，并移动到最上面
    int jt = 1; //箭头位置，范围1~3
    
//游戏主体
private:  //用户无需知道的函数
    //向右移动
    void r_move() {
        if (jt != 3) {
            jt++;
            wrong = "";
        }
        else wrong = "无法再向右移动\a";
        if (xzk != 0 && xzk != 3) {
            xzk++;
        }
        all_move();
    }
    //向左移动
    void l_move() {
        if (jt != 1) {
            jt--;
            wrong = "";
        }
        else wrong = "无法再向左移动\a";
        if (xzk != 0 && xzk != 1) {
            xzk--;
        }
        all_move();
    }
    //拿起汉诺塔
    void get() {
        if (a[jt - 1][0] != 0 && xzk == 0) {
            for (j = 0; j < 10; j++) {
                if (a[jt - 1][j] == 0) {
                    kb = a[jt - 1][j - 1];
                    a[jt - 1][j - 1] = 0;
                    xzk = jt;
                    string k2 = "|", k3(n2, ' ');
                    string ky = k3 + k2 + k3;
                    if (xzk == 1) {
                        cout << "\033[" << 17 - j << ";H" << ky << "\033[u";
                    }
                    if (xzk == 2) {
                        cout << "\033[" << 17 - j << ";42H" << ky << "\033[u";
                    }
                    if (xzk == 3) {
                        cout << "\033[" << 17 - j << ";83H" << ky << "\033[u";
                    }
                    break;
                }
            }
            wrong = "";
        }
        else wrong = "无法拿取\a";
        all_move();
    }
    //放下汉诺塔
    void put() {
        for (j = 0; j < 10; j++) if (a[jt - 1][j] == 0) break;
        if (j == 0 || (a[jt - 1][j - 1] > kb && xzk != 0)) {
            a[jt - 1][j] = kb;
            string k3(n2, ' ');
            string ko = k3 + " " + k3;
            if (xzk == 1) {
                cout << "\033[" << 16 - j << ";H" << h[kb] << "\033[u";
            }
            if (xzk == 2) {
                cout << "\033[" << 16 - j << ";42H" << h[kb] << "\033[u";
            }
            if (xzk == 3) {
                cout << "\033[" << 16 - j << ";83H" << h[kb] << "\033[u";
            }
            kb = 0;
            xzk = 0;
            wrong = "";
            count++;
        }
        else wrong = "无法放下\a";
        all_move();
        pd_win();
    }
    //是否显示说明
    inline void t_txts() {
        txt = (txt + 1) % 2;
        wrong = "";
        all_move();
    }
    //重新开始
    void p_resume() {
        xz1 = xz;
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 10; j++) {
                if (i == 0 && xz1 > 0) a[i][j] = xz1--;
                else a[i][j] = 0;
            }
        }
        txt = 1;
        xzk = 0;
        jt = 1;
        kb = 0;
        game_xs();
    }
    //增加难度后，重新开始
    void u_d() {
        if (xz != 8) {
            xz++;
            xz1 = xz;
            for (i = 0; i < 3; i++) {
                for (j = 0; j < 10; j++) {
                    if (i == 0 && xz1 > 0) a[i][j] = xz1--;
                    else a[i][j] = 0;
                }
            }
            txt = 1;
            xzk = 0;
            jt = 1;
            kb = 0;
            game_xs();
        }
        else {
            wrong = "再难就有点不人道了\a";
            all_move();
        }
    }
    //减低难度后，重新开始
    void d_e() {
        if (xz != 1 && cg != 1) {
            xz--;
            xz1 = xz;
            for (i = 0; i < 3; i++) {
                for (j = 0; j < 10; j++) {
                    if (i == 0 && xz1 > 0) a[i][j] = xz1--;
                    else a[i][j] = 0;
                }
            }
            txt = 1;
            xzk = 0;
            jt = 1;
            kb = 0;            
            game_xs();
        }
        else {
            wrong = "我觉得没有比这更简单的了\a";
            all_move();
        }
    }
    //退出
    inline void esc() {
        if (cg == 1) {
            cg_jg();
        }
        else {
            win = 1;
        }
    }
    //输出箭头符合
    inline void jtt() {
        // 将标准输出流的字符集设置为 UTF-8
        _setmode(_fileno(stdout), _O_U16TEXT);
        // 输出向上指的箭头
        wcout << L"\u2191";
        // 将标准输出流的字符集恢复为默认值
        _setmode(_fileno(stdout), _O_TEXT);
    }
    //更新游戏界面
    void all_move() {
        // 箭头位置
        if (jt == 1) {
            cout << "\033[20;H"; // 将光标移动到第 20 行,箭头所在行
            cout << "\033[2K"; // 清除内容
            cout << "\033[20;19H"; // 移动光标
            jtt();
        }
        else if (jt == 2) {
            cout << "\033[20;H" << "\033[2K" << "\033[20;60H";
            jtt();
        }
        else if (jt == 3) {
            cout << "\033[20;H" << "\033[2K" << "\033[20;101H";
            jtt();
        }
        // 将光标移动到第 4 行开头，拿取塔所在的行，清除内容
        cout << "\033[4;H" << "\033[2K"; 
        if (xzk != 0) {
            if (xzk == 1) cout << hu[kb];
            if (xzk == 2) cout << "\033[4;42H" << hu[kb];
            if (xzk == 3) cout << "\033[4;83H" << hu[kb];
        }
        // 说明文字的显与隐
        if (txt == 1) cout << "\033[1;H" << "\033[2K" << txts;
        else cout << "\033[1;H" << "\033[2K";
        // 计数
        cout << "\033[93m\033[2;7H" << count << "\033[2;23H" << endl;
        // 显示错误信息，给予提示
        cout << "\033[21;H" << "\033[2K"; // 将光标移动到第 20 行开头，警告文字所在行，清除内容
        cout << "\033[1;33m" << wrong << "\033[0m" << "\n"; // 显示警告        
        cout << "\033[u";
    }
    inline void game_xs() {
        system("cls");
        game_show();
    }    
    void onKeyDown(unsigned short key) {
        switch (key) {
        case VK_UP:
        case 'W':
            if (!isKeyPressed[key]) {
                get();
                isKeyPressed[key] = true; // 标记按键已按下
            }
            break;
        case VK_DOWN:
        case 'S':
            if (!isKeyPressed[key]) {
                if (game_se == 1) se_fang();
                put();
                isKeyPressed[key] = true;
            }
            break;
        case VK_LEFT:
        case 'A':
            if (!isKeyPressed[key]) {
                if (game_se == 1) se_dian();
                l_move();
                isKeyPressed[key] = true;
            }
            break;
        case VK_RIGHT:
        case 'D':
            if (!isKeyPressed[key]) {
                if (game_se == 1) se_dian();
                r_move();
                isKeyPressed[key] = true;
            }
            break;
        case VK_ESCAPE:
            if (!isKeyPressed[key]) {
                esc();
                isKeyPressed[key] = true;
            }
            break;
        case 'U':
            if (!isKeyPressed[key]) {
                if (cg != 1) {
                    u_d();
                }
                isKeyPressed[key] = true;
            }
            break;
        case 'J':
            if (!isKeyPressed[key]) {
                if (cg != 1) {
                    d_e();
                }
                isKeyPressed[key] = true;
            }
            break;
        case 'P':
            if (!isKeyPressed[key]) {
                if (cg != 1) {
                    p_resume();
                }
                isKeyPressed[key] = true;
            }
            break;
        case 'T':
            if (!isKeyPressed[key]) {
                t_txts();
                isKeyPressed[key] = true;
            }
            break;
        default:
            break;
        }
    }
    //根据难度，显示游戏初始界面
    void game_show() {
        count = 0;
        times = 0.0;
        string k1(n2, '-'), k2 = "|", k3(n2, ' ');
        string ko = k3 + " " + k3;
        string ky = k3 + k2 + k3;
        string kd = k1 + k2 + k1;
        // 头部
        for (i = 0; i < 6; i++) {
            if (i == 0) cout << txts << "\n";
            else if (i == 1) {
                cout << "\033[93m步数：" << count << "    " << "时间：" << times << endl;
            }
            else if (i == 2) {
                if (xz >= 1 && xz <= 3) {
                    cout << "\033[90m" << "难度" + to_string(xz) << "\033[0m" << "\n";
                }
                else if (xz == 4) {
                    cout << "\033[97m" << "难度4" << "\033[0m" << "\n";
                }
                else if (xz == 5) {
                    cout << "\033[33m" << "难度5" << "\033[0m" << "\n";
                }
                else if (xz == 6) {
                    cout << "\033[38;5;208m" << "难度6" << "\033[0m" << "\n";
                }
                else if (xz == 7) {
                    cout << "\033[35m" << "难度7" << "\033[0m" << "\n";
                }
                else {
                    cout << "\033[31m" << "难度8" << "\033[0m" << "\n";
                }
            }
            else cout << "\n";
        };
        // 主体
        for (k = 9; k >= 0; k--) {
            if (a[0][k] != 0) {
                cout << h[a[0][k]];
            }
            else {
                cout << ky;
            }
            cout << "    ";
            if (a[1][k] != 0) {
                cout << h[a[1][k]];
            }
            else {
                cout << ky;
            }
            cout << "    ";
            if (a[2][k] != 0) {
                cout << h[a[2][k]];
            }
            else {
                cout << ky;
            }
            cout << "\n";
        }
        cout << kd + "    " + kd + "    " + kd << "\n";
        // 箭头选择区域
        for (i = 0; i < 3; i++) {
            if (i == 2) {
                cout << k3;
                jtt();
                cout << "\n";
            }
            else cout << "\n";
        }
        cout << "\n\n\n\n\n\n\n\033[s";//储存光标位置
        start_time = GetTickCount();  // 记录开始时间
    }
    //获取当前时间
    string now_time() {
        // 获取当前时间
        time_t now = time(nullptr);
        // 转换为本地时间
        tm local_time;
        localtime_s(&local_time, &now);
        // 构造输出格式
        stringstream ss;
        ss << local_time.tm_year + 1900 << "-" << local_time.tm_mon + 1 << "-"
            << local_time.tm_mday << " " << local_time.tm_hour << ":" << local_time.tm_min
            << ":" << local_time.tm_sec;
        return ss.str();
    }
    //显示“征服之路”的结果
    void cg_jg() {
        system("cls");
        if (sf_dr == 1) {
            ifstream ifs("jr.json");
            json j;
            ifs >> j;
            ifs.close();
            string time_cg = j.at(username).at("fire").at(2);
            cout << username << ",你好！下面是你这次“征服之路”的最终结果：\n" << endl;
            cout << "\t最高难度：" << xz << setw(18) << "最终得分：" << cg_df << "\t创下时间：" << time_cg << endl;
        }
        else {
            cout << "游客,你好！下面是你这次“征服之路”的结果：\n" << endl;
            cout << "\t最高难度：" << xz << setw(18) << "最终得分：" << cg_df << endl;
        }
        cout << endl; cout << endl;
        win = 1;
        system("pause");
    }
    //判断是否赢了
    void pd_win() {
        if (a[2][xz - 1] == 1) {
            se_win();
            double score = 0.0;
                double pp = (pow(2, xz) - 1);
            score += 0.5 * (100 - (count - pp) * (50 / pp));
            score += 0.5 * (100 - (times - pp) * (40 / pp));
            if (cg == 1) cg_df += score * (xz + 2) * 0.1;
            if (yk != 1 && sf_dr != 0) {
                ifstream ifs("jr.json");
                json update_jr;
                ifs >> update_jr;
                ifs.close();
                int n_count = update_jr.at(username).at(to_string(xz)).at(0) + 1;
                update_jr[username][to_string(xz)][0] = n_count; //通过次数+1
                double o_score = update_jr.at(username).at(to_string(xz)).at(1);
                if (o_score < score) {
                    update_jr[username][to_string(xz)][1] = score;
                    update_jr[username][to_string(xz)][2] = now_time();
                }
                if (cg == 1) {
                    if (update_jr.at(username).at("fire").at(1) < cg_df) {
                        update_jr[username]["fire"][0] = xz;
                        update_jr[username]["fire"][1] = cg_df;
                        update_jr[username]["fire"][2] = now_time();
                    }
                }
                ofstream ofs("jr.json");
                ofs << update_jr.dump(4) << endl;
            }
            if (cg == 1) {
                if (xz == 8) {
                    cout << "\033[1;32m\033[21;H\t\t\t恭喜你通过最后一关，来看看你的得分吧！" << endl;
                    system("pause");
                    cg_jg();
                }
                else {
                    cout << "\033[1;32m\033[21;H恭喜你通过这一关\033[0m" << endl;
                    cout << "\nKeep pushing forward!!!" << endl;
                    cout << "\n休息3秒进行下一个难度" << endl;
                    Sleep(3000);
                    u_d();
                }
            }
            else {
                cout << "\033[1;32m\033[21;H\t\t\t\t\t\t！！！！！！！！！" << endl;
                cout << "\033[1;32m\033[22;H\t\t\t\t\t\t！！！你赢了！！！" << endl;
                cout << "\033[1;32m\033[23;H\t\t\t\t\t\t！！！！！！！！！\033[0m" << endl;
                cout << "\033[1;33m\t\t\t\t\t\t得分：" << score << "\033[0m" << endl;
                cout << "\n\t\t\t\t\t再来一局吗？(会增加难度)\n\t\t\t\t\t还是退出回主菜单？" << endl;
                cout << "\t\t\t\t\t那么，你选择：再来一局（回车）；回到主菜单（按exc）" << endl;
                while (1) {
                    if (GetAsyncKeyState(VK_ESCAPE)) {
                        win = 1;
                        break;
                    }
                    if (GetAsyncKeyState(VK_RETURN)) {
                        cin.ignore(1024, '\n');
                        u_d();
                        break;
                    }
                }
            }
        }
    }
public:
    //构造函数
    HNT() {
        cleanup_bgm();
        if (cg == 1) playLoop(TEXT("se/bgm2.wav"));
        else playLoop(L"se/bgm1.wav");
        // 预处理部分
        system("cls");
        if (cg == 1) {
            txts = txts2;
            cout << "欢迎来到“征服之路”，挑战极限，超越自我！（5秒后开始）" << endl;
            for (i = 5; i > 0; i--) {
                cout << "\033[2;H" << i << "!" << endl;
                if (i == 1) cout << "\033[3;H" << "Go!!!" << endl;
                Sleep(1000);
            }
            xz = 1;
        }
        else {
            txts = txts1;
            while (1) {
                cout << "\033[?25h"; //显示光标
                cout << "请选择汉诺塔的层数(范围1~8): ";
                cin >> xz;
                if (cin.fail()) {                // 如果输入失败
                    cout << "输入有误，请重新输入" << endl;
                    cin.clear();                 // 清空错误状态
                    cin.ignore(1024， '\n');  // 忽略缓冲区中的所有字符，直到遇到换行符
                    continue;
                }
                if (xz >= 1 && xz <= 8) {
                    cout << "成功，请愉悦游玩" << endl;
                    system("pause");
                    break;
                }
                else {
                    cout << "超出范围，请重新输入" << endl;
                    continue;
                }
                cout << "\033[?25l";
            }
            cin.ignore(1024， '\n');
        }
        string h1 = "[", h2 = "]";
        int xz1 = xz;
        h[0] = hu[0] = "";
        for (int i = 1; i < 9; i++) {
            string hh(2 * i, ' ');
            string kon((n2 - 2 * i - 1)， ' ');
            h[i] = kon + h1 + hh + "|" + hh + h2 + kon;
            hu[i] = kon + h1 + hh + " " + hh + h2 + kon;
        }
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 10; j++) {
                if (i == 0 && xz1 > 0) a[i][j] = xz1--;
                else a[i][j] = 0;
            }
        }
        game_xs();
    }
    //游戏主体
    void play() {
        //正式开始
        while (true) {
            for (i = 0; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) onKeyDown(i);
                else isKeyPressed[i] = false; // 按键松开后重置状态
            }
            if (win != 0) break;
            DWORD current_time = GetTickCount();  // 获取当前时间
            times = (current_time - start_time) / 1000.0;  // 计算时间差
            printf_s("\033[93m\033[2;18H%.2f s\033[u", times); // 输出时间
        }
        cleanup_bgm();
    }
};

string HNT_menu::username = "";
