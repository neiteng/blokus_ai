#include "move_enumerator_v3.hpp"
#include <ncurses.h>
#include <iostream>
#include <chrono>
#include <queue>
#include <sstream>

MoveEnumeratorV3::MoveEnumeratorV3(const Field& originalField, const ConstPlayerPtrList& playerPtrList, std::shared_ptr<const PieceAnalyst> pieceAnalystPtr_)
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
						Change change;
						listMove(playerId, x, y, defId, varId, change);
						//この手はもう登録された
						moveTable[playerId][x][y][defId][varId].isAlrListed = true;
					}
				}
			}
		}
	}

	territoryArea.fill(0);

	//勢力範囲を記録
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		for(int x = 0; x < Field::SIZE; x++)
		{
			for(int y = 0; y < Field::SIZE; y++)
			{
				necessaryTable[playerId][x][y].isValid = false;
			}
		}
	}
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		for(int x = 0; x < Field::SIZE; x++)
		{
			for(int y = 0; y < Field::SIZE; y++)
			{
				if(!cellTable[playerId][x][y].canSet) continue;
				bool isNec = false;
				int dx = 1;
				int dy = 1;
				for(int i = 0; i < 4; i++)
				{
					int nx = x + dx;
					int ny = y + dy;
					if(nx >= 0 && nx < Field::SIZE && ny >= 0 && ny < Field::SIZE && field[nx][ny] && field[nx][ny].playerId == playerId)
					{
						isNec = true;
						break;
					}

					int temp = dx;
					dx = -dy;
					dy = temp;
				}
				if(!isNec) continue;

				//必要条件セルとして登録し、勢力範囲を計算
				addNecessaryCell(playerId, x, y);
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
	initialNecessaryTable = necessaryTable;
	initialNecessaryList = necessaryList;
	initialLandlordList = landlordList;
}

void MoveEnumeratorV3::add(int playerId, const MoveCand& moveCand)
{
	history.push(Change());
	auto& change = history.top();
	change.clearedIsValidPtrList.reserve(4096);
	change.setIsAlrListedPtrList.reserve(4096);
	change.clearedCanSetPtrList.reserve(256);
	change.pushbackedCellInfoInfluencePtrList.reserve(4096);
	change.pushbackedDefInfoInfluencePtrList.reserve(1024);

	change.moveInfoList = moveInfoList;

	//auto t0 = std::chrono::system_clock::now();
	//使われたピースを記録
	defTable[playerId][moveCand.defId].isUsed = true;
	change.setIsUsedDefPtr = &defTable[playerId][moveCand.defId].isUsed;
	//同じピースが使われている合法手を全て無効化
	for(int i = 0; i < (int)defTable[playerId][moveCand.defId].influence.elem_num(); i++)
	{
		auto& isValid = *defTable[playerId][moveCand.defId].influence[i];
		if(isValid)
		{
			isValid = false;
			change.clearedIsValidPtrList.push_back(&isValid);
		}
	}

	const auto& detail = pieceAnalystPtr->get(moveCand.defId, moveCand.varId);

	std::array<std::vector<vec2>, PLAYER_NUM> killedCellLandlord;//canSetがfalseにされたセルの地主セルの集合
	std::array<std::array<std::array<bool, Field::SIZE>, Field::SIZE>, PLAYER_NUM> isAddedToKilledCellLandlord = {};
	for(const auto& pos : detail.cellList)
	{
		//影響テーブルに登録された合法手を無効化
		int x = moveCand.x + pos.x;
		int y = moveCand.y + pos.y;
		for(int i = 0; i < (int)cellTable[playerId][x][y].influence.elem_num(); i++)
		{
			auto& isValid = *cellTable[playerId][x][y].influence[i];
			if(isValid)
			{
				isValid = false;
				change.clearedIsValidPtrList.push_back(&isValid);
			}
		}

		for(int i = 0; i < PLAYER_NUM; i++)
		{
			//置けるかどうかのテーブルを更新
			auto& canSet = cellTable[i][x][y].canSet;
			if(canSet)
			{
				canSet = false;
				change.clearedCanSetPtrList.push_back(&canSet);

				for(int j = 0; j < (int)landlordList[i][x][y].elem_num(); j++)
				{
					const auto& landlordPos = landlordList[i][x][y][j];
					if(!isAddedToKilledCellLandlord[i][landlordPos.x][landlordPos.y])
					{
						killedCellLandlord[i].push_back(landlordPos);
						isAddedToKilledCellLandlord[i][landlordPos.x][landlordPos.y] = true;
					}
				}
			}
		}
	}
	for(const auto& pos : detail.neighborList)
	{
		int x = moveCand.x + pos.x;
		int y = moveCand.y + pos.y;
		if(x < 0 || x >= Field::SIZE || y < 0 || y >= Field::SIZE) continue;
		auto& canSet = cellTable[playerId][x][y].canSet;
		if(canSet)
		{
			canSet = false;
			change.clearedCanSetPtrList.push_back(&canSet);

			for(int j = 0; j < (int)landlordList[playerId][x][y].elem_num(); j++)
			{
				const auto& landlordPos = landlordList[playerId][x][y][j];
				if(!isAddedToKilledCellLandlord[playerId][landlordPos.x][landlordPos.y])
				{
					killedCellLandlord[playerId].push_back(landlordPos);
					isAddedToKilledCellLandlord[playerId][landlordPos.x][landlordPos.y] = true;
				}
			}
		}
	}
	//std::ostringstream oss;
	{
		std::array<std::array<bool, Field::SIZE>, Field::SIZE> territoryTable;
		for(int pid = 0; pid < PLAYER_NUM; pid++)
		{
			for(const auto& pos : killedCellLandlord[pid])
			{
				//この必要条件セルは勢力範囲を再計算する
				const auto& territoryList = bfsTerritory(pid, pos.x, pos.y);

				for(auto& line : territoryTable) line.fill(false);
				for(int i = 0; i < (int)territoryList.elem_num(); i++)
				{
					const auto& terPos = territoryList[i];
					territoryTable[terPos.x][terPos.y] = true;
				}
				for(int i = 0; i < (int)necessaryTable[pid][pos.x][pos.y].territoryCellList.elem_num(); i++)
				{
					const auto& oldPos = necessaryTable[pid][pos.x][pos.y].territoryCellList[i];
					if(!territoryTable[oldPos.x][oldPos.y])
					{
						//新しいテリトリーには含まれず前のテリトリーには含まれるから削除
						auto& territoryCell = landlordList[pid][oldPos.x][oldPos.y];
						int index = 0;
						for(index = 0; index < (int)territoryCell.elem_num(); index++)
						{
							const auto& landlord = territoryCell[index];
							if(landlord.x == pos.x && landlord.y == pos.y) break;
						}
						//if(index == territoryCell.elem_num()) continue;
						//indexのやつを消す
						const auto& back = territoryCell[territoryCell.elem_num() - 1];
						territoryCell[index] = back;
						territoryCell.pop_back();
						if(territoryCell.empty()) territoryArea[pid]--;

						change.territoryLossList.push_back( { pid, oldPos, pos } );
					}
				}
				necessaryTable[pid][pos.x][pos.y].territoryCellList = territoryList;
				//勢力範囲がなくなったらもう潰されたということ
				if(territoryList.empty())
				{
					const auto& back = necessaryList[pid][necessaryList[pid].elem_num() - 1];
					necessaryList[pid][necessaryTable[pid][pos.x][pos.y].index] = back;
					necessaryTable[pid][back.x][back.y].index = necessaryTable[pid][pos.x][pos.y].index;
					necessaryTable[pid][pos.x][pos.y].isValid = false;
					necessaryList[pid].pop_back();

					change.necessaryLossList.push_back( { pid, pos } );
				}
			}
		}
	}

	//必要条件セルの追加
	for(const auto& nec : detail.necessaryCellList)
	{
		int x = moveCand.x + nec.pos.x;
		int y = moveCand.y + nec.pos.y;
		if(x < 0 || x >= Field::SIZE || y < 0 || y >= Field::SIZE || !cellTable[playerId][x][y].canSet || necessaryTable[playerId][x][y].isValid) continue;

		addNecessaryCell(playerId, x, y);
		change.necessaryAdditionList.push_back( { playerId, vec2(x, y) } );
	}

	for(int defId = 0; defId < PIECE_NUM; defId++)
	{
		if(defTable[playerId][defId].isUsed) continue;
		int varNum = pieceAnalystPtr->variationNum(defId);
		for(const auto& necDetail : detail.necessaryCellList)
		{
			//必要条件セルの座標
			int nx = moveCand.x + necDetail.pos.x;
			int ny = moveCand.y + necDetail.pos.y;
			if(nx < 0 || nx >= Field::SIZE || ny < 0 || ny >= Field::SIZE || !cellTable[playerId][nx][ny].canSet) continue;

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
					auto& isAlrListed = moveTable[playerId][px][py][defId][varId].isAlrListed;
					if(isAlrListed) continue;

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
					listMove(playerId, px, py, defId, varId, change);
					isAlrListed = true;
					change.setIsAlrListedPtrList.push_back(&isAlrListed);
					listedList.push_back(&isAlrListed);
				}
			}
		}
	}

	cleanUpMoveInfoList();

	//fieldをアップデート
	for(const auto& pos : detail.cellList)
	{
		int x = moveCand.x + pos.x;
		int y = moveCand.y + pos.y;
		field[x][y].isFilled = true;
		field[x][y].playerId = playerId;
		change.setIsFilledPtrList.push_back(&field[x][y].isFilled);
	}
}

