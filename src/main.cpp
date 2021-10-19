#include <array>
#include <memory>
#include <ncurses.h>
#include <stack>
#include <iostream>
#include <tuple>
#include <set>
#include <chrono>
#include "field.hpp"
#include "blokus_dumper.hpp"
#include "human_player.hpp"
#include "game_manager.hpp"
#include "player_color.hpp"
#include "computer_player_v1.hpp"
#include "piece_detail.hpp"
#include "move_enumerator_v1.hpp"
#include "move_enumerator_v2.hpp"
#include "move_enumerator_v3.hpp"

#include <random>

class drand
{
	private:
		std::random_device seed;
		std::mt19937 rand;
	public:
		drand() : rand(seed()) {}
		double operator()() { return rand() / static_cast<double>(0x100000000); }
};

void influenceSphereTest();
void moveEnumeratorV2SpeedTest();
void moveEnumeratorV2AccuracyTest();

int main(int argc, char* argv[])
{
	//moveEnumeratorV2AccuracyTest();
	//moveEnumeratorV2SpeedTest();
	//influenceSphereTest();
	//return 0;

	initscr();

	auto field = std::make_shared<Field>();
	auto manager = std::make_shared<GameManager>(field);
	auto dumper = std::make_shared<BlokusDumper>(manager, field);
	manager->setDumper(dumper);

	manager->startGame();

	endwin();

	return 0;
}

void influenceSphereTest()
{
	initscr();

	auto field = std::make_shared<Field>();
	auto manager = std::make_shared<GameManager>(field);
	auto dumper = std::make_shared<BlokusDumper>(manager, field);

	PlayerPtrList playerPtrList =
	{ {
		std::make_shared<HumanPlayer>(PlayerColor::RED),
		std::make_shared<HumanPlayer>(PlayerColor::GREEN),
		std::make_shared<HumanPlayer>(PlayerColor::BLUE),
		std::make_shared<HumanPlayer>(PlayerColor::YELLOW)
	} };
	ConstPlayerPtrList constPlayerPtrList;
	for(std::size_t i = 0; i < playerPtrList.size(); i++) constPlayerPtrList[i] = playerPtrList[i];
	dumper->setConstPlayerPtrList(constPlayerPtrList);

	auto pieceAnalyst = std::make_shared<PieceAnalyst>(PIECE_DEF);
	MoveEnumeratorV3 moveEnumerator(*field, constPlayerPtrList, pieceAnalyst);

	std::stack<std::tuple<int, vec2, Piece>> history;

	for(int playerId = 0; ; )
	{
		const auto& moveCand = moveEnumerator.getMove(playerId, 0);
		moveEnumerator.add(playerId, moveCand);

		const auto& detail = pieceAnalyst->get(moveCand.defId, moveCand.varId);
		Piece piece(detail.defId);
		piece.rotate(detail.rotate);
		piece.reflectX(detail.reflectX);
		field->set(moveCand.x, moveCand.y, piece, playerId);

		for(int i = 0; i < PLAYER_NUM; i++)
		{
			dumper->dump();
			for(int x = 0; x < Field::SIZE; x++)
			{
				for(int y = 0; y < Field::SIZE; y++)
				{
					if(moveEnumerator.isTerritory(i, x, y))
					{
						mvaddch(2 * y + 1, 3 * x + 1, 'x');
					}
				}
			}
			getch();
		}
		while(true)
		{
			int command = getch();
			if(command == '\n')
			{
				//history.push_back( { playerId, vec2(moveCand.x, moveCand.y), piece } );
				for(int i = 1; i <= PLAYER_NUM; i++)
				{
					int j = (playerId + i) % PLAYER_NUM;
					if(moveEnumerator.getMoveNum(j) != 0)
					{
						playerId = j;
						break;
					}
					if(i == PLAYER_NUM)
					{
						goto GAME_TERMINAL;
					}
				}
				break;
			}
			else if(command == 'u')
			{
				if(history.empty()) continue;
			}
		}
	}
GAME_TERMINAL:

	endwin();
}

