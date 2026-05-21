#include "pch.h"
#include "TextObject.h"
#include "GameEngine.h"
#include "SquareMeshVbo.h"

TextObject::TextObject()
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


TextObject::~TextObject()
{
	if (texture != 0 && ownsTexture) {
		glDeleteTextures(1, &texture);
		texture = 0;
	}
}

void TextObject::Render(glm::mat4 globalModelTransform)
{
	SquareMeshVbo *squareMesh = dynamic_cast<SquareMeshVbo *> (GameEngine::GetInstance()->GetRenderer()->GetMesh(SquareMeshVbo::MESH_NAME));

	GLuint modelMatixId = GameEngine::GetInstance()->GetRenderer()->GetModelMatrixAttrId();
	GLuint modeId = GameEngine::GetInstance()->GetRenderer()->GetModeUniformId();
	GLuint colorId = GameEngine::GetInstance()->GetRenderer()->GetColorUniformId();

	glBindTexture(GL_TEXTURE_2D, texture);

	if (modelMatixId == -1) {
		cout << "Error: Can't perform transformation " << endl;
		return;
	}

	glm::mat4 currentMatrix = this->getTransform();

	if (squareMesh != nullptr) {

		currentMatrix = globalModelTransform * currentMatrix;
		glUniformMatrix4fv(modelMatixId, 1, GL_FALSE, glm::value_ptr(currentMatrix));
		glUniform4f(colorId, 1.0f, 1.0f, 1.0f, alpha);
		glUniform1i(modeId, 1);
		//squareMesh->resetTexcoord();
		squareMesh->Render();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void TextObject::Update(float deltaTime)
{
}

void TextObject::LoadText(string text, SDL_Color textColor, int fontSize)
{
 if (text.empty())
	{
		SetSize(0.0f, 0.0f);
		return;
	}

	glBindTexture( GL_TEXTURE_2D, texture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	TTF_Font * font = TTF_OpenFont("../Resource/Texture/fonts/Tomorrow-Regular.ttf", fontSize);
	if (font)
	{
		SDL_Surface* surfaceMessage = TTF_RenderText_Blended(font, text.c_str(), textColor);
     if (!surfaceMessage)
		{
			SetSize(0.0f, 0.0f);
			TTF_CloseFont(font);
			return;
		}
		GLenum fmt = (surfaceMessage->format->Rmask == 0x000000FF) ? GL_RGBA : GL_BGRA;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surfaceMessage->w, surfaceMessage->h, 0, fmt, GL_UNSIGNED_BYTE, surfaceMessage->pixels);
		SetSize(surfaceMessage->w, -surfaceMessage->h);
		SDL_FreeSurface(surfaceMessage);
		TTF_CloseFont(font);
	}
	else
	{
		cout << "Error: " << TTF_GetError() << endl;
		return;
	}
	
}

void TextObject::LoadTextWrappedTagged(const string& taggedText, SDL_Color defaultColor, int fontSize, int maxWidth)
{
    if (taggedText.empty()) { SetSize(0.0f, 0.0f); return; }

    TTF_Font* font = TTF_OpenFont("../Resource/Texture/fonts/Tomorrow-Regular.ttf", fontSize);
    if (!font) { return; }

    const SDL_Color green = { 100, 220, 100, 255 };

    // Parse <green>...</green> tags into colored segments
    struct Segment { string text; SDL_Color color; };
    vector<Segment> segments;
    {
        size_t pos = 0;
        const string openTag = "<green>", closeTag = "</green>";
        while (pos < taggedText.size()) {
            size_t open = taggedText.find(openTag, pos);
            if (open == string::npos) {
                segments.push_back({ taggedText.substr(pos), defaultColor });
                break;
            }
            if (open > pos) segments.push_back({ taggedText.substr(pos, open - pos), defaultColor });
            size_t inner = open + openTag.size();
            size_t close = taggedText.find(closeTag, inner);
            if (close == string::npos) { segments.push_back({ taggedText.substr(inner), green }); break; }
            segments.push_back({ taggedText.substr(inner, close - inner), green });
            pos = close + closeTag.size();
        }
    }

    // Flatten into (word, color) tokens, preserving newlines
    struct Token { string word; SDL_Color color; bool newline = false; };
    vector<Token> tokens;
    for (const auto& seg : segments) {
        string cur;
        for (size_t i = 0; i <= seg.text.size(); i++) {
            char c = (i < seg.text.size()) ? seg.text[i] : '\0';
            if (c == '\n' || c == ' ' || c == '\0') {
                if (!cur.empty()) { tokens.push_back({ cur, seg.color, false }); cur.clear(); }
                if (c == '\n') tokens.push_back({ "", seg.color, true });
            } else {
                cur += c;
            }
        }
    }

    // Word-wrap tokens into lines
    int spaceW = 0, lineH = TTF_FontLineSkip(font);
    { int h; TTF_SizeUTF8(font, " ", &spaceW, &h); }

    struct Run { string text; SDL_Color color; int x = 0; };
    struct Line { vector<Run> runs; int width = 0; };
    vector<Line> lines;
    Line curLine;
    int curX = 0;

    for (const auto& tok : tokens) {
        if (tok.newline) { lines.push_back(curLine); curLine = Line(); curX = 0; continue; }
        int w = 0, h = 0;
        TTF_SizeUTF8(font, tok.word.c_str(), &w, &h);
        int gap = (curX == 0) ? 0 : spaceW;
        if (curX > 0 && curX + gap + w > maxWidth) { lines.push_back(curLine); curLine = Line(); curX = 0; gap = 0; }
        int runX = curX + gap;
        if (!curLine.runs.empty()) {
            auto& last = curLine.runs.back();
            if (last.color.r == tok.color.r && last.color.g == tok.color.g && last.color.b == tok.color.b) {
                if (gap > 0) last.text += ' ';
                last.text += tok.word;
                curX = runX + w;
                curLine.width = curX;
                continue;
            }
        }
        curLine.runs.push_back({ tok.word, tok.color, runX });
        curX = runX + w;
        curLine.width = curX;
    }
    lines.push_back(curLine);

    // Calculate canvas size
    int canvasW = 1, canvasH = (int)lines.size() * lineH;
    for (const auto& ln : lines) if (ln.width > canvasW) canvasW = ln.width;
    if (canvasH < 1) canvasH = 1;

    // Create RGBA canvas (transparent)
    SDL_Surface* canvas = SDL_CreateRGBSurface(0, canvasW, canvasH, 32,
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    if (!canvas) { TTF_CloseFont(font); return; }
    SDL_FillRect(canvas, nullptr, SDL_MapRGBA(canvas->format, 0, 0, 0, 0));

    // Blit each colored run onto the canvas
    int lineY = 0;
    for (const auto& ln : lines) {
        for (const auto& run : ln.runs) {
            if (run.text.empty()) continue;
            SDL_Surface* surf = TTF_RenderUTF8_Blended(font, run.text.c_str(), run.color);
            if (surf) {
                SDL_Rect dst = { run.x, lineY, surf->w, surf->h };
                SDL_BlitSurface(surf, nullptr, canvas, &dst);
                SDL_FreeSurface(surf);
            }
        }
        lineY += lineH;
    }

    TTF_CloseFont(font);

    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLenum fmt = (canvas->format->Rmask == 0x000000FF) ? GL_RGBA : GL_BGRA;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, canvas->w, canvas->h, 0, fmt, GL_UNSIGNED_BYTE, canvas->pixels);
    SetSize((float)canvasW, -(float)canvasH);
    SDL_FreeSurface(canvas);
}

void TextObject::LoadTextWrapped(string text, SDL_Color textColor, int fontSize, int maxWidth)
{
  if (text.empty())
	{
		SetSize(0.0f, 0.0f);
		return;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	TTF_Font* font = TTF_OpenFont("../Resource/Texture/fonts/Tomorrow-Regular.ttf", fontSize);
	if (font)
	{
		SDL_Surface* surfaceMessage = TTF_RenderText_Blended_Wrapped(font, text.c_str(), textColor, maxWidth);
		if (surfaceMessage)
		{
			GLenum fmt = (surfaceMessage->format->Rmask == 0x000000FF) ? GL_RGBA : GL_BGRA;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surfaceMessage->w, surfaceMessage->h, 0, fmt, GL_UNSIGNED_BYTE, surfaceMessage->pixels);
			SetSize((float)surfaceMessage->w, -(float)surfaceMessage->h);
			SDL_FreeSurface(surfaceMessage);
		}
		TTF_CloseFont(font);
	}
	else
	{
		cout << "Error: " << TTF_GetError() << endl;
		return;
	}
}