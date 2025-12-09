#ifndef INCLUDEGUARD_READAIRFILE_HEADER
#define INCLUDEGUARD_READAIRFILE_HEADER

#include <stdint.h>			// uint32_tとかのやつ
#include <fstream>			// ファイル読み取り
#include <filesystem>		// ファイル検索
#include <vector>			// 可変長配列
#include <unordered_map>	// ハッシュ的なやつ
#include <regex>			// 正規表現
#include <charconv>			// 文字列数字変換のやつ
#include <system_error>		// std::errcのやつ
#include <limits>			// std::numeric_limitsのやつ

namespace SAELib {
	namespace ReadAirFile_detail {
		using ksize_t = uint32_t;
		inline constexpr ksize_t KSIZE_MAX = std::numeric_limits<ksize_t>::max();

		namespace ReadAirFileFormat {
			inline constexpr double kVersion = 1.00;
			inline constexpr std::string_view kSystemDirectoryName = "SAELib";
			inline constexpr std::string_view kErrorLogFileName = "SAELib_AirErrorLog";
		};

		namespace AIRFormat {
			inline constexpr std::string_view kExtension = ".air";
			inline constexpr ksize_t kFileSizeLimit = 0xffffffff;
		};

		struct T_Bit {
		private:
			const int32_t kBitStart;
			const int32_t kBitRange;
		public:
			constexpr T_Bit(int32_t BitStart, int32_t BitRange) : kBitStart(BitStart), kBitRange(BitRange) {}
			[[nodiscard]] constexpr int32_t BitStart() const noexcept { return kBitStart; }
			[[nodiscard]] constexpr int32_t BitRange() const noexcept { return kBitRange; }
			[[nodiscard]] constexpr int32_t BitMask() const noexcept { return kBitRange << kBitStart; }
			[[nodiscard]] constexpr int32_t BitSet(int32_t Value) const noexcept { return (Value & kBitRange) << kBitStart; }
			[[nodiscard]] constexpr int32_t BitGet(int32_t Value) const noexcept { return (Value >> kBitStart) & kBitRange; }
		};

		struct Convert {
		private:
			inline static constexpr T_Bit kSpriteGroupNo = T_Bit(0, 65535);
			inline static constexpr T_Bit kSpriteImageNo = T_Bit(16, 65535);
			inline static constexpr T_Bit kElemFacing = T_Bit(0, 1);
			inline static constexpr T_Bit kElemVFacing = T_Bit(1, 1);
			inline static constexpr T_Bit kElemAlphaA = T_Bit(2, 511);
			inline static constexpr T_Bit kElemAlphaS = T_Bit(11, 511);
			inline static constexpr T_Bit kElemAlphaD = T_Bit(20, 511);
			inline static constexpr T_Bit kDummySpriteGroupNo = T_Bit(29, 1);
			inline static constexpr T_Bit kDummySpriteImageNo = T_Bit(30, 1);
			inline static constexpr T_Bit kElemLoopStart = T_Bit(0, 2147483647);
			inline static constexpr T_Bit kExistLoopStart = T_Bit(32, 1);

		public:
			[[nodiscard]] inline static constexpr int32_t EncodeSpriteNumber(int32_t GroupNo, int32_t ImageNo) noexcept {
				return kSpriteGroupNo.BitSet(GroupNo) | kSpriteImageNo.BitSet(ImageNo);
			}
			[[nodiscard]] inline static constexpr int32_t EncodeAnimExtraParam(bool Facing, bool VFacing, int32_t AlphaA, int32_t AlphaS, int32_t AlphaD, bool DummySpriteGroupNo, bool DummySpriteImageNo) noexcept {
				return kElemFacing.BitSet(Facing) | kElemVFacing.BitSet(VFacing) | 
					kElemAlphaA.BitSet(AlphaA) | kElemAlphaS.BitSet(AlphaS) | kElemAlphaD.BitSet(AlphaD) | 
					kDummySpriteGroupNo.BitSet(DummySpriteGroupNo) | kDummySpriteImageNo.BitSet(DummySpriteImageNo);
			}
			[[nodiscard]] inline static constexpr int32_t EncodeLoopStart(int32_t Elem, bool Exist) noexcept {
				return kElemLoopStart.BitSet(Elem) | kExistLoopStart.BitSet(Exist);
			}

			[[nodiscard]] inline static constexpr int32_t DecodeSpriteGroupNo(int32_t SpriteNumber) noexcept { return kSpriteGroupNo.BitGet(SpriteNumber); }
			[[nodiscard]] inline static constexpr int32_t DecodeSpriteImageNo(int32_t SpriteNumber) noexcept { return kSpriteImageNo.BitGet(SpriteNumber); }
			[[nodiscard]] inline static constexpr int32_t DecodeElemFacing(int32_t ExtraParam) noexcept { return kElemFacing.BitGet(ExtraParam); }
			[[nodiscard]] inline static constexpr int32_t DecodeElemVFacing(int32_t ExtraParam) noexcept { return kElemVFacing.BitGet(ExtraParam); }
			[[nodiscard]] inline static constexpr int32_t DecodeElemAlphaA(int32_t ExtraParam) noexcept { return kElemAlphaA.BitGet(ExtraParam); }
			[[nodiscard]] inline static constexpr int32_t DecodeElemAlphaS(int32_t ExtraParam) noexcept { return kElemAlphaS.BitGet(ExtraParam); }
			[[nodiscard]] inline static constexpr int32_t DecodeElemAlphaD(int32_t ExtraParam) noexcept { return kElemAlphaD.BitGet(ExtraParam); }
			[[nodiscard]] inline static constexpr int32_t DecodeDummySpriteGroupNo(int32_t ExtraParam) noexcept { return kDummySpriteGroupNo.BitGet(ExtraParam); }
			[[nodiscard]] inline static constexpr int32_t DecodeDummySpriteImageNo(int32_t ExtraParam) noexcept { return kDummySpriteImageNo.BitGet(ExtraParam); }
			[[nodiscard]] inline static constexpr int32_t DecodeElemLoopStart(int32_t LoopStart) noexcept { return kElemLoopStart.BitGet(LoopStart); }
			[[nodiscard]] inline static constexpr int32_t DecodeExistLoopStart(int32_t LoopStart) noexcept { return kExistLoopStart.BitGet(LoopStart); }
		};

		struct T_CheckRange {
		private:
			const int32_t kMinValue;
			const int32_t kMaxValue;
		public:
			constexpr T_CheckRange(int32_t MinValue, int32_t MaxValue) : kMinValue(MinValue), kMaxValue(MaxValue) {}
			[[nodiscard]] constexpr bool CheckRange(int32_t Value) const noexcept { return Value >= kMinValue && Value <= kMaxValue; }
		};

		namespace CheckRange {
			inline constexpr T_CheckRange kAnimNumber = T_CheckRange(0, 2147483647);
			inline constexpr T_CheckRange kSpriteGroupNo = T_CheckRange(-1, 65535);
			inline constexpr T_CheckRange kSpriteImageNo = T_CheckRange(-1, 65535);
			inline constexpr T_CheckRange kElemPosX = T_CheckRange(-2147483648, 2147483647);
			inline constexpr T_CheckRange kElemPosY = T_CheckRange(-2147483648, 2147483647);
			inline constexpr T_CheckRange kElemTime = T_CheckRange(-1, 2147483647);
			inline constexpr T_CheckRange kElemAlpha = T_CheckRange(0, 256);
		}
		
		struct T_Config {
		private:
			T_Config() = default;
			~T_Config() = default;
			T_Config(const T_Config&) = delete;
			T_Config& operator=(const T_Config&) = delete;

		private:
			int32_t BitFlag_ = {};
			// 設定一覧
			// &1 = このライブラリが例外を投げるか
			// &2 = エラーログファイルを生成するか
			// &4 = SAELibファイルを作成するか
			// &8 = 
			// 
			// SAELibファイルの生成パス指定
			// AIRデータ検索開始ディレクトリパス指定
			// 

			inline static constexpr int32_t kThrowError = 1 << 0;
			inline static constexpr int32_t kCreateLogFile = 1 << 1;
			inline static constexpr int32_t kCreateSAELibFile = 1 << 2;
			inline static constexpr int32_t kDefaultConfig = 0;

