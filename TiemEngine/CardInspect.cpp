#include "CardInspect.h"

#include "AttackAction.h"
#include "MoveAction.h"
#include "BuffAction.h"
#include "DebuffAction.h"
#include "EnergyAction.h"
#include "GameObject.h"
#include "ImageObject.h"
#include "TextObject.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace
{
    std::string ReplaceToken(const std::string& text, const std::string& token, const std::string& value)
    {
        std::string out = text;
        std::size_t pos = 0;
        while ((pos = out.find(token, pos)) != std::string::npos)
        {
            out.replace(pos, token.size(), value);
            pos += value.size();
        }
        return out;
    }
}

CardInspect::CardInspect()
    : Card("__inspect__")
{
}

CardInspect::~CardInspect()
{
}

std::string CardInspect::GetKeywordTitle(const Action* action, const std::string& fallbackCode) const
{
    if (!action)
    {
        if (fallbackCode == "oc") return "Overclock";
        if (fallbackCode == "fas") return "Fast";
        if (fallbackCode == "del") return "Delete";
        if (fallbackCode == "per") return "Persist";
        if (fallbackCode == "pre") return "PreLoad";
        if (fallbackCode == "temp") return "Temp";
        if (fallbackCode == "lag") return "Lag";
        return fallbackCode;
    }

    if (const auto* move = dynamic_cast<const MoveAction*>(action))
    {
        switch (move->getSubType())
        {
        case MoveSubType::Move: return "Move";
        case MoveSubType::Retreat: return "Retreat";
        default: break;
        }
    }

    if (const auto* buff = dynamic_cast<const BuffAction*>(action))
    {
        switch (buff->getSubType())
        {
        case BuffSubType::Overclock: return "Overclock";
        case BuffSubType::Shield: return "Shield";
        case BuffSubType::Barrier: return "Barrier";
        case BuffSubType::Heal: return "Heal";
        case BuffSubType::Fast: return "Fast";
        case BuffSubType::Persist: return "Persist";
        case BuffSubType::PreLoad: return "PreLoad";
        default: break;
        }
    }

    if (const auto* debuff = dynamic_cast<const DebuffAction*>(action))
    {
        switch (debuff->getSubType())
        {
        case DebuffSubType::Weaken: return "Weaken";
        case DebuffSubType::Delay: return "Delay";
        case DebuffSubType::Lag: return "Lag";
        case DebuffSubType::Corrupt: return "Corrupt";
        default: break;
        }
    }

    if (const auto* energy = dynamic_cast<const EnergyAction*>(action))
    {
        switch (energy->getSubType())
        {
        case EnergySubType::EnergyCard: return "Energy";
        case EnergySubType::Generate: return "Generate";
        case EnergySubType::Consume: return "Consume";
        default: break;
        }
    }

    if (dynamic_cast<const AttackAction*>(action))
        return "Attack";

    switch (action->getType())
    {
    case ActionType::Attack: return "Attack";
    case ActionType::Move: return "Move";
    case ActionType::Debuff: return "Debuff";
    case ActionType::Buff: return "Buff";
    case ActionType::Energy: return "Energy";
    default: break;
    }

    return fallbackCode;
}

std::string CardInspect::BuildInspectText(Card* cardData, const CardSystem& cardSystem) const
{
    if (!cardData) return "";

    const GameDataLoader& loader = cardSystem.GetDataLoader();

    std::unordered_set<std::string> seen;
    std::vector<std::pair<Action*, std::string>> orderedActions;
    std::unordered_map<std::string, int> values;

    for (Action* a : cardData->getActions())
    {
        if (!a) continue;

        const std::string code = a->getActionCode();
        if (code.empty()) continue;

        if (seen.insert(code).second)
        {
            orderedActions.push_back({ a, code });
        }

        values[code] = a->getValue();
    }

    auto addCardFlag = [&](const std::string& code, int value)
    {
        values[code] = value;
        if (seen.insert(code).second)
        {
            orderedActions.push_back({ nullptr, code });
        }
    };

    if (cardData->getOverclockValue() > 0) addCardFlag("oc", cardData->getOverclockValue());
    if (cardData->getIsFast()) addCardFlag("fas", 0);
    if (cardData->getIsDeleteAfterUse()) addCardFlag("del", 0);
    if (cardData->getIsPersist()) addCardFlag("per", 0);
    if (cardData->getIsPreLoad()) addCardFlag("pre", 0);
    if (cardData->getIsTemp()) addCardFlag("temp", 0);
    if (cardData->getIsLag()) addCardFlag("lag", 0);

    std::string out;
    bool first = true;

    for (const auto& item : orderedActions)
    {
        Action* action = item.first;
        const std::string& code = item.second;

        std::string desc = loader.getActionDescription(code);
        if (desc.empty()) continue;

        auto it = values.find(code);
        if (it != values.end())
        {
            const std::string valueText = std::to_string(it->second);
            desc = ReplaceToken(desc, "X", valueText);
        }

        std::string title;
        if (action)
        {
            title = GetKeywordTitle(action, code);
        }
        else
        {
            title = GetKeywordTitle(nullptr, code);
        }

        if (!first) out += "\n\n";
        out += title + "\n" + desc;
        first = false;
    }

    return out;
}

