#include "GameDataLoader.h"

static std::string trim_(const std::string& s)
{
    size_t a = 0;
    size_t b = s.size();

    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;

    return s.substr(a, b - a);
}

static std::vector<std::string> splitCsvRowRespectQuotes_(const std::string& line)
{
    std::vector<std::string> out;
    std::string cur;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                cur.push_back('"');
                ++i;
            }
            else {
                inQuotes = !inQuotes;
            }
        }
        else if (ch == '\t' && !inQuotes) {      
            out.push_back(cur);
            cur.clear();
        }
        else {
            cur.push_back(ch);
        }
    }
    out.push_back(cur);
    return out;
}

GameDataLoader::GameDataLoader()
{
}

GameDataLoader::~GameDataLoader()
{
    for (Card* c : cards) {
        if (c) delete c;
    }
    cards.clear();
    

    for (Action* a : actions_list) {
        if (a) delete a;
    }
    actions_list.clear();
}

bool GameDataLoader::loadPatternsFromFile(const std::string& filename,
    std::string* outError)
{
    patternMap.clear();
    actionPattern.clear();

    std::ifstream file(filename);
    if (!file.is_open()) {
        if (outError) *outError = "Failed to open pattern file: " + filename;
        return false;
    }

    std::string line;
    std::string currentId;
    std::vector<std::string> grid;

    auto flushCurrent = [&]() {
        if (!currentId.empty() && !grid.empty()) {
            int oCol = -1;
            int oRow = -1;
            for (int r = 0; r < (int)grid.size(); ++r) {
                for (int c = 0; c < (int)grid[r].size(); ++c) {
                    if (grid[r][c] == 'O') {
                        oRow = r;
                        oCol = c;
                        break;
                    }
                }
                if (oRow >= 0) break;
            }
            AttackPattern p = AttackPattern::fromGrid(grid, 'X', oCol, oRow);
            patternMap[currentId] = p;
        }
        currentId.clear();
        grid.clear();
    };

    while (std::getline(file, line)) {
        std::string trimmed = trim_(line);
        if (trimmed.empty()) {
            flushCurrent();
            continue;
        }

        if (trimmed == "PatternAttack") {
            continue;
        }

        if (!trimmed.empty() && (trimmed[0] == '.' || trimmed[0] == 'X' || trimmed[0] == 'O')) {
            grid.push_back(trimmed);
            continue;
        }

        flushCurrent();

        std::istringstream iss(trimmed);
        std::string id;
        iss >> id;
        currentId = id;

        std::string rest;
        std::getline(iss, rest);
        rest = trim_(rest);
        if (!rest.empty()) {
            grid.push_back(rest);
        }
    }

    flushCurrent();

    if (patternMap.empty()) {
        if (outError) *outError = "No patterns loaded from file: " + filename;
        return false;
    }

    return true;
}

const AttackPattern* GameDataLoader::findPatternByName(const std::string& id) const
{
    auto it = patternMap.find(id);
    if (it == patternMap.end()) return nullptr;
    return &it->second;
}

const AttackPattern* GameDataLoader::getPatternForAction(const Action* a) const
{
    auto it = actionPattern.find(a);
    if (it == actionPattern.end()) return nullptr;
    return it->second;
}


// Card-level flags (fas, te, del, oc) 
Action* GameDataLoader::createAction(const std::string& code, int value, float multiplier,const std::string& patternId, Card* card,
    std::string* outError)
{
    Action* newAction = nullptr;

    if (code == "atk") {
        auto* a = new AttackAction();
        a->setValue(value);
        a->setBaseValue(value);
        a->setMultiplier(multiplier);
        a->setActionCode(code);

        if (!patternId.empty()) {
            const AttackPattern* pat = findPatternByName(patternId);
            if (!pat) {
                if (outError) {
                    *outError = "Unknown pattern id '" + patternId + "' for card '" + card->getName() + "'";
                } 
                delete a;
                return nullptr;
            }
            actionPattern[a] = pat;
        }
        newAction = a;
    }
    else if (code == "mov" || code == "re") {
        auto* a = new MoveAction(MoveAction::codeToSubType(code));
        a->setValue(value);
        a->setBaseValue(value);
        a->setMultiplier(multiplier);
        a->setActionCode(code);
        newAction = a;
    }
    else if (code == "sh" || code == "ba" || code == "ge" ||
             code == "he" || code == "fas") {
        auto* a = new BuffAction(BuffAction::codeToSubType(code));
        a->setValue(value);
        a->setBaseValue(value);
        a->setMultiplier(multiplier);
        a->setActionCode(code);
        newAction = a;
    }
    else if (code == "wk" || code == "dl" || code == "cr") {
        auto* a = new DebuffAction(DebuffAction::codeToSubType(code));
        a->setValue(value);
        a->setBaseValue(value);
        a->setMultiplier(multiplier);
        a->setActionCode(code);
        newAction = a;
    }
    else if (code == "en" || code == "co" || code == "gc") {
        auto* a = new EnergyAction(EnergyAction::codeToSubType(code));
        a->setValue(value);
        a->setBaseValue(value);
        a->setMultiplier(multiplier);
        a->setActionCode(code);
        newAction = a;
    }
    else {
        // Unknown code — log a warning and skip rather than crash
        std::cout << "[GameDataLoader] Warning: unknown action code '" << code
                  << "' on card '" << card->getName() << "', skipping." << std::endl;
        return nullptr;
    }

    return newAction;
}