			// SAELibファイルのパス
			std::filesystem::path SAELibFilePath_ = {};

			// AIRファイル検索開始パス
			std::filesystem::path AIRSearchPath_ = {};

		public:
			[[nodiscard]] static T_Config& Instance() {
				static T_Config instance;
				return instance;
			}

		public:
			[[nodiscard]] int32_t BitFlag() const noexcept { return BitFlag_; }
			[[nodiscard]] bool ThrowError() const noexcept { return (BitFlag_ & kThrowError) != 0; }
			[[nodiscard]] bool CreateLogFile() const noexcept { return (BitFlag_ & kCreateLogFile) != 0; }
			[[nodiscard]] bool CreateSAELibFile() const noexcept { return (BitFlag_ & kCreateSAELibFile) != 0; }
			[[nodiscard]] const std::filesystem::path& SAELibFilePath() const noexcept { return SAELibFilePath_; }
			[[nodiscard]] const std::filesystem::path& AIRSearchPath() const noexcept { return AIRSearchPath_; }

			void InitConfig() { BitFlag_ = kDefaultConfig; }
			void ThrowError(bool flag) { BitFlag_ = (BitFlag_ & ~kThrowError) | (flag ? kThrowError : 0); }
			void CreateLogFile(bool flag) { BitFlag_ = (BitFlag_ & ~kCreateLogFile) | (flag ? kCreateLogFile : 0); }
			void CreateSAELibFile(bool flag) { BitFlag_ = (BitFlag_ & ~kCreateSAELibFile) | (flag ? kCreateSAELibFile : 0); }
			void SAELibFilePath(const std::filesystem::path& Path) { SAELibFilePath_ = (Path.empty() ? std::filesystem::current_path() : Path); }
			void AIRSearchPath(const std::filesystem::path& Path) { AIRSearchPath_ = (Path.empty() ? std::filesystem::current_path() : Path); }
		};


		// ユーザーが参照する範囲なのでkプレフィックスはなしで

		/**
		* @brief エラー情報
		*
		* 　このライブラリが使用するエラー情報の名前空間です
		*/
		namespace ErrorMessage {

			/**
			* @brief エラー情報構造体
			*
			* 　このライブラリが出力するエラー情報の構造体です
			*
			* @param const int32_t ID
			* @param const char* const Name
			* @param const char* const Message
			*/
			struct T_ErrorInfo {
			public:
				const int32_t ID;
				const char* const Name;
				const char* const Message;
			};

			/**
			* @brief エラーID情報
			*
			* 　このライブラリが出力するエラーIDのenumです
			*
			* @return enum ErrorID エラーID
			*/
			enum ErrorID : int32_t {
				InvalidAIRExtension,
				LoadAIRInvalidPath,
				AIRSearchInvalidPath,
				AIRFileNotFound,
				AIRFileSizeOver,
				EmptyAIRFilePath,
				OpenAIRFileFailed,
				DuplicateAnimNumber,
				AnimNumberNotFound,
				AnimIndexNotFound,
				SAELibFolderInvalidPath,
				CreateSAELibFolderFailed,
				CreateErrorLogFileFailed,
				WriteErrorLogFileFailed,
				CloseErrorLogFileFailed,
				AnimNumberOutOfRange,
				SpriteGroupNoOutOfRange,
				SpriteImageNoOutOfRange,
				ElemPosXOutOfRange,
				ElemPosYOutOfRange,
				ElemTimeOutOfRange,
				ElemAlphaAOutOfRange,
				ElemAlphaSOutOfRange,
				ElemAlphaDOutOfRange,
				EmptyAnimElem,
				AIRFileReadFailed,
				FromCharsConvertFailed,
			};

			/**
			* @brief エラー情報配列
			*
			* 　このライブラリが出力するエラー情報の配列です
			*
			* @return T_ErrorInfo ErrorInfo エラー情報
			*/
			inline constexpr T_ErrorInfo ErrorInfo[] = {
				{ InvalidAIRExtension,			"InvalidAIRExtension",			"ファイルの拡張子が.airではありません" },
				{ LoadAIRInvalidPath,			"LoadAIRInvalidPath",			"AIRファイル読み込み関数のパスが正しくありません" },
				{ AIRSearchInvalidPath,			"AIRSearchInvalidPath",			"AIRファイル検索フォルダのパスが正しくありません" },
				{ AIRFileNotFound,				"AIRFileNotFound",				"AIRファイルが見つかりません" },
				{ AIRFileSizeOver,				"AIRFileSizeOver",				"AIRファイルサイズが許容値を超えています" },
				{ EmptyAIRFilePath,				"EmptyAIRFilePath",				"AIRファイルパスが指定されていません" },
				{ OpenAIRFileFailed,			"OpenAIRFileFailed",			"AIRファイルが開けませんでした" },
				{ DuplicateAnimNumber,			"DuplicateAnimNumber",			"アニメリストの番号が重複しています" },
				{ AnimNumberNotFound,			"AnimNumberNotFound",			"指定した番号がアニメリストから見つかりません" },
				{ AnimIndexNotFound,			"AnimIndexNotFound",			"指定したインデックスがアニメリストから見つかりません" },
				{ SAELibFolderInvalidPath,		"SAELibFolderInvalidPath",		"SAELibフォルダのパスが正しくありません" },
				{ CreateSAELibFolderFailed,		"CreateSAELibFolderFailed",		"SAELibフォルダの作成に失敗しました" },
				{ CreateErrorLogFileFailed,		"CreateErrorLogFileFailed",		"エラーログファイルの作成に失敗しました" },
				{ WriteErrorLogFileFailed,		"WriteErrorLogFileFailed",		"エラーログファイルへの書き込みに失敗しました" },
				{ CloseErrorLogFileFailed,		"CloseErrorLogFileFailed",		"エラーログファイルへの書き込みが正常に終了しませんでした" },
				{ AnimNumberOutOfRange,			"AnimNumberOutOfRange",			"アニメーション番号が登録可能な数値の範囲(0～2147483647)ではありません" },
				{ SpriteGroupNoOutOfRange,		"SpriteGroupNoOutOfRange",		"グループ番号が登録可能な数値の範囲(-1～65535)ではありません" },
				{ SpriteImageNoOutOfRange,		"SpriteImageNoOutOfRange",		"イメージ番号が登録可能な数値の範囲(-1～65535)ではありません" },
				{ ElemPosXOutOfRange,			"ElemPosXOutOfRange",			"X座標が登録可能な数値の範囲(-2147483648～2147483647)ではありません" },
				{ ElemPosYOutOfRange,			"ElemPosYOutOfRange",			"Y座標が登録可能な数値の範囲(-2147483648～2147483647)ではありません" },
				{ ElemTimeOutOfRange,			"ElemTimeOutOfRange",			"アニメタイマーが登録可能な数値の範囲(-2147483648～2147483647)ではありません" },
				{ ElemAlphaAOutOfRange,			"ElemAlphaAOutOfRange",			"AlphaAが登録可能な数値の範囲(0～256)ではありません" },
				{ ElemAlphaSOutOfRange,			"ElemAlphaSOutOfRange",			"AlphaSが登録可能な数値の範囲(0～256)ではありません" },
				{ ElemAlphaDOutOfRange,			"ElemAlphaDOutOfRange",			"AlphaDが登録可能な数値の範囲(0～256)ではありません" },
				{ EmptyAnimElem,				"EmptyAnimElem",				"アニメーション内容が登録されていません" },
				{ AIRFileReadFailed,			"AIRFileReadFailed",			"AIRファイルの読み取り中にエラーが発生しました" },
				{ FromCharsConvertFailed,		"FromCharsConvertFailed",		"取得した文字列の変換に失敗しました" },
			};

			/**
			* @brief エラー情報のサイズ取得
			*
			* 　エラー情報の配列サイズを取得します
			*
			* @return size_t ErrorInfoSize エラー情報配列サイズ
			*/
			inline constexpr size_t ErrorInfoSize = sizeof(ErrorInfo) / sizeof(ErrorInfo[0]);

