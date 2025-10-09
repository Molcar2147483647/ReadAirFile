#ifndef INCLUDEGUARD_READAIRFILE_HEADER
#define INCLUDEGUARD_READAIRFILE_HEADER

#include <iostream> // 標準入力/出力
#include <fstream> // ファイル読み取り
#include <filesystem> // ファイル検索
#include <regex> // 正規表現
#include <vector> // 可変長配列
#include <unordered_map> // ハッシュ的なやつ

namespace ReadAirFile {
	// private:
	namespace __detail {
		struct T_AnimData {
		public:
			struct T_AnimElem {
			private:
				int Sprite = 0; // SpriteID(32767), SpriteNumber(32767), Facing, VFacing
				int ElemTime = 0;
				int Alpha = 0; // AlphaA(255), AlphaS(255), AlphaD(255)
				double Pos[2] = {};

				static constexpr int SpriteBit = 15;
				static constexpr int SpriteIDMask = (1 << SpriteBit) - 1;
				static constexpr int SpriteNumberMask = (1 << SpriteBit) * SpriteIDMask;
				static constexpr int FacingMask = (1 << (SpriteBit * 2));
				static constexpr int VFacingMask = (1 << (SpriteBit * 2 + 1));
				static constexpr int AlphaBit = 8;
				static constexpr int AlphaAMask = (1 << AlphaBit) - 1;
				static constexpr int AlphaSMask = (1 << AlphaBit) * AlphaAMask;
				static constexpr int AlphaDMask = (1 << AlphaBit) * AlphaSMask;
			public:
				int SpriteID() const { return (Sprite & SpriteIDMask); }
				int SpriteNumber() const { return (Sprite & SpriteNumberMask) >> SpriteBit; }
				bool Facing() const { return (Sprite & FacingMask) == FacingMask; }
				bool VFacing() const { return (Sprite & VFacingMask) == VFacingMask; }
				int Time() const { return ElemTime; }
				int AlphaA() const { return (Alpha & AlphaAMask); }
				int AlphaS() const { return (Alpha & AlphaSMask) >> AlphaBit; }
				int AlphaD() const { return (Alpha & AlphaDMask) >> (AlphaBit * 2); }
				double PosX() const { return Pos[0]; }
				double PosY() const { return Pos[1]; }

				void SpriteID(int value) { Sprite = (Sprite & ~SpriteIDMask) | (value & SpriteIDMask); }
				void SpriteNumber(int value) { Sprite = (Sprite & ~SpriteNumberMask) | ((value << SpriteBit) & SpriteNumberMask); }
				void Facing(bool value) { Sprite = (Sprite & ~FacingMask) | (value ? FacingMask : 0); }
				void VFacing(bool value) { Sprite = (Sprite & ~VFacingMask) | (value ? VFacingMask : 0); }
				void Time(int value) { ElemTime = value; }
				void AlphaA(int value) { Alpha = (Alpha & ~AlphaAMask) | (value & AlphaAMask); }
				void AlphaS(int value) { Alpha = (Alpha & ~AlphaSMask) | ((value << AlphaBit) & AlphaSMask); }
				void AlphaD(int value) { Alpha = (Alpha & ~AlphaDMask) | ((value << AlphaBit * 2) & AlphaDMask); }
				void PosX(double value) { Pos[0] = value; }
				void PosY(double value) { Pos[1] = value; }
			};
		private:
			int AnimID = 0;
			int Loopstart = 0; // ループ始点(2147483647), ループ存在(-2147483648)
			std::vector<T_AnimElem> AnimElem = {};
		public:
			int ID() const { return AnimID; }
			bool Loop() const { return (Loopstart < 0); }
			int LoopElem() const { return (Loopstart & 0x7FFFFFFF); }
			auto& Elem() { return AnimElem; }
			auto& Elem(int value) { return AnimElem[value]; }