void CardInspect::BuildInspectGrid(Card* cardData, const CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList)
{
    std::vector<std::pair<IVec2, int>> cells;

    if (cardData)
    {
        for (Action* a : cardData->getActions())
        {
            AttackAction* atk = dynamic_cast<AttackAction*>(a);
            if (!atk) continue;

            const AttackPattern* pat = cardSystem.GetDataLoader().getPatternForAction(a);
            if (pat) {
                cells = pat->applyTo(0, 0);
                break;
            }
        }
    }

    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (const auto& c : cells)
    {
        minX = std::min(minX, c.first.x);
        maxX = std::max(maxX, c.first.x);
        minY = std::min(minY, c.first.y);
        maxY = std::max(maxY, c.first.y);
    }

    const float cellSize = 78.0f;
    const float centerX = 560.0f;
    const float centerY = 30.0f;
    const float z = 915.0f;

    const float totalW = (maxX - minX + 1) * cellSize;
    const float totalH = (maxY - minY + 1) * cellSize;
    const float left = centerX - totalW * 0.5f + cellSize * 0.5f;
    const float top = centerY + totalH * 0.5f - cellSize * 0.5f;

    std::unordered_set<long long> attackSet;
    for (const auto& c : cells)
    {
        long long key = (static_cast<long long>(c.first.x) << 32) ^ static_cast<unsigned int>(c.first.y);
        attackSet.insert(key);
    }

    for (int gx = minX; gx <= maxX; ++gx)
    {
        for (int gy = minY; gy <= maxY; ++gy)
        {
            GameObject* tile = new GameObject();
            tile->SetSize(cellSize - 10.0f, -(cellSize - 10.0f));

            float px = left + (gx - minX) * cellSize;
            float py = top - (gy - minY) * cellSize;
            tile->SetPosition(glm::vec3(px, py, z));

            const bool isOrigin = (gx == 0 && gy == 0);
            long long key = (static_cast<long long>(gx) << 32) ^ static_cast<unsigned int>(gy);
            const bool isAttack = attackSet.find(key) != attackSet.end();

            if (isOrigin) {
                tile->SetColor(0.95f, 0.95f, 0.95f, 0.95f);
            }
            else if (isAttack) {
                tile->SetColor(0.95f, 0.25f, 0.15f, 0.95f);
            }
            else {
                tile->SetColor(0.80f, 0.80f, 0.80f, 0.90f);
            }

            inspectObjects.push_back(tile);
            objectsList.push_back(tile);
        }
    }
}