			/**
			* @brief エラー名の取得
			*
			* 　エラーIDに応じたエラー名を取得します
			*
			* @param int32_t ErrorID エラーID
			* @return const char* ErrorName エラー名
			*/
			inline constexpr const char* ErrorName(int32_t ID) { return ErrorInfo[ID].Name; }

			/**
			* @brief エラーメッセージの取得
			*
			* 　エラーIDに応じたエラーメッセージを取得します
			*
			* @param int32_t ErrorID エラーID
			* @return const char* ErrorMessage エラーメッセージ
			*/
			inline constexpr const char* ErrorMessage(int32_t ID) { return ErrorInfo[ID].Message; }
		}

		struct T_ErrorHandle {
		private:
			T_ErrorHandle() = default;
			~T_ErrorHandle() = default;
			T_ErrorHandle(const T_ErrorHandle&) = delete;
			T_ErrorHandle& operator=(const T_ErrorHandle&) = delete;

		private:
			struct T_ErrorList {
			private:
				const int32_t kErrorID;
				const int32_t kErrorValue = 0;
				const int32_t kErrorLine = 0;

			public:
				[[nodiscard]] int32_t ErrorID() const noexcept { return kErrorID; }
				[[nodiscard]] int32_t ErrorValue() const noexcept { return kErrorValue; }
				[[nodiscard]] int32_t ErrorLine() const noexcept { return kErrorLine; }
				[[nodiscard]] const char* const ErrorName() const noexcept { return ErrorMessage::ErrorInfo[kErrorID].Name; }
				[[nodiscard]] const char* const ErrorMessage() const noexcept { return ErrorMessage::ErrorInfo[kErrorID].Message; }

				T_ErrorList(int32_t ErrorID) : kErrorID(ErrorID) {}
				T_ErrorList(int32_t ErrorID, int32_t ErrorValue) : kErrorID(ErrorID), kErrorValue(ErrorValue) {}
				T_ErrorList(int32_t ErrorID, int32_t ErrorValue, int32_t ErrorLine) : kErrorID(ErrorID), kErrorValue(ErrorValue), kErrorLine(ErrorLine) {}
			};
			std::vector<T_ErrorList> ErrorList = {};

			void InitErrorList() { ErrorList.clear(); }

		public:
			[[nodiscard]] static T_ErrorHandle& Instance() {
				static T_ErrorHandle instance;
				return instance;
			}

		public:
			void AddErrorList(int32_t ErrorID) { ErrorList.emplace_back(T_ErrorList(ErrorID)); }
			void AddErrorList(int32_t ErrorID, int32_t ErrorValue) { ErrorList.emplace_back(T_ErrorList(ErrorID, ErrorValue)); }
			void AddErrorList(int32_t ErrorID, int32_t ErrorValue, int32_t ErrorLine) { ErrorList.emplace_back(T_ErrorList(ErrorID, ErrorValue, ErrorLine)); }

			[[noreturn]] void ThrowError(int32_t ErrorID) const { throw std::runtime_error(ErrorMessage::ErrorInfo[ErrorID].Name); }
			[[noreturn]] void ThrowError(int32_t ErrorID, int32_t ErrorValue) const { throw std::runtime_error(ErrorMessage::ErrorInfo[ErrorID].Name); }
			[[noreturn]] void ThrowError(int32_t ErrorID, int32_t ErrorValue, int32_t ErrorLine) const { throw std::runtime_error(ErrorMessage::ErrorInfo[ErrorID].Name); }

			void SetError(int32_t ErrorID) {
				if (!T_Config::Instance().ThrowError()) {
					AddErrorList(ErrorID);
					return;
				}
				ThrowError(ErrorID);
			}

			void SetError(int32_t ErrorID, int32_t ErrorValue) {
				if (!T_Config::Instance().ThrowError()) {
					AddErrorList(ErrorID, ErrorValue);
					return;
				}
				ThrowError(ErrorID, ErrorValue);
			}

			void SetError(int32_t ErrorID, int32_t ErrorValue, int32_t ErrorLine) {
				if (!T_Config::Instance().ThrowError()) {
					AddErrorList(ErrorID, ErrorValue, ErrorLine);
					return;
				}
				ThrowError(ErrorID, ErrorValue, ErrorLine);
			}

			void WriteErrorLog(std::ofstream& File) {
				File << "ReadAirFile ErrorLog" << "\n";
				File << "エラー数: " << ErrorList.size() << "\n";

				for (auto& Error : ErrorList) {
					File << "\nエラー名: " << Error.ErrorName() << "\n";
					File << "エラー内容: " << Error.ErrorMessage() << "\n";
					if (Error.ErrorID() == ErrorMessage::AnimNumberOutOfRange ||
						Error.ErrorID() == ErrorMessage::SpriteGroupNoOutOfRange ||
						Error.ErrorID() == ErrorMessage::SpriteImageNoOutOfRange ||
						Error.ErrorID() == ErrorMessage::ElemPosXOutOfRange ||
						Error.ErrorID() == ErrorMessage::ElemPosYOutOfRange ||
						Error.ErrorID() == ErrorMessage::ElemTimeOutOfRange ||
						Error.ErrorID() == ErrorMessage::ElemAlphaAOutOfRange ||
						Error.ErrorID() == ErrorMessage::ElemAlphaSOutOfRange ||
						Error.ErrorID() == ErrorMessage::ElemAlphaDOutOfRange ||
						Error.ErrorID() == ErrorMessage::EmptyAnimElem) {
						File << "エラー箇所: " << Error.ErrorLine() << "行目" << "\n";
						File << "エラー値: " << Error.ErrorValue() << "\n";
					}
					if (Error.ErrorID() == ErrorMessage::DuplicateAnimNumber ||
						Error.ErrorID() == ErrorMessage::AnimNumberNotFound || 
						Error.ErrorID() == ErrorMessage::AnimIndexNotFound) {
						File << "エラー値: " << Error.ErrorValue() << "\n";
					}
				}
				File.flush();

				if (File.fail() || File.bad()) {
					if (T_Config::Instance().ThrowError()) {
						ThrowError(ErrorMessage::WriteErrorLogFileFailed);
					}
				}
				File.close();
				if (File.fail() || File.bad()) {
					if (T_Config::Instance().ThrowError()) {
						ThrowError(ErrorMessage::CloseErrorLogFileFailed);
					}
				}
			}
		};

		// パスを扱う時の補助
		struct T_FilePathSystem {
		private:
			std::filesystem::path Path_ = {};
			std::error_code ErrorCode_ = {};

		public:
			const std::filesystem::path& Path() const noexcept { return Path_; }
			const std::error_code& ErrorCode() const noexcept { return ErrorCode_; }

			void SetPath(const std::filesystem::path& Path) {
				ErrorCode_.clear();
				Path_ = weakly_canonical(std::filesystem::absolute(Path), ErrorCode_);
				if (ErrorCode_) { Path_.clear(); }
			}

			void CreateDirectory(const std::filesystem::path& Path) {
				ErrorCode_.clear();
				if (std::filesystem::exists(Path)) { return; }
				std::filesystem::create_directories(Path, ErrorCode_);

			}

		public:
			T_FilePathSystem() = default;

			T_FilePathSystem(const std::filesystem::path& Path) {
				SetPath(Path);
			}

			[[nodiscard]] bool empty() const noexcept { return Path_.empty(); }
		};

		// スプライトリストの画像番号の重複チェック＆存在確認
		struct T_UnorderedMap {
		private:
			std::unordered_map<int32_t, int32_t> UnorderedMap = {};

		public:
			void Register(int32_t value) {
				UnorderedMap[value] = static_cast<int32_t>(UnorderedMap.size());
			}

		public:
			T_UnorderedMap() = default;

			void reserve(ksize_t value) { UnorderedMap.reserve(value); }
			void clear() { UnorderedMap.clear(); }
			void shrink_to_fit() { UnorderedMap.rehash(0); }
		
			[[nodiscard]] int32_t find(int32_t input) {
				auto it = UnorderedMap.find(input);
				if (it != UnorderedMap.end()) { return it->second; }
				return -1;
			}

