#include "computer_player_v1.hpp"
#include "move_enumerator_v1.hpp"
#include "move_enumerator_v2.hpp"
#include "board_node.hpp"
#include "blokus_dumper.hpp"
#include <cmath>
#include <iostream>
#include <utility>
#include <ncurses.h>
#include <chrono>
#include <string>
#include <sstream>

std::shared_ptr<BoardNode> bestChild(const std::shared_ptr<BoardNode>& parentPtr, double c)
{
	int t = parentPtr->trialNum;
	std::size_t bestIndex;
	double max;
	for(std::size_t i = 0; i < parentPtr->childPtrList.size(); i++)
	{
		const auto& childPtr = parentPtr->childPtrList[i];
		double u = childPtr->victoryNum / childPtr->trialNum + c * std::sqrt(2.0 * std::log((double)t) / childPtr->trialNum);
		if(i == 0 || u >= max)
		{
			max = u;
			bestIndex = i;
		}
	}
	return parentPtr->childPtrList[bestIndex];
}

std::tuple<vec2, Piece> ComputerPlayerV1::turn(const Field& field, int yourId, const ConstPlayerPtrList& constPlayerPtrList)
{
	cursorX = cursorY = Field::SIZE / 2;
	cursorPiece = Piece(unusedPieceDefIdList.front());
	selectedIndex = 0;

	MoveEnumeratorV1 moveEnumerator(field, constPlayerPtrList, pieceAnalyst);

	auto rootNodePtr = std::make_shared<BoardNode>();
	rootNodePtr->playerId = (yourId + PLAYER_NUM - 1) % PLAYER_NUM;
	rootNodePtr->victoryNum = 0;
	rootNodePtr->trialNum = 0;
	rootNodePtr->isChildPtrListMade = false;
	auto currentNodePtr = rootNodePtr;

	auto start = std::chrono::system_clock::now();
	auto latestRefleshTime = start;

	std::vector<int> playoutNumList;

	for(int trialCount = 0; trialCount < 1000000; trialCount++)
	{
		auto end = std::chrono::system_clock::now();
		if(std::chrono::duration_cast<std::chrono::seconds>(end - start).count() > 20)
		{
			break;
		}
		int depth = 0;
		while(true)
		{
			if(!currentNodePtr->isChildPtrListMade)
			{
				//まだ子ノードが作られていない場合、作る
				int playerId;
				int moveNum;
				for(int i = 1; i <= PLAYER_NUM; i++)
				{
					playerId = (currentNodePtr->playerId + i) % PLAYER_NUM;
					moveNum = moveEnumerator.getMoveNum(playerId);
					if(moveNum != 0) break;
					if(i == PLAYER_NUM) goto TREE_POLICY_TERMINAL;
				}
				currentNodePtr->childPtrList.resize(moveNum);
				for(int moveId = 0; moveId < moveNum; moveId++)
				{
					auto childPtr = std::make_shared<BoardNode>();
					childPtr->playerId = playerId;
					childPtr->move = moveEnumerator.getMove(playerId, moveId);
					childPtr->parentPtr = currentNodePtr;
					childPtr->isChildPtrListMade = false;
					childPtr->victoryNum = 0;
					childPtr->trialNum = 0;
					currentNodePtr->childPtrList[moveId] = childPtr;
				}
				currentNodePtr->triedIndex = moveNum;
				currentNodePtr->isChildPtrListMade = true;
			}
			//もし子ノードを全て試行していないなら
			if(currentNodePtr->triedIndex > 0)
			{
				currentNodePtr->triedIndex--;
				currentNodePtr = currentNodePtr->childPtrList[currentNodePtr->triedIndex];
				moveEnumerator.add(currentNodePtr->playerId, currentNodePtr->move);
				break;
			}
			else
			{
				//子ノードは全て試行されたから、子ノードの中から一番有望なノードを選ぶ
				currentNodePtr = bestChild(currentNodePtr, 1.0);
				moveEnumerator.add(currentNodePtr->playerId, currentNodePtr->move);
			}
			depth++;
		}

TREE_POLICY_TERMINAL:
		while(playoutNumList.size() <= (std::size_t)depth) playoutNumList.push_back(0);
		playoutNumList[depth]++;
		//プレイアウトする
		for(int playerId = currentNodePtr->playerId; ; )
		{
			int moveNum;
			for(int i = 1; i <= PLAYER_NUM; i++)
			{
				int nextPlayerId = (playerId + i) % PLAYER_NUM;
				moveNum = moveEnumerator.getMoveNum(nextPlayerId);
				if(moveNum != 0)
				{
					playerId = nextPlayerId;
					break;
				}
				if(i == PLAYER_NUM) goto UCT_TERMINAL;
			}
			int index = int(real_dist(mt_rand) * moveNum);
			moveEnumerator.add(playerId, moveEnumerator.getMove(playerId, index));
		}
UCT_TERMINAL:
		//スコアをつける
		std::array<std::pair<int, int>, PLAYER_NUM> scoreList;
		for(int i = 0; i < (int)scoreList.size(); i++) scoreList[i].second = i;
		for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
		{
			int score = 0;
			for(int defId = 0; defId < PIECE_NUM; defId++)
			{
				if(!moveEnumerator.isUsed(playerId, defId))
				{
					score -= PIECE_DEF[defId].size();
				}
			}
			scoreList[playerId].first = score;
		}
		std::sort(scoreList.begin(), scoreList.end(), std::greater<std::pair<int, int>>());

		std::array<double, PLAYER_NUM> victoryNumList;
		for(int rank = 0; rank < PLAYER_NUM; rank++)
		{
			victoryNumList[scoreList[rank].second] = double(PLAYER_NUM - rank) / PLAYER_NUM;
		}

		//親ノードへ反映
		while(currentNodePtr != rootNodePtr)
		{
			currentNodePtr->victoryNum += victoryNumList[currentNodePtr->playerId];
			currentNodePtr->trialNum++;
			currentNodePtr = currentNodePtr->parentPtr.lock();
		}

		rootNodePtr->trialNum++;

		moveEnumerator.init();

		auto now = std::chrono::system_clock::now();

		if(std::chrono::duration_cast<std::chrono::milliseconds>(now - latestRefleshTime).count() > 200)
		{
			latestRefleshTime = now;
			randomWalk();

			if(auto dumper = getDumper())
			{
				dumper->dump();
			}
		}
	}
	const auto& bestChildNodePtr = bestChild(rootNodePtr, 0);
	const auto& bestMove = bestChildNodePtr->move;
	const auto& bestPieceDetail = pieceAnalyst->get(bestMove.defId, bestMove.varId);
	Piece bestPiece(bestMove.defId);
	bestPiece.rotate(bestPieceDetail.rotate);
	bestPiece.reflectX(bestPieceDetail.reflectX);

	moveCursorTo(bestMove, 50);

	//debug
	double avr = 0;
	for(std::size_t i = 0; i < rootNodePtr->childPtrList.size(); i++)
	{
		double c = 0;
		double t = rootNodePtr->trialNum;
		const auto& childPtr = rootNodePtr->childPtrList[i];
		double u = childPtr->victoryNum / childPtr->trialNum + c * std::sqrt(2.0 * std::log((double)t) / childPtr->trialNum);
		avr += u;
	}
	avr /= rootNodePtr->childPtrList.size();
	double sig = 0;
	for(std::size_t i = 0; i < rootNodePtr->childPtrList.size(); i++)
	{
		double c = 0;
		double t = rootNodePtr->trialNum;
		const auto& childPtr = rootNodePtr->childPtrList[i];
		double u = childPtr->victoryNum / childPtr->trialNum + c * std::sqrt(2.0 * std::log((double)t) / childPtr->trialNum);
		sig += (u - avr) * (u - avr);
	}
	sig /= rootNodePtr->childPtrList.size();
	sig = std::sqrt(sig);

	std::ostringstream oss;
	oss << sig << std::endl;
	for(int i = 0; i < (int)playoutNumList.size(); i++)
	{
		oss << i << " : " << playoutNumList[i] << std::endl;
	}
	if(auto dumper = getDumper())
	{
		dumper->setDebugLog(oss.str());
		dumper->dump();
	}

	return { vec2(bestMove.x, bestMove.y), bestPiece };
}