void MoveEnumeratorV3::listMove(int playerId, int x, int y, int defId, int varId, Change& change)
{
	auto& moveInfo = moveTable[playerId][x][y][defId][varId];
	moveInfo.isValid = true;

	moveInfoList[playerId].push_back(&moveInfo);

	bool* isValidPtr = &moveInfo.isValid;
	defTable[playerId][defId].influence.push_back(isValidPtr);
	change.pushbackedDefInfoInfluencePtrList.push_back(&defTable[playerId][defId].influence);

	const auto& detail = pieceAnalystPtr->get(defId, varId);

	//他プレイヤーがセットした場合の影響テーブルに登録
	for(const auto& pos : detail.cellList)
	{
		int cx = x + pos.x;
		int cy = y + pos.y;
		for(int i = 0; i < PLAYER_NUM; i++)
		{
			if(i == playerId) continue;
			cellTable[i][cx][cy].influence.push_back(isValidPtr);
			change.pushbackedCellInfoInfluencePtrList.push_back(&cellTable[i][cx][cy].influence);
		}
	}
	//自プレイヤーがセットした場合の影響テーブルに登録
	for(const auto& pos : detail.neighborList)
	{
		int cx = x + pos.x;
		int cy = y + pos.y;
		if(cx < 0 || cx >= Field::SIZE || cy < 0 || cy >= Field::SIZE) continue;
		cellTable[playerId][cx][cy].influence.push_back(isValidPtr);
		change.pushbackedCellInfoInfluencePtrList.push_back(&cellTable[playerId][cx][cy].influence);
	}
}

