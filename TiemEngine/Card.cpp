#include "Card.h"


static std::string getCardFrameName(const std::string& rarityCode)
{
    if (rarityCode == "sta") return "BG_Frame/card_fr_gy.png";
    if (rarityCode == "com") return "BG_Frame/card_fr_gn.png";
    if (rarityCode == "rar") return "BG_Frame/card_fr_br.png";
    if (rarityCode == "leg") return "BG_Frame/card_fr_or.png";
    if (rarityCode == "spc") return "BG_Frame/card_fr_pk.png";
    return "BG_Frame/card_fr_gy.png";
}

static std::string getTypeIconName(const std::string& typeCode)
{
    if (typeCode == "atk") return "BG_Type/card_ty_atk.png";
    if (typeCode == "mov") return "BG_Type/card_ty_mv.png";
    if (typeCode == "eng") return "BG_Type/card_ty_er.png";
    if (typeCode == "buf") return "BG_Type/card_ty_bf.png";
    if (typeCode == "dbf") return "BG_Type/card_ty_db.png";
    return "BG_Type/card_ty_atk.png";
}

static std::string getStarOverlayName(int level)
{
    switch (level) {
    case 1: return "BG_Stars/star_gd1.png";
    case 2: return "BG_Stars/star_gd2.png";
    case 3: return "BG_Stars/star_gd3.png";
    default: return "";
    }
}

static std::string getBackgroundName()
{
    return "BG_card/card_bg.png";
}

Card::Card(const std::string& n)
    : name(n), level(0), rarityCode("sta"), typeCode("atk")  
{
}

Card::~Card()
{
    for (Action* a : actions) {
        delete a;
    }
    actions.clear();
    
    DestroyVisuals();
}

const std::string& Card::getName() const
{
    return name;
}

const std::vector<Action*>& Card::getActions() const
{
    return actions;
}

void Card::addAction(Action* action)
{
    if (action) {
        actions.push_back(action);
    }
}

void Card::do_action()
{
    for (Action* action : actions)
        action->do_action();
}

void Card::CreateVisuals()
{
    if (visualsCreated) return;
    
    std::string basePath = "../Resource/Texture/cards/";
    
    // RENDER ORDER (bottom to top):
    
    // 1. Background (bottom layer)
    background = new ImageObject();
    background->SetSize(280.0f, -410.0f);
    background->SetTexture(basePath + getBackgroundName());
    
    // 2. Star Overlay 
    if (level > 0) {
        starOverlay = new ImageObject();
        starOverlay->SetSize(280.0f, -410.0f);
        starOverlay->SetTexture(basePath + getStarOverlayName(level));
    }
    
    // 3. Type Icon
    typeIcon = new ImageObject();
    typeIcon->SetSize(280.0f, -410.0f);
    typeIcon->SetTexture(basePath + getTypeIconName(typeCode));
    
    // 4. Main Visual
    visual = new ImageObject();
    visual->SetSize(280.0f, -410.0f);
    visual->SetTexture(basePath + "BG_visual/" + name + ".png");
    
    // 5. Card Frame (based on rarity)
    cardFrame = new ImageObject();
    cardFrame->SetSize(280.0f, -410.0f);
    cardFrame->SetTexture(basePath + getCardFrameName(rarityCode));
    
    // 6. Card Name Text 
    nameText = new TextObject();
    SDL_Color textColor = { 255, 255, 255, 255 }; 
    nameText->LoadText(name, textColor, 20);
    
    visualsCreated = true;
}

void Card::DestroyVisuals()
{
    if (!visualsCreated) return;
    
    if (background) { delete background; background = nullptr; }
    if (starOverlay) { delete starOverlay; starOverlay = nullptr; }
    if (typeIcon) { delete typeIcon; typeIcon = nullptr; }
    if (visual) { delete visual; visual = nullptr; }
    if (cardFrame) { delete cardFrame; cardFrame = nullptr; }
    if (nameText) { delete nameText; nameText = nullptr; }
    
    visualsCreated = false;
}

std::vector<DrawableObject*> Card::GetAllLayers() const
{
    std::vector<DrawableObject*> layers;
    if (background) layers.push_back(background);
    if (starOverlay) layers.push_back(starOverlay);
    if (typeIcon) layers.push_back(typeIcon);
    if (visual) layers.push_back(visual);
    if (cardFrame) layers.push_back(cardFrame);
    if (nameText) layers.push_back(nameText);
    return layers;
}