void CardInspect::Show(Card* cardData, CardSystem& cardSystem, std::vector<DrawableObject*>& objectsList)
{
    if (!cardData) {
        Hide(objectsList);
        return;
    }

    Hide(objectsList);
    cardSystem.UpdateHover(glm::vec3(0.0f, 0.0f, 0.0f), true, objectsList);

    GameObject* panel = new GameObject();
    panel->SetSize(1920.0f, 1080.0f);
    panel->SetPosition(glm::vec3(0.0f, 0.0f, 900.0f));
    panel->SetColor(0.0f, 0.0f, 0.0f, 0.86f);
    inspectObjects.push_back(panel);
    objectsList.push_back(panel);

    if (!cardData->HasVisuals()) {
        cardData->CreateVisuals();
    }

    auto cloneCardLayer = [&](ImageObject* src) {
        if (!src) return;
        ImageObject* copy = new ImageObject();
        copy->SetTextureId(src->GetTextureId());
        copy->SetSize(364.0f, -533.0f);
        copy->SetPosition(glm::vec3(-640.0f, 40.0f, 910.0f));
        inspectObjects.push_back(copy);
        objectsList.push_back(copy);
    };

    cloneCardLayer(cardData->GetBackground());
    cloneCardLayer(cardData->GetStarOverlay());
    cloneCardLayer(cardData->GetTypeIcon());
    cloneCardLayer(cardData->GetVisual());
    cloneCardLayer(cardData->GetCardFrame());

    const float inspectCardW = 364.0f;
    const float inspectCardH = 533.0f;
    const glm::vec3 inspectCardPos(-640.0f, 40.0f, 910.0f);

    if (cardData->GetNameText())
    {
        TextObject* nameOnCard = new TextObject();
        SDL_Color cardNameColor = { 245, 245, 245, 255 };
        nameOnCard->LoadText(cardData->getName(), cardNameColor, 22);

        glm::vec3 local = cardData->GetNameText()->GetLocalPosition();
        float leftAnchorX = (inspectCardPos.x - (inspectCardW * 0.5f)) + (local.x * inspectCardW);
        float centeredX = leftAnchorX + (nameOnCard->GetSize().x * 0.5f);
        float centeredY = inspectCardPos.y + (local.y * inspectCardH);
        nameOnCard->SetPosition(glm::vec3(centeredX, centeredY, 912.0f));

        inspectObjects.push_back(nameOnCard);
        objectsList.push_back(nameOnCard);
    }

    if (cardData->GetDescriptionText())
    {
        std::string cardDesc = cardData->getDescription();
        for (Action* a : cardData->getActions())
        {
            if (!a) continue;
            cardDesc = ReplaceToken(cardDesc, "{" + a->getActionCode() + "}", std::to_string(a->getValue()));
        }

        if (cardData->getOverclockValue() > 0)
        {
            cardDesc = ReplaceToken(cardDesc, "{oc}", std::to_string(cardData->getOverclockValue()));
        }

        TextObject* descOnCard = new TextObject();
        SDL_Color cardDescColor = { 220, 220, 220, 255 };
        descOnCard->LoadTextWrapped(cardDesc, cardDescColor, 18, 300);

        glm::vec3 local = cardData->GetDescriptionText()->GetLocalPosition();
        float leftAnchorX = (inspectCardPos.x - (inspectCardW * 0.5f)) + (local.x * inspectCardW);
        float centeredX = leftAnchorX + (descOnCard->GetSize().x * 0.5f);
        float centeredY = inspectCardPos.y + (local.y * inspectCardH);
        descOnCard->SetPosition(glm::vec3(centeredX, centeredY, 912.0f));

        inspectObjects.push_back(descOnCard);
        objectsList.push_back(descOnCard);
    }

    TextObject* title = new TextObject();
    title->SetPosition(glm::vec3(-10.0f, 420.0f, 920.0f));
    SDL_Color titleColor = { 245, 245, 245, 255 };
    title->LoadText(cardData->getName(), titleColor, 46);
    inspectObjects.push_back(title);
    objectsList.push_back(title);

    std::string keywordText = BuildInspectText(cardData, cardSystem);
    if (!keywordText.empty())
    {
        TextObject* body = new TextObject();
        body->SetPosition(glm::vec3(-80.0f, 105.0f, 920.0f));
        SDL_Color bodyColor = { 230, 230, 230, 255 };
        body->LoadTextWrapped(keywordText, bodyColor, 28, 700);
        inspectObjects.push_back(body);
        objectsList.push_back(body);
    }

    BuildInspectGrid(cardData, cardSystem, objectsList);

    inspectedCard = cardData;
    visible = true;
}

void CardInspect::Hide(std::vector<DrawableObject*>& objectsList)
{
    if (!visible && inspectObjects.empty()) return;

    for (DrawableObject* obj : inspectObjects)
    {
        if (!obj) continue;

        auto it = std::find(objectsList.begin(), objectsList.end(), obj);
        if (it != objectsList.end()) {
            objectsList.erase(it);
        }

        delete obj;
    }

    inspectObjects.clear();
    inspectedCard = nullptr;
    visible = false;
}
