#ifndef MOVE_ENUMERATOR_V2_HPP
#define MOVE_ENUMERATOR_V2_HPP

#include <array>
#include <vector>
#include <memory>
#include "silo.hpp"
#include "piece.hpp"
#include "piece_detail.hpp"
#include "field.hpp"
#include "player.hpp"
#include "move_cand.hpp"

class MoveEnumeratorV2
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

		void listMove(int playerId, int x, int y, int defId, int varId);
		void cleanUpMoveInfoList();

	public:
		MoveEnumeratorV2(const Field& originalField, const ConstPlayerPtrList& playerPtrList, std::shared_ptr<const PieceAnalyst> pieceAnalystPtr_);

		int getMoveNum(int playerId) const;
		const MoveCand& getMove(int playerId, int moveId) const;
		bool isUsed(int playerId, int defId) const;

		void add(int playerId, const MoveCand& moveCand);
		void init();
};

#endif
