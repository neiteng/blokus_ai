#include "move_enumerator_v1.hpp"
#include <iostream>
#include <chrono>

MoveEnumeratorV1::MoveEnumeratorV1(const Field& originalField, const ConstPlayerPtrList& playerPtrList, std::shared_ptr<const PieceAnalyst> pieceAnalystPtr_)
	: pieceAnalystPtr(pieceAnalystPtr_)
{
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		moveInfoList[playerId].reserve(2048);
	}

	//盤面をコピー
	for(int y = 0; y < Field::SIZE; y++)
	{
		for(int x = 0; x < Field::SIZE; x++)
		{
			field[x][y] = originalField.get(x, y);
		}
	}

	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		for(int x = 0; x < Field::SIZE; x++)
		{
			for(int y = 0; y < Field::SIZE; y++)
			{
				for(int defId = 0; defId < PIECE_NUM; defId++)
				{
					for(int varId = 0; varId < MAX_VAR_NUM; varId++)
					{
						moveTable[playerId][x][y][defId][varId] = { { x, y, defId, varId }, false, false, false };
					}
				}
			}
		}
	}
	//どのピースが使われたかの表を初期化
	for(int i = 0; i < PLAYER_NUM; i++)
	{
		for(int j = 0; j < PIECE_NUM; j++) defTable[i][j].isUsed = true;
		for(int id : playerPtrList[i]->getUnusedPieceDefIdList()) defTable[i][id].isUsed = false;
	}

	//canSetを初期化
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		for(int x = 0; x < Field::SIZE; x++)
		{
			for(int y = 0; y < Field::SIZE; y++)
			{
				cellTable[playerId][x][y].canSet = true;
			}
		}
	}
	//置けないセルを潰す
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		for(int x = 0; x < Field::SIZE; x++)
		{
			for(int y = 0; y < Field::SIZE; y++)
			{
				const auto& cell = field[x][y];
				if(!cell) continue;

				cellTable[playerId][x][y].canSet = false;
				if(cell.playerId == playerId)
				{
					//近傍セルを潰す
					int dx = 1;
					int dy = 0;
					for(int i = 0; i < 4; i++)
					{
						int nx = x + dx;
						int ny = y + dy;
						if(nx >= 0 && nx < Field::SIZE && ny >= 0 && ny < Field::SIZE)
						{
							cellTable[playerId][nx][ny].canSet = false;
						}

						int temp = dx;
						dx = -dy;
						dy = temp;
					}
				}
			}
		}
	}

	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		for(int defId = 0; defId < PIECE_NUM; defId++)
		{
			if(defTable[playerId][defId].isUsed) continue;

			int varNum = pieceAnalystPtr->variationNum(defId);
			for(int varId = 0; varId < varNum; varId++)
			{
				for(int x = 0; x < Field::SIZE; x++)
				{
					for(int y = 0; y < Field::SIZE; y++)
					{
						//この手はすでにブロックが置いてあるところと被っていたり、自分と同じ色のブロックと辺で接したりしていないか？
						bool canSet = true;
						const auto& detail = pieceAnalystPtr->get(defId, varId);
						for(const auto& pos : detail.cellList)
						{
							int cx = x + pos.x;
							int cy = y + pos.y;
							if(cx < 0 || cx >= Field::SIZE || cy < 0 || cy >= Field::SIZE || !cellTable[playerId][cx][cy].canSet)
							{
								canSet = false;
								break;
							}
						}
						if(!canSet) continue;

						//この手は同じ色のブロックと頂点で接しているか？
						canSet = false;
						for(const auto& necDetail : detail.necessaryCellList)
						{
							const auto& pos = necDetail.pos;
							int cx = x + pos.x;
							int cy = y + pos.y;
							if(((cx == -1 || cx == Field::SIZE) && (cy == -1 || cy == Field::SIZE)) || (cx >= 0 && cx < Field::SIZE && cy >= 0 && cy < Field::SIZE && field[cx][cy] && field[cx][cy].playerId == playerId))
							{
								canSet = true;
								break;
							}
						}
						if(!canSet) continue;

						//この手は合法なので登録する
						listMove(playerId, x, y, defId, varId);
						//この手はもう登録された
						moveTable[playerId][x][y][defId][varId].isAlrListed = true;
					}
				}
			}
		}
	}

	//初期状態を記録
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		for(int x = 0; x < Field::SIZE; x++)
		{
			for(int y = 0; y < Field::SIZE; y++)
			{
				for(int defId = 0; defId < PIECE_NUM; defId++)
				{
					for(int varId = 0; varId < MAX_VAR_NUM; varId++)
					{
						moveTable[playerId][x][y][defId][varId].initialIsAlrListed = moveTable[playerId][x][y][defId][varId].isAlrListed;
					}
				}
				cellTable[playerId][x][y].initialCanSet = cellTable[playerId][x][y].canSet;
				cellTable[playerId][x][y].initialInfluenceSize = cellTable[playerId][x][y].influence.elem_num();
			}
		}
		for(int defId = 0; defId < PIECE_NUM; defId++)
		{
			defTable[playerId][defId].initialIsUsed = defTable[playerId][defId].isUsed;
			defTable[playerId][defId].initialInfluenceSize = defTable[playerId][defId].influence.elem_num();
		}
	}
	initialMoveInfoList = moveInfoList;
	initialField = field;
}

