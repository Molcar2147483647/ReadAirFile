# ReadAirFile
## 基本仕様
SAEツールで使用される.airファイルを読み取り、アニメリスト等のデータを取得できます  

## 想定環境
Windows11  
C++言語標準：ISO C++17標準  

上記の環境で動作することを確認しています  
上記以外の環境での動作は保証しません  

## クラス/名前空間の概要
### class SAELib::AIR
読み込んだAIRファイルのデータが格納される  
インスタンスを生成して使用する  

### class SAELib::AIR::AnimData
格納されたデータのAnimパラメータを取得する際に使用するクラス  

### class SAELib::AIR::ElemData
格納されたデータのElemパラメータを取得する際に使用するクラス  

### class SAELib::AIRConfig
ReadAirFileライブラリの動作設定が可能  
インスタンス生成不可  

### namespace SAELib::AIRError
本ライブラリが扱うエラー情報のまとめ  
throwされた例外をcatchするために使用する  

## クラス/名前空間の関数一覧
## class SAELib::AIR
### デフォルトコンストラクタ
コンストラクタの引数を指定した場合、指定した引数でLoadAIR関数を実行します  
引数を指定しない場合、ファイル読み込みは行いません  
```
SAELib::AIR air;
```
### 指定されたAIRファイルを読み込み
実行ファイルから子階層へファイル名を検索して読み込みます  
第二引数指定時は指定した階層からファイル名を検索します(AIRConfigよりも優先されます)  
実行時に既存の要素は初期化、上書きされます  
```
air.LoadAIR("kfm.air");                 // 実行ファイルの階層から検索
air.LoadAIR("kfm.air", "C:/MugenData"); // 指定パスから検索
```
引数1 const std::string& FileName ファイル名(拡張子 .air は省略可)  
引数2 const std::string& FilePath 対象のパス(省略時は実行ファイルの子階層を探索)  
戻り値 bool 読み込み結果 (false = 失敗：true = 成功)

### 指定番号の存在確認
読み込んだAIRデータを検索し、指定番号が存在するかを確認します  
```
air.ExistAnimNumber(5150); // アニメ番号5150が存在するか確認
```
引数1 int32_t AnimNumber アニメ番号  
戻り値 bool 検索結果 (false = 存在なし : true = 存在あり)

### 指定番号のデータへのアクセス
指定したアニメ番号のAIRデータへアクセスします  
対象が存在しない場合はAIRConfig::SetThrowErrorの設定に準拠します  
```
air.GetAnimData(5150); // アニメ番号5150のデータを取得
```
引数1 int32_t AnimNumber アニメ番号  
戻り値1 対象が存在する GetAnimData(AnimNumber)のデータ  
戻り値2 対象が存在しない AIRConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)  

### 指定インデックスデータの存在確認
読み込んだAIRデータを検索し、指定インデックスのデータ存在するかを確認します  
```
air.ExistAnimDataIndex(0); // 0番目のデータが存在するか確認
```
引数1 int32_t index データ配列インデックス  
戻り値 bool 検索結果 (false = 存在なし : true = 存在あり)

### 指定インデックスのデータへアクセス
AIRデータへ指定したインデックスでアクセスします  
対象が存在しない場合はAIRConfig::SetThrowErrorの設定に準拠します  
```
air.GetAnimDataIndex(0); // 0番目のデータを取得
```
引数1 int32_t index データ配列インデックス  
戻り値1 対象が存在する GetAnimDataIndex(index)のデータ  
戻り値2 対象が存在しない AIRConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)  

### AIRデータのアニメ数を取得
読み込んだAIRデータのアニメ数を返します  
```
air.NumAnim(); // アニメ数を取得
```
戻り値 int32_t NumAnim アニメ数  

### AIRデータのファイル名を取得
読み込んだAIRデータの拡張子を除いたファイル名を返します  
```
air.FileName(); // ファイル名を取得
```
戻り値 const std::string& FileName ファイル名  

### AIRデータの初期化
読み込んだAIRデータを初期化します  
```
air.clear(); // AIRデータの初期化
```
戻り値 なし(void)  

