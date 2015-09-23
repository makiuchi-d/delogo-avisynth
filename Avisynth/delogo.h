//
//	Copylight (C) 2003 MakKi
//
//	AvisynthがGPLなので、このソフトもGPLにします。
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .
//
//
/** @file
 *	@brief 基底クラス
 */
#ifndef ___DELOGO_H
#define ___DELOGO_H

#include <windows.h>
#include <math.h>
#include "avisynth.h"
#include "../logo.h"

enum {
	LOGO_FADE_MAX = 256,	/// フェード時の不透明度最大値
	LOGO_DEFAULT_DEPTH = 128,	/// 不透明度（深度）初期値
};

/*---------------------------------------------------------------------------*/
/** TYPE
 *@{
 */
/// for EraseLOGO
struct Erase {
	static const char *Name(void){ return "EraseLOGO"; }
};
/// for AddLOGO
struct Add {
	static const char *Name(void){ return "AddLOGO"; }
};

//@}
/*****************************************************************************/
/** 基底クラス
 */
template <class TYPE,class COLOR>
class deLOGO : public GenericVideoFilter {
protected:
	/// 配列ポインタ管理用auto_ptr
	template<class T>
	class aptr{
		T *ptr;
	public:
		aptr(void):ptr(NULL){}
		aptr(T *t):ptr(t){}
		~aptr(){ delete[] ptr; }
		aptr<T> &operator =(T *t){
			delete[] ptr; ptr=t; return *this;
		}
		aptr<T> &operator =(aptr<T> &t){
			delete[] ptr; ptr=t.ptr; t.ptr=NULL; return *this;
		}
		T* get(void){ return ptr; }
		aptr<T> &ref(void){ return *this; }
		bool operator ==(T* t){ return ptr==t; }
		bool operator !(void){ return !ptr; }
	};
	::LOGO_HEADER lgh;
	aptr<typename COLOR::LOGO_PIXEL> logodata;
	int start, fadein, fadeout, end;

public:
	deLOGO(const PClip &clip,const char *logofile,const char *logoname,
				int pos_x,int pos_y,int depth,int y,int u,int v,
				int _start,int _fadein,int _fadeout,int _end,
				IScriptEnvironment *env)
		: GenericVideoFilter(clip), logodata(NULL),
		  start(_start),fadein(_fadein),fadeout(_fadeout),end(_end)
	{
		const VideoInfo &vi = clip->GetVideoInfo();
		child->SetCacheHints(CACHE_NOTHING,0);

		try{
			aptr<::LOGO_PIXEL> srcdata;
			// 読み込み
			srcdata = ReadLogoData(logofile,logoname);
			// 位置、深度調節
			if(pos_x!=0 || pos_y!=0 || depth!=LOGO_DEFAULT_DEPTH){
				AdjustLogo(srcdata.ref(),pos_x,pos_y,depth);
			}
			// 色調節
			if(y || u || v){
				ColorTuning(srcdata.ref(),y,u,v);
			}

			// COLOR::Convertはロゴデータをnew[]して返す
			logodata = COLOR::Convert(srcdata.get(),lgh);
		}
		catch(const char *err){
			env->ThrowError("%s: %s",TYPE::Name(),err);
		}
	}
	~deLOGO(){}