void MoveEnumeratorV1::add(int playerId, const MoveCand& moveCand)
{
	//auto t0 = std::chrono::system_clock::now();
	//使われたピースを記録
	defTable[playerId][moveCand.defId].isUsed = true;
	//同じピースが使われている合法手を全て無効化
	for(int i = 0; i < (int)defTable[playerId][moveCand.defId].influence.elem_num(); i++)
	{
		*defTable[playerId][moveCand.defId].influence[i] = false;
	}

	const auto& detail = pieceAnalystPtr->get(moveCand.defId, moveCand.varId);

	for(const auto& pos : detail.cellList)
	{
		//影響テーブルに登録された合法手を無効化
		int x = moveCand.x + pos.x;
		int y = moveCand.y + pos.y;
		for(int i = 0; i < (int)cellTable[playerId][x][y].influence.elem_num(); i++)
		{
			*cellTable[playerId][x][y].influence[i] = false;
		}

		//置けるかどうかのテーブルを更新
		for(int i = 0; i < PLAYER_NUM; i++)
		{
			cellTable[i][x][y].canSet = false;
		}
	}
	for(const auto& pos : detail.neighborList)
	{
		int x = moveCand.x + pos.x;
		int y = moveCand.y + pos.y;
		if(x < 0 || x >= Field::SIZE || y < 0 || y >= Field::SIZE) continue;
		cellTable[playerId][x][y].canSet = false;
	}

	//auto t1 = std::chrono::system_clock::now();
	for(int defId = 0; defId < PIECE_NUM; defId++)
	{
		if(defTable[playerId][defId].isUsed) continue;
		for(const auto& necDetail : detail.necessaryCellList)
		{
			//必要条件セルの座標
			int nx = moveCand.x + necDetail.pos.x;
			int ny = moveCand.y + necDetail.pos.y;
			if(nx < 0 || nx >= Field::SIZE || ny < 0 || ny >= Field::SIZE || !cellTable[playerId][nx][ny].canSet) continue;

			int varNum = pieceAnalystPtr->variationNum(defId);
			for(int varId = 0; varId < varNum; varId++)
			{
				const auto& candDetail = pieceAnalystPtr->get(defId, varId);
				for(const auto& edgeDetail : candDetail.edgeList)
				{
					if((~necDetail.allowNeighbor & edgeDetail.hasNeighbor).any()) continue;
					//ピースの中心座標
					int px = nx - edgeDetail.pos.x;
					int py = ny - edgeDetail.pos.y;
					if(px < 0 || px >= Field::SIZE || py < 0 || py >= Field::SIZE) continue;
					//すでにリストに上がっていたらスルーする
					if(moveTable[playerId][px][py][defId][varId].isAlrListed) continue;

					bool canSet = true;
					for(const auto& pos : candDetail.cellList)
					{
						int x = px + pos.x;
						int y = py + pos.y;
						if(x < 0 || x >= Field::SIZE || y < 0 || y >= Field::SIZE || !cellTable[playerId][x][y].canSet)
						{
							canSet = false;
							break;
						}
					}
					if(!canSet) continue;
					listMove(playerId, px, py, defId, varId);
					moveTable[playerId][px][py][defId][varId].isAlrListed = true;
					listedList.push_back(&moveTable[playerId][px][py][defId][varId].isAlrListed);
				}
			}
		}
	}
	//auto t2 = std::chrono::system_clock::now();

	cleanUpMoveInfoList();
	//auto t3 = std::chrono::system_clock::now();
	/*
	std::cout << "t1 - t0 : " << std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() << std::endl;
	std::cout << "t2 - t1 : " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << std::endl;
	std::cout << "t3 - t2 : " << std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count() << std::endl;
	*/

	//fieldをアップデート
	for(const auto& pos : detail.cellList)
	{
		int x = moveCand.x + pos.x;
		int y = moveCand.y + pos.y;
		field[x][y].isFilled = true;
		field[x][y].playerId = playerId;
	}
}

