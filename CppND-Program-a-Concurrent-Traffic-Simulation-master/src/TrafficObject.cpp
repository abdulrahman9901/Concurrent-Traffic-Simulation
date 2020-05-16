#include <algorithm>
#include <iostream>
#include <chrono>
#include "TrafficObject.h"

// init static variable
int TrafficObject::_idCnt = 0;

std::mutex TrafficObject::_mtx;

void TrafficObject::setPosition(double a, double b)
{
    _posX = a;
    _posY = b;
}

void TrafficObject::getPosition(double &a, double &b)
{
    a = _posX;
    b = _posY;
}

TrafficObject::TrafficObject()
{
    _type = ObjectType::noObject;
    _id = _idCnt++;
}

TrafficObject::~TrafficObject()
{
    // set up thread barrier before this object is destroyed
    std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
        t.join();
    });
}