void MoveEnumeratorV3::addNecessaryCell(int playerId, int x, int y)
{
	if(necessaryTable[playerId][x][y].isValid) return;
	necessaryList[playerId].push_back(vec2(x, y));
	necessaryTable[playerId][x][y].isValid = true;
	necessaryTable[playerId][x][y].index = necessaryList[playerId].elem_num() - 1;
	const auto& territory = bfsTerritory(playerId, x, y);
	necessaryTable[playerId][x][y].territoryCellList = territory;

	for(int i = 0; i < (int)territory.elem_num(); i++)
	{
		const auto& pos = territory[i];
		if(landlordList[playerId][pos.x][pos.y].empty()) territoryArea[playerId]++;
		landlordList[playerId][pos.x][pos.y].push_back(vec2(x, y));
	}
}

//現在のcanSetでの勢力範囲を計算
silo<vec2, MoveEnumeratorV3::MAX_TERRITORY_AREA> MoveEnumeratorV3::bfsTerritory(int playerId, int x, int y)
{
	silo<vec2, MAX_TERRITORY_AREA> ret;
	if(!cellTable[playerId][x][y].canSet) return ret;
	ret.push_back(vec2(x, y));

	std::array<std::array<bool, Field::SIZE>, Field::SIZE> isAdded = {};
	isAdded[x][y] = true;

	int begin = 0;
	for(int depth = 0; depth < TERRITORY_SIZE - 1; depth++)
	{
		int end = ret.elem_num();
		for(int index = begin; index < end; index++)
		{
			const auto& center = ret[index];

			int dx = 1;
			int dy = 0;
			for(int i = 0; i < 4; i++)
			{
				int nx = center.x + dx;
				int ny = center.y + dy;
				if(nx >= 0 && nx < Field::SIZE && ny >= 0 && ny < Field::SIZE && cellTable[playerId][nx][ny].canSet && !isAdded[nx][ny])
				{
					ret.push_back(vec2(nx, ny));
					isAdded[nx][ny] = true;
				}

				int temp = dx;
				dx = -dy;
				dy = temp;
			}
		}
		begin = end;
	}
	return ret;
}