			[[nodiscard]] bool exist(int32_t value) { return find(value) >= 0; }
			[[nodiscard]] bool empty() const noexcept { return UnorderedMap.empty(); }
			[[nodiscard]] ksize_t size() const noexcept { return static_cast<ksize_t>(UnorderedMap.size()); }
		};

		// アニメデータ管理
		struct T_AirAnimData {
		private:
			struct T_AnimList {
			private:
				const int32_t kAnimNumber;		//
				const ksize_t kElemDataStart;	//
				const int32_t kElemDataSize;	//
				const int32_t kLoopstart;		// ループ開始枚数(2147483647), ループ存在判定(1)

			public:
				[[nodiscard]] int32_t AnimNumber() const noexcept { return kAnimNumber; }
				[[nodiscard]] ksize_t ElemDataStart() const noexcept { return kElemDataStart; }
				[[nodiscard]] int32_t ElemDataSize() const noexcept { return kElemDataSize; }
				[[nodiscard]] int32_t ElemLoopstart() const noexcept { return Convert::DecodeElemLoopStart(kLoopstart); }
				[[nodiscard]] bool ExistLoopstart() const noexcept { return Convert::DecodeExistLoopStart(kLoopstart); }

				T_AnimList(int32_t AnimNumber, int32_t Loopstart, ksize_t ElemDataStart, int32_t ElemDataSize)
					: kAnimNumber(AnimNumber), kLoopstart(Loopstart), kElemDataStart(ElemDataStart), kElemDataSize(ElemDataSize) {
				}
			};

			struct T_ElemData {
			private:
				const int32_t kSpriteNumber;	// GroupNo(65535) ImageNo(65535)
				const int32_t kPosX;			// PosX(-2147483648～2147483647)
				const int32_t kPosY;			// PosY(-2147483648～2147483647)
				const int32_t kElemTime;		// ElemTime(-2147483648～2147483647)
				const int32_t kExtraParam;		// Facing(1), VFacing(1), AlphaA(511), AlphaS(511), AlphaD(511), DummySpriteGroupNo(1), DummySpriteImageNo(1)

			public:
				[[nodiscard]] int32_t GroupNo() const noexcept { return DummySpriteGroupNo() ? -1 : Convert::DecodeSpriteGroupNo(kSpriteNumber); }
				[[nodiscard]] int32_t ImageNo() const noexcept { return DummySpriteImageNo() ? -1 : Convert::DecodeSpriteImageNo(kSpriteNumber); }
				[[nodiscard]] int32_t PosX() const noexcept { return kPosX; }
				[[nodiscard]] int32_t PosY() const noexcept { return kPosY; }
				[[nodiscard]] int32_t ElemTime() const noexcept { return kElemTime; }
				[[nodiscard]] int32_t Facing() const noexcept { return Convert::DecodeElemFacing(kExtraParam) ? -1 : 1; }
				[[nodiscard]] int32_t VFacing() const noexcept { return Convert::DecodeElemVFacing(kExtraParam) ? -1 : 1; }
				[[nodiscard]] int32_t AlphaA() const noexcept { return Convert::DecodeElemAlphaA(kExtraParam); }
				[[nodiscard]] int32_t AlphaS() const noexcept { return Convert::DecodeElemAlphaS(kExtraParam); }
				[[nodiscard]] int32_t AlphaD() const noexcept { return Convert::DecodeElemAlphaD(kExtraParam); }
				[[nodiscard]] int32_t DummySpriteGroupNo() const noexcept { return Convert::DecodeDummySpriteGroupNo(kExtraParam); }
				[[nodiscard]] int32_t DummySpriteImageNo() const noexcept { return Convert::DecodeDummySpriteImageNo(kExtraParam); }

				T_ElemData(int32_t SpriteNumber, int32_t PosX, int32_t PosY, int32_t ElemTime, int32_t ExtraParam)
					: kSpriteNumber(SpriteNumber), kPosX(PosX), kPosY(PosY)
					, kElemTime(ElemTime), kExtraParam(ExtraParam) {
				}
			};

			std::vector<T_AnimList> AnimList_ = {};
			std::vector<T_ElemData> ElemData_ = {};
		public:
			[[nodiscard]] const std::vector<T_AnimList>& AnimList() const noexcept { return AnimList_; }
			[[nodiscard]] const std::vector<T_ElemData>& ElemData() const noexcept { return ElemData_; }
			[[nodiscard]] const T_AnimList& AnimList(ksize_t index) const noexcept { return AnimList_[index]; }
			[[nodiscard]] const T_ElemData& ElemData(ksize_t index) const noexcept { return ElemData_[index]; }

			void AddAnimList(int32_t AnimNumber, int32_t Loopstart, ksize_t ElemDataStart, int32_t ElemDataSize) {
				AnimList_.emplace_back(T_AnimList(AnimNumber, Loopstart, ElemDataStart, ElemDataSize));
			}

			void AddElemData(int32_t SpriteNumber, int32_t PosX, int32_t PosY, int32_t ElemTime, int32_t ExtraParam) {
				ElemData_.emplace_back(T_ElemData(SpriteNumber, PosX, PosY, ElemTime, ExtraParam));
			}

		public:
			T_AirAnimData() = default;

			void reserve(ksize_t NumImage, ksize_t FileSize, ksize_t PaletteSize) {
				AnimList_.reserve(NumImage);
				ElemData_.reserve(NumImage);
			}

			void clear() {
				AnimList_.clear();
				ElemData_.clear();
			}

			void shrink_to_fit() {
				AnimList_.shrink_to_fit();
				ElemData_.shrink_to_fit();
			}

			[[nodiscard]] bool empty() const noexcept { return AnimList_.empty() && ElemData_.empty(); }
		};

		struct T_ActionBegin {
		private:
			inline static constexpr int32_t kAnimNumberIndex = 1;

			const std::smatch& kRegexMatch;
			const int32_t kTextLineCount;
			const int32_t kAnimNumber;

			[[nodiscard]] int32_t FromChars(int32_t index) const noexcept {
				int32_t ChersValue;
				std::from_chars_result Result = std::from_chars(&*kRegexMatch[index].first, (&*kRegexMatch[index].first) + kRegexMatch[index].length(), ChersValue);

				if (Result.ec != std::errc{}) {
					T_ErrorHandle::Instance().SetError(ErrorMessage::FromCharsConvertFailed, ChersValue, kTextLineCount);
					return -1;
				}

				// ここに範囲チェック
				if (!CheckRange::kAnimNumber.CheckRange(ChersValue)) {
					T_ErrorHandle::Instance().SetError(ErrorMessage::AnimNumberOutOfRange, ChersValue, kTextLineCount);
					return -1;
				}
				return ChersValue;
			}

			[[nodiscard]] int32_t GetAnimNumber() const noexcept { return FromChars(kAnimNumberIndex); }

		public:
			int32_t AnimNumber() const noexcept { return kAnimNumber; }

			T_ActionBegin(std::smatch& RegexMatch, int32_t TextLineCount)
				: kRegexMatch(RegexMatch), kTextLineCount(TextLineCount)
				, kAnimNumber(GetAnimNumber()) {
			}
		};

		struct T_AnimParam {
		private:
			inline static constexpr int32_t kGroupNoIndex = 1;
			inline static constexpr int32_t kImageNoIndex = 2;
			inline static constexpr int32_t kPosXIndex = 3;
			inline static constexpr int32_t kPosYIndex = 4;
			inline static constexpr int32_t kElemTimeIndex = 5;
			inline static constexpr int32_t kFacingIndex = 6;
			inline static constexpr int32_t kVFacingIndex = 7;
			inline static constexpr int32_t kAlphaIndex = 8;
			inline static constexpr int32_t kAlphaAValueIndex = 9;
			inline static constexpr int32_t kAlphaSValueIndex = 11;
			inline static constexpr int32_t kAlphaDValueIndex = 13;

			const std::smatch& kRegexMatch;
			const int32_t kTextLineCount;
			const int32_t kSpriteNumber;	// GroupNo(65535) ImageNo(65535)
			const int32_t kPosX;			// PosX(-2147483648～2147483647)
			const int32_t kPosY;			// PosY(-2147483648～2147483647)
			const int32_t kElemTime;		// ElemTime(-2147483648～2147483647)
			const int32_t kExtraParam;		// Facing(1), VFacing(1), AlphaA(511), AlphaS(511), AlphaD(511), DummySprite=-1(1)
		
