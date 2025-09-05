#pragma once

class Button {
public:
    static void setMenu(bool menu);
    static bool getMenu();

private:
    static bool isMenu;
};
