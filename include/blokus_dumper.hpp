#ifndef BLOKUS_DUMPER_HPP
#define BLOKUS_DUMPER_HPP

#include <ncurses.h>
#include <unordered_map>
#include <string>
#include "player_color.hpp"
#include "player.hpp"

class GameManager;
class Field;

class BlokusDumper
{
	private:
		std::shared_ptr<const GameManager> gameManager;
		std::shared_ptr<const Field> field;
		ConstPlayerPtrList playerPtrList;
		std::unordered_map<PlayerColor, int> cellColorPairMap;

		int cellWidth = 2;
		int cellHeight = 1;
		std::string debugLog;

	public:
		BlokusDumper(std::shared_ptr<const GameManager> gameManager_, std::shared_ptr<const Field> field_);

		void dump();

		void setConstPlayerPtrList(const ConstPlayerPtrList& constPlayerPtrList);
		void setCellWidth(int w);
		void setCellHeight(int h);
		void setDebugLog(const std::string& str);
};

#endif
