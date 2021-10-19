#include "game_manager.hpp"
#include "blokus_dumper.hpp"
#include "ncurses.h"
#include "human_player.hpp"
#include "computer_player_v1.hpp"
#include <sstream>

GameManager::GameManager(std::shared_ptr<Field> field_)
	: curPlayerId(0),
	field(field_) {}

int GameManager::getCurrentPlayerId() const
{
	return curPlayerId;
}

bool GameManager::isPlaceable()
{
	for(int defId : unusedPieceDefIdTable[curPlayerId])
	{
		Piece piece(defId);
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 2; j++)
			{
				for(int x = 0; x < Field::SIZE; x++)
				{
					for(int y = 0; y < Field::SIZE; y++)
					{
						if(field->canSet(x, y, piece, curPlayerId))
						{
							return true;
						}
					}
				}

				piece.reflectX();
			}
			piece.rotate();
		}
	}
	return false;
}

void GameManager::startGame(int startPlayerId)
{
	playerPtrList =
	{ {
		std::make_shared<HumanPlayer>(PlayerColor::RED),
		std::make_shared<HumanPlayer>(PlayerColor::GREEN),
		std::make_shared<HumanPlayer>(PlayerColor::BLUE),
		std::make_shared<HumanPlayer>(PlayerColor::YELLOW)
	} };
	ConstPlayerPtrList constPlayerPtrList;
	for(std::size_t i = 0; i < playerPtrList.size(); i++) constPlayerPtrList[i] = playerPtrList[i];
	if(auto dumper = getDumper())
	{
		dumper->setConstPlayerPtrList(constPlayerPtrList);
		for(std::size_t i = 0; i < playerPtrList.size(); i++) playerPtrList[i]->setDumper(dumper);
	}

	while(true)
	{
		for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
		{
			std::ostringstream oss;
			oss << "is player" << (playerId + 1) << " a computer? (yN)";
			statusMessage = oss.str();
			if(auto dumper = getDumper())
			{
				dumper->dump();
			}

			PlayerColor playerColor;
			switch(playerId)
			{
				case 0:
					playerColor = PlayerColor::RED;
					break;
				case 1:
					playerColor = PlayerColor::GREEN;
					break;
				case 2:
					playerColor = PlayerColor::BLUE;
					break;
				case 3:
					playerColor = PlayerColor::YELLOW;
					break;
			}

			while(true)
			{
				char in = getch();
				if(in == 'y' || in == 'Y')
				{
					playerPtrList[playerId] = std::make_shared<ComputerPlayerV1>(playerColor);
					constPlayerPtrList[playerId] = playerPtrList[playerId];
					break;
				}
				else if(in == 'n' || in == 'N')
				{
					playerPtrList[playerId] = std::make_shared<HumanPlayer>(playerColor);
					constPlayerPtrList[playerId] = playerPtrList[playerId];
					break;
				}
			}
			auto& unusedPieceDefIdList = unusedPieceDefIdTable[playerId];
			unusedPieceDefIdList.clear();
			for(std::size_t i = 0; i < PIECE_NUM; i++)
			{
				unusedPieceDefIdList.push_back(i);
			}
			playerPtrList[playerId]->setUnusedPieceDefIdList(unusedPieceDefIdList);
			scoreList[playerId] = 0;
		}
		field->clear();

		if(auto dumper = getDumper())
		{
			dumper->setConstPlayerPtrList(constPlayerPtrList);
			for(std::size_t i = 0; i < playerPtrList.size(); i++) playerPtrList[i]->setDumper(dumper);
		}

		std::array<bool, PLAYER_NUM> isPlaceableTable;
		std::fill(isPlaceableTable.begin(), isPlaceableTable.end(), false);

		for(int playerCounter = startPlayerId; ; playerCounter++)
		{
			curPlayerId = playerCounter % playerPtrList.size();

			isPlaceableTable[curPlayerId] = isPlaceable();

			if(isPlaceableTable[curPlayerId])
			{
				std::ostringstream oss;
				oss << "player" << (curPlayerId + 1) << "'s turn";
				statusMessage = oss.str();
				//置けるピースがあるのなら置いてもらう
				auto curPlayer = playerPtrList[curPlayerId];
				auto& unusedPieceDefIdList = unusedPieceDefIdTable[curPlayerId];

				ConstPlayerPtrList constPlayerPtrList;
				std::copy(playerPtrList.begin(), playerPtrList.end(), constPlayerPtrList.begin());

				auto [ v_, pp ] = curPlayer->turn(*field, curPlayerId, constPlayerPtrList);
				int px = v_.x;
				int py = v_.y;

				//ピースが今まで使われていないかチェック
				int defId = pp.getDefId();
				bool isUnusedPiece = false;
				auto usedPieceIt = unusedPieceDefIdList.begin();
				for(; usedPieceIt != unusedPieceDefIdList.end(); ++usedPieceIt)
				{
					if((*usedPieceIt) == defId)
					{
						isUnusedPiece = true;
						break;
					}
				}
				//今まで使われてないかつフィールドに置けたら
				if(isUnusedPiece && field->set(px, py, pp, curPlayerId))
				{
					unusedPieceDefIdList.erase(usedPieceIt);
					scoreList[curPlayerId] += pp.getCellList().size();
				}
				else
				{
					playerCounter--;
				}

				//もうピース全部置いてしまったらもう置けない
				if(unusedPieceDefIdList.empty()) isPlaceableTable[curPlayerId] = false;

				curPlayer->setUnusedPieceDefIdList(unusedPieceDefIdList);

				if(auto dumper = getDumper())
				{
					dumper->dump();
				}
			}

			//全てのプレイヤーが置けなくなったら
			if(!std::any_of(isPlaceableTable.begin(), isPlaceableTable.end(), [](bool b) { return b; }))
			{
				//ゲーム終了
				break;
			}
		}

		std::vector<int> winnerList;
		int winnersScore = 0;
		for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
		{
			int score = scoreList[playerId];
			if(score >= winnersScore)
			{
				if(score > winnersScore)
				{
					winnerList.clear();
					winnersScore = score;
				}
				winnerList.push_back(playerId);
			}
		}
		std::ostringstream oss;
		for(int i = 0; i < (int)winnerList.size(); i++)
		{
			oss << "player" << (winnerList[i] + 1);
			if(i <= (int)winnerList.size() - 2) oss << " and ";
		}
		oss << " won. continue? (yN)";
		statusMessage = oss.str();
		if(auto dumper = getDumper())
		{
			dumper->dump();
		}
		while(true)
		{
			char input = getch();
			if(input == 'y' || input == 'Y') break;
			if(input == 'n' || input == 'N') return;
		}
	}
}

int GameManager::getScore(int playerId) const
{
	return scoreList[playerId];
}

const std::string& GameManager::getStatusMessage() const
{
	return statusMessage;
}
