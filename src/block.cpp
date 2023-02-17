#include <cassert>
#include <iostream>
#include "common.h"
#include "block.h"
#include "color.h"

CBlock::CBlock()
    : _count(0) {}

bool CBlock::isValid() const
{
    assert(MAX_COUNT == _count);

    for (int i = 0; i < _count - 1; ++i)
    {
        for (int j = i + 1; j < _count; ++j)
        {
            if (UNSELECTED == _numbers[i]->value || UNSELECTED == _numbers[j]->value)
                continue;

            if (_numbers[i]->value == _numbers[j]->value)
                return false;
        }
    }

    return true;
}

bool CBlock::isFull() const
{
    for (int i = 0; i < _count; ++i)
    {
        point_value_t *p_point_value = _numbers[i];
        if (nullptr == p_point_value || UNSELECTED == p_point_value->value)
            return false;
    }
    return true;
}

void CBlock::print() const
{
    std::cout << "\u2503" << " ";
    for (int i = 0; i < _count; ++i)
    {
        auto number = *(_numbers[i]);
        // if (0 == number.value)
        //     std::cout << ' ' << " \u2503 ";
        if (number.state == State::ERASED)
        {
            if (0 == number.value)
                std::cout << ' ' << " \u2503 ";
            else
                std::cout << Color::Modifier(Color::FG_GREEN) << number.value << Color::Modifier(Color::RESET) << " \u2503 ";
        }
        else if (number.state == State::IFOCUS)
        {
            std::cout << Color::Modifier(Color::FG_BLACK) 
            << Color::Modifier(Color::BG_WHITE) << number.value << " "
            << Color::Modifier(Color::RESET) << "\u2503 ";
        }
        else if (number.state == State::EFOCUS)
        {
            if (0 == number.value)
                std::cout << Color::Modifier(Color::FG_GREEN) 
                << Color::Modifier(Color::BG_WHITE) << "  "
                << Color::Modifier(Color::RESET) << "\u2503 ";
            else
                std::cout << Color::Modifier(Color::FG_GREEN) 
                << Color::Modifier(Color::BG_WHITE) << number.value << " "
                << Color::Modifier(Color::RESET) << "\u2503 ";
        }
        else
        {
            if (0 == number.value)
                std::cout << ' ' << " \u2503 ";
            else
                std::cout << number.value << " \u2503 ";
        }
    }
    std::cout << std::endl;
}

void CBlock::push_back(point_value_t *point)
{
    assert(nullptr != point);
    _numbers[_count++] = point;
}