void ComputerPlayerV1::moveCursorTo(const MoveCand& move, int interval)
{
	auto latestRefleshTime = std::chrono::system_clock::now();

	cursorPiece = Piece(unusedPieceDefIdList[(selectedIndex + 1) % unusedPieceDefIdList.size()]);

	int targetPieceIndex = 0;
	while(unusedPieceDefIdList[targetPieceIndex] != move.defId) targetPieceIndex++;

	const auto& detail = pieceAnalyst->get(move.defId, move.varId);

	int toRotate = detail.rotate;
	int toReflectX = detail.reflectX;
	int toChange = 0;
	int distanceBitweenTargetAndSelected = std::abs(targetPieceIndex - selectedIndex);
	if(distanceBitweenTargetAndSelected < unusedPieceDefIdList.size() / 2.0)
	{
		toChange = targetPieceIndex - selectedIndex;
	}
	else
	{
		toChange = unusedPieceDefIdList.size() - distanceBitweenTargetAndSelected;
		if(targetPieceIndex > selectedIndex)
		{
			toChange *= -1;
		}
	}

	for(int fre = 0; fre < Field::SIZE * 2 + PIECE_NUM / 2 + 4 + 1; )
	{
		auto now = std::chrono::system_clock::now();
		if(std::chrono::duration_cast<std::chrono::milliseconds>(now - latestRefleshTime).count() > interval)
		{
			latestRefleshTime = now;

			int toMoveX = move.x - cursorX;
			int toMoveY = move.y - cursorY;

			int sumOperation = std::abs(toChange) + std::abs(toMoveX) + std::abs(toMoveY);
			if(toChange == 0)
			{
				if(toRotate == 0) sumOperation += toReflectX;
				else sumOperation += toRotate;
			}

			if(sumOperation == 0) break;

			double p_x = (double)std::abs(toMoveX) / sumOperation;
			double p_y = (double)std::abs(toMoveY) / sumOperation;
			double p_c = (double)std::abs(toChange) / sumOperation;
			double p_rot = (toChange == 0 ? (double)toRotate / sumOperation : 0);
			double p_ref = (toChange == 0 && toRotate == 0 ? (double)toReflectX / sumOperation : 0);

			double r = real_dist(mt_rand);
			if(r < p_x)
			{
				int disp = (toMoveX > 0 ? 1 : -1);
				cursorX += disp;
			}
			else if((r -= p_x) < p_y)
			{
				int disp = (toMoveY > 0 ? 1 : -1);
				cursorY += disp;
			}
			else if((r -= p_y) < p_c)
			{
				int disp = (toChange > 0 ? 1 : -1);
				selectedIndex = (selectedIndex + disp + unusedPieceDefIdList.size()) % unusedPieceDefIdList.size();
				cursorPiece = Piece(unusedPieceDefIdList[selectedIndex]);
				toChange -= disp;
			}
			else if((r -= p_c) < p_rot)
			{
				cursorPiece.rotate();
				toRotate--;
			}
			else if((r -= p_rot) < p_ref)
			{
				cursorPiece.reflectX();
				toReflectX--;
			}
			remedialSqueezeout();

			if(auto dumper = getDumper())
			{
				dumper->dump();
			}

			fre++;
		}
	}

	Piece p(move.defId);
	p.rotate(detail.rotate);
	p.reflectX(detail.reflectX);
	cursorPiece = p;
	cursorX = move.x;
	cursorY = move.y;
	if(auto dumper = getDumper())
	{
		dumper->dump();
	}
}