void moveEnumeratorV2AccuracyTest()
{
	initscr();

	auto field = std::make_shared<Field>();
	auto manager = std::make_shared<GameManager>(field);
	auto dumper = std::make_shared<BlokusDumper>(manager, field);

	PlayerPtrList playerPtrList =
	{ {
		std::make_shared<HumanPlayer>(PlayerColor::RED),
		std::make_shared<HumanPlayer>(PlayerColor::GREEN),
		std::make_shared<HumanPlayer>(PlayerColor::BLUE),
		std::make_shared<HumanPlayer>(PlayerColor::YELLOW)
	} };
	ConstPlayerPtrList constPlayerPtrList;
	for(std::size_t i = 0; i < playerPtrList.size(); i++) constPlayerPtrList[i] = playerPtrList[i];
	dumper->setConstPlayerPtrList(constPlayerPtrList);

	auto pieceAnalyst = std::make_shared<PieceAnalyst>(PIECE_DEF);
	MoveEnumeratorV3 moveEnumerator(*field, constPlayerPtrList, pieceAnalyst);

	std::array<std::array<bool, PIECE_NUM>, PLAYER_NUM> isUsed = {};

	struct Change
	{
		int playerId;
		int x;
		int y;
		Piece piece;
	};
	std::stack<Change> history;

	for(int playerId = 0; playerId >= 0; )
	{
		dumper->dump();
		attrset(0);

		for(int i = 0; i < PLAYER_NUM; i++)
		{
			std::array<std::array<std::array<std::array<bool, MAX_VAR_NUM>, PIECE_NUM>, Field::SIZE>, Field::SIZE> table;
			int moveNum = 0;
			for(int x = 0; x < Field::SIZE; x++)
			{
				for(int y = 0; y < Field::SIZE; y++)
				{
					for(int defId = 0; defId < PIECE_NUM; defId++)
					{
						for(int varId = 0; varId < pieceAnalyst->variationNum(defId); varId++)
						{
							const auto& detail = pieceAnalyst->get(defId, varId);
							Piece p(defId);
							p.rotate(detail.rotate);
							p.reflectX(detail.reflectX);
							if(field->canSet(x, y, p, i) && !isUsed[i][defId])
							{
								table[x][y][defId][varId] = true;
								moveNum++;
							}
							else
							{
								table[x][y][defId][varId] = false;
							}
						}
					}
				}
			}

			bool isTrue = true;
			if(moveNum != moveEnumerator.getMoveNum(i))
			{
				isTrue = false;
			}
			else
			{
				for(int moveId = 0; moveId < moveNum; moveId++)
				{
					const auto& moveCand = moveEnumerator.getMove(i, moveId);
					if(!table[moveCand.x][moveCand.y][moveCand.defId][moveCand.varId])
					{
						isTrue = false;
						break;
					}
				}
			}
			mvprintw(42, i * 4, "%d", isTrue);
			mvprintw(43, i * 4, "%d", moveNum);
			mvprintw(44, i * 4, "%d", moveEnumerator.getMoveNum(i));

			dumper->dump();
			for(int x = 0; x < Field::SIZE; x++)
			{
				for(int y = 0; y < Field::SIZE; y++)
				{
					if(moveEnumerator.isTerritory(i, x, y))
					{
						mvaddch(2 * y + 1, 3 * x + 1, 'x');
					}
				}
			}
			getch();
		}

		int input = getch();
		switch(input)
		{
			case '\n':
				{
					const auto& moveCand = moveEnumerator.getMove(playerId, 0);
					moveEnumerator.add(playerId, moveCand);

					const auto& detail = pieceAnalyst->get(moveCand.defId, moveCand.varId);
					Piece piece(detail.defId);
					piece.rotate(detail.rotate);
					piece.reflectX(detail.reflectX);
					field->set(moveCand.x, moveCand.y, piece, playerId);

					isUsed[playerId][detail.defId] = true;

					history.push( { playerId, moveCand.x, moveCand.y, piece } );

					for(int i = 1; i <= PLAYER_NUM; i++)
					{
						int j = (playerId + i) % PLAYER_NUM;
						if(moveEnumerator.getMoveNum(j) != 0)
						{
							playerId = j;
							break;
						}
						if(i == PLAYER_NUM)
						{
							playerId = -1;
						}
					}
					break;
				}
			case 'u':
				{
					if(moveEnumerator.isInitial()) break;
					moveEnumerator.undo();
					const auto& change = history.top();
					field->remove(change.x, change.y, change.piece);
					playerId = change.playerId;
					history.pop();
					isUsed[change.playerId][change.piece.getDefId()] = false;
					break;
				}
		}
	}

	endwin();
}

void moveEnumeratorV2SpeedTest()
{
	auto field = std::make_shared<Field>();
	auto manager = std::make_shared<GameManager>(field);

	PlayerPtrList playerPtrList =
	{ {
		std::make_shared<HumanPlayer>(PlayerColor::RED),
		std::make_shared<HumanPlayer>(PlayerColor::GREEN),
		std::make_shared<HumanPlayer>(PlayerColor::BLUE),
		std::make_shared<HumanPlayer>(PlayerColor::YELLOW)
	} };
	ConstPlayerPtrList constPlayerPtrList;
	for(std::size_t i = 0; i < playerPtrList.size(); i++) constPlayerPtrList[i] = playerPtrList[i];

	auto pieceAnalyst = std::make_shared<PieceAnalyst>(PIECE_DEF);

	int sampleNum = 1000;

	auto t1 = std::chrono::system_clock::now();

	MoveEnumeratorV2 moveEnumerator(*field, constPlayerPtrList, pieceAnalyst);

	for(int count = 0; count < sampleNum; count++)
	{
		for(int playerId = 0; playerId >= 0; )
		{
			const auto& moveCand = moveEnumerator.getMove(playerId, 0);
			moveEnumerator.add(playerId, moveCand);

			for(int i = 1; i <= PLAYER_NUM; i++)
			{
				int j = (playerId + i) % PLAYER_NUM;
				if(moveEnumerator.getMoveNum(j) != 0)
				{
					playerId = j;
					break;
				}
				if(i == PLAYER_NUM)
				{
					playerId = -1;
				}
			}
		}
		moveEnumerator.init();
	}

	auto t2 = std::chrono::system_clock::now();

	double t = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / (double)sampleNum;
	std::cout << t << std::endl;
}
