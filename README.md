# zsmfv SMF ビューア ver1.5
                                        by Zhidao
                                        2001. 8.26. 作成
                                        2020.10.26. 最終更新
------------------------------------------------------------
### 【説明】

SMFの中身を標準出力にダンプするツール。
ZMMLCのデバッグ用に作りました。

------------------------------------------------------------
### 【使い方】

% zsmfv filename

で、標準出力にファイルの内容が書き出されます。出力の書式は公
に定められているものではありませんが、人にとっての可読性を考
えています。

filenameは拡張子（.mid）省略可能です。

% zsmfv -t filename

で、トータル演奏時間のみ表示します。

------------------------------------------------------------
### 【著作権・免責事項】

ZSMFVは、著作権を放棄しないフリーソフトです。
本ソフト及びアーカイブの著作権は作成者であるZhidaoに属します。
配布、転載は自由としますが、アーカイブの内容を許可なく改変し
ないようにお願いします。また、営利目的での使用は御遠慮下さい。
改造は個人での使用に於ては全く自由です。

本ソフトを使用した結果起こったいかなる事故、不都合に関しても
当方は責任を負いかねます。

------------------------------------------------------------
### 【謝辞】

デバッグにご協力頂いたMash氏に感謝します。

------------------------------------------------------------
### 【履歴】

 - 2020.10.26. READMEをmarkdown形式に変更。
 - 2007.12.15. ver1.5。gccの型判別厳格化に対応。
 - 2003. 6.14. ver1.4。トータル演奏時間のみの表示機能追加。
 - 2003. 6.13. ver1.3。コード表示部のバグ修正。
 - 2002. 2.17. ver1.02。読み込みバイトサイズ表示部のバグ修正。
 - 2001.10.13. ver1.01。マルチポートイベントに対応。
 - 2001. 8.26. ver1.00作成。
