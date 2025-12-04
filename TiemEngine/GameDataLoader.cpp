#include "GameDataLoader.h"



// --------- small helpers ---------

static std::string trim_(const std::string& s)
{
    size_t a = 0;
    size_t b = s.size();

    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;

    return s.substr(a, b - a);
}

// split by TAB, respecting quotes (for safety)
static std::vector<std::string> splitCsvRowRespectQuotes_(const std::string& line)
{
    std::vector<std::string> out;
    std::string cur;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                // escaped quote
                cur.push_back('"');
                ++i;
            }
            else {
                inQuotes = !inQuotes;
            }
        }
        else if (ch == '\t' && !inQuotes) {      // <--- TAB separator
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

// --------- ctor / dtor ---------

GameDataLoader::GameDataLoader()
{
}

GameDataLoader::~GameDataLoader()
{
    for (Card* c : cards)      delete c;
    for (Action* a : actions_list) delete a;
}

// --------- patterns ---------

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
            // blank line = end of current pattern
            flushCurrent();
            continue;
        }

        // optional header
        if (trimmed == "PatternAttack") {
            continue;
        }

        // grid row
        if (!trimmed.empty() && (trimmed[0] == '.' || trimmed[0] == 'X')) {
            grid.push_back(trimmed);
            continue;
        }

        // otherwise: new pattern id
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

// --------- cards + actions (CardAction.txt) ---------

bool GameDataLoader::loadFromFile(const std::string& filename,
    std::string* outError)
{
    // Clear old
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
    bool skippedCardHeader = false;   // for the "Card" line

    while (std::getline(file, line)) {
        std::string trimmed = trim_(line);
        if (trimmed.empty()) {
            continue; // ignore blank lines
        }

        // first non-empty line may be "Card"
        if (!skippedCardHeader) {
            if (trimmed == "Card") {
                skippedCardHeader = true;
                continue;
            }
            // if not "Card", just fall through (file without that header)
            skippedCardHeader = true;
        }

        std::vector<std::string> cells = splitCsvRowRespectQuotes_(line);
        for (auto& c : cells) c = trim_(c);
        if (cells.empty()) continue;

        // skip header row "Name  Damage  Move  Pattern"
        if (cells[0] == "Name") {
            continue;
        }

        // columns: Name, Damage, Move, Pattern
        std::string name = cells.size() > 0 ? cells[0] : "";
        std::string sDmg = cells.size() > 1 ? cells[1] : "";
        std::string sMove = cells.size() > 2 ? cells[2] : "";
        std::string patternId = cells.size() > 3 ? cells[3] : "";

        if (name.empty()) {
            // weird line, ignore quietly
            continue;
        }

        int damage = 0;
        int move = 0;

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

        if (damage <= 0 && move <= 0) {
            if (outError) *outError =
                "Card must have either damage or move: " + name;
            return false;
        }

        // ----- create Card -----
        Card* card = new Card(name);

        // attack part
        if (damage > 0) {
            auto* atk = new AttackAction();
            atk->setValue(damage);
            actions_list.push_back(atk);
            card->addAction(atk);

            if (!patternId.empty()) {
                const AttackPattern* pat = findPatternByName(patternId);
                if (!pat) {
                    if (outError) {
                        *outError = "Unknown pattern id '" + patternId +
                            "' for card '" + name + "'";
                    }
                    delete card;
                    return false;
                }
                actionPattern[atk] = pat;
            }
        }

        // move part
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
