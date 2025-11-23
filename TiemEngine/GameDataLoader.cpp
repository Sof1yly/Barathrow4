#include "GameDataLoader.h"
#include <fstream>
#include <sstream>
#include <array>
#include <cctype>
using namespace std;

static std::string trim_(const std::string& s) {
    size_t a = 0;
    size_t b = s.size();

    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) {
        ++a;
    }
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) {
        --b;
    }
    return s.substr(a, b - a);
}


GameDataLoader::GameDataLoader() = default;

GameDataLoader::~GameDataLoader() {
    for (Card* c : cards) delete c;
    cards.clear();
    for (Action* a : actions_list) delete a;
    actions_list.clear();
    actions.clear();
}


static vector<string> splitCsvRowRespectQuotes_(const string& line) {
    vector<string> out;
    string cur;
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
        else if (ch == ',' && !inQuotes) {
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


Action* GameDataLoader::findActionByName(const string& name) const {
    for (const NameAction& na : actions) {
        if (na.name == name) return na.action;
    }
    return nullptr;
}


bool GameDataLoader::parseActionRow(const vector<string>& cols, string* error) {
    if (cols.size() < 3) {
        if (error) *error = "Not enough columns for action (need Name,Damage,Move,Pat)";
        return false;
    }
    const string& name = cols[0];
    const string& sDmg = cols[1];
    const string& sMove = cols[2];

    string patternId;
    if (cols.size() >= 4) {
        patternId = trim_(cols[3]);
    }

    if (name.empty()) 
    { 
        if (error) 
        *error = "Action name is empty"; 
        return false; 
    }
    if (findActionByName(name) != nullptr) {
        if (error) 
            *error = "Duplicate action name: " + name; 
        return false;
    }

    int damage = 0, move = 0;
    try 
    { 
        if (!sDmg.empty()) 
        damage = stoi(sDmg); 
    }
    catch (...) 
    { 
        if (error) 
            *error = "Invalid damage: " + sDmg; 
        return false; 
    }
    try 
    { 
        if (!sMove.empty()) 
        move = stoi(sMove); 
    }
    catch (...) { 
        if (error) 
        *error = "Invalid move: " + sMove; 
        return false; 
    }

    if (damage > 0 && move > 0) 
    { 
        if (error) 
        *error = "Action has both damage and move: " + name; 
        return false; 
    }
    if (damage <= 0 && move <= 0) 
    { 
        if (error) 
        *error = "Action must have either damage or move: " + name; 
        return false; 
    }
    Action* a = nullptr;
    bool isAttack = false;

    if (damage > 0) {
        auto* aa = new AttackAction();
        aa->setValue(damage);
        a = aa;
        isAttack = true;
    }
    else {
        auto* ma = new MoveAction();
        ma->setValue(move);
        a = ma;
    }

    actions.push_back({ name, a });
    actions_list.push_back(a);

    if (isAttack && !patternId.empty()) {
        const AttackPattern* pat = findPatternByName(patternId);
        if (!pat) {
            if (error) {
                *error = "Unknown pattern id '" + patternId + "' for action '" + name + "'";
            }
            return false;
        }

        actionPattern[a] = pat;
    }

    return true;
}

bool GameDataLoader::parseCardRow(const vector<string>& cols, string* error) {
    if (cols.empty() || cols[0].empty()) { if (error) *error = "Card name is empty"; return false; }

    const string& cardName = cols[0];
    Card* c = new Card(cardName);

    // cols[1] is a single cell that may contain comma-separated action names (possibly quoted in the CSV).
    if (cols.size() >= 2 && !cols[1].empty()) {
        // split by ',' inside this single cell (no quotes to worry about now)
        string cell = cols[1];
        // strip any surrounding spaces
        size_t start = 0, end = cell.size();
        while (start < end && isspace(static_cast<unsigned char>(cell[start]))) ++start;
        while (end > start && isspace(static_cast<unsigned char>(cell[end - 1]))) --end;
        cell = cell.substr(start, end - start);

        // split action list by commas
        size_t pos = 0;
        while (true) {
            size_t comma = cell.find(',', pos);
            string namePart = (comma == string::npos) ? cell.substr(pos) : cell.substr(pos, comma - pos);

            size_t i = 0, j = namePart.size();
            while (i < j && isspace(static_cast<unsigned char>(namePart[i]))) i++;
            while (j > i && isspace(static_cast<unsigned char>(namePart[j - 1]))) j--;
            namePart = namePart.substr(i, j - i);

            if (!namePart.empty()) {
                Action* a = findActionByName(namePart);
                if (!a) {
                    if (error) *error = "Action not found for card '" + cardName + "': " + namePart;
                    delete c;
                    return false;
                }
                c->addAction(a);
            }

            if (comma == string::npos) break;
            pos = comma + 1;
        }
    }

    cards.push_back(c);
    return true;
}

bool GameDataLoader::loadFromFile(const string& filename, string* outError) {
    for (Card* c : cards) delete c; cards.clear();
    for (Action* a : actions_list) delete a; actions_list.clear();
    actions.clear();

    ifstream file(filename);
    if (!file.is_open()) {
        if (outError) *outError = "Failed to open file: " + filename;
        return false;
    }

    // We’ll first collect rows, then build actions, then cards.
    vector<array<string, 4>> actionRows; // {Name,Damage,Move,Pattern}
    vector<pair<string, string>> cardRows; // {CardName, ActionCell}

    string line;
    bool skippedHeaderRow0 = false;
    bool skippedHeaderRow1 = false;

    while (getline(file, line)) {
        if (line.empty()) continue;

        vector<string> cells = splitCsvRowRespectQuotes_(line);
        // Row 0: Action,,,,,Card,
        // Row 1: Name,Damage,Move,,,Name,Action
        if (!skippedHeaderRow0) {
            // First row must contain "Action" and "Card" 
            bool hasAction = false, hasCard = false;
            for (const auto& c : cells) {
                if (c == "Action") hasAction = true;
                if (c == "Card")   hasCard = true;
            }
            if (hasAction && hasCard) { skippedHeaderRow0 = true; continue; }
        }
        if (!skippedHeaderRow1) {
            bool looksLikeHeader1 = (cells.size() >= 7 &&
                cells[0] == "Name" && cells[1] == "Damage" && cells[2] == "Move" &&
                cells[5] == "Name" && cells[6] == "Action");
            if (looksLikeHeader1) { skippedHeaderRow1 = true; continue; }
        }

        // Safe-accessor: auto fill missing cells as empty
        auto getCell = [&](size_t idx) -> string {
            return (idx < cells.size()) ? cells[idx] : string();
            };

        // Action part is columns 0,1,2,3
        string aName = getCell(0);
        string aDmg = getCell(1);
        string aMove = getCell(2);
        string aPat = getCell(3);

        // Card part is columns 5,6
        string cName = getCell(5);
        string cActs = getCell(6); 

        // If there is an action row on this line, store it
        if (!aName.empty() || !aDmg.empty() || !aMove.empty()||aPat.empty()) {
            // Only push if it looks like a valid action row (has a name at least)
            if (!aName.empty()) {
                actionRows.push_back({ aName, aDmg, aMove, aPat});
            }
                
        }

        // If there is a card row on this line, store it
        if (!cName.empty() || !cActs.empty()) {
            if (!cName.empty()) {
                cardRows.emplace_back(cName, cActs);
            }
                
        }
    }

    // Build all actions first
    for (const auto& row : actionRows) {
        vector<string> cols;
        cols.push_back(row[0]); // Name
        cols.push_back(row[1]); // Damage
        cols.push_back(row[2]); // Move
		cols.push_back(row[3]); // Pattern
        if (!parseActionRow(cols, outError)) return false;
    }

    // Then build all cards
    for (const auto& row : cardRows) {
        vector<string> cols;
        cols.push_back(row.first);   // Card name
        cols.push_back(row.second);  // single cell containing comma-separated actions
        if (!parseCardRow(cols, outError)) return false;
    }

    return true;
}

bool GameDataLoader::loadPatternsFromFile(const std::string& filename, std::string* outError)
{
    patternMap.clear();
    actionPattern.clear();  

    std::ifstream file(filename);
    if (!file.is_open()) {
        if (outError != nullptr) {
            *outError = "Failed to open pattern file: " + filename;
        }
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
            // blank line = end of current pattern (if any)
            flushCurrent();
            continue;
        }

        // skip header
        if (trimmed == "PatternAttack") {
            continue;
        }

        // row that starts with '.' or 'X' is a grid row
        if (!trimmed.empty() && (trimmed[0] == '.' || trimmed[0] == 'X')) {
            grid.push_back(trimmed);
            continue;
        }

        // otherwise this starts a new pattern, e.g. "A1   .X."
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

    // flush last one at EOF
    flushCurrent();

    if (patternMap.empty()) {
        if (outError != nullptr) {
            *outError = "No patterns loaded from file: " + filename;
        }
        return false;
    }

    return true;
}

const AttackPattern* GameDataLoader::findPatternByName(const std::string& id) const
{
    auto it = patternMap.find(id);
    if (it == patternMap.end()) {
        return nullptr;
    }
    return &it->second;
}

const AttackPattern* GameDataLoader::getPatternForAction(const Action* a) const
{
    auto it = actionPattern.find(a);
    if (it == actionPattern.end()) {
        return nullptr;
    }
    return it->second;
}