			void ID(int value) { AnimID = (value & 0x7FFFFFFF); }
			void Loop(int value) { Loopstart = (value >= 0 ? (value | 0x80000000) : 0); }
		};
		inline std::vector<T_AnimData> AnimData = {};
		inline std::unordered_map<int, int> AnimDataMap;

		inline auto& AnimRef() { return AnimData; }
		inline auto& AnimRef(int value) { return AnimData[value]; }
		inline const auto& ConvertAnimData() { return AnimDataMap; }

		inline void RegisterAnimDataIndex(int value, int value2) { AnimDataMap[value] = value2; }

		inline int GetAnimDataIndex(const int input) {
			auto it = ConvertAnimData().find(input);
			if (it != ConvertAnimData().end()) { return it->second; }
			return -1;
		}

		// Target[0] == '-' で負数判定
		inline bool IsUInt8ValueInRange(const std::string& Target) {
			return (Target[0] == '-' ? false : (Target.size() < 3 || Target.size() == 3 && Target <= "255"));
		}
		inline bool IsUInt15ValueInRange(const std::string& Target) {
			return (Target[0] == '-' ? false : (Target.size() < 5 || Target.size() == 5 && Target <= "32767"));
		}
		inline bool IsUInt31ValueInRange(const std::string& Target) {
			return (Target[0] == '-' ? false : (Target.size() < 10 || Target.size() == 10 && Target <= "2147483647"));
		}
		inline bool IsInt32ValueInRange(const std::string& Target) {
			if (Target[0] == '-') {
				return (Target.size() < 11 || Target.size() == 11 && Target <= "-2147483648");
			}
			else {
				return (Target.size() < 10 || Target.size() == 10 && Target <= "2147483647");
			}
		}

		struct T_AnimParamLogHandle {
		private:
			struct T_ErrorHandle {
			private:
				const int LineNumber = 0;
				const int AnimID = 0;
				const int AnimElem = 0;
				const std::string ParamName;
				const std::string ParamValue;
				const std::string Message;
			public:
				int Line() const { return LineNumber; }
				int Anim() const { return AnimID; }
				int Elem() const { return AnimElem; }
				const auto& Name() const { return ParamName; }
				const auto& Value() const { return ParamValue; }
				const auto& Msg() const { return Message; }

				T_ErrorHandle(int line, int anim, int elem, const std::string& name, const std::string& param, const std::string& msg)
					: LineNumber(line), AnimID(anim), AnimElem(elem), ParamName(name), ParamValue(param), Message(msg) {}
			};
			std::vector<T_ErrorHandle> ErrorHandle = {};
		public:
			auto& Error() { return ErrorHandle; }
			auto& Error(int value) { return ErrorHandle[value]; }

			void SetError(int line, int anim, int elem, const std::string& name, const std::string& param, const std::string& msg) {
				Error().push_back(T_ErrorHandle(line, anim, elem, name, param, msg));
			}

			void PrintLog() {
				std::cout << "\nエラー数: " << Error().size() << std::endl;
				if (!Error().size()) {
					std::cout << "エラーは確認できませんでした" << std::endl;
				}
				for (int i = 0; i < Error().size(); ++i) {
					std::cout << "\nエラー箇所: " << Error(i).Line() << "行目" << std::endl;
					std::cout << "アニメ番号: " << (Error(i).Anim() < 0 ? "-" : std::to_string(Error(i).Anim())) << std::endl;
					std::cout << "アニメ位置番号: " << (Error(i).Elem() < 0 ? "-" : std::to_string(Error(i).Elem())) << std::endl;
					std::cout << "該当パラメータ: " << Error(i).Name() << std::endl;
					std::cout << "エラー値: " << Error(i).Value() << std::endl;
					std::cout << "内容: " << Error(i).Msg() << std::endl;
				}
			}
		};
		inline T_AnimParamLogHandle AnimParamLogHandle = {};

		inline auto& AnimParamLog() { return AnimParamLogHandle; }

