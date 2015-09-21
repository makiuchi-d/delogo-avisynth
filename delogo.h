//
//	Copylight (C) 2003 MakKi
//
//	AvisynthがGPLなため、このソフトもGPLになるらしいです｡
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
#ifndef ___DELOGO_H
#define ___DELOGO_H

#include <windows.h>
#include "avisynth.h"
#include "logo.h"

#define LOGO_FADE_MAX      256
#define LOGO_DEFAULT_DEPTH 128

/********************************************************************
* 	Init関数
*		Init_ErazeLOGO
*		Init_AddLOGO
*********************************************************************/
extern AVSValue __cdecl Create_EraseLOGO(AVSValue args, void *user_data, IScriptEnvironment *env);
extern AVSValue __cdecl Create_AddLOGO(AVSValue args, void *user_data, IScriptEnvironment *env);

inline void Init_EraseLOGO(IScriptEnvironment* env)
{
	env->AddFunction("EraseLOGO","c[logofile]s[logoname]s[pos_x]i[pos_y]i[depth]i[yc_y]i[yc_u]i[yc_v]i[start]i[fadein]i[fadeout]i[end]i",Create_EraseLOGO, 0);
}
inline void Init_AddLOGO(IScriptEnvironment* env)
{
	env->AddFunction("AddLOGO","c[logofile]s[logoname]s[pos_x]i[pos_y]i[depth]i[yc_y]i[yc_u]i[yc_v]i[start]i[fadein]i[fadeout]i[end]i",Create_AddLOGO, 0);
}
enum {
	LOGOFILE =1, LOGONAME,
	POS_X, POS_Y, DEPTH,
	YC_Y , YC_U , YC_V ,
	START, F_IN , F_OUT, END
};

/*===================================================================
* 	class deLOGO_Base
*==================================================================*/

class deLOGO_Base : public GenericVideoFilter {

protected:
	BYTE logodata[LOGO_MAXSIZE];
	int  _start, _fadein, _fadeout, _end;

	LOGO_HEADER* lgh;

	deLOGO_Base(const PClip& clip,const char* logofile,const char* logoname,int pos_x,int pos_y,
			int depth,int y,int u,int v,int start,int fadein,int fadeout,int end,  IScriptEnvironment *env,const char* filtername) : GenericVideoFilter(clip)
	{
		const VideoInfo& vi = clip->GetVideoInfo();

		_start   = start;
		_fadein  = fadein;
		_fadeout = fadeout;
		_end     = end;

		child->SetCacheHints(CACHE_NOTHING, 0);

		lgh = (LOGO_HEADER*)logodata;
		try{
			ReadLogoData(logofile,logoname);
			if(pos_x!=0 || pos_y!=0 || depth!=LOGO_DEFAULT_DEPTH)
				AdjustLogo(pos_x,pos_y,depth);
			if(y!=0 || u!=0 || v!= 0)
				ColorTuning(y*16,u*16,v*16);
		}
		catch(char* err){
			env->ThrowError("%s: %s",filtername,err);
		}
	}
	void ReadLogoData(const char* logofile,const char* logoname);
	void AdjustLogo(int x,int y,int depth);	// AviUtlオリジナル色空間のまま
	void ColorTuning(int y,int u,int v)	// 色調整
	{
		int i,j;
		LOGO_PIXEL* lgp = (LOGO_PIXEL*)(lgh +1);

		for(i=lgh->h;i;--i){
			for(j=lgh->w;j;--j){
				lgp->y  += y;
				lgp->cb += u;
				lgp->cr += v;
				++lgp;
			}
		}
	}
	void Logo_YUY2();
	void Logo_YV420(){};

	int CalcFade(int n)//,PVideoFrame &src)
	{
		if(n<_start || (_end<n && _end>=_start) )	// 範囲外
			return 0;

		int fade;
	
		if(n < _start+_fadein)			// フェードイン
			fade = ((n -_start)*2 +1)*LOGO_FADE_MAX / (_fadein *2);
		else if(n > _end-_fadeout && _end>=0)		// フェードアウト
			fade = ((_end - n)*2 +1)*LOGO_FADE_MAX / (_fadeout *2);
		else	// 通常
			fade = LOGO_FADE_MAX;

		return fade;
	}

	int Abs(int x)
	{	return ((x<0)? -x : x);	}
	int Clamp(int n, int l, int h)
	{	return (n < l) ? l : (n > h) ? h : n;	}

	// ITU-R TB.601に合うようにAviUtlのYCを圧縮
	int TB601_Y(int y)
	{	return ((y +(16<<4)) * 220 +128)/ 256;	}
	int TB601_C(int c)
	{	return (c * 225 +128)/ 256;	}
public:

};


/*===================================================================
* 	class ErazeLOGO
*==================================================================*/

class EraseLOGO_YUY2 : public deLOGO_Base {

public:
	EraseLOGO_YUY2(const PClip& clip,const char* logofile,const char* logoname,
			int pos_x,int pos_y,int depth,int y,int u,int v,int start,int fadein,int fadeout,int end,IScriptEnvironment *env) 
					: deLOGO_Base(clip,logofile,logoname,pos_x,pos_y,depth,y,u,v,start,fadein,fadeout,end,env,GetName())
	{	Logo_YUY2();	}

	static const char* GetName(){	return "EraseLOGO";	}

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
};

/*===================================================================
* 	class AddLOGO_YUY2
*==================================================================*/

class AddLOGO_YUY2 : public deLOGO_Base {

public:
	AddLOGO_YUY2(const PClip& clip,const char* logofile,const char* logoname,
			int pos_x,int pos_y,int depth,int y,int u,int v,int start,int fadein,int fadeout,int end,IScriptEnvironment *env) 
					: deLOGO_Base(clip,logofile,logoname,pos_x,pos_y,depth,y,u,v,start,fadein,fadeout,end,env,GetName())
	{ Logo_YUY2(); }

	static const char* GetName(){	return "AddLOGO";	}

	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment *env);
};


#endif