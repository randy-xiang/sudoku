#include <cmath>
#include <iostream>
#include <fstream>
#include <memory.h>
#include <map>
#include <unordered_map>
#include <vector>
#include "common.h"
#include "scene.h"
#include "color.h"
#include "utility.inl"

CScene::CScene(int index)
    : _max_column(pow(index, 2))
    , _cur_point({0, 0})
{
    init();
}

CScene::~CScene()
{
    if(keyMap) delete keyMap;
}

void CScene::show() const
{
    // Clear screen
    cls();

    // Always print instruction on the top
    printInstruction();

    printUnderline();

    for (int row = 0; row < _max_column; ++row)
    {
        CBlock block = _row_block[row];
        block.print();
        printUnderline(row);
    }
}

void CScene::setMode(KeyMode mode)
{
    switch (mode)
    {
    case KeyMode::NORMAL:
        keyMap = new Normal;
        break;

    case KeyMode::VIM:
        keyMap = new Vim;
        break;
    }
}

void CScene::printInstruction() const
{
    message("操作说明：");
    message("0 删除已填入数字");
    message("u 撤销上一步操作");
    message("Enter 尝试通关");
    message("Esc 退出游戏");
    message("w 光标上移↑");
    message("a 光标左移←");
    message("s 光标下移↓");
    message("d 光标右移→");
}

void CScene::printUnderline(int line_no) const
{
    for (int colunm = 0; colunm < 9; ++colunm)
    {
        std::cout << "\u254B" << "\u2501" << "\u2501"<< "\u2501";
        // if (_cur_point.y == line_no && _cur_point.x == colunm)
        //     std::cout << Color::Modifier(Color::BG_WHITE) 
        //     << Color::Modifier(Color::FG_BLACK) 
        //     << Color::Modifier(Color::BLINK) 
        //     << Color::Modifier(Color::BOLD) << "-" << "-" 
        //     << Color::Modifier(Color::RESET);
        // else
            // std::cout << "\u2501"<< "\u2501" ;
    }
    std::cout << "\u254B" << std::endl;
}

void CScene::init()
{
    memset(_map, UNSELECTED, sizeof _map);

    // column_block 所有列
    for (int col = 0; col < _max_column; ++col)
    {
        CBlock column_block;

        for (int row = 0; row < _max_column; ++row)
        {
            column_block.push_back(_map + row * 9 + col);
        }
        _column_block[col] = column_block;
    }

    // row_block 所有行
    for (int row = 0; row < _max_column; ++row)
    {
        CBlock row_block;

        for (int col = 0; col < _max_column; ++col)
        {
            row_block.push_back(_map + row * 9 + col);
        }
        _row_block[row] = row_block;
    }

    // xy_block 所有九宫格, [行][列]
    for (int block_index = 0; block_index < _max_column; ++block_index)
    {
        CBlock xy_block;

        int xy_begin = block_index / 3 * 27 + block_index % 3 * 3;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                xy_block.push_back(_map + xy_begin + i * 9 + j);
            }
        }
        _xy_block[block_index / 3][block_index % 3] = xy_block;
    }

    return;
}

bool CScene::setCurValue(const int nCurValue, int &nLastValue)
{
    auto point = _map[_cur_point.x + _cur_point.y * 9];
    if (point.state == State::EFOCUS)
    {
        nLastValue = point.value;
        setValue(nCurValue);
        return true;
    }
    else
        return false;
}

void CScene::setValue(const point_t& p, const int value)
{
    _map[p.x + p.y * 9].value = value;
}

void CScene::setValue(const int value)
{
    auto p = _cur_point;
    this->setValue(p, value);
}

// set = true, set focus
// set = false, unset focus
// Block's state will transite between INITED and IFOCUS
// or between ERASED and EFOCUS
void CScene::setFocus(const int x, const int y, const bool set = true)
{
    int idx = x + y * 9;
    if (set)
    {
        if (_map[idx].state == State::INITED)
            _map[idx].state = State::IFOCUS;
        else if (_map[idx].state == State::ERASED)
            _map[idx].state = State::EFOCUS;
        else
            return;
    }
    else
    {
        if (_map[idx].state == State::IFOCUS)
            _map[idx].state = State::INITED;
        else if (_map[idx].state == State::EFOCUS)
            _map[idx].state = State::ERASED;
        else
            return;
    }
}

// 选择count个格子清空
void CScene::eraseRandomGrids(const int count)
{
    point_value_t p = {UNSELECTED, State::ERASED};

    std::vector<int> v(81);
    for (int i = 0; i < 81; ++i) {
        v[i] = i;
    }

    for (int i = 0; i < count; ++i) {
        int r = random(0, v.size() - 1);
        _map[v[r]] = p;
        v.erase(v.begin() + r);
    }
}

bool CScene::isComplete()
{
    // 任何一个block未被填满，则肯定未完成
    for (size_t i = 0; i < 81; ++i)
    {
        if (_map[i].value == UNSELECTED)
            return false;
    }

    // 同时block里的数字还要符合规则
    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 9; ++col)
        {
            if (!_row_block[row].isValid() || 
                !_column_block[col].isValid() || 
                !_xy_block[row / 3][col / 3].isValid())
                return false;
        }
    }

    return true;
}