void ComputerPlayerV1::randomWalk()
{
	double r = real_dist(mt_rand);
	double p_move = 0.2;
	double p_piece = 0.4;
	if(r < p_move)
	{
		r = real_dist(mt_rand);
		if(r < 0.25) cursorX++;
		else if(r < 0.5) cursorX--;
		else if(r < 0.75) cursorY++;
		else cursorY--;
	}
	else if(r < p_move + p_piece)
	{
		r = real_dist(mt_rand);
		if(r < 0.2) cursorPiece.rotate();
		else if(r < 0.4) cursorPiece.reflectX();
		else if(r < 0.6) cursorPiece.reflectY();
		else
		{
			selectedIndex = (selectedIndex + 1) % unusedPieceDefIdList.size();
			cursorPiece = Piece(unusedPieceDefIdList[selectedIndex]);
		}
	}
	remedialSqueezeout();
}

void ComputerPlayerV1::remedialSqueezeout()
{
	//はみ出し是正
	int minX = std::numeric_limits<int>::max();
	int minY = std::numeric_limits<int>::max();
	int maxX = std::numeric_limits<int>::min();
	int maxY = std::numeric_limits<int>::min();
	for(auto [ dx, dy ] : cursorPiece.getCellList())
	{
		minX = std::min(minX, cursorX + dx);
		minY = std::min(minY, cursorY + dy);
		maxX = std::max(maxX, cursorX + dx);
		maxY = std::max(maxY, cursorY + dy);
	}

	cursorX += std::max(0, 0 - minX);
	cursorX -= std::max(0, maxX - (Field::SIZE - 1));
	cursorY += std::max(0, 0 - minY);
	cursorY -= std::max(0, maxY - (Field::SIZE - 1));
}

ComputerPlayerV1::ComputerPlayerV1(PlayerColor color_) :
	Player(color_), real_dist(0.0, 1.0)
{
	std::random_device rand_dev;
	mt_rand.seed(rand_dev());

	pieceAnalyst = std::make_shared<PieceAnalyst>(PIECE_DEF);
}