	PVideoFrame __stdcall GetFrame(int n,IScriptEnvironment *env);

protected:
	/// ロゴデータファイル読み込み
	::LOGO_PIXEL *ReadLogoData(const char *logofile,const char *logoname)
	{
		class FILE_HANDLE{
			HANDLE h;
		public:
			FILE_HANDLE(void):h(INVALID_HANDLE_VALUE){}
			FILE_HANDLE(HANDLE _h):h(_h){}
			~FILE_HANDLE(){
				CloseHandle(h);
			}
			operator HANDLE(void){ return h; }
			operator bool(void){ return h!=INVALID_HANDLE_VALUE; }
			bool operator==(HANDLE hh){ return h==hh; }
			bool operator!=(HANDLE hh){ return h!=hh; }
		};

		if(logofile==NULL){
			throw "The logofile is not specified. - ロゴデータファイルが指定されていません";
		}

		// ファイルを開く
		FILE_HANDLE hfile = CreateFile(logofile,GENERIC_READ,0,0,OPEN_EXISTING,
									   FILE_ATTRIBUTE_NORMAL,NULL);
		if(hfile==INVALID_HANDLE_VALUE){
			throw "The logofile is not found. - ロゴデータファイルが見つかりません";
		}
		if(GetFileSize(hfile,NULL) < sizeof(LOGO_HEADER)+LOGO_FILE_HEADER_STR_SIZE){
			throw "The logo file is irregular. -  ロゴデータファイルが不正です";
		}

		// ロゴデータ数取得
		unsigned char num;
		DWORD readed = 0;
		SetFilePointer(hfile,LOGO_FILE_HEADER_STR_SIZE,0,FILE_BEGIN);
		ReadFile(hfile,&num,1,&readed,NULL);
		if(readed!=1){
			throw "Failed in reading logofile. - ロゴデータの読み込みに失敗しました";
		}

		// 該当ロゴを探す
		int i;
		for(i=num;i;--i){
			// LOGO_HEADER読み込み
			readed = 0;
			ReadFile(hfile,&lgh,sizeof(LOGO_HEADER),&readed,NULL);
			if(readed!=sizeof(LOGO_HEADER)){
				throw "Failed in reading logofile. - ロゴデータの読み込みに失敗しました";
			}

			//指定ロゴ発見
			if(logoname==NULL || lstrcmp(logoname,lgh.name)==0){
				break;
			}

			// 次へ
			SetFilePointer(hfile,LOGO_PIXELSIZE(&lgh),0,FILE_CURRENT);
		}
		if(i==0){
			throw "The specified data is not found. - 指定されたロゴデータが見つかりません";
		}

		// メモリ確保
		LOGO_PIXEL *ptr = new ::LOGO_PIXEL[lgh.h * lgh.w];
		if(ptr==NULL){
			throw "Failed in memory allocation. - メモリ確保に失敗しました";
		}

		// データ読み込み
		memset(ptr,0,LOGO_PIXELSIZE(&lgh));
		ReadFile(hfile,ptr,LOGO_PIXELSIZE(&lgh),&readed,NULL);

		return ptr;
	}

