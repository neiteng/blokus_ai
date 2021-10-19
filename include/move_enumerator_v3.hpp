#ifndef MOVE_ENUMERATOR_V3_HPP
#define MOVE_ENUMERATOR_V3_HPP

#include <array>
#include <vector>
#include <stack>
#include <memory>
#include "silo.hpp"
#include "piece.hpp"
#include "piece_detail.hpp"
#include "field.hpp"
#include "player.hpp"
#include "move_cand.hpp"

class MoveEnumeratorV3
{
	private:
		struct MoveInfo
		{
			MoveCand move;
			bool isValid;
			bool isAlrListed;
			bool initialIsAlrListed;
		};

		struct CellInfo
		{
			bool canSet;
			silo<bool*, PLAYER_NUM * PIECE_NUM * MAX_VAR_NUM> influence;//このセルにplayerIdのブロックが置かれた時に無効化されるべきisValidのアドレス
			bool initialCanSet;
			int initialInfluenceSize;
		};

		struct DefInfo
		{
			bool isUsed;
			silo<bool*, MAX_VAR_NUM * Field::SIZE * Field::SIZE> influence;//isUsed = trueになった時にfalseになるisValidのアドレス
			bool initialIsUsed;
			int initialInfluenceSize;
		};

		//player, x, y, def, var
		std::array<std::array<std::array<std::array<std::array<MoveInfo, MAX_VAR_NUM>, PIECE_NUM>, Field::SIZE>, Field::SIZE>, PLAYER_NUM> moveTable;
		std::array<std::array<std::array<CellInfo, Field::SIZE>, Field::SIZE>, PLAYER_NUM> cellTable;
		std::array<std::array<DefInfo, PIECE_NUM>, PLAYER_NUM> defTable;
		std::array<std::vector<MoveInfo*>, PLAYER_NUM> moveInfoList;

		silo<bool*, PLAYER_NUM * Field::SIZE * Field::SIZE * PIECE_NUM * MAX_VAR_NUM> listedList;//初期化後に登録された合法手のisAlrListedへのポインタ

		std::array<std::vector<MoveInfo*>, PLAYER_NUM> initialMoveInfoList;

		std::array<std::array<Cell, Field::SIZE>, Field::SIZE> field;
		std::array<std::array<Cell, Field::SIZE>, Field::SIZE> initialField;

		std::shared_ptr<const PieceAnalyst> pieceAnalystPtr;

		//history
		struct Change
		{
			std::vector<bool*> clearedIsValidPtrList;
			std::vector<bool*> setIsAlrListedPtrList;
			std::vector<bool*> clearedCanSetPtrList;
			std::vector<silo<bool*, PLAYER_NUM * PIECE_NUM * MAX_VAR_NUM>*> pushbackedCellInfoInfluencePtrList;
			bool* setIsUsedDefPtr;
			std::vector<silo<bool*, MAX_VAR_NUM * Field::SIZE * Field::SIZE>*> pushbackedDefInfoInfluencePtrList;
			std::array<std::vector<MoveInfo*>, PLAYER_NUM> moveInfoList;
			std::vector<bool*> setIsFilledPtrList;
			struct TerritoryLoss
			{
				int playerId;
				vec2 territoryPos;
				vec2 landlordPos;
			};
			std::vector<TerritoryLoss> territoryLossList;
			struct NecessaryLoss
			{
				int playerId;
				vec2 pos;
			};
			std::vector<NecessaryLoss> necessaryLossList;
			struct NecessaryAddition
			{
				int playerId;
				vec2 pos;
			};
			std::vector<NecessaryAddition> necessaryAdditionList;
		};

		void listMove(int playerId, int x, int y, int defId, int varId, Change& change);
		void cleanUpMoveInfoList();

		//勢力範囲関連
		static constexpr int TERRITORY_SIZE = 3;
		static constexpr int MAX_TERRITORY_AREA = 2 * TERRITORY_SIZE * (TERRITORY_SIZE - 1) + 1;
		struct NecessaryCell
		{
			bool isValid;//そもそもこのセルは必要条件セルなのか
			int index;//necessaryListのどこに存在するか
			silo<vec2, MAX_TERRITORY_AREA> territoryCellList;//このセルが抑えている範囲
		};
		std::array<std::array<std::array<NecessaryCell, Field::SIZE>, Field::SIZE>, PLAYER_NUM> necessaryTable;
		std::array<silo<vec2, Field::SIZE * Field::SIZE>, PLAYER_NUM> necessaryList;
		std::array<std::array<std::array<silo<vec2, MAX_TERRITORY_AREA>, Field::SIZE>, Field::SIZE>, PLAYER_NUM> landlordList;
		std::array<std::array<std::array<NecessaryCell, Field::SIZE>, Field::SIZE>, PLAYER_NUM> initialNecessaryTable;
		std::array<silo<vec2, Field::SIZE * Field::SIZE>, PLAYER_NUM> initialNecessaryList;
		std::array<std::array<std::array<silo<vec2, MAX_TERRITORY_AREA>, Field::SIZE>, Field::SIZE>, PLAYER_NUM> initialLandlordList;

		silo<vec2, MAX_TERRITORY_AREA> bfsTerritory(int playerId, int x, int y);
		void addNecessaryCell(int playerId, int x, int y);

		std::array<int, PLAYER_NUM> territoryArea;//勢力範囲の面積

		std::stack<Change> history;

	public:
		MoveEnumeratorV3(const Field& originalField, const ConstPlayerPtrList& playerPtrList, std::shared_ptr<const PieceAnalyst> pieceAnalystPtr_);

		int getMoveNum(int playerId) const;
		const MoveCand& getMove(int playerId, int moveId) const;
		bool isUsed(int playerId, int defId) const;

		void add(int playerId, const MoveCand& moveCand);
		bool isInitial() const;
		void undo();
		void init();

		bool isTerritory(int playerId, int x, int y) const;
		int getTerritoryArea(int playerId) const;
};

#endif