void MoveEnumeratorV3::cleanUpMoveInfoList()
{
	for(int playerId = 0; playerId < PLAYER_NUM; playerId++)
	{
		//isValid == trueなMoveCandを前に詰める
		auto bound = std::partition(moveInfoList[playerId].begin(), moveInfoList[playerId].end(), [](MoveInfo* m) { return m->isValid; });

		//削除
		moveInfoList[playerId].erase(bound, moveInfoList[playerId].end());
	}
}

int MoveEnumeratorV3::getMoveNum(int playerId) const
{
	return moveInfoList[playerId].size();
}

const MoveCand& MoveEnumeratorV3::getMove(int playerId, int moveId) const
{
	return moveInfoList[playerId][moveId]->move;
}

bool MoveEnumeratorV3::isUsed(int playerId, int defId) const
{
	return defTable[playerId][defId].isUsed;
}

bool MoveEnumeratorV3::isInitial() const
{
	return history.empty();
}

void MoveEnumeratorV3::undo()
{
	auto& change = history.top();
	for(bool* ptr : change.clearedIsValidPtrList) *ptr = true;
	change.clearedIsValidPtrList.clear();
	for(bool* ptr : change.setIsAlrListedPtrList) *ptr = false;
	change.setIsAlrListedPtrList.clear();
	for(bool* ptr : change.clearedCanSetPtrList) *ptr = true;
	change.clearedCanSetPtrList.clear();
	for(auto ptr : change.pushbackedCellInfoInfluencePtrList) ptr->pop_back();
	change.pushbackedCellInfoInfluencePtrList.clear();
	*(change.setIsUsedDefPtr) = false;
	for(auto ptr : change.pushbackedDefInfoInfluencePtrList) ptr->pop_back();
	change.pushbackedDefInfoInfluencePtrList.clear();
	moveInfoList = change.moveInfoList;
	for(bool* ptr : change.setIsFilledPtrList) *ptr = false;
	change.setIsFilledPtrList.clear();
	for(const auto& loss : change.territoryLossList)
	{
		landlordList[loss.playerId][loss.territoryPos.x][loss.territoryPos.y].push_back(loss.landlordPos);
		necessaryTable[loss.playerId][loss.landlordPos.x][loss.landlordPos.y].territoryCellList.push_back(loss.territoryPos);
	}
	change.territoryLossList.clear();
	for(const auto& loss : change.necessaryLossList)
	{
		necessaryList[loss.playerId].push_back(loss.pos);
		necessaryTable[loss.playerId][loss.pos.x][loss.pos.y].isValid = true;
		necessaryTable[loss.playerId][loss.pos.x][loss.pos.y].index = necessaryList[loss.playerId].elem_num() - 1;
	}
	for(const auto& add : change.necessaryAdditionList)
	{
		const auto& territory = bfsTerritory(add.playerId, add.pos.x, add.pos.y);
		for(int i = 0; i < territory.elem_num(); i++)
		{
			const auto& pos = territory[i];
			auto& landlord = landlordList[add.playerId][pos.x][pos.y];
			for(int j = 0; j < landlord.elem_num(); j++)
			{
				if(landlord[j].x == pos.x && landlord[j].y == pos.y)
				{
					landlord[j] = landlord[landlord.elem_num() - 1];
					landlord.pop_back();
					break;
				}
			}
		}
	}
	history.pop();
}

void MoveEnumeratorV3::init()
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
	necessaryTable = initialNecessaryTable;
	necessaryList = initialNecessaryList;
	landlordList = initialLandlordList;
	while(!history.empty()) history.pop();
}

bool MoveEnumeratorV3::isTerritory(int playerId, int x, int y) const
{
	return !landlordList[playerId][x][y].empty();
}

int MoveEnumeratorV3::getTerritoryArea(int playerId) const
{
	return territoryArea[playerId];
}