	/// 位置・深度調節
	void AdjustLogo(aptr<::LOGO_PIXEL> &logodata,int x,int y,int depth)
	{

		int adjx,adjy;
		if(x>=0){
			lgh.x = lgh.x + int(x/4);
			adjx = x %4;
		}
		else{
			lgh.x = lgh.x + int((x-3)/4);
			adjx = (4+ (x%4)) %4;
		}
		if(y>=0){
			lgh.y = lgh.y + int(y/4);
			adjy = y % 4;
		}
		else{
			lgh.y = lgh.y + int((y-3)/4);
			adjy = (4+ (y%4)) %4;
		}

		if(adjx==0 && adjy==0) return;

		int pitch = lgh.w;
		// 1/4単位調節のため、1増やす
		int w = ++lgh.w;
		int h = ++lgh.h;

		aptr<::LOGO_PIXEL> dstdata;
		dstdata = new ::LOGO_PIXEL[(lgh.h+1) * (lgh.w+1)];
		if(dstdata==NULL){
			throw "Failed on memory allocation. - メモリ確保に失敗しました";
		}

		::LOGO_PIXEL *df = logodata.get();
		::LOGO_PIXEL *ex = dstdata.get();
		int i,j;

		//----------------------------------------------------- 一番上の列
		ex[0].dp_y  = df[0].dp_y *(4-adjx)*(4-adjy)*depth/128/16;	// 左端
		ex[0].dp_cb = df[0].dp_cb*(4-adjx)*(4-adjy)*depth/128/16;
		ex[0].dp_cr = df[0].dp_cr*(4-adjx)*(4-adjy)*depth/128/16;
		ex[0].y  = df[0].y;
		ex[0].cb = df[0].cb;
		ex[0].cr = df[0].cr;
		for(i=1;i<w-1;i++){									//中
			// Y
			ex[i].dp_y = (df[i-1].dp_y*adjx*(4-adjy)
						  + df[i].dp_y*(4-adjx)*(4-adjy)) *depth/128/16;
			if(ex[i].dp_y)
				ex[i].y  = (df[i-1].y*Abs(df[i-1].dp_y)*adjx*(4-adjy)
							+ df[i].y * Abs(df[i].dp_y)*(4-adjx)*(4-adjy))
					/(Abs(df[i-1].dp_y)*adjx*(4-adjy) + Abs(df[i].dp_y)*(4-adjx)*(4-adjy));
			// Cb
			ex[i].dp_cb = (df[i-1].dp_cb*adjx*(4-adjy)
						   + df[i].dp_cb*(4-adjx)*(4-adjy)) *depth/128/16;
			if(ex[i].dp_cb)
				ex[i].cb = (df[i-1].cb*Abs(df[i-1].dp_cb)*adjx*(4-adjy)
							+ df[i].cb * Abs(df[i].dp_cb)*(4-adjx)*(4-adjy))
					/ (Abs(df[i-1].dp_cb)*adjx*(4-adjy)+Abs(df[i].dp_cb)*(4-adjx)*(4-adjy));
			// Cr
			ex[i].dp_cr = (df[i-1].dp_cr*adjx*(4-adjy)
						   + df[i].dp_cr*(4-adjx)*(4-adjy)) *depth/128/16;
			if(ex[i].dp_cr)
				ex[i].cr = (df[i-1].cr*Abs(df[i-1].dp_cr)*adjx*(4-adjy)
							+ df[i].cr * Abs(df[i].dp_cr)*(4-adjx)*(4-adjy))
					/ (Abs(df[i-1].dp_cr)*adjx*(4-adjy)+Abs(df[i].dp_cr)*(4-adjx)*(4-adjy));
		}
		ex[i].dp_y  = df[i-1].dp_y * adjx *(4-adjy)*depth/128/16;	// 右端
		ex[i].dp_cb = df[i-1].dp_cb* adjx *(4-adjy)*depth/128/16;
		ex[i].dp_cr = df[i-1].dp_cr* adjx *(4-adjy)*depth/128/16;
		ex[i].y  = df[i-1].y;
		ex[i].cb = df[i-1].cb;
		ex[i].cr = df[i-1].cr;

		//----------------------------------------------------------- 中
		for(j=1;j<h-1;j++){
			// 輝度Y		//---------------------- 左端
			ex[j*w].dp_y = (df[(j-1)*pitch].dp_y*(4-adjx)*adjy
							+ df[j*pitch].dp_y*(4-adjx)*(4-adjy)) *depth/128/16;
			if(ex[j*w].dp_y)
				ex[j*w].y = (df[(j-1)*pitch].y*Abs(df[(j-1)*pitch].dp_y)*(4-adjx)*adjy
							 + df[j*pitch].y*Abs(df[j*pitch].dp_y)*(4-adjx)*(4-adjy))
					/ (Abs(df[(j-1)*pitch].dp_y)*(4-adjx)*adjy+Abs(df[j*pitch].dp_y)*(4-adjx)*(4-adjy));
			// 色差(青)Cb
			ex[j*w].dp_cb = (df[(j-1)*pitch].dp_cb*(4-adjx)*adjy
							 + df[j*pitch].dp_cb*(4-adjx)*(4-adjy)) *depth/128/ 16;
			if(ex[j*w].dp_cb)
				ex[j*w].cb = (df[(j-1)*pitch].cb*Abs(df[(j-1)*pitch].dp_cb)*(4-adjx)*adjy
							  + df[j*pitch].cb*Abs(df[j*pitch].dp_cb)*(4-adjx)*(4-adjy))
					/ (Abs(df[(j-1)*pitch].dp_cb)*(4-adjx)*adjy+Abs(df[j*pitch].dp_cb)*(4-adjx)*(4-adjy));
			// 色差(赤)Cr
			ex[j*w].dp_cr = (df[(j-1)*pitch].dp_cr*(4-adjx)*adjy
							 + df[j*pitch].dp_cr*(4-adjx)*(4-adjy)) *depth/128/ 16;
			if(ex[j*w].dp_cr)
				ex[j*w].cr = (df[(j-1)*pitch].cr*Abs(df[(j-1)*pitch].dp_cr)*(4-adjx)*adjy
							  + df[j*pitch].cr*Abs(df[j*pitch].dp_cr)*(4-adjx)*(4-adjy))
					/ (Abs(df[(j-1)*pitch].dp_cr)*(4-adjx)*adjy+Abs(df[j*pitch].dp_cr)*(4-adjx)*(4-adjy));
			for(i=1;i<w-1;i++){	//------------------ 中
				// Y
				ex[j*w+i].dp_y = (df[(j-1)*pitch+i-1].dp_y*adjx*adjy
								  + df[(j-1)*pitch+i].dp_y*(4-adjx)*adjy
								  + df[j*pitch+i-1].dp_y*adjx*(4-adjy)
								  + df[j*pitch+i].dp_y*(4-adjx)*(4-adjy) ) *depth/128/16;
				if(ex[j*w+i].dp_y)
					ex[j*w+i].y = (df[(j-1)*pitch+i-1].y*Abs(df[(j-1)*pitch+i-1].dp_y)*adjx*adjy
								   + df[(j-1)*pitch+i].y*Abs(df[(j-1)*pitch+i].dp_y)*(4-adjx)*adjy
								   + df[j*pitch+i-1].y*Abs(df[j*pitch+i-1].dp_y)*adjx*(4-adjy)
								   + df[j*pitch+i].y*Abs(df[j*pitch+i].dp_y)*(4-adjx)*(4-adjy) )
						/ (Abs(df[(j-1)*pitch+i-1].dp_y)*adjx*adjy + Abs(df[(j-1)*pitch+i].dp_y)*(4-adjx)*adjy
						   + Abs(df[j*pitch+i-1].dp_y)*adjx*(4-adjy)+Abs(df[j*pitch+i].dp_y)*(4-adjx)*(4-adjy));
				// Cb
				ex[j*w+i].dp_cb = (df[(j-1)*pitch+i-1].dp_cb*adjx*adjy
								   + df[(j-1)*pitch+i].dp_cb*(4-adjx)*adjy
								   + df[j*pitch+i-1].dp_cb*adjx*(4-adjy)
								   + df[j*pitch+i].dp_cb*(4-adjx)*(4-adjy) ) *depth/128/16;
				if(ex[j*w+i].dp_cb)
					ex[j*w+i].cb = (df[(j-1)*pitch+i-1].cb*Abs(df[(j-1)*pitch+i-1].dp_cb)*adjx*adjy
									+ df[(j-1)*pitch+i].cb*Abs(df[(j-1)*pitch+i].dp_cb)*(4-adjx)*adjy
									+ df[j*pitch+i-1].cb*Abs(df[j*pitch+i-1].dp_cb)*adjx*(4-adjy)
									+ df[j*pitch+i].cb*Abs(df[j*pitch+i].dp_cb)*(4-adjx)*(4-adjy) )
						/ (Abs(df[(j-1)*pitch+i-1].dp_cb)*adjx*adjy + Abs(df[(j-1)*pitch+i].dp_cb)*(4-adjx)*adjy
						   + Abs(df[j*pitch+i-1].dp_cb)*adjx*(4-adjy) + Abs(df[j*pitch+i].dp_cb)*(4-adjx)*(4-adjy));
				// Cr
				ex[j*w+i].dp_cr = (df[(j-1)*pitch+i-1].dp_cr*adjx*adjy
								   + df[(j-1)*pitch+i].dp_cr*(4-adjx)*adjy
								   + df[j*pitch+i-1].dp_cr*adjx*(4-adjy)
								   + df[j*pitch+i].dp_cr*(4-adjx)*(4-adjy) ) *depth/128/16;
				if(ex[j*w+i].dp_cr)
					ex[j*w+i].cr = (df[(j-1)*pitch+i-1].cr*Abs(df[(j-1)*pitch+i-1].dp_cr)*adjx*adjy
									+ df[(j-1)*pitch+i].cr*Abs(df[(j-1)*pitch+i].dp_cr)*(4-adjx)*adjy
									+ df[j*pitch+i-1].cr*Abs(df[j*pitch+i-1].dp_cr)*adjx*(4-adjy)
									+ df[j*pitch+i].cr*Abs(df[j*pitch+i].dp_cr)*(4-adjx)*(4-adjy) )
						/ (Abs(df[(j-1)*pitch+i-1].dp_cr)*adjx*adjy +Abs(df[(j-1)*pitch+i].dp_cr)*(4-adjx)*adjy
						   + Abs(df[j*pitch+i-1].dp_cr)*adjx*(4-adjy)+Abs(df[j*pitch+i].dp_cr)*(4-adjx)*(4-adjy));
			}
			// Y		//----------------------- 右端
			ex[j*w+i].dp_y = (df[(j-1)*pitch+i-1].dp_y*adjx*adjy
							  + df[j*pitch+i-1].dp_y*adjx*(4-adjy)) *depth/128/ 16;
			if(ex[j*w+i].dp_y)
				ex[j*w+i].y = (df[(j-1)*pitch+i-1].y*Abs(df[(j-1)*pitch+i-1].dp_y)*adjx*adjy
							   + df[j*pitch+i-1].y*Abs(df[j*pitch+i-1].dp_y)*adjx*(4-adjy))
					/ (Abs(df[(j-1)*pitch+i-1].dp_y)*adjx*adjy+Abs(df[j*pitch+i-1].dp_y)*adjx*(4-adjy));
			// Cb
			ex[j*w+i].dp_cb = (df[(j-1)*pitch+i-1].dp_cb*adjx*adjy
							   + df[j*pitch+i-1].dp_cb*adjx*(4-adjy)) *depth/128/ 16;
			if(ex[j*w+i].dp_cb)
				ex[j*w+i].cb = (df[(j-1)*pitch+i-1].cb*Abs(df[(j-1)*pitch+i-1].dp_cb)*adjx*adjy
								+ df[j*pitch+i-1].cb*Abs(df[j*pitch+i-1].dp_cb)*adjx*(4-adjy))
					/ (Abs(df[(j-1)*pitch+i-1].dp_cb)*adjx*adjy+Abs(df[j*pitch+i-1].dp_cb)*adjx*(4-adjy));
			// Cr
			ex[j*w+i].dp_cr = (df[(j-1)*pitch+i-1].dp_cr*adjx*adjy
							   + df[j*pitch+i-1].dp_cr*adjx*(4-adjy)) *depth/128/ 16;
			if(ex[j*w+i].dp_cr)
				ex[j*w+i].cr = (df[(j-1)*pitch+i-1].cr*Abs(df[(j-1)*pitch+i-1].dp_cr)*adjx*adjy
								+ df[j*pitch+i-1].cr*Abs(df[j*pitch+i-1].dp_cr)*adjx*(4-adjy))
					/ (Abs(df[(j-1)*pitch+i-1].dp_cr)*adjx*adjy+Abs(df[j*pitch+i-1].dp_cr)*adjx*(4-adjy));
		}
		//--------------------------------------------------------- 一番下
		ex[j*w].dp_y  = df[(j-1)*pitch].dp_y *(4-adjx)*adjy *depth/128/16;	// 左端
		ex[j*w].dp_cb = df[(j-1)*pitch].dp_cb*(4-adjx)*adjy *depth/128/16;
		ex[j*w].dp_cr = df[(j-1)*pitch].dp_cr*(4-adjx)*adjy *depth/128/16;
		ex[j*w].y  = df[(j-1)*pitch].y;
		ex[j*w].cb = df[(j-1)*pitch].cb;
		ex[j*w].cr = df[(j-1)*pitch].cr;
		for(i=1;i<w-1;i++){		// 中
			// Y
			ex[j*w+i].dp_y = (df[(j-1)*pitch+i-1].dp_y * adjx * adjy
							  + df[(j-1)*pitch+i].dp_y * (4-adjx) *adjy) *depth/128/16;
			if(ex[j*w+i].dp_y)
				ex[j*w+i].y = (df[(j-1)*pitch+i-1].y*Abs(df[(j-1)*pitch+i-1].dp_y)*adjx*adjy
							   + df[(j-1)*pitch+i].y*Abs(df[(j-1)*pitch+i].dp_y)*(4-adjx)*adjy)
					/ (Abs(df[(j-1)*pitch+i-1].dp_y)*adjx*adjy +Abs(df[(j-1)*pitch+i].dp_y)*(4-adjx)*adjy);
			// Cb
			ex[j*w+i].dp_cb = (df[(j-1)*pitch+i-1].dp_cb * adjx * adjy
							   + df[(j-1)*pitch+i].dp_cb * (4-adjx) *adjy) *depth/128/16;
			if(ex[j*w+i].dp_cb)
				ex[j*w+i].cb = (df[(j-1)*pitch+i-1].cb*Abs(df[(j-1)*pitch+i-1].dp_cb)*adjx*adjy
								+ df[(j-1)*pitch+i].cb*Abs(df[(j-1)*pitch+i].dp_cb)*(4-adjx)*adjy )
					/ (Abs(df[(j-1)*pitch+i-1].dp_cb)*adjx*adjy +Abs(df[(j-1)*pitch+i].dp_cb)*(4-adjx)*adjy);
			// Cr
			ex[j*w+i].dp_cr = (df[(j-1)*pitch+i-1].dp_cr * adjx * adjy
							   + df[(j-1)*pitch+i].dp_cr * (4-adjx) *adjy) *depth/128/16;
			if(ex[j*w+i].dp_cr)
				ex[j*w+i].cr = (df[(j-1)*pitch+i-1].cr*Abs(df[(j-1)*pitch+i-1].dp_cr)*adjx*adjy
								+ df[(j-1)*pitch+i].cr*Abs(df[(j-1)*pitch+i].dp_cr)*(4-adjx)*adjy)
					/ (Abs(df[(j-1)*pitch+i-1].dp_cr)*adjx*adjy +Abs(df[(j-1)*pitch+i].dp_cr)*(4-adjx)*adjy);
		}
		ex[j*w+i].dp_y  = df[(j-1)*pitch+i-1].dp_y *adjx*adjy *depth/128/16;	// 右端
		ex[j*w+i].dp_cb = df[(j-1)*pitch+i-1].dp_cb*adjx*adjy *depth/128/16;
		ex[j*w+i].dp_cr = df[(j-1)*pitch+i-1].dp_cr*adjx*adjy *depth/128/16;
		ex[j*w+i].y  = df[(j-1)*pitch+i-1].y;
		ex[j*w+i].cb = df[(j-1)*pitch+i-1].cb;
		ex[j*w+i].cr = df[(j-1)*pitch+i-1].cr;

		logodata = dstdata;	// logodata開放, dstdataが代入されて残る
	}