### AIRデータの存在確認
読み込んだAIRデータの空かを判定します  
```
air.empty(); // AIRデータの存在確認
```
戻り値 bool 判定結果 (false = データが存在：true = データが空)  

### AIRデータのデータサイズを取得
読み込んだAIRデータのデータサイズを返します  
```
air.size(); // AIRデータサイズを取得
```
戻り値 size_t AIRDataSize AIRデータサイズ 

## class SAELib::AIR::AnimData
### ダミーデータ判断
自身がダミーデータであるかを確認します  
AIRConfig::SetThrowErrorの設定がOFFの場合にエラー回避のために使用されます  
```
air.GetAnimData(XXX).IsDummy(); // ダミーデータ判断
```
戻り値 bool 判定結果 (false = 自身が正常なデータ：true = 自身がダミーデータ)  

### アニメ番号の取得
SAEで設定したアニメ番号を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).AnimNumber(); // アニメ番号を取得
```
戻り値 int32_t AnimNumber アニメ番号 

### ループ開始位置の存在確認
SAEで設定したアニメのループ開始位置が存在するかを確認します  
ダミーデータの場合は false を返します  
```
air.GetAnimData(XXX).ExistLoopstart(); // ループ開始位置の存在確認
```
戻り値 bool 判定結果 (false = 存在なし : true = 存在あり)

### ループ開始位置の取得
SAEで設定したアニメのループ開始位置を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).ElemLoopstart(); // ループ開始位置の取得
```
戻り値 int32_t ElemLoopstart ループ開始位置

### アニメ枚数の取得
SAEで設定したアニメ枚数を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).ElemDataSize(); // アニメ枚数の取得
```
戻り値 int32_t ElemDataSize アニメ枚数  

### 指定インデックスのデータへアクセス
AIRデータへ指定したインデックスでアクセスします  
対象が存在しない場合はAIRConfig::SetThrowErrorの設定に準拠します  
```
air.GetAnimData(XXX).GetElemData(YYY); // アニメ番号XXXのYYY枚数目のデータを取得
```
引数1 int32_t index データ配列インデックス  
戻り値1 対象が存在する GetElemData(index)のデータ  
戻り値2 対象が存在しない AIRConfig::SetThrowError (false = ダミーデータの参照：true = 例外を投げる)  

## class SAELib::AIR::ElemData
### ダミーデータ判断
自身がダミーデータであるかを確認します  
AIRConfig::SetThrowErrorの設定がOFFの場合にエラー回避のために使用されます  
```
air.GetAnimData(XXX).GetElemData(YYY).IsDummy(); // ダミーデータ判断
```
戻り値 bool 判定結果 (false = 自身が正常なデータ：true = 自身がダミーデータ)  

### グループ番号の取得
SAEで設定したグループ番号を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).GroupNo(); // アニメ番号XXXのYYY枚目のグループ番号を取得
```
戻り値 int32_t GroupNo グループ番号

### イメージ番号の取得
SAEで設定したイメージ番号を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).ImageNo(); // アニメ番号XXXのYYY枚目のイメージ番号を取得
```
戻り値 int32_t ImageNo イメージ番号

### X座標の取得
SAEで設定したX座標を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).PosX(); // アニメ番号XXXのYYY枚目のX座標を取得
```
戻り値 int32_t PosX X座標

### Y座標の取得
SAEで設定したY座標を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).PosY(); // アニメ番号XXXのYYY枚目のY座標を取得
```
戻り値 int32_t PosY Y座標

### フレーム表示時間の取得
SAEで設定したフレーム表示時間を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).ElemTime(); // アニメ番号XXXのYYY枚目のフレーム表示時間を取得
```
戻り値 int32_t ElemTime フレーム表示時間

### 水平方向の取得
SAEで設定した水平方向を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).Facing(); // アニメ番号XXXのYYY枚目の水平方向を取得
```
戻り値 int32_t Facing 水平方向 (1 = 正方向： -1 = 負方向)

### 垂直方向の取得
SAEで設定した垂直方向を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).VFacing(); // アニメ番号XXXのYYY枚目の垂直方向を取得
```
戻り値 int32_t VFacing 垂直方向 (1 = 正方向： -1 = 負方向)

