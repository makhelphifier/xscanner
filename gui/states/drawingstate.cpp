// gui/states/drawingstate.cpp
#include "drawingstate.h"
#include "drawingstatemachine.h" // 需要包含状态机定义

DrawingState::DrawingState(DrawingStateMachine* machine, QObject *parent)
    : QObject(parent), m_machine(machine)
{
    // 可以在这里进行一些通用的初始化
}

// 注意：纯虚函数的实现不需要在这里提供