	/// 色調整
	void ColorTuning(aptr<::LOGO_PIXEL> &logodata,int y,int u,int v)
	{
		::LOGO_PIXEL* lgp = logodata.get();
		for(int i=lgh.h;i;--i){
			for(int j=lgh.w;j;--j){
				lgp->y  += y;
				lgp->cb += u;
				lgp->cr += v;
				++lgp;
			}
		}
	}

	/// フェードによる深度計算
	int CalcFade(int n)
	{
		if(n<start || (end<n && end>=start)){	// 範囲外
			return 0;
		}
		if(n < start+fadein){			// フェードイン
			return ((n - start)*2 +1)*LOGO_FADE_MAX / (fadein *2);
		}
		else if(n > end-fadeout && end>=0){		// フェードアウト
			return ((end - n)*2 +1)*LOGO_FADE_MAX / (fadeout *2);
		}
		// 通常
		return LOGO_FADE_MAX;
	}

	int Abs(int x){ return abs(x); }
	int Clamp(int n, int l, int h){	return min(max(n,l),h); }

	int YtoAUY(int y){ return ((y*1197)>>6) - 299; }
	int CtoAUC(int c){ return (c*4681 - 128*4681 +164)>>8; }
	int AUYtoY(int y){ return Clamp((y*219 + 383 + (16<<12))>>12,0,255);}
	int AUCtoC(int c){ return Clamp((c*7 + 2048*7 + 66 + (16<<7))>>7,0,255); }
};


#endif
