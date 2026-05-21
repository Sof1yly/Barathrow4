#pragma once
#include <vector>
#include "Level.h"
#include "EventScene.h"
#include "ImageObject.h"
#include "DrawableObject.h"

class EventPage : public Level
{
private:
    std::vector<DrawableObject*> objectsList;
    EventScene eventScene;

    void GetRealMousePos(int x, int y, float& rx, float& ry) const;
    void TransitionToLevel(EventScene::EffectType effect);

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