bool GameDataLoader::loadFromFile(const std::string& filename,std::string* outError)
{
    for (Card* c : cards)          delete c;
    cards.clear();
    for (Action* a : actions_list) delete a;
    actions_list.clear();
    actions.clear();
    actionPattern.clear();

    std::ifstream file(filename);
    if (!file.is_open()) {
        if (outError) *outError = "Failed to open file: " + filename;
        return false;
    }

    std::string line;
    bool skippedCardHeader = false;

    while (std::getline(file, line)) {
        std::string trimmed = trim_(line);
        if (trimmed.empty()) continue;

        if (!skippedCardHeader) {
            if (trimmed == "Card") { skippedCardHeader = true; continue; }
            skippedCardHeader = true;
        }

        std::vector<std::string> cells = splitCsvRowRespectQuotes_(line);
        for (auto& c : cells) c = trim_(c);
        if (cells.empty()) continue;
        if (cells[0] == "Name") continue;

        // Columns: Name, Effects, Pattern, Level, Rarity, Type, Desc
        std::string name        = cells.size() > 0 ? cells[0] : "";
        std::string actionStr   = cells.size() > 1 ? cells[1] : "";
        std::string patternId   = cells.size() > 2 ? cells[2] : "";
        std::string sLevel      = cells.size() > 3 ? cells[3] : "0";
        std::string rarityCode  = cells.size() > 4 ? cells[4] : "sta";
        std::string typeCode    = cells.size() > 5 ? cells[5] : "atk";
        std::string description = cells.size() > 6 ? cells[6] : "";

        if (name.empty()) continue;

        int level = 0;
        try { if (!sLevel.empty()) level = std::stoi(sLevel); }
        catch (...) {
            if (outError) *outError = "Invalid level: " + sLevel;
            return false;
        }

        // Parse semicolon-separated action entries: code or code:value or code:valueXmultiplier
        std::vector<std::pair<std::string, std::string>> actionEntries;
        {
            std::istringstream ss(actionStr);
            std::string token;
            while (std::getline(ss, token, ';')) {
                token = trim_(token);
                if (token.empty()) continue;
                size_t colonPos = token.find(':');
                std::string code = colonPos != std::string::npos
                    ? trim_(token.substr(0, colonPos)) : token;
                std::string val  = colonPos != std::string::npos
                    ? trim_(token.substr(colonPos + 1)) : "0";
                actionEntries.push_back({ code, val });
            }
        }

        if (actionEntries.empty()) {
            if (outError) *outError = "Card has no actions: " + name;
            return false;
        }

        Card* card = new Card(name);
        card->setLevel(level);
        card->setRarityCode(rarityCode);
        card->setTypeCode(typeCode);
        card->setDescription(description);

        for (auto& entry : actionEntries) {
            const std::string& code = entry.first;
            const std::string& sVal = entry.second;

            // ---- Card-level flags (no action object created) ----
            if (code == "fas") { card->setIsFast(true);         continue; }
            if (code == "te")  { card->setIsTemp(true);         continue; }
            if (code == "del") { card->setIsDeleteAfterUse(true); continue; }
            if (code == "oc") {
                int ocVal = 0;
                try { if (!sVal.empty()) ocVal = std::stoi(sVal); } catch (...) {}
                card->setOverclockValue(ocVal);
                continue;
            }

            // ---- Parse numeric value and optional multiplier (valueXmult) ----
            int   value      = 0;
            float multiplier = 1.0f;
            {
                size_t xPos = sVal.find('x');
                if (xPos != std::string::npos) {
                    std::string sV = sVal.substr(0, xPos);
                    std::string sM = sVal.substr(xPos + 1);
                    try { if (!sV.empty()) value      = std::stoi(sV);  }
                    catch (...) {
                        if (outError)
                            *outError = "Invalid value in action '" + code + "': " + sV;
                        delete card; return false;
                    }
                    try { if (!sM.empty()) multiplier = std::stof(sM);  }
                    catch (...) {
                        if (outError)
                            *outError = "Invalid multiplier in action '" + code + "': " + sM;
                        delete card; return false;
                    }
                }
                else {
                    try { if (!sVal.empty()) value = std::stoi(sVal); }
                    catch (...) {
                        if (outError)
                            *outError = "Invalid value in action '" + code + "': " + sVal;
                        delete card; return false;
                    }
                }
            }

            // ---- Create the typed action object via factory ----
            Action* newAction = createAction(code, value, multiplier,
                                             patternId, card, outError);
            if (!newAction) {
                if (outError && !outError->empty()) {
                    delete card; return false;
                }
                continue;
            }

            actions_list.push_back(newAction);
            card->addAction(newAction);
        }

        cards.push_back(card);
    }

    return true;
}
