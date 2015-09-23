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
#ifndef ___YUY2_H
#define ___YUY2_H

#include "../logo.h"
#include "delogo.h"

/// COLOR = YUY2
struct YUY2 {
	typedef struct {
		short dp_y1, y1;
		short dp_u,  u;
		short dp_y2, y2;
		short dp_v,  v;
	} LOGO_PIXEL;

	static LOGO_PIXEL *Convert(::LOGO_PIXEL *src,::LOGO_HEADER &lgh)
	{
		int srcw = lgh.w;
		bool oddx = lgh.x % 2;
		if(oddx){
			lgh.x -= 1;
			lgh.w += 1;
			--srcw;
		}
		bool oddw = lgh.w % 2;
		if(oddw){
			lgh.w += 1;
			--srcw;
		}

		LOGO_PIXEL *p  = new LOGO_PIXEL[lgh.w/2 * lgh.h];
		if(p==NULL){
			throw "Failed in memory allocation. - メモリ確保に失敗しました";
		}
		LOGO_PIXEL *dst = p;

		for(int i=lgh.h;i;--i){
			if(oddx){
				dst->dp_y1 = dst->dp_u = dst->dp_v = 0;
				dst->dp_y2 = src->dp_y; dst->y2 = src->dp_y;
				++dst;
				++src;
			}
			for(int j=srcw/2;j;--j){
				dst->dp_y1 = src[0].dp_y; dst->y1 = src[0].y;
				dst->dp_u = src[0].dp_cb; dst->u = src[0].cb;
				dst->dp_v = src[0].dp_cr; dst->v = src[0].cr;
				dst->dp_y2 = src[1].dp_y; dst->y2 = src[1].y;
				++dst;
				src += 2;
			}
			if(oddw){
				dst->dp_y1 = src->dp_y; dst->y1 = src->y;
				dst->dp_u = src->dp_cb; dst->u = src->cb;
				dst->dp_v = src->dp_cr; dst->v = src->cr;
				dst->dp_y2 = dst->y2 = 0;
				++src;
				++dst;
			}
		}
		return p;
	}
};

/*****************************************************************************/
/* GetFrame */

/// EraseLOGO YUY2
template <>
PVideoFrame __stdcall deLOGO<Erase,YUY2>::GetFrame(int n,IScriptEnvironment *env)
{
	PVideoFrame frame(child->GetFrame(n,env));

	// フェードによる不透明度計算
	int fade = CalcFade(n);
	if(fade==0) return frame;

	env->MakeWritable(&frame);

	int logo_w = min(lgh.w,frame->GetRowSize()-lgh.x);	// lgh.w,lgh.xは必ず偶数
	int logo_h = min(lgh.h,frame->GetHeight()-lgh.y);

	int dst_x = lgh.x;
	int dst_y = lgh.y;
	int logo_x =0;
	int logo_y =0;
	if(dst_x<0){
		logo_x = -dst_x;
		logo_w -= logo_x;
		dst_x = 0;
	}
	if(dst_y<0){
		logo_y = -dst_y;
		logo_h -= logo_y;
		dst_y = 0;
	}
	if(logo_w<=0 || logo_h<=0) return frame;	// 画面外

	const int logo_pitch = lgh.w /2;
	const int logo_pitch_r = logo_pitch - logo_w /2;
	const int dst_pitch = frame->GetPitch();
	const int dst_pitch_r = dst_pitch - logo_w*2;

	BYTE *dst = frame->GetWritePtr() + dst_x * 2 + dst_pitch * dst_y;
	YUY2::LOGO_PIXEL *lgp = logodata.get() + logo_x/2 + logo_pitch * logo_y;

	for(int i=logo_h;i;--i){
		for(int j=logo_w/2;j;--j){
			// Y1
			int dp = (lgp->dp_y1 * fade +LOGO_FADE_MAX/2)/ LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				dst[0] = AUYtoY((YtoAUY(dst[0])*LOGO_MAX_DP
								 - lgp->y1*dp +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			// U : Cb
			dp = (lgp->dp_u * fade +LOGO_FADE_MAX/2)/ LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				dst[1] = AUCtoC((CtoAUC(dst[1])*LOGO_MAX_DP
								 - lgp->u*dp +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			// Y2
			dp = (lgp->dp_y2 * fade +LOGO_FADE_MAX/2)/ LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				dst[2] = AUYtoY((YtoAUY(dst[2])*LOGO_MAX_DP
								 - lgp->y2*dp +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			// V : Cr
			dp = (lgp->dp_v * fade +LOGO_FADE_MAX/2)/ LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				dst[3] = AUCtoC((CtoAUC(dst[3])*LOGO_MAX_DP
								 - lgp->v*dp +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			dst += 4;
			lgp += 1;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	return frame;
}


/// AddLOGO YUY2
template <>
PVideoFrame __stdcall deLOGO<Add,YUY2>::GetFrame(int n,IScriptEnvironment *env)
{
	PVideoFrame frame(child->GetFrame(n,env));

	int fade = CalcFade(n);
	if(fade==0) return frame;

	env->MakeWritable(&frame);

	int logo_w = min(lgh.w,frame->GetRowSize()-lgh.x);	// lgh.w,lgh.xは必ず偶数
	int logo_h = min(lgh.h,frame->GetHeight()-lgh.y);

	int dst_x = lgh.x;
	int dst_y = lgh.y;
	int logo_x =0;
	int logo_y =0;
	if(dst_x<0){
		logo_x = -dst_x;
		logo_w -= logo_x;
		dst_x = 0;
	}
	if(dst_y<0){
		logo_y = -dst_y;
		logo_h -= logo_y;
		dst_y = 0;
	}
	if(logo_w<=0 || logo_h<=0) return frame;	// 画面外

	const int logo_pitch = lgh.w /2;
	const int logo_pitch_r = logo_pitch - logo_w /2;
	const int dst_pitch = frame->GetPitch();
	const int dst_pitch_r = dst_pitch - logo_w*2;

	BYTE *dst = frame->GetWritePtr() + dst_x * 2 + dst_pitch * dst_y;
	YUY2::LOGO_PIXEL *lgp = logodata.get() + logo_x/2 + logo_pitch * logo_y;

	for(int i=logo_h;i;--i){
		for(int j=logo_w/2;j;--j){
			// Y1
			int dp = (lgp->dp_y1 * fade +LOGO_FADE_MAX/2) / LOGO_FADE_MAX;
			if(dp){
				dst[0] = AUYtoY((YtoAUY(dst[0])*(LOGO_MAX_DP-dp) + lgp->y1*dp
								 + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			// U : Cb
			dp = (lgp->dp_u * fade +LOGO_FADE_MAX/2)/ LOGO_FADE_MAX;
			if(dp){
				dst[1] = AUCtoC((CtoAUC(dst[1])*(LOGO_MAX_DP-dp) + lgp->u*dp
								 + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			// Y2
			dp = (lgp->dp_y2 * fade +LOGO_FADE_MAX/2)/ LOGO_FADE_MAX;
			if(dp){
				dst[2] = AUYtoY((YtoAUY(dst[2])*(LOGO_MAX_DP-dp) + lgp->y2*dp
								 + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			// V : Cr
			dp = (lgp->dp_v * fade +LOGO_FADE_MAX/2)/ LOGO_FADE_MAX;
			if(dp){
				dst[3] = AUCtoC((CtoAUC(dst[3])*(LOGO_MAX_DP-dp) + lgp->v*dp
								 + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			dst += 4;
			lgp += 1;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	return frame;
}

#endif