void MoveEnumeratorV1::listMove(int playerId, int x, int y, int defId, int varId)
{
	auto& moveInfo = moveTable[playerId][x][y][defId][varId];
	//この手は合法手であると記録
	moveInfo.isValid = true;

	moveInfoList[playerId].push_back(&moveInfo);

	bool* isValidPtr = &moveInfo.isValid;
	defTable[playerId][defId].influence.push_back(isValidPtr);

	const auto& detail = pieceAnalystPtr->get(defId, varId);

	//他プレイヤーがセットした場合の影響テーブルに登録
	for(const auto& pos : detail.cellList)
	{
		int cx = x + pos.x;
		int cy = y + pos.y;
		if(cx < 0 || cx >= Field::SIZE || cy < 0 || cy >= Field::SIZE) continue;
		for(int i = 0; i < PLAYER_NUM; i++)
		{
			if(i == playerId) continue;
			cellTable[i][cx][cy].influence.push_back(isValidPtr);
		}
	}
	//自プレイヤーがセットした場合の影響テーブルに登録
	for(const auto& pos : detail.neighborList)
	{
		int cx = x + pos.x;
		int cy = y + pos.y;
		if(cx < 0 || cx >= Field::SIZE || cy < 0 || cy >= Field::SIZE) continue;
		cellTable[playerId][cx][cy].influence.push_back(isValidPtr);
	}
}

void MoveEnumeratorV1::cleanUpMoveInfoList()
{
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		//isValid == trueなMoveCandを前に詰める
		auto bound = std::partition(moveInfoList[playerId].begin(), moveInfoList[playerId].end(), [](MoveInfo* m) { return m->isValid; });

		//削除
		moveInfoList[playerId].erase(bound, moveInfoList[playerId].end());
	}
}

int MoveEnumeratorV1::getMoveNum(int playerId) const
{
	return moveInfoList[playerId].size();
}

const MoveCand& MoveEnumeratorV1::getMove(int playerId, int moveId) const
{
	return moveInfoList[playerId][moveId]->move;
}

bool MoveEnumeratorV1::isUsed(int playerId, int defId) const
{
	return defTable[playerId][defId].isUsed;
}

void MoveEnumeratorV1::init()
{
	//初期状態を復元
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		for(int x = 0; x < Field::SIZE; x++)
		{
			for(int y = 0; y < Field::SIZE; y++)
			{
				cellTable[playerId][x][y].canSet = cellTable[playerId][x][y].initialCanSet;
				cellTable[playerId][x][y].influence.resize(cellTable[playerId][x][y].initialInfluenceSize);
			}
		}
		for(int defId = 0; defId < PIECE_NUM; defId++)
		{
			defTable[playerId][defId].isUsed = defTable[playerId][defId].initialIsUsed;
			defTable[playerId][defId].influence.resize(defTable[playerId][defId].initialInfluenceSize);
		}
	}
	for(int i = 0; i < (int)listedList.elem_num(); i++)
	{
		*listedList[i] = false;
	}
	listedList.resize(0);
	moveInfoList = initialMoveInfoList;
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		for(auto moveInfoPtr : moveInfoList[playerId])
		{
			moveInfoPtr->isValid = true;
		}
	}
	field = initialField;
}
