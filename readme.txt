-----------------------------------------------------------------------
      透過性ロゴ フィルタ for AviSynth 2.5  ver 0.05  by MakKi
-----------------------------------------------------------------------

【機能】

  BS･CSでよく見かけるような半透明ロゴを付加または除去します。

【書式】

  ・ロゴ付加
        AddLOGO(logofile="", logoname="",
                  pos_x=0, pos_y=0, depth=128,
                  yc_y=0,  yc_u=0,  yc_v=0,
                  start=0, fadein=0, fadeout=0, end=-1,
                  interlaced=false)
  ・ロゴ除去
        EraseLOGO(logofile="", logoname="",
                  pos_x=0, pos_y=0, depth=128,
                  yc_y=0,  yc_u=0,  yc_v=0,
                  start=0, fadein=0, fadeout=0, end=-1,
                  interlaced=false)

     logofile         : ロゴファイル名（省略不可）  *.ldp、*.lgdのどちらも使用できます。
     logoname         : ロゴ名        （省略可）    省略した場合はロゴデータファイルの先頭に保存されているロゴを使用します。
     pos_x, pox_y     : 位置調整                    1/4ピクセル単位に調整します。
     yc_y, yc_u, yc_v : 色調整                      ロゴの色を調整します。
     depth            : 不透明度（深度）調整        128で100%として調整します。
     start, end       : 開始･終了フレーム           フレーム番号で指定してください。end<startの時はstart以降の全てのフレームで実行されます。
     fadein, fadeout  : フェードフレーム数          ロゴをフェードさせます。
     interlaced       : インターレースフラグ        YV12をインターレースとして扱います。YUY2では無視されます。

  start, end, interlaced 以外のパラメータはAviUtl用透過性ロゴフィルタのパラメータと共通です。
  ロゴデータの作成にはAviUtl用ロゴ解析プラグインを使用してください。

  ※YUY2,YV12専用です。必要に応じて呼び出す前に色空間変換してください。
  ※AviSynth 2.0x系で使用するには、warpsharpプラグイン付属のLoadPlginEx.dllを使用してください。
        warpsharp配布場所：http://www.geocities.co.jp/SiliconValley-PaloAlto/2382/


【注意】

  このプログラムはフリーソフトウェアであり、GPLライセンスにて提供されます。
  このプログラムによって損害を負った場合でも、作者は責任を負いません。
　プログラムのソースコードは配布元にて公開しています。

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA,
    or visit http://www.gnu.org/copyleft/gpl.html .


【配布元】

  MakKi's SoftWare
  http://mksoft.hp.infoseek.co.jp/

【更新履歴】

  2008/01/07   ver 0.05  ・ロゴファイルの最大データ数拡張に対応
  2007/06/06   ver 0.04  ・depthが効かない場合があるバグを修正
                         ・yc_y,yc_u,yc_vの効きが弱いバグを修正
  2007/03/21   ver 0.03  ・全部書き直し
                         ・色差の計算での不要な処理を削除
                         ・内部処理をAviUtl版とほぼ同じにした
                         ・YV12に対応
  2004/02/11   ver 0.02  ・ロゴの色の調整機能を追加
  2003/11/05   ver 0.01d ・細かなバグ修正
  2003/11/02   ver 0.01c ・pos_x,pos_yの挙動がおかしかったのを修正
                         ・Cropしてある時ロゴが崩れるバグ修正
  2003/11/01   ver 0.01b ・pos_x,pos_yに負の値を入れても正方向になっていたバグ修正
  2003/10/26   ver 0.01a ・pos_x,pos_yが-200未満の時エラーになることがあったのを修正
  2003/10/01   ver 0.01  ・公開


mailto:makki_d210@yahoo.co.jp