### アルファ値Aの取得
SAEで設定したアルファ値Aを返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).AlphaA(); // アニメ番号XXXのYYY枚目のアルファ値Aを取得
```
戻り値 int32_t AlphaA アルファ値A

### アルファ値Sの取得
SAEで設定したアルファ値Sを返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).AlphaS(); // アニメ番号XXXのYYY枚目のアルファ値Sを取得
```
戻り値 int32_t AlphaS アルファ値S

### アルファ値Dの取得
SAEで設定したアルファ値Dを返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).GetElemData(YYY).AlphaD(); // アニメ番号XXXのYYY枚目のアルファ値Dを取得
```
戻り値 int32_t AlphaD アルファ値D

## class SAELib::AIRConfig
### エラー出力切り替え設定/取得
このライブラリ関数で発生したエラーを例外として投げるかログとして記録するかを指定できます  
```
SAELib::AIRConfig::SetThrowError(bool flag); // エラー出力切り替え設定
```
引数1 bool (false = ログとして記録する：true = 例外を投げる)  
戻り値 なし(void)  
```
SAELib::AIRConfig::GetThrowError(); // エラー出力切り替え設定を取得
```
戻り値 bool (false = ログとして記録する：true = 例外を投げる)

### エラーログファイルを作成設定/取得
このライブラリ関数で発生したエラーのログファイルを作成するかどうか指定できます  
```
SAELib::AIRConfig::SetCreateLogFile(bool flag); // エラーログファイルを作成設定
```
引数1 bool (false = ログファイルを作成しない：true = ログファイルを作成する)  
戻り値 なし(void)  
```
SAELib::AIRConfig::GetCreateLogFile(); // エラーログファイルを作成設定を取得  
```
戻り値 bool (false = ログファイルを作成しない：true = ログファイルを作成する)  

### SAELibフォルダを作成設定/取得
ファイルの出力先としてSAELibファイルを使用するかを指定できます  
```
SAELib::AIRConfig::SetCreateSAELibFile(bool flag, const std::string& Path = ""); // SAELibフォルダを作成設定
```
引数1 bool (false = SAELibファイルを使用しない：true = SAELibファイルを使用する)  
引数2 const std::string& SAELibフォルダ作成先(省略時はパスの設定なし)  
戻り値 なし(void)  
```
SAELib::AIRConfig::GetCreateSAELibFile(); // SAELibフォルダを作成設定を取得  
```
戻り値 bool (false = SAELibファイルを使用しない：true = SAELibファイルを使用する)  

### SAELibフォルダのパス設定/取得
SAELibファイルの作成パスを指定できます  
```
SAELib::AIRConfig::SetSAELibFilePath(const std::string& Path = ""); // SAELibフォルダのパス設定
```
引数1 const std::string& SAELibFilePath SAELibフォルダ作成先  
戻り値 なし(void)  
```
SAELib::AIRConfig::GetSAELibFilePath(); // SAELibフォルダを作成パス取得  
```
戻り値 const std::string& SAELibFilePath SAELibフォルダ作成先  

### AIRファイルの検索パス設定/取得
AIRファイルの検索先のパスを指定できます  
AIRコンストラクタもしくはLoadAIR関数で検索先のパスを指定しない場合、この設定のパスで検索します  
```
SAELib::AIRConfig::SetAIRSearchPath(const std::string& Path = ""); // AIRファイルの検索パス設定  
```
引数1 const std::string& AIRSearchPath AIRファイルの検索先のパス  
戻り値 なし(void)  
```
SAELib::AIRConfig::GetAIRSearchPath(); // AIRファイルの検索パス取得  
```
戻り値 const std::string& AIRSearchPath AIRファイルの検索先のパス  

## namespace SAELib::AIRError
### エラーID情報  
このライブラリが出力するエラーIDのenumです  
```
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
  AirFileReadFailed,
  FromCharsConvertFailed,
};
```

### エラー情報配列
このライブラリが出力するエラー情報の配列です  
```
struct T_ErrorInfo {
public:
  const int32_t ID;
  const char* const Name;
  const char* const Message;
};
      
