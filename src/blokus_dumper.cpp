#include "blokus_dumper.hpp"
#include "game_manager.hpp"
#include <cmath>
#include <sstream>

void BlokusDumper::dump()
{
	clear();

	move(0, 0);

	//カーソルのブロック
	std::array<std::array<bool, Field::SIZE>, Field::SIZE> cursorTable;
	for(auto& ar : cursorTable) ar.fill(false);

	const auto& currentPlayer = playerPtrList[gameManager->getCurrentPlayerId()];
	int cursorX = currentPlayer->getCursorX();
	int cursorY = currentPlayer->getCursorY();
	for(auto [ dx, dy ] : currentPlayer->getCursorPiece().getCellList())
	{
		int rx = cursorX + dx;
		int ry = cursorY + dy;
		if(rx < 0 || rx >= Field::SIZE || ry < 0 || ry >= Field::SIZE) continue;
		cursorTable[rx][ry] = true;
	}

	//盤面描画
	for(std::size_t y = 0; y < Field::SIZE; y++)
	{
		attrset(0);
		for(std::size_t x = 0; x < Field::SIZE; x++)
		{
			addch('+');
			char c = ' ';
			if(cursorTable[x][y] || cursorTable[x][std::max(0, (int)y - 1)])
			{
				c = '-';
			}
			for(int w = 0; w < cellWidth; w++)
			{
				addch(c);
			}
		}
		addch('+');
		addch('\n');

		for(int h = 0; h < cellHeight; h++)
		{
			for(std::size_t x = 0; x < Field::SIZE; x++)
			{
				attrset(0);
				if(cursorTable[x][y] || cursorTable[std::max(0, (int)x - 1)][y])
				{
					addch('|');
				}
				else
				{
					addch(' ');
				}

				const auto& cell = field->get(x, y);
				if(cursorTable[x][y])
				{
					attrset(COLOR_PAIR(cellColorPairMap[currentPlayer->getColor()]));
				}
				else if(cell)
				{
					const auto& playerPtr = playerPtrList[cell.playerId];
					attrset(COLOR_PAIR(cellColorPairMap[playerPtr->getColor()]));
				}
				else
				{
					attrset(0);
				}
				for(int w = 0; w < cellWidth; w++)
				{
					addch(' ');
				}
			}
			attrset(0);
			if(cursorTable[Field::SIZE - 1][y])
			{
				addch('|');
			}
			else
			{
				addch(' ');
			}
			addch('\n');
		}
	}
	attrset(0);
	for(std::size_t x = 0; x < Field::SIZE; x++)
	{
		addch('+');
		for(int w = 0; w < cellWidth; w++)
		{
			if(cursorTable[x][Field::SIZE - 1])
			{
				addch('-');
			}
			else
			{
				addch(' ');
			}
		}
	}
	addch('+');
	addch('\n');

	int scoreY = Field::SIZE * (cellHeight + 1) + 1;
	int scoreViewWidth = 6;
	//スコア描画
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		move(scoreY, playerId * scoreViewWidth);
		attrset(COLOR_PAIR(cellColorPairMap[playerPtrList[playerId]->getColor()]));
		addch(' ');
		addch(' ');
		attrset(0);
		printw("%d", gameManager->getScore(playerId));
	}
	//ステータスメッセージ描画
	attrset(A_REVERSE);
	auto statusMessage = gameManager->getStatusMessage();
	while((int)statusMessage.size() < Field::SIZE * (cellWidth + 1) + 1 - PLAYER_NUM * scoreViewWidth)
	{
		statusMessage.push_back(' ');
	}
	mvprintw(scoreY, PLAYER_NUM * scoreViewWidth, statusMessage.c_str());

	int setX = Field::SIZE * (cellWidth + 1) + 1;

	//持ち駒描画
	for(std::size_t playerId = 0; playerId < playerPtrList.size(); playerId++)
	{
		std::ostringstream oss;
		oss << "player" << (playerId + 1);
		attrset(0);
		mvprintw(10 * playerId, setX + 1, oss.str().c_str());

		const auto& playerPtr = playerPtrList[playerId];
		attrset(COLOR_PAIR(cellColorPairMap[playerPtr->getColor()]));
		const auto& defIdList = playerPtr->getUnusedPieceDefIdList();
		for(std::size_t defIndex = 0; defIndex < defIdList.size(); defIndex++)
		{
			int defId = defIdList[defIndex];
			const auto& cellList = PIECE_DEF[defId];

			int columnNum = (int)std::ceil((float)PIECE_NUM / 2);
			int lineNum = (int)std::ceil((float)PIECE_NUM / columnNum);
			int cx = 10 * (defIndex % columnNum) + 4 + setX;
			int cy = lineNum * 5 * playerId + 5 * (defIndex / columnNum) + 2;

			for(auto [ dx, dy ] : cellList)
			{
				move(cy + dy, cx + dx * 2);
				addch(' ');
				addch(' ');
			}
		}
	}

	attrset(0);
	mvprintw(scoreY + 1, 0, debugLog.c_str());

	refresh();
}

BlokusDumper::BlokusDumper(std::shared_ptr<const GameManager> gameManager_, std::shared_ptr<const Field> field_) :
	gameManager(gameManager_),
	field(field_)
{
	noecho();
	cbreak();
	keypad(stdscr, true);

	int scrwidth, scrheight;
	getmaxyx(stdscr, scrheight, scrwidth);
	cellHeight = (scrheight - (Field::SIZE + 1)) / Field::SIZE;
	cellWidth = (scrwidth - PIECE_NUM * 5 - (Field::SIZE + 1)) / Field::SIZE;
	double cellAspect = 2;// width / height
	cellWidth = std::min(cellWidth, int(cellHeight * cellAspect));
	cellHeight = int(cellWidth / cellAspect);

	start_color();

	init_pair(1, COLOR_WHITE, COLOR_RED);
	cellColorPairMap.insert(std::make_pair(PlayerColor::RED, 1));
	init_pair(2, COLOR_WHITE, COLOR_GREEN);
	cellColorPairMap.insert(std::make_pair(PlayerColor::GREEN, 2));
	init_pair(3, COLOR_WHITE, COLOR_BLUE);
	cellColorPairMap.insert(std::make_pair(PlayerColor::BLUE, 3));
	init_pair(4, COLOR_WHITE, COLOR_YELLOW);
	cellColorPairMap.insert(std::make_pair(PlayerColor::YELLOW, 4));
}

void BlokusDumper::setConstPlayerPtrList(const ConstPlayerPtrList& constPlayerPtrList)
{
	playerPtrList = constPlayerPtrList;
}

void BlokusDumper::setCellWidth(int w)
{
	cellWidth = w;
}

void BlokusDumper::setCellHeight(int h)
{
	cellHeight = h;
}

void BlokusDumper::setDebugLog(const std::string& str)
{
	debugLog = str;
}
