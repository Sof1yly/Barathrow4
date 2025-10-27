#pragma once
#include <unordered_map>
#include "Card.h"
#include "DrawableObject.h"
#include "GameObject.h"

class Hand {
	private:
		struct CardHand {
			GameObject* view=nullptr;

			float homeX = 0.0f, homeY = 0.0f, homeRotDeg = 0.0f;
		};
		vector<CardHand> cardsIn;

		void computeLayout(int idx, int N, float& outX, float& outY, float& outRotDeg) const;
		void applyHome(const CardHand& c)const;

	public:
		bool GetHomeFor(class GameObject* view, glm::vec3& outPos, float& outRotDeg) const;
		void CreateVisualHand(int cardCount, vector<DrawableObject*>& objectsList);
		bool Owns(GameObject* view)const;
		void SnapBack(GameObject* view);

		void UseCardAndRefan(GameObject* view);

		void Refan();

		
};