		namespace ErrorMessage {
			constexpr const char* RangeBeginAction = "アニメーション番号が登録可能な数値の範囲(0～2147483647)を超えています";
			constexpr const char* DuplicationBeginAction = "アニメーションが重複して登録されています";
			constexpr const char* EmptyAnimElem = "アニメーション内容が登録されていません";
			constexpr const char* RangeSpriteID = "SpriteIDが登録可能な数値の範囲(0～32767)を超えています";
			constexpr const char* RangeSpriteNumber = "SpriteNumberが登録可能な数値の範囲(0～32767)を超えています";
			constexpr const char* RangePosX = "X座標が登録可能な数値の範囲(-2147483648～2147483647)を超えています";
			constexpr const char* RangePosY = "Y座標が登録可能な数値の範囲(-2147483648～2147483647)を超えています";
			constexpr const char* RangeTime = "アニメタイマーが登録可能な数値の範囲(-2147483648～2147483647)を超えています";
			constexpr const char* RangeAlphaA = "AlphaAが登録可能な数値の範囲(0～255)を超えています";
			constexpr const char* RangeAlphaS = "AlphaSが登録可能な数値の範囲(0～255)を超えています";
			constexpr const char* RangeAlphaD = "AlphaDが登録可能な数値の範囲(0～255)を超えています";
		};

		inline std::string FindFilePath(const char* FileName) {
			std::filesystem::path AbsolutePath = std::filesystem::canonical(std::filesystem::current_path());
			std::filesystem::path PartPath;

			for (const auto& part : AbsolutePath) {
				PartPath /= part;

				if (std::filesystem::exists(PartPath / FileName)) {
					return (PartPath / FileName).string();
				}
			}
			return "";
		}
	} // __detail

	// public:
	// 外部からのアニメ取得
	inline const auto& Anim(int id) {
		using namespace __detail;

		int ConvertID = GetAnimDataIndex(id);
		if (ConvertID >= 0) {
			return AnimRef(ConvertID);
		}
		throw std::runtime_error("存在しないアニメーションを指定しました\nID:" + std::to_string(id));
	}
 
	inline const auto& Anim(int id, int elem) {
		using namespace __detail;

		int ConvertID = GetAnimDataIndex(id);
		if (ConvertID >= 0 && elem >= 0 && elem < AnimRef(ConvertID).Elem().size()) {
			return AnimRef(ConvertID).Elem(elem);
		}
		throw std::runtime_error("存在しないアニメーションを指定しました\nID:" + std::to_string(id) + ", Elem:" + std::to_string(elem));
	}

