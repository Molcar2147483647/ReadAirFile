# ReadAirFile
## 基本仕様
SAEツールで使用される.airファイルを読み取り、スプライトリスト等のデータを取得できます  

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

## class SAELib::AIR::AnimData
### ダミーデータ判断
自身がダミーデータであるかを確認します  
AIRConfig::SetThrowErrorの設定がOFFの場合にエラー回避のために使用されます  
```
air.GetAnimData(XXX).IsDummy(); // ダミーデータ判断
```
戻り値 bool 判定結果 (false = 自身が正常なデータ：true = 自身がダミーデータ))  

### アニメ番号の取得
SAEで設定したアニメ番号を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).AnimNumber(); // アニメ番号を取得
```
戻り値 int32_t AnimNumber アニメ番号 

### ループ開始位置の取得
SAEで設定したループ開始位置を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).Loopstart(); // ループ開始位置
```
戻り値 int32_t Loopstart ループ開始位置  

### アニメ枚数の取得
SAEで設定したアニメ枚数を返します  
ダミーデータの場合は 0 を返します  
```
air.GetAnimData(XXX).ElemDataSize(); // アニメ枚数
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
戻り値 bool 判定結果 (false = 自身が正常なデータ：true = 自身がダミーデータ))  

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










