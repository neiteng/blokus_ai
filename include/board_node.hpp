#ifndef BOARD_NODE_HPP
#define BOARD_NODE_HPP

#include <vector>
#include <memory>
#include "move_cand.hpp"

struct BoardNode
{
	//playerIdがmoveを打ってこの盤面になった
	int playerId;
	MoveCand move;
	std::vector<std::shared_ptr<BoardNode>> childPtrList;
	int triedIndex;
	std::weak_ptr<BoardNode> parentPtr;
	bool isChildPtrListMade;
	double victoryNum;
	int trialNum;
};

#endif
