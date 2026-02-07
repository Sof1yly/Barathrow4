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
            AttackPattern p = AttackPattern::fromGrid(grid, 'X');
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

        if (!trimmed.empty() && (trimmed[0] == '.' || trimmed[0] == 'X')) {
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

bool GameDataLoader::loadFromFile(const std::string& filename,
    std::string* outError)
{
    for (Card* c : cards)      delete c;
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
        if (trimmed.empty()) {
            continue;
        }

        if (!skippedCardHeader) {
            if (trimmed == "Card") {
                skippedCardHeader = true;
                continue;
            }
            skippedCardHeader = true;
        }

        std::vector<std::string> cells = splitCsvRowRespectQuotes_(line);
        for (auto& c : cells) c = trim_(c);
        if (cells.empty()) continue;

        if (cells[0] == "Name") {
            continue;
        }

        // Columns: Name, Damage, Move, Pattern, Level, RarityCode, TypeCode, Desc
        std::string name = cells.size() > 0 ? cells[0] : "";
        std::string sDmg = cells.size() > 1 ? cells[1] : "";
        std::string sMove = cells.size() > 2 ? cells[2] : "";
        std::string patternId = cells.size() > 3 ? cells[3] : "";
        std::string sLevel = cells.size() > 4 ? cells[4] : "0";    
        std::string rarityCode = cells.size() > 5 ? cells[5] : "sta";
        std::string typeCode = cells.size() > 6 ? cells[6] : "atk";
        std::string description = cells.size() > 7 ? cells[7] : "";

        if (name.empty()) {
            continue;
        }

        int damage = 0;
        int move = 0;
        int level = 0;

        try {
            if (!sDmg.empty())
                damage = std::stoi(sDmg);
        }
        catch (...) {
            if (outError) *outError = "Invalid damage: " + sDmg;
            return false;
        }

        try {
            if (!sMove.empty())
                move = std::stoi(sMove);
        }
        catch (...) { 
            if (outError) *outError = "Invalid move: " + sMove;
            return false;
        }
        
        try {
            if(!sLevel.empty())
                level = std::stoi(sLevel);
        }
        catch (...) {
            if (outError)*outError = "Invalid level: " + sLevel;
            return false;
        }

        if (damage <= 0 && move <= 0) {
            if (outError) *outError =
                "Card must have either damage or move: " + name;
            return false;
        }

        Card* card = new Card(name);

        card->setLevel(level);
        card->setRarityCode(rarityCode);
        card->setTypeCode(typeCode);
        card->setDescription(description);

        if (damage > 0) {
            auto* atk = new AttackAction();
            atk->setValue(damage);
            actions_list.push_back(atk);
            card->addAction(atk);

            if (!patternId.empty()) {
                const AttackPattern* pat = findPatternByName(patternId);
                if (!pat) {
                    if (outError) {
                        *outError = "Unknown pattern id '" + patternId +"' for card '" + name + "'";
                    }
                    delete card;
                    return false;
                }
                actionPattern[atk] = pat;
            }
        }

        if (move > 0) {
            auto* mv = new MoveAction();
            mv->setValue(move);
            actions_list.push_back(mv);
            card->addAction(mv);
        }

        cards.push_back(card);
    }

    return true;
}