void CScene::save(const char *filename) {
    std::fstream fs;
    // TODO: check whether the file has existed
    fs.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);

    // save _map
    for (int i = 0; i < 81; i++) {
        fs << _map[i].value << ' ' << static_cast<int>(_map[i].state) << std::endl;
    }

    // save _cur_point
    fs << _cur_point.x << ' ' << _cur_point.y << std::endl;

    // save _vCommand
    fs << _vCommand.size() << std::endl;
    for (CCommand command : _vCommand) {
        point_t point = command.getPoint();
        fs << point.x << ' ' << point.y << ' '
           << command.getPreValue() << ' '
           << command.getCurValue() << std::endl;
    }

    fs.close();
}

void CScene::load(const char *filename) {
    std::fstream fs;
    // TODO: check whether the file has existed
    fs.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);

    // load _map
    for (int i = 0; i < 81; i++) {
        int tmpState;
        fs >> _map[i].value >> tmpState;
        _map[i].state = static_cast<State>(tmpState);
    }

    // load _cur_point
    fs >> _cur_point.x >> _cur_point.y;

    // load _vCommand
    int commandSize;
    fs >> commandSize;
    for (int i = 0; i < commandSize; i++) {
        point_t point;
        int preValue, curValue;
        fs >> point.x >> point.y >> preValue >> curValue;
        _vCommand.emplace_back(this, point, preValue, curValue);
    }
}

void CScene::play()
{
    show();

    char key = '\0';
    while (1)
    {
        key = getch();
        if (key >= '0' && key <= '9')
        {
            CCommand oCommand(this);
            if (!oCommand.execute(key - '0'))
            {
                std::cout << "此处无法修改 This number can't be modified." << std::endl;
            }
            else
            {
                _vCommand.push_back(std::move(oCommand));  // XXX: move without move constructor
                show();
                continue;
            }
        }
        if (key == keyMap->ESC)
        {
            message("退出游戏 quit game ? [Y/N]");
            std::string strInput;
            std::cin >> strInput;
            if (strInput[0] == 'y' || strInput[0] == 'Y')
            {
                message("保存游戏进度吗 Do you want to save the game progress ? [Y/N]");
                std::cin >> strInput;
                if (strInput[0] == 'y' || strInput[0] == 'Y')
                {
                    message("input path of the progress file: ", false);
                    std::cin >> strInput;
                    save(strInput.c_str());
                }
                exit(0);
            } else
            {
                message("继续 Continue.");
            }
        }
        else if (key == keyMap->U)
        {
            if (_vCommand.empty())
                message("无法继续撤销 No more action to undo.");
            else
            {
                CCommand& oCommand = _vCommand.back();
                oCommand.undo();
                _vCommand.pop_back();
                show();
            }
        }
        else if (key == keyMap->LEFT)
        {
            this->setFocus(_cur_point.x, _cur_point.y, false);
            _cur_point.x = (_cur_point.x - 1) < 0 ? 0 : _cur_point.x - 1;
            this->setFocus(_cur_point.x, _cur_point.y, true);
            show();
        }
        else if (key == keyMap->RIGHT)
        {
            this->setFocus(_cur_point.x, _cur_point.y, false);
            _cur_point.x = (_cur_point.x + 1) > 8 ? 8 : _cur_point.x + 1;
            this->setFocus(_cur_point.x, _cur_point.y, true);
            show();
        }
        else if (key == keyMap->DOWN)
        {
            this->setFocus(_cur_point.x, _cur_point.y, false);
            _cur_point.y = (_cur_point.y + 1) > 8 ? 8 : _cur_point.y + 1;
            this->setFocus(_cur_point.x, _cur_point.y, true);
            show();
        }
        else if (key == keyMap->UP)
        {
            this->setFocus(_cur_point.x, _cur_point.y, false);
            _cur_point.y = (_cur_point.y - 1) < 0 ? 0 : _cur_point.y - 1;
            this->setFocus(_cur_point.x, _cur_point.y, true);
            show();
        }
        else if (key == keyMap->ENTER)
        {
            if (isComplete())
            {
                message("恭喜你！你胜利啦！Congratulation! You win!");
                getchar();
                exit(0);
            }
            else
            {
                message("抱歉，还没通关 Sorry, not completed.");
            }
        }
    }
}

// 一个场景可以多次被初始化
void CScene::generate()
{
    // XXX: pseudo random
    static char map_pattern[10][10] = {
        "ighcabfde",
        "cabfdeigh",
        "fdeighcab",
        "ghiabcdef",
        "abcdefghi",
        "defghiabc",
        "higbcaefd",
        "bcaefdhig",
        "efdhigbca"};

    std::vector<char> v = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};

    // 产生字母到数字的随机映射
    std::unordered_map<char, int> hash_map;
    for (int i = 1; i <= 9; ++i)
    {
        int r = random(0, v.size() - 1);
        hash_map[v[r]] = i;
        v.erase(v.begin() + r);
    }

    // 填入场景
    for (int row = 0; row < 9; ++row)
    {
        for (int col = 0; col < 9; ++col)
        {
            point_t point = {row, col};
            char key = map_pattern[row][col];
            setValue(point, hash_map[key]);
        }
    }

    assert(isComplete());

    return;
}

bool CScene::setPointValue(const point_t &stPoint, const int nValue)
{
    auto point = _map[stPoint.x + stPoint.y * 9];
    if (State::ERASED == point.state)
    {
        _cur_point = stPoint;
        setValue(nValue);
        return true;
    }
    else
        return false;
}

point_t CScene::getCurPoint()
{
    return _cur_point;
}
