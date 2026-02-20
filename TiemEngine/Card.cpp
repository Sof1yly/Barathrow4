#include "Card.h"
#include <sstream>


using namespace std;


static string getCardFrameName(const string& rarityCode)
{
    if (rarityCode == "sta") return "BG_Frame/card_fr_gy.png";
    if (rarityCode == "com") return "BG_Frame/card_fr_gn.png";
    if (rarityCode == "rar") return "BG_Frame/card_fr_br.png";
    if (rarityCode == "leg") return "BG_Frame/card_fr_or.png";
    if (rarityCode == "spc") return "BG_Frame/card_fr_pk.png";
    return "BG_Frame/card_fr_gy.png";
}

static string getTypeIconName(const string& typeCode)
{
    if (typeCode == "atk") return "BG_Type/card_ty_atk.png";
    if (typeCode == "mov") return "BG_Type/card_ty_mv.png";
    if (typeCode == "eng") return "BG_Type/card_ty_er.png";
    if (typeCode == "buf") return "BG_Type/card_ty_bf.png";
    if (typeCode == "dbf") return "BG_Type/card_ty_db.png";
    return "BG_Type/card_ty_atk.png";
}

static string getStarOverlayName(int level)
{
    switch (level) {
    case 1: return "BG_Stars/star_gd1.png";
    case 2: return "BG_Stars/star_gd2.png";
    case 3: return "BG_Stars/star_gd3.png";
    default: return "";
    }
}

static string getBackgroundName()
{
    return "BG_card/card_bg.png";
}

static string replacePlaceholder(const string& text, const string& token, int value)
{
    string result = text;
    string placeholder = "{" + token + "}";
    size_t pos = 0;
    while ((pos = result.find(placeholder, pos)) != string::npos) {
        result.replace(pos, placeholder.size(), to_string(value));
        pos += to_string(value).size();
    }
    return result;
}

Card::Card(const string& n)
    : name(n), description(""), level(0), rarityCode("sta"), typeCode("atk")  
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

const string& Card::getName() const
{
    return name;
}

const vector<Action*>& Card::getActions() const
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
    
    string basePath = "../Resource/Texture/cards/";
    
    // RENDER ORDER 
    
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
    nameText->LoadText(name, textColor, 18);

    // 7. Description Text resolve {atk}, {mov}
    if (!description.empty()) {
        string resolved = description;
        for (Action* a : actions) {
            const string& code = a->getActionCode();
            if (code == "atk" || code == "mov" || code == "re" || code == "oc") {
                resolved = replacePlaceholder(resolved, code, a->getValue());
            }
        }
        if (overclockValue > 0)
            resolved = replacePlaceholder(resolved, "oc", overclockValue);

        descriptionText = new TextObject();
        SDL_Color descColor = { 220, 220, 220, 255 };
        descriptionText->LoadTextWrapped(resolved, descColor, 16, 240);
    }
    
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
    if (descriptionText) { delete descriptionText; descriptionText = nullptr; }
    
    visualsCreated = false;
}

void Card::RefreshDescriptionText()
{
    if (!visualsCreated || description.empty()) return;

    string resolved = description;
    for (Action* a : actions) {
        const string& code = a->getActionCode();
        if (code == "atk" || code == "mov" || code == "re" || code == "oc") {
            resolved = replacePlaceholder(resolved, code, a->getValue());
        }
    }
    if (overclockValue > 0)
        resolved = replacePlaceholder(resolved, "oc", overclockValue);

    if (descriptionText) {
        SDL_Color descColor = { 220, 220, 220, 255 };
        descriptionText->LoadTextWrapped(resolved, descColor, 16, 240);
    }
}

vector<DrawableObject*> Card::GetAllLayers() const
{
    vector<DrawableObject*> layers;
    if (background) layers.push_back(background);
    if (starOverlay) layers.push_back(starOverlay);
    if (typeIcon) layers.push_back(typeIcon);
    if (visual) layers.push_back(visual);
    if (cardFrame) layers.push_back(cardFrame);
    if (nameText) layers.push_back(nameText);
    if (descriptionText) layers.push_back(descriptionText);
    return layers;
}