			[[nodiscard]] int32_t FromChars(int32_t index) const noexcept {
				if (kRegexMatch.size() <= index || !kRegexMatch[index].matched || !kRegexMatch[index].length()) { return 0; }

				int32_t ChersValue;
				std::from_chars_result Result = std::from_chars(&*kRegexMatch[index].first, (&*kRegexMatch[index].first) + kRegexMatch[index].length(), ChersValue);
				
				if (Result.ec != std::errc{}) {
					T_ErrorHandle::Instance().SetError(ErrorMessage::FromCharsConvertFailed, ChersValue, kTextLineCount);
					return 0;
				}

				switch (index) {
				case kGroupNoIndex:
					if (!CheckRange::kSpriteGroupNo.CheckRange(ChersValue)) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::SpriteGroupNoOutOfRange, ChersValue, kTextLineCount);
						return 0;
					}
					break;
				case kImageNoIndex:
					if (!CheckRange::kSpriteImageNo.CheckRange(ChersValue)) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::SpriteImageNoOutOfRange, ChersValue, kTextLineCount);
						return 0;
					}
					break;
				case kPosXIndex:
					if (!CheckRange::kElemPosX.CheckRange(ChersValue)) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::ElemPosXOutOfRange, ChersValue, kTextLineCount);
						return 0;
					}
					break;
				case kPosYIndex:
					if (!CheckRange::kElemPosY.CheckRange(ChersValue)) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::ElemPosYOutOfRange, ChersValue, kTextLineCount);
						return 0;
					}
					break;
				case kElemTimeIndex:
					if (!CheckRange::kElemTime.CheckRange(ChersValue)) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::ElemTimeOutOfRange, ChersValue, kTextLineCount);
						return 0;
					}
					break;
				case kAlphaAValueIndex:
					if (!CheckRange::kElemAlpha.CheckRange(ChersValue)) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::ElemAlphaAOutOfRange, ChersValue, kTextLineCount);
						return 0;
					}
					break;
				case kAlphaSValueIndex:
					if (!CheckRange::kElemAlpha.CheckRange(ChersValue)) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::ElemAlphaSOutOfRange, ChersValue, kTextLineCount);
						return 0;
					}
					break;
				case kAlphaDValueIndex:
					if (!CheckRange::kElemAlpha.CheckRange(ChersValue)) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::ElemAlphaDOutOfRange, ChersValue, kTextLineCount);
						return 0;
					}
					break;
				}

				return ChersValue;
			}

			[[nodiscard]] bool FromCharsFacing() const noexcept { return (kRegexMatch[kFacingIndex] == "" ? false : true); }
			[[nodiscard]] bool FromCharsVFacing() const noexcept { return (kRegexMatch[kVFacingIndex] == "" ? false : true); }

			[[nodiscard]] int32_t FromCharsAlpha(int32_t Alpha) const noexcept {
				for (int32_t AlphaLoop = kAlphaIndex; AlphaLoop < kAlphaDValueIndex; AlphaLoop += 2) {
					if (Alpha == 'A' && kRegexMatch[AlphaLoop] == "A" ||
						Alpha == 'S' && kRegexMatch[AlphaLoop] == "S" ||
						Alpha == 'D' && kRegexMatch[AlphaLoop] == "D") {
						return FromChars(AlphaLoop + 1);
					}
				}
				return 0;
			}

			[[nodiscard]] int32_t GetGroupNo() const noexcept { return FromChars(kGroupNoIndex); }
			[[nodiscard]] int32_t GetImageNo() const noexcept { return FromChars(kImageNoIndex); }
			[[nodiscard]] int32_t GetPosX() const noexcept { return FromChars(kPosXIndex); }
			[[nodiscard]] int32_t GetPosY() const noexcept { return FromChars(kPosYIndex); }
			[[nodiscard]] int32_t GetElemTime() const noexcept { return FromChars(kElemTimeIndex); }
			[[nodiscard]] bool GetFacing() const noexcept { return FromCharsFacing(); }
			[[nodiscard]] bool GetVFacing() const noexcept { return FromCharsVFacing(); }
			[[nodiscard]] int32_t GetAlphaA() const noexcept { return FromCharsAlpha('A'); }
			[[nodiscard]] int32_t GetAlphaS() const noexcept { return FromCharsAlpha('S'); }
			[[nodiscard]] int32_t GetAlphaD() const noexcept { return FromCharsAlpha('D'); }
			[[nodiscard]] bool GetDummySpriteGroupNo() const noexcept { return GetGroupNo() < 0 || GetGroupNo() > 65535; }
			[[nodiscard]] bool GetDummySpriteImageNo() const noexcept { return GetImageNo() < 0 || GetImageNo() > 65535; }

		public:
			int32_t SpriteNumber() const noexcept { return kSpriteNumber; }
			int32_t GroupNo() const noexcept { return Convert::DecodeSpriteGroupNo(kSpriteNumber); }
			int32_t ImageNo() const noexcept { return Convert::DecodeSpriteImageNo(kSpriteNumber); }
			int32_t PosX() const noexcept { return kPosX; }
			int32_t PosY() const noexcept { return kPosY; }
			int32_t ElemTime() const noexcept { return kElemTime; }
			int32_t ExtraParam() const noexcept { return kExtraParam; }
			int32_t Facing() const noexcept { return Convert::DecodeElemFacing(kExtraParam); }
			int32_t VFacing() const noexcept { return Convert::DecodeElemVFacing(kExtraParam); }
			int32_t AlphaA() const noexcept { return Convert::DecodeElemAlphaA(kExtraParam); }
			int32_t AlphaS() const noexcept { return Convert::DecodeElemAlphaS(kExtraParam); }
			int32_t AlphaD() const noexcept { return Convert::DecodeElemAlphaD(kExtraParam); }
			int32_t DummySpriteGroupNo() const noexcept { return Convert::DecodeDummySpriteGroupNo(kExtraParam); }
			int32_t DummySpriteImageNo() const noexcept { return Convert::DecodeDummySpriteImageNo(kExtraParam); }

			T_AnimParam(std::smatch& RegexMatch, int32_t TextLineCount)
				: kRegexMatch(RegexMatch), kTextLineCount(TextLineCount)
				, kSpriteNumber(Convert::EncodeSpriteNumber(GetGroupNo(), GetImageNo()))
				, kPosX(GetPosX()), kPosY(GetPosY()), kElemTime(GetElemTime())
				, kExtraParam(Convert::EncodeAnimExtraParam(GetFacing(), GetVFacing(), GetAlphaA(), GetAlphaS(), GetAlphaD(), GetDummySpriteGroupNo(), GetDummySpriteImageNo()))
			{}
		};

		struct T_LoadAirFile {
		private:
			const std::string kFileName = {};
			const std::string kFilePath = {};
			const uintmax_t kFileSize = 0;
			std::ifstream File = {};
			const bool kCheckError = false;

			[[nodiscard]] const std::string EnsureAirExtension(const std::filesystem::path& FileName) const {
				std::filesystem::path FixedFileName = FileName;
				if (FixedFileName.extension() != AIRFormat::kExtension) {
					if (!FixedFileName.extension().empty()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::InvalidAIRExtension);
					}
					FixedFileName.replace_extension(AIRFormat::kExtension);
				}
				return FixedFileName.string();
			}

			[[nodiscard]] const std::string FindFilePathDown(const std::string& FilePath) const {
				T_FilePathSystem AirFolder;
				if (!FilePath.empty()) {
					AirFolder.SetPath(FilePath);
					if (AirFolder.ErrorCode()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::LoadAIRInvalidPath);
					}
				}
				if (FilePath.empty() || AirFolder.ErrorCode() && !T_Config::Instance().AIRSearchPath().empty()) {
					AirFolder.SetPath(T_Config::Instance().AIRSearchPath());
					if (AirFolder.ErrorCode()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::AIRSearchInvalidPath);
					}
				}
				const std::filesystem::path AbsolutePath = (std::filesystem::exists(AirFolder.Path()) ? AirFolder.Path() : std::filesystem::canonical(std::filesystem::current_path()));

				for (const auto& entry : std::filesystem::recursive_directory_iterator(
					AbsolutePath, std::filesystem::directory_options::skip_permission_denied)) {
					if (!entry.is_regular_file()) { continue; }
					if (entry.path().filename() == kFileName) {
						return entry.path().string();
					}
				}

				T_ErrorHandle::Instance().SetError(ErrorMessage::AIRFileNotFound);
				return {};
			}

			[[nodiscard]] bool CheckFileSize() const {
				if (kFileSize <= UINT32_MAX) { return false; }
				T_ErrorHandle::Instance().SetError(ErrorMessage::AIRFileSizeOver);
				return true;
			}
			[[nodiscard]] bool CheckFilePath() const {
				if (!FilePath().empty()) { return false; }
				T_ErrorHandle::Instance().SetError(ErrorMessage::EmptyAIRFilePath);
				return true;
			}
			[[nodiscard]] bool CheckFileOpen() {
				File.open(FilePath());
				if (File.is_open()) { return false; }
				T_ErrorHandle::Instance().SetError(ErrorMessage::OpenAIRFileFailed);
				return true;
			}

			[[nodiscard]] bool CheckFileError() { return CheckFileSize() || CheckFilePath() || CheckFileOpen(); }

		public:
			[[nodiscard]] const std::string& FileName() const noexcept { return kFileName; }
			[[nodiscard]] const std::string& FilePath() const noexcept { return kFilePath; }
			[[nodiscard]] ksize_t FileSize() const noexcept { return static_cast<ksize_t>(kFileSize); }
			[[nodiscard]] bool CheckError() const noexcept { return kCheckError; }

		public:
			[[nodiscard]] bool ReadAirFile(T_UnorderedMap& AnimNumberUMap, T_AirAnimData& AirAnimData) {
				// 正規表現
				std::smatch RegexMatch;
				const std::regex AnimationDataBegin_re(R"(\s*\[Begin Action (-?\d+)\]\s*(?:;(.*))?)");
				const std::regex AnimationDataParam_re(R"(\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,?\s*(H?)\s*(V?)\s*,?\s*([ADS]?)\s*(-?\d+)?\s*,?\s*([ADS]?)\s*(-?\d+)?\s*([ADS]?)\s*(-?\d+)?\s*)");
				const std::regex AnimationDataLoop_re(R"(\s*Loopstart)");
				const std::regex AnimationDataValue_re(R"(-?\d+)");

				std::string TextLine = {};
				int32_t TextLineCount = 0;
				bool FoundAnimData = false;
				bool FoundElemData = false;
				int32_t AnimNumber = 0;
				int32_t Loopstart = 0;
				ksize_t ElemStart = 0;
				int32_t ElemDataSize = 0;

				while (File.good()) {
					++TextLineCount;
					if (!std::getline(File, TextLine) || TextLine.empty()) { continue; }

					// [Begin Action XXX] の検索
					if (std::regex_match(TextLine, RegexMatch, AnimationDataBegin_re)) {

						// 空アニメ警告
						if (FoundAnimData && !FoundElemData) {
							T_ErrorHandle::Instance().SetError(ErrorMessage::EmptyAnimElem, 0, TextLineCount);
						}
						else if (ElemDataSize) {
							AirAnimData.AddAnimList(AnimNumber, Loopstart, ElemStart, ElemDataSize);
							Loopstart = 0;
							ElemDataSize = 0;
						}

						T_ActionBegin ActionBegin(RegexMatch, TextLineCount);

						// アニメーション重複チェック
						if (ActionBegin.AnimNumber() < 0 || AnimNumberUMap.exist(ActionBegin.AnimNumber())) {
							if (ActionBegin.AnimNumber() >= 0) {
								T_ErrorHandle::Instance().SetError(ErrorMessage::DuplicateAnimNumber, ActionBegin.AnimNumber(), TextLineCount);
							}
							FoundAnimData = false;
							FoundElemData = false;
							ElemDataSize = 0;
							continue;
						}
						AnimNumberUMap.Register(ActionBegin.AnimNumber());
						FoundAnimData = true;
						FoundElemData = false;
						AnimNumber = ActionBegin.AnimNumber();
						ElemStart = static_cast<ksize_t>(AirAnimData.ElemData().size());
					}
					else if (FoundAnimData) {
						// Loopstart検知
						if (!Convert::DecodeExistLoopStart(Loopstart) && std::regex_match(TextLine, AnimationDataLoop_re)) {
							Loopstart = Convert::EncodeLoopStart(ElemDataSize, true);
							continue;
						}

						// アニメーションパラメータの検索
						if (!std::regex_match(TextLine, RegexMatch, AnimationDataParam_re)) { continue; }

						FoundElemData = true;
						++ElemDataSize;
						T_AnimParam AnimParam(RegexMatch, TextLineCount);
						AirAnimData.AddElemData(AnimParam.SpriteNumber(), AnimParam.PosX(), AnimParam.PosY(), AnimParam.ElemTime(), AnimParam.ExtraParam());
					}
				}

				if (File.bad() || File.fail() && !File.eof()) {
					T_ErrorHandle::Instance().SetError(ErrorMessage::AIRFileReadFailed);
					return false;
				}

				return true;
			}

		public:
			T_LoadAirFile(const std::string& FileName, const std::string& FilePath)
				: kFileName(EnsureAirExtension(FileName)), kFilePath(FindFilePathDown(FilePath))
				, kFileSize(kFilePath.empty() ? 0 : std::filesystem::file_size(kFilePath)), kCheckError(CheckFileError()) {
			}
		};

		struct T_AIRData {
		private:
			int32_t NumAnim_ = 0;
			std::string FileName_ = {};
			T_UnorderedMap AnimNumberUMap = {};
			T_AirAnimData AirAnimData = {};

			void NumAnim(int32_t value) noexcept { NumAnim_ = value; }
			void FileName(const std::string& value) noexcept { FileName_ = value; }

			bool LoadAIRFile(const std::string& FileName_, const std::string& FilePath_) {
				if (!empty()) { clear(); }
				T_LoadAirFile LoadAIRFile(FileName_, FilePath_);
				if (LoadAIRFile.CheckError()) { return false; }

				if (!LoadAIRFile.ReadAirFile(AnimNumberUMap, AirAnimData)) { return false; }
			
				FileName(LoadAIRFile.FileName());
				NumAnim(static_cast<int32_t>(AnimNumberUMap.size()));

				// ログ出力
				if (T_Config::Instance().CreateLogFile()) {
					T_FilePathSystem SAELibFile(T_Config::Instance().SAELibFilePath() / (T_Config::Instance().CreateSAELibFile() ? ReadAirFileFormat::kSystemDirectoryName : ""));
					if (SAELibFile.ErrorCode()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::SAELibFolderInvalidPath);
						return false;
					}
					if (T_Config::Instance().CreateSAELibFile()) {
						SAELibFile.CreateDirectory(SAELibFile.Path());
						if (SAELibFile.ErrorCode()) {
							T_ErrorHandle::Instance().SetError(ErrorMessage::CreateSAELibFolderFailed);
							return false;
						}
					}

					const std::string ErrorLogFileName = std::string(ReadAirFileFormat::kErrorLogFileName) + "_" + FileName() + ".txt";
					std::ofstream ErrorLogFile(SAELibFile.Path() / ErrorLogFileName);
					if (!ErrorLogFile.is_open()) {
						T_ErrorHandle::Instance().SetError(ErrorMessage::CreateErrorLogFileFailed);
					}
					T_ErrorHandle::Instance().WriteErrorLog(ErrorLogFile);
				}

				return true;
			}

			// ユーザー向けのT_ElemDataアクセス手段
			struct T_AccessData_Elem {
			private:
				const T_AirAnimData* const kAirAnimDataPtr;
				const ksize_t kElemDataIndex; // 配列Index(最大値のときダミーデータフラグとして使用)

				const auto& ParamRef() const noexcept { return kAirAnimDataPtr->ElemData(kElemDataIndex); }

			public:
				/**
				* @brief ダミーデータ判断
				*
				* 　自身がダミーデータであるかを確認します
				*
				* 　AIRConfig::SetThrowErrorの設定がOFFの場合にエラー回避のために使用されます
				*
				* @return bool (false = 自身が正常なデータ：true = 自身がダミーデータ)
				*/
				bool IsDummy() const noexcept { return kElemDataIndex == KSIZE_MAX; }

				/**
				* @brief グループ番号の取得
				*
				* 　SAEで設定したグループ番号を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t GroupNo グループ番号
				*/
				int32_t GroupNo() const noexcept { return (IsDummy() ? 0 : ParamRef().GroupNo()); }

				/**
				* @brief イメージ番号の取得
				*
				* 　SAEで設定したイメージ番号を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t ImageNo イメージ番号
				*/
				int32_t ImageNo() const noexcept { return (IsDummy() ? 0 : ParamRef().ImageNo()); }

				/**
				* @brief X座標の取得
				*
				* 　SAEで設定したX座標を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t PosX X座標
				*/
				int32_t PosX() const noexcept { return (IsDummy() ? 0 : ParamRef().PosX()); }

				/**
				* @brief Y座標の取得
				*
				* 　SAEで設定したY座標を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t PosY X座標
				*/
				int32_t PosY() const noexcept { return (IsDummy() ? 0 : ParamRef().PosY()); }

				/**
				* @brief フレーム表示時間の取得
				*
				* 　SAEで設定したフレーム表示時間を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t ElemTime フレーム表示時間
				*/
				int32_t ElemTime() const noexcept { return (IsDummy() ? 0 : ParamRef().ElemTime()); }

				/**
				* @brief 水平方向の取得
				*
				* 　SAEで設定した画像の水平方向を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t Facing 水平方向 (1 = 正方向： -1 = 負方向)
				*/
				int32_t Facing() const noexcept { return (IsDummy() ? 0 : ParamRef().Facing()); }

				/**
				* @brief 垂直方向の取得
				*
				* 　SAEで設定した画像の垂直方向を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t VFacing 垂直方向 (1 = 正方向： -1 = 負方向)
				*/
				int32_t VFacing() const noexcept { return (IsDummy() ? 0 : ParamRef().VFacing()); }

				/**
				* @brief アルファ値Aの取得
				*
				* 　SAEで設定したアルファ値Aを返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t AlphaA アルファ値A
				*/
				int32_t AlphaA() const noexcept { return (IsDummy() ? 0 : ParamRef().AlphaA()); }

				/**
				* @brief アルファ値Sの取得
				*
				* 　SAEで設定したアルファ値Sを返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t AlphaS アルファ値S
				*/
				int32_t AlphaS() const noexcept { return (IsDummy() ? 0 : ParamRef().AlphaS()); }

				/**
				* @brief アルファ値Dの取得
				*
				* 　SAEで設定したアルファ値Dを返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t AlphaD アルファ値D
				*/
				int32_t AlphaD() const noexcept { return (IsDummy() ? 0 : ParamRef().AlphaD()); }

				T_AccessData_Elem(const T_AirAnimData* const AirAnimDataPtr, const ksize_t ElemDataIndex) : kAirAnimDataPtr(AirAnimDataPtr), kElemDataIndex(ElemDataIndex) {}
			};

			// ユーザー向けのT_AnimListアクセス手段
			struct T_AccessData_Anim {
			private:
				const T_AirAnimData* const kAirAnimDataPtr;
				const ksize_t kAnimListIndex; // 配列Index(最大値のときダミーデータフラグとして使用)

				const auto& ParamRef() const noexcept { return kAirAnimDataPtr->AnimList(kAnimListIndex); }

			public:
				/**
				* @brief ダミーデータ判断
				*
				* 　自身がダミーデータであるかを確認します
				*
				* 　AIRConfig::SetThrowErrorの設定がOFFの場合にエラー回避のために使用されます
				*
				* @return bool (false = 自身が正常なデータ：true = 自身がダミーデータ)
				*/
				bool IsDummy() const noexcept { return kAnimListIndex == KSIZE_MAX; }

				/**
				* @brief アニメ番号の取得
				*
				* 　SAEで設定したアニメ番号を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t AnimNumber アニメ番号
				*/
				int32_t AnimNumber() const noexcept { return (IsDummy() ? 0 : ParamRef().AnimNumber()); }

				/**
				* @brief ループ開始位置の存在確認
				*
				* 　SAEで設定したアニメのループ開始位置が存在するかを確認します
				*
				* 　ダミーデータの場合は false を返します
				*
				* @return bool 判定結果 (false = 存在なし : true = 存在あり)
				*/
				bool ExistLoopstart() const noexcept { return (IsDummy() ? false : ParamRef().ExistLoopstart()); }

				/**
				* @brief ループ開始位置の取得
				*
				* 　SAEで設定したアニメのループ開始位置を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t ElemLoopstart ループ開始位置
				*/
				int32_t ElemLoopstart() const noexcept { return (IsDummy() ? 0 : ParamRef().ElemLoopstart()); }

				/**
				* @brief アニメ枚数の取得
				*
				* 　SAEで設定したアニメ枚数を返します
				*
				* 　ダミーデータの場合は 0 を返します
				*
				* @return int32_t ElemDataSize アニメ枚数
				*/
				int32_t ElemDataSize() const noexcept { return (IsDummy() ? 0 : ParamRef().ElemDataSize()); }
				
				/**
				* @brief 指定インデックスのデータへアクセス
				*
				* 　AIRデータへ指定したインデックスでアクセスします
				*
				* 　対象が存在しない場合はAIRConfig::SetThrowErrorの設定に準拠します
				*
				* @param int32_t index データ配列インデックス
				* @retval 対象が存在する ElemData
				* @retval 対象が存在しない AIRConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)
				*/
				T_AccessData_Elem GetElemData(int32_t index) const {
					if (!IsDummy() && index >= 0 && index < ElemDataSize()) {
						return T_AccessData_Elem(kAirAnimDataPtr, ParamRef().ElemDataStart() + index);
					}
					if (!T_Config::Instance().ThrowError()) {
						return T_AccessData_Elem(kAirAnimDataPtr, KSIZE_MAX);
					}
					T_ErrorHandle::Instance().ThrowError(ErrorMessage::AnimIndexNotFound, index);
				}

				T_AccessData_Anim(const T_AirAnimData* const AirAnimDataPtr, const ksize_t DataListIndex) : kAirAnimDataPtr(AirAnimDataPtr), kAnimListIndex(DataListIndex) {}
			};

		public:
			/**
			* @brief AIRデータの画像グループ数を取得
			*
			* 　読み込んだAIRデータの画像グループ数を返します
			*
			* @return int32_t NumGroup 画像グループ数
			*/
			int32_t NumAnim() const noexcept { return NumAnim_; }

			/**
			* @brief AIRデータのファイル名を取得
			*
			* 　読み込んだAIRデータの拡張子を除いたファイル名を返します
			*
			* @return const std::string& FileName ファイル名
			*/
			const std::string& FileName() const noexcept { return FileName_; }

			/**
			* @brief AIRデータの初期化
			*
			* 　読み込んだAIRデータを初期化します
			*
			* @note
			*/
			void clear() {
				NumAnim(0);
				FileName_.clear();
				AnimNumberUMap.clear();
				AirAnimData.clear();
			}

			/**
			* @brief AIRデータの存在確認
			*
			* 　読み込んだAIRデータの空かを判定します
			*
			* @return bool 判定結果 (false = データが空：true = データが存在)
			*/
			bool empty() const noexcept {
				return FileName().empty() && AnimNumberUMap.empty() && AirAnimData.empty();
			}

		public:
			using AnimData = T_AccessData_Anim;
			using ElemData = T_AccessData_Elem;

			T_AIRData() = default;

			T_AIRData(const std::string& FileName, const std::string& FilePath = "")
			{
				LoadAIRFile(FileName, FilePath);
			}

			/**
			* @brief 指定されたAIRファイルを読み込み
			*
			* 　実行ファイルから子階層へファイル名を検索して読み込みます
			*
			* 　第二引数指定時は指定した階層からファイル名を検索します(AIRConfigよりも優先されます)
			*
			* 　実行時に既存の要素は初期化、上書きされます
			*
			* @param const std::string& FileName ファイル名 (拡張子 .air は省略可)
			* @param const std::string& FilePath 対象のパス (省略時は実行ファイルの子階層を探索)
			* @return bool 読み込み結果 (false = 失敗：true = 成功)
			*/
			bool LoadAIR(const std::string& FileName, const std::string& FilePath = "") {
				return LoadAIRFile(FileName, FilePath);
			}

			/**
			* @brief 指定番号の存在確認
			*
			* 　読み込んだAIRデータを検索し、指定番号が存在するかを確認します
			*
			* @param int32_t AnimNumber アニメ番号
			* @return bool 検索結果 (false = 存在なし : true = 存在あり)
			*/
			bool ExistAnimNumber(int32_t AnimNumber) {
				return AnimNumberUMap.exist(AnimNumber);
			}

			/**
			* @brief 指定番号のデータへアクセス
			*
			* 　指定したアニメ番号のAIRデータへアクセスします
			*
			* 　対象が存在しない場合はAIRConfig::SetThrowErrorの設定に準拠します
			*
			* @param int32_t AnimNumber アニメ番号
			* @retval 対象が存在する AnimData
			* @retval 対象が存在しない AIRConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)
			*/
			const AnimData GetAnimData(int32_t AnimNumber) {
				if (int32_t Value = AnimNumberUMap.find(AnimNumber); Value >= 0) { // SpriteExist(GroupNo, ImageNo)と同義
					return AnimData(&AirAnimData, Value);
				}
				if (!T_Config::Instance().ThrowError()) {
					return AnimData(&AirAnimData, KSIZE_MAX);
				}
				T_ErrorHandle::Instance().ThrowError(ErrorMessage::AnimNumberNotFound, AnimNumber);
			}

			/**
			* @brief 指定インデックスデータの存在確認
			*
			* 　読み込んだAIRデータを検索し、指定インデックスのデータ存在するかを確認します
			*
			* @param int32_t index データ配列インデックス
			* @return bool 検索結果 (false = 存在なし : true = 存在あり)
			*/
			bool ExistAnimDataIndex(int32_t AnimDataIndex) const {
				return static_cast<ksize_t>(AnimDataIndex) < AnimNumberUMap.size();
			}

			/**
			* @brief 指定インデックスのデータへアクセス
			*
			* 　AIRデータへ指定したインデックスでアクセスします
			*
			* 　対象が存在しない場合はAIRConfig::SetThrowErrorの設定に準拠します
			*
			* @param int32_t index データ配列インデックス
			* @retval 対象が存在する AnimData
			* @retval 対象が存在しない AIRConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)
			*/
			const AnimData GetAnimDataIndex(int32_t index) const {
				if (ExistAnimDataIndex(index)) {
					return AnimData(&AirAnimData, index);
				}
				if (!T_Config::Instance().ThrowError()) {
					return AnimData(&AirAnimData, KSIZE_MAX);
				}
				T_ErrorHandle::Instance().ThrowError(ErrorMessage::AnimIndexNotFound, index);
			}


		}; // struct T_AIRData
	} // ReadAirFile_detail

	// 使用ユーザー向けの名前設定

	/**
	* @brief AIRファイルを扱うクラス
	*
	* 　- コンストラクタの引数を指定した場合、指定した引数でLoadAIR関数を実行します
	*
	* 　- 引数を指定しない場合、ファイル読み込みは行いません
	*
	* @param const std::string& FileName ファイル名 (拡張子 .air は省略可)
	* @param const std::string& FilePath 対象のパス (省略時は実行ファイルの子階層を探索)
	*/
	using AIR = ReadAirFile_detail::T_AIRData;

	/**
	* @brief ReadAirFileのエラー情報空間
	*/
	namespace AIRError = ReadAirFile_detail::ErrorMessage;

	/**
	* @brief ReadAirFileのコンフィグ設定空間
	*/
	namespace AIRConfig {

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Setter /////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		* @brief エラー出力の切り替え設定
		*
		* 　このライブラリ関数で発生したエラーを例外として投げるかログとして記録するかを指定できます
		*
		* @param bool flag (false = ログとして記録する：true = 例外を投げる)
		*/
		inline void SetThrowError(bool flag) { ReadAirFile_detail::T_Config::Instance().ThrowError(flag); }

		/**
		* @brief エラーログファイルを作成設定
		*
		* 　このライブラリ関数で発生したエラーのログファイルを出力するかどうか指定できます
		*
		* @param bool flag (false = ログファイルを出力しない：true = ログファイルを出力する)
		*/
		inline void SetCreateLogFile(bool flag) { ReadAirFile_detail::T_Config::Instance().CreateLogFile(flag); }

		/**
		* @brief SAELibフォルダを作成設定
		*
		* 　ファイルの出力先としてSAELibファイルを使用するかを指定できます
		*
		* @param bool flag (false = SAELibファイルを使用しない：true = SAELibファイルを使用する)
		* @param const std::string& Path SAELibフォルダ作成先 (省略時はパスの設定なし)
		*/
		inline void SetCreateSAELibFile(bool flag, const std::string& Path = "") {
			ReadAirFile_detail::T_Config::Instance().CreateSAELibFile(flag);
			if (!Path.empty()) {
				ReadAirFile_detail::T_Config::Instance().SAELibFilePath(Path);
			}
		}

		/**
		* @brief SAELibフォルダのパス設定
		*
		* 　SAELibファイルの作成パスを指定できます
		*
		* @param const std::string& Path SAELibフォルダ作成先
		*/
		inline void SetSAELibFilePath(const std::string& Path = "") { ReadAirFile_detail::T_Config::Instance().SAELibFilePath(Path); }

		/**
		* @brief AIRファイルの検索パス設定
		*
		* 　AIRファイルの検索先のパスを指定できます
		*
		* 　AIRコンストラクタもしくはLoadAIR関数で検索先のパスを指定しない場合、この設定のパスで検索します
		*
		* @param const std::string& Path AIRファイルの検索先のパス
		*/
		inline void SetAIRSearchPath(const std::string& Path = "") { ReadAirFile_detail::T_Config::Instance().AIRSearchPath(Path); }

		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Getter /////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		* @brief エラー出力切り替え設定取得
		*
		* 　Config設定のエラー出力切り替え設定を取得します
		*
		* @return bool エラー設定出力切り替え設定(false = OFF：true = ON)
		*/
		inline bool GetThrowError() { return ReadAirFile_detail::T_Config::Instance().ThrowError(); }

		/**
		* @brief エラーログファイルを作成設定取得
		*
		* 　Config設定のエラーログファイルを作成設定を取得します
		*
		* @return bool エラーログファイルを作成設定(false = OFF：true = ON)
		*/
		inline bool GetCreateLogFile() { return ReadAirFile_detail::T_Config::Instance().CreateLogFile(); }

		/**
		* @brief SAELibフォルダを作成設定取得
		*
		* 　Config設定のSAELibフォルダを作成設定を取得します
		*
		* @return bool SAELibフォルダを作成設定(false = OFF：true = ON)
		*/
		inline bool GetCreateSAELibFile() { return ReadAirFile_detail::T_Config::Instance().CreateSAELibFile(); }

		/**
		* @brief Config設定取得
		*
		* 　Config設定のフラグをまとめて取得します
		*
		* @return int32_t Config設定
		*/
		inline int32_t GetConfigFlag() { return ReadAirFile_detail::T_Config::Instance().BitFlag(); }

		/**
		* @brief SAELibフォルダを作成パス取得
		*
		* 　Config設定のSAELibフォルダを作成パスを取得します
		*
		* @return const std::filesystem::path& SAELibフォルダを作成パス
		*/
		inline const std::filesystem::path& GetSAELibFilePath() { return ReadAirFile_detail::T_Config::Instance().SAELibFilePath(); }

		/**
		* @brief AIRファイルの検索パス取得
		*
		* 　Config設定のAIRファイルの検索パスを取得します
		*
		* @return const std::filesystem::path& AIRファイルの検索パス
		*/
		inline const std::filesystem::path& GetAIRSearchPath() { return ReadAirFile_detail::T_Config::Instance().AIRSearchPath(); }
	}
} // namespace SAELib
#endif