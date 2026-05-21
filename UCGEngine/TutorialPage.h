#pragma once
#include <vector>
#include "Level.h"
#include "ImageObject.h"
#include "TextObject.h"
#include "DrawableObject.h"

// Shown the very first time a player clicks Start.
// Five full-screen PNG pages from Resource/Texture/tutorial/1-5.png.
// Left half of screen  -> previous page  (page 1: nothing)
// Right half of screen -> next page      (page 5: MarkPlayed + GS_LEVEL1)
class TutorialPage : public Level
{
private:
    std::vector<DrawableObject*> objectsList;

    static constexpr int PAGE_COUNT = 5;
    ImageObject* pages[PAGE_COUNT] = {};  // one per tutorial image
    TextObject*  hintText          = nullptr;
    int          currentPage       = 0;   // 0-based index

    void GetRealMousePos(int x, int y, float& rx, float& ry) const;
    void ShowPage(int index);

public:
    void LevelLoad()   override;
    void LevelInit()   override;
    void LevelUpdate() override;
    void LevelDraw()   override;
    void LevelFree()   override;
    void LevelUnload() override;
    void HandleKey(char key) override;
    void HandleMouse(int type, int x, int y) override;
};