constexpr T_ErrorInfo ErrorInfo[] = {
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
  { AirFileReadFailed,			"AirFileReadFailed",			"AIRファイルの読み取り中にエラーが発生しました" },
  { FromCharsConvertFailed,		"FromCharsConvertFailed",		"取得した文字列の変換に失敗しました" },
};

```

### エラー情報のサイズ取得
エラー情報の配列サイズを取得します　　
```
SAELib::AIRError::ErrorInfoSize; // エラー情報配列サイズ
```
戻り値 size_t ErrorInfoSize エラー情報配列サイズ

### エラー名取得
エラーIDに応じたエラー名を取得します  
```
SAELib::AIRError::ErrorName(ErrorID); // ErrorIDのエラー名を取得
```
引数1 int32_t ErrorID エラーID  
戻り値 const char* ErrorName エラー名  

### エラーメッセージ取得
エラーIDに応じたエラーメッセージを取得します  
```
SAELib::AIRError::ErrorMessage(ErrorID); // ErrorIDのエラーメッセージを取得
```
引数1 int32_t ErrorID エラーID  
戻り値 const char* ErrorMessage エラーメッセージ  

## 使用例
```
#include "h_ReadAirFile.h"
#include <iostream> // 標準入力/出力

int main()
{
	SAELib::AIRConfig::SetThrowError(false);		// このライブラリで発生したエラーを例外として処理しない
	SAELib::AIRConfig::SetCreateSAELibFile(false);	// SAELibファイルの作成を許可しない
	SAELib::AIRConfig::SetSAELibFilePath();			// SAELibファイルの作成階層を指定
	SAELib::AIRConfig::SetCreateLogFile(false);		// このライブラリで発生したエラーログの作成を許可する
	SAELib::AIRConfig::SetAIRSearchPath("../../");	// AIRファイルの検索開始階層を指定

	// kfmのairファイルを読み込む
	SAELib::AIR air("kfm");

	// アニメ番号5150が存在するか確認
	if (air.ExistAnimNumber(5150)) {
		// アニメ番号5150の1枚目の画像番号を取得
		air.GetAnimData(5150).GetElemData(1).GroupNo();
		air.GetAnimData(5150).GetElemData(1).ImageNo();
	}

	// すべてのアニメ情報を出力
	for (int32_t AnimIndex = 0; air.NumAnim() > AnimIndex; ++AnimIndex) {
		std::cout << "\nExist: " << air.ExistAnimDataIndex(AnimIndex) << std::endl;
		std::cout << "Number: " << air.GetAnimDataIndex(AnimIndex).AnimNumber() << std::endl;
		std::cout << "ElemLoopstart: " << air.GetAnimDataIndex(AnimIndex).ElemLoopstart() << std::endl;
		std::cout << "ExistLoopstart: " << air.GetAnimDataIndex(AnimIndex).ExistLoopstart() << std::endl;
		std::cout << "ElemDataSize: " << air.GetAnimDataIndex(AnimIndex).ElemDataSize() << std::endl;
		for (int32_t i = 0; i < air.GetAnimDataIndex(AnimIndex).ElemDataSize(); ++i) {
			std::cout << "\nElem: " << i << std::endl;
			std::cout << "GroupNo: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).GroupNo() << std::endl;
			std::cout << "ImageNo: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).ImageNo() << std::endl;
			std::cout << "PosX: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).PosX() << std::endl;
			std::cout << "PosY: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).PosY() << std::endl;
			std::cout << "ElemTime: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).ElemTime() << std::endl;
			std::cout << "Facing: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).Facing() << std::endl;
			std::cout << "VFacing: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).VFacing() << std::endl;
			std::cout << "AlphaA: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).AlphaA() << std::endl;
			std::cout << "AlphaS: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).AlphaS() << std::endl;
			std::cout << "AlphaD: " << air.GetAnimDataIndex(AnimIndex).GetElemData(i).AlphaD() << std::endl;
		}
	}

	// 存在しない番号はダミーデータに
	std::cout << air.GetAnimData(-666).GetElemData(-666).IsDummy() << std::endl;

	return 0;
}
```
