#pragma once

class App {
public:
    virtual ~App() = default;
    
    virtual void startUp() = 0;
    virtual void update() = 0;
    virtual void run() = 0;
};
