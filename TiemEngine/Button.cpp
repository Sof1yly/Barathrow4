#include "Button.h"

bool Button::isMenu = false;

void Button::setMenu(bool menu) {
    isMenu = menu;
}

bool Button::getMenu() {
    return isMenu;
}