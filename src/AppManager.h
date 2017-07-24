#pragma once

class WIPApp
{
public:
    virtual bool init() = 0;
    virtual void run() = 0;
    int window_w;
    int window_h;
};