	// アニメーションファイルの読み込み
	inline bool LoadAirFile(const char *FileName) {
		using namespace __detail;

		const std::string Path = FindFilePath(FileName);
		std::ifstream file;
		
		// 指定のファイルが見つからなければ終了
		if (Path.empty()) { return false; }
		file.open(Path);
		if (!file) { return false; }

		std::string textline;
		std::smatch match;
		const std::regex AnimationDataBegin_re(R"(\s*\[Begin Action (-?\d+)\]\s*(?:;(.*))?)");
		const std::regex AnimationDataParam_re(R"(\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,?\s*(H?)\s*(V?)\s*,?\s*([ADS]?)\s*(-?\d+)?\s*,?\s*([ADS]?)\s*(-?\d+)?\s*([ADS]?)\s*(-?\d+)?\s*)");
		const std::regex AnimationDataLoop_re(R"(\s*Loopstart)");
		const std::regex AnimationDataValue_re(R"(-?\d+)");
		// std::regex AnimationDataBegin_re(R"(\s*(\d+)(?:\s*,\s*(\d+)){4}\s*)"); 各値をキャプチャするため繰り返しは使用不可らしい
		
		// 行の取得＆カウント
		int TextLineCount = 0;
		auto GetTextLine = [&TextLineCount](std::ifstream& file, std::string& textline) -> std::istream& {
			++TextLineCount;
			return std::getline(file, textline);
		};
		
		// 1行目読み取り
		GetTextLine(file, textline);
		while (file.good()) {

			// [Begin Action XXX] の検索
			if (std::regex_match(textline, match, AnimationDataBegin_re)) {
				// XXX の範囲チェック
				if (!IsUInt31ValueInRange(match[1])) {
					AnimParamLog().SetError(TextLineCount, -1, -1, "Begin Action", match[1].str(), ErrorMessage::RangeBeginAction);
					GetTextLine(file, textline);
					continue;
				}
				std::cout << "\n文字列: " << match[0] << std::endl;
				std::cout << "見つかった数値: " << match[1] << std::endl;

				// 見つかった値を格納
				int AnimationID = std::stoi(match[1].str());

				// アニメーション重複チェック
				if (!AnimRef().size() || GetAnimDataIndex(AnimationID) == -1) {
					AnimRef().push_back(T_AnimData());
					RegisterAnimDataIndex(AnimationID, AnimRef().size() - 1);
					AnimRef().back().ID(AnimationID);
				}
				else {
					AnimParamLog().SetError(TextLineCount, -1, -1, "Begin Action", match[1].str(), ErrorMessage::DuplicationBeginAction);
					GetTextLine(file, textline);
					continue;
				}
			}
			else {
				GetTextLine(file, textline);
				continue;
			}

			// アニメーションパラメータの取得
			do {
				if (!GetTextLine(file, textline)) { continue; }

				auto& AnimData = AnimRef().back();
				// Loopstart検知
				if (std::regex_match(textline, AnimationDataLoop_re)) {
					AnimData.Loop(AnimData.Elem().size());
					if (!GetTextLine(file, textline)) { continue; }
				}

				// アニメーションパラメータの検索
				if (!std::regex_match(textline, match, AnimationDataParam_re)) { continue; }
			 
				std::cout << "取得パラメータ: " << match[0] << std::endl;
				std::cout << "SpriteID: " << match[1] << std::endl;
				std::cout << "SpriteNumber: " << match[2] << std::endl;
				std::cout << "PosX: " << match[3] << std::endl;
				std::cout << "PosY: " << match[4] << std::endl;
				std::cout << "Time: " << match[5] << std::endl;
				std::cout << "Facing: " << match[6] << std::endl;
				std::cout << "VFacing: " << match[7] << std::endl;
				std::cout << "アルファ1: " << match[8] << std::endl;
				std::cout << "アルファ1値: " << match[9] << std::endl;
				std::cout << "アルファ2: " << match[10] << std::endl;
				std::cout << "アルファ2値: " << match[11] << std::endl;
				std::cout << "アルファ3: " << match[12] << std::endl;
				std::cout << "アルファ3値: " << match[13] << std::endl;

				AnimData.Elem().push_back(T_AnimData::T_AnimElem());
				auto& AnimElem = AnimData.Elem(AnimData.Elem().size() - 1);

				// 取得データ格納(値の範囲チェック有り)
				if (IsUInt15ValueInRange(match[1])) { AnimElem.SpriteID(std::stoi(match[1].str())); }
				else { AnimParamLog().SetError(TextLineCount, AnimData.ID(), AnimData.Elem().size(), "SpriteID", match[1].str(), ErrorMessage::RangeSpriteID); }
				if (IsUInt15ValueInRange(match[2])) { AnimElem.SpriteNumber(std::stoi(match[2].str())); }
				else { AnimParamLog().SetError(TextLineCount, AnimData.ID(), AnimData.Elem().size(), "SpriteNumber", match[2].str(), ErrorMessage::RangeSpriteNumber); }
				if (IsInt32ValueInRange(match[3])) { AnimElem.PosX(std::stoi(match[3].str())); }
				else { AnimParamLog().SetError(TextLineCount, AnimData.ID(), AnimData.Elem().size(), "PosX", match[3].str(), ErrorMessage::RangePosX); }
				if (IsInt32ValueInRange(match[4])) { AnimElem.PosY(std::stoi(match[4].str())); }
				else { AnimParamLog().SetError(TextLineCount, AnimData.ID(), AnimData.Elem().size(), "PosY", match[4].str(), ErrorMessage::RangePosY); }
				if (IsInt32ValueInRange(match[5])) { AnimElem.Time(std::stoi(match[5].str())); }
				else { AnimParamLog().SetError(TextLineCount, AnimData.ID(), AnimData.Elem().size(), "Time", match[5].str(), ErrorMessage::RangeTime); }
				if (match[6] == "H") { AnimElem.Facing(true); }
				if (match[7] == "V") { AnimElem.VFacing(true); }
				for (int AlphaLoop = 8; AlphaLoop < 8 + 3 * 2; AlphaLoop += 2) {
					if (match[AlphaLoop] == "A" && std::regex_match(match[AlphaLoop + 1].str(), AnimationDataValue_re)) {
						if (IsUInt8ValueInRange(match[AlphaLoop + 1])) { AnimElem.AlphaA(std::stoi(match[AlphaLoop + 1])); }
						else { AnimParamLog().SetError(TextLineCount, AnimData.ID(), AnimData.Elem().size(), "AlphaA", match[AlphaLoop + 1].str(), ErrorMessage::RangeAlphaA); }
					}
					else if (match[AlphaLoop] == "S" && std::regex_match(match[AlphaLoop + 1].str(), AnimationDataValue_re)) {
						if (IsUInt8ValueInRange(match[AlphaLoop + 1])) { AnimElem.AlphaS(std::stoi(match[AlphaLoop + 1])); }
						else { AnimParamLog().SetError(TextLineCount, AnimData.ID(), AnimData.Elem().size(), "AlphaS", match[AlphaLoop + 1].str(), ErrorMessage::RangeAlphaS); }
					}
					else if (match[AlphaLoop] == "D" && std::regex_match(match[AlphaLoop + 1].str(), AnimationDataValue_re)) {
						if (IsUInt8ValueInRange(match[AlphaLoop + 1])) { AnimElem.AlphaD(std::stoi(match[AlphaLoop + 1])); }
						else { AnimParamLog().SetError(TextLineCount, AnimData.ID(), AnimData.Elem().size(), "AlphaD", match[AlphaLoop + 1].str(), ErrorMessage::RangeAlphaD); }
					}
					else if (match[AlphaLoop] == "") {
						break;
					}
				}
			} while (!std::regex_match(textline, match, AnimationDataBegin_re) && file.good());
			
			// 空アニメ警告
			if (!AnimRef().back().Elem().size()) {
				AnimParamLog().SetError(TextLineCount, AnimRef().back().ID(), -1, "AnimElem", "-", ErrorMessage::EmptyAnimElem);
			}
		}
		if (file.bad() || file.fail() && !file.eof()) {
			throw std::runtime_error("ファイルの読み取りが正常ではありませんでした");
		}

		// 最終出力
		for (int i = 0; i < AnimRef().size(); ++i) {
			std::cout << "\nアニメ番号: " << AnimRef(i).ID() << std::endl;
			if (AnimRef(i).Loop()) {
				std::cout << "ループ始点: " << AnimRef(i).LoopElem() << std::endl;
			}
			else {
				std::cout << "ループなし" << std::endl;
			}
			for (int j = 0; j < AnimRef(i).Elem().size(); ++j) {
				std::cout << "画像ID: " << AnimRef(i).Elem(j).SpriteID() << std::endl;
				std::cout << "画像Number: " << AnimRef(i).Elem(j).SpriteNumber() << std::endl;
			}
		}

		// ログ確認
		AnimParamLog().PrintLog();

		std::cout << "\n関数は正常に実行されました" << std::endl;

		return true;
	}
};

#endif