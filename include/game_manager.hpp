#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <vector>
#include <array>
#include <memory>
#include <string>
#include "player.hpp"
#include "field.hpp"
#include "piece.hpp"
#include "dumpable.hpp"

class GameManager : public Dumpable
{
	public:
		GameManager(std::shared_ptr<Field> field_);
		int getCurrentPlayerId() const;
		int getScore(int playerId) const;

		void startGame(int startPlayerId = 0);

		const std::string& getStatusMessage() const;

	private:
		PlayerPtrList playerPtrList;
		std::array<std::vector<int>, PLAYER_NUM> unusedPieceDefIdTable;
		int curPlayerId;
		std::shared_ptr<Field> field;
		std::string statusMessage;
		std::array<int, PLAYER_NUM> scoreList;

		bool isPlaceable();
};

#endif
