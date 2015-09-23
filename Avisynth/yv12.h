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
#ifndef ___YV12_H
#define ___YV12_H

#include "../logo.h"
#include "delogo.h"

/*****************************************************************************/
/** COLOR = YV12
 *
 * struct YV12p : プログレYV12
 * struct YV12i : インタレYV12
 */
template <bool interlaced>
struct YV12 {

	typedef struct {
		short dp,c;
	} LOGO_PIXEL;

	static LOGO_PIXEL *Convert(::LOGO_PIXEL *src,::LOGO_HEADER &lgh)
	{
		bool oddx = lgh.x %2;
		if(oddx){
			lgh.x -= 1;
			lgh.w += 1;
		}
		bool oddy = lgh.y %2;
		if(oddy){
			lgh.y -= 1;
			lgh.h += 1;
		}
		bool oddw = lgh.w %2;
		if(oddw){
			lgh.w += 1;
		}
		bool oddh = lgh.h %2;
		if(oddh){
			lgh.h += 1;
		}

		LOGO_PIXEL *p = new LOGO_PIXEL[lgh.w * lgh.h * 3 / 2];
		if(p==NULL){
			throw "Failed in memory allocation. - メモリ確保に失敗しました";
		}
		LOGO_PIXEL *dst = p;

		// Y
		ConvertY(dst,src,lgh,oddx,oddy,oddw,oddh);
		dst += lgh.w * lgh.h;
		ConvertU(dst,src,lgh,oddx,oddy,oddw,oddh);
		dst += lgh.w * lgh.h / 4;
		ConvertV(dst,src,lgh,oddx,oddy,oddw,oddh);

		return p;
	}

protected:

	static void ConvertU(LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh);
	static void ConvertV(LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh);
	static void ConvertY(LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh)
	{
		const int w = lgh.w;
		const int srcw = w - (oddx?1:0) - (oddw?1:0);
		const int srch = lgh.h - (oddy?1:0) - (oddh?1:0);
		if(oddy){
			for(int i=w;i;--i){
				dst->dp = 0;
				++dst;
			}
		}
		for(int i=srch;i;--i){
			if(oddx){
				dst->dp = 0;
				++dst;
			}
			for(int j=srcw;j;--j){
				dst->dp = src->dp_y;
				dst->c  = src->y;
				++src;
				++dst;
			}
			if(oddw){
				dst->dp = 0;
				++dst;
			}
		}
		if(oddh){
			for(int i=w;i;--i){
				dst->dp = 0;
				++dst;
			}
		}
	}
};

typedef class YV12<true> YV12i;
typedef class YV12<false> YV12p;

/*---------------------------------------------------------------------------*/
namespace YV12_internal {
	struct ChromaU {
		static int DP(::LOGO_PIXEL *p){ return p->dp_cb; }
		static int Cx(::LOGO_PIXEL *p){ return p->cb; }
	};
	struct ChromaV {
		static int DP(::LOGO_PIXEL *p){ return p->dp_cr; }
		static int Cx(::LOGO_PIXEL *p){ return p->cr; }
	};

	template <class Chroma>
	struct ConvertUVp {
		static void Convert(YV12p::LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh);
	};
	template<class Chroma>
	struct ConvertUVi {
		static void Convert(YV12i::LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh);
	};
};

/*===========================================================================*/
inline void YV12p::ConvertU(LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh)
{
	using namespace YV12_internal;
	ConvertUVp<ChromaU>::Convert(dst,src,lgh,oddx,oddy,oddw,oddh);
}
inline void YV12p::ConvertV(LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh)
{
	using namespace YV12_internal;
	ConvertUVp<ChromaV>::Convert(dst,src,lgh,oddx,oddy,oddw,oddh);
}
inline void YV12i::ConvertU(LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh)
{
	using namespace YV12_internal;
	ConvertUVi<ChromaU>::Convert(dst,src,lgh,oddx,oddy,oddw,oddh);
}
inline void YV12i::ConvertV(LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh)
{
	using namespace YV12_internal;
	ConvertUVi<ChromaV>::Convert(dst,src,lgh,oddx,oddy,oddw,oddh);
}

/*===========================================================================*/
/** プログレYV12用
 */
template<class Chroma>
inline void YV12_internal::ConvertUVp<Chroma>::Convert(YV12p::LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh)
{
	const int srcw = lgh.w - (oddx?1:0) - (oddw?1:0);
	const int srch = lgh.h - (oddy?1:0) - (oddh?1:0);
	const int srcw_r = (srcw - (oddx?1:0) - (oddw?1:0))/2 + (oddw?1:0);	// oddw=tの時は一個多く
	const int srch_r = (srch - (oddy?1:0) - (oddh?1:0))/2;
	::LOGO_PIXEL *s1 = src;

	if(oddy){
		if(oddx){
			dst->dp = 0;
			++dst;
			++s1;
		}
		for(int j=srcw_r;j;--j){
			dst->dp = (Chroma::DP(s1) +1)/2;
			dst->c = Chroma::Cx(s1);
			++dst;
			s1 += 2;
		}
		if(oddw){	// 多くしてる分を減
			--s1;
		}
	}
	::LOGO_PIXEL *s2 = s1 + srcw;
	for(int i=srch_r;i;--i){
		if(oddx){
			dst->dp = 0;
			++dst;
			++s1;
			++s2;
		}
		for(int j=srcw_r;j;--j){
			int dp  = Chroma::DP(s1) + Chroma::DP(s2);
			dst->dp = (dp +1)/2;
			if(dp){
				dst->c = (Chroma::Cx(s1)*Chroma::DP(s1)
						  + Chroma::Cx(s2)*Chroma::DP(s2) + (dp+1)/2) /dp;
			}
			++dst;
			s1 += 2;
			s2 += 2;
		}
		if(oddw){
			--s1;
			--s2;
		}
		s1 += srcw;
		s2 += srcw;
	}
	if(oddh){
		if(oddx){
			dst->dp = 0;
			++dst;
			++s1;
		}
		for(int j=srcw_r;j;--j){
			dst->dp = (Chroma::DP(s1) +1)/2;
			dst->c = Chroma::Cx(s1);
			++dst;
			s1 += 2;
		}
	}
}

/*---------------------------------------------------------------------------*/
/** インタレYV12用
 * インタレYV12補完
 *  h=4n+0,4n+1 -> UpperLine:LowerLine = 3:1
 *  h=4n+2,4n+3 -> UpperLine:LowerLine = 1:3
 */
template <class Chroma>
inline void YV12_internal::ConvertUVi<Chroma>::Convert(YV12i::LOGO_PIXEL *dst,::LOGO_PIXEL *src,const ::LOGO_HEADER &lgh,bool oddx,bool oddy,bool oddw,bool oddh)
{
	const int srcw = lgh.w - (oddx?1:0) - (oddw?1:0);
	const int srcw_r = (srcw - (oddx?1:0) - (oddw?1:0))/2 + (oddw?1:0);

	const int top = (lgh.y + (oddy?1:0)) % 4;
	const int btm = (lgh.y + lgh.h - (oddh?1:0)) % 4;
	const int srch_r = (lgh.h - (top?4-top:0) - btm) / 4;


	::LOGO_PIXEL *s1, *s2;

	// 上段処理
	if(top>=2){
		::LOGO_PIXEL *s;
		if(top==3){	// y = 4n+3
			s1 = src + srcw;
			s = src;
		}
		else{	// y = 4n+2
			s1 = src + srcw * 2;
			s = src + srcw;
		}
		if(oddx){
			dst->dp = 0;
			++dst; ++s;
		}
		for(int j=srcw_r;j;--j){
			dst->dp = (Chroma::DP(s) * 3 +2)/4;
			dst->c = Chroma::Cx(s);
			++dst; s += 2;
		}
	}
	else{
		if(oddy){	// y = 4n+1
			s1 = src + srcw;
			if(oddx){
				dst->dp = 0;
				++dst; ++s1;
			}
			for(int j=srcw_r;j;--j){
				dst->dp = (Chroma::DP(s1) +2)/4;
				if(dst->dp){
					dst->c = Chroma::Cx(s1);
				}
				++dst; s1 += 2;
			}
			s1 = src;
			s2 = src + srcw * 2;
			if(oddx){
				dst->dp = 0;
				++dst; ++s1; ++s2;
			}
			for(int j=srcw_r;j;--j){
				int dp = Chroma::DP(s1) + Chroma::DP(s2) *3;
				dst->dp = (dp +2)/4;
				if(dp){
					dst->c = (Chroma::Cx(s1)*Chroma::DP(s1)
							  + Chroma::Cx(s2)*Chroma::DP(s2)*3 + (dp+1)/2) /dp;
				}
				++dst; s1 += 2; s2 += 2;
			}
			s1 = src + srcw * 3;
		}
		else{	// y = 4n
			s1 = src;
		}
	}

	// 中段処理
	s2 = s1 + srcw *2;
	for(int i=srch_r;i;--i){
		int j;
		if(oddx){
			dst->dp = 0;
			++dst; ++s1; ++s2;
		}
		for(j=srcw_r;j;--j){
			int dp = Chroma::DP(s1) *3 + Chroma::DP(s2);
			dst->dp = (dp +2)/4;
			if(dp){
				dst->c = (Chroma::Cx(s1) * Chroma::DP(s1) *3
						  + Chroma::Cx(s2) * Chroma::DP(s2)
						  + (dp+1)/2) / dp;
			}
			++dst; s1 += 2; s2 += 2;
		}
		if(oddw){
			--s1; --s2;
		}
		if(oddx){
			dst->dp = 0;
			++dst; ++s1; ++s2;
		}
		for(j=srcw_r;j;--j){
			int dp = Chroma::DP(s1) + Chroma::DP(s2) *3;
			dst->dp = (dp +2)/4;
			if(dp){
				dst->c = (Chroma::Cx(s1) * Chroma::DP(s1)
						  + Chroma::Cx(s2) * Chroma::DP(s2) *3
						  + (dp+1)/2) / dp;
			}
			++dst; s1 += 2; s2 += 2;
		}
		if(oddw){
			--s1; --s2;
		}

		s1 = s2;
		s2 = s1 + srcw *2;
	}

	// 下段処理
	if(btm==3){
		int j;
		if(oddx){
			dst->dp = 0;
			++dst; ++s1; ++s2;
		}
		for(j=srcw_r;j;--j){
			int dp = Chroma::DP(s1) *3 + Chroma::DP(s2);
			dst->dp = (dp +2)/4;
			if(dp){
				dst->c = (Chroma::Cx(s1) * Chroma::DP(s1) *3
						  + Chroma::Cx(s2) * Chroma::DP(s2)
						  + (dp+1)/2) / dp;
			}
			++dst; s1+=2; s2+=2;
		}
		if(oddw){
			--s1; --s2;
		}
		if(oddx){
			dst->dp = 0;
			++dst; ++s1; ++s2;
		}
		for(j=srcw_r;j;--j){
			dst->dp = (Chroma::DP(s1) +2)/4;
			dst->c = Chroma::Cx(s1);
			++dst; s1+=2; s2+=2;
		}
	}
	else if(btm){	// btm == 1 or 2
		if(oddx){
			dst->dp = 0;
			++dst; ++s1;
		}
		for(int j=srcw_r;j;--j){
			dst->dp = (Chroma::DP(s1) * 3 +2)/4;
			dst->c = Chroma::Cx(s1);
			++dst; s1+=2;
		}
	}
}


/*****************************************************************************/
/* GetFrame */
template <>
PVideoFrame __stdcall deLOGO<Erase,YV12p>::GetFrame(int n,IScriptEnvironment *env)
{
	PVideoFrame frame(child->GetFrame(n,env));
	int fade = CalcFade(n);
	if(fade==0) return frame;

	env->MakeWritable(&frame);

	// ロゴのxywh, frameのwhは全部偶数
	int logo_w = min(lgh.w, frame->GetRowSize()-lgh.x);
	int logo_h = min(lgh.h, frame->GetHeight()-lgh.y);
	int dst_x = lgh.x;
	int dst_y = lgh.y;
	int logo_x = 0;
	int logo_y = 0;
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

	//Y
	int dst_pitch = frame->GetPitch(PLANAR_Y);
	int dst_pitch_r = dst_pitch - logo_w;
	BYTE *dst = frame->GetWritePtr(PLANAR_Y) + dst_x + dst_y * dst_pitch;
	int logo_pitch = lgh.w;
	int logo_pitch_r = logo_pitch - logo_w;
	YV12p::LOGO_PIXEL *lgp = logodata.get() + logo_x + logo_y * lgh.w;
	int i,j;
	for(i=logo_h;i;--i){
		for(j=logo_w;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				*dst = AUYtoY((YtoAUY(*dst)*LOGO_MAX_DP - lgp->c*dp
							   +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	//U
	dst_pitch = frame->GetPitch(PLANAR_U);
	dst_pitch_r = dst_pitch - logo_w/2;
	dst = frame->GetWritePtr(PLANAR_U) + (dst_x + dst_y * dst_pitch)/2;
	logo_pitch = lgh.w /2;
	logo_pitch_r = logo_pitch - logo_w/2;
	lgp = logodata.get() + (lgh.w*lgh.h) + (logo_x + logo_y * lgh.w)/2;
	for(i=logo_h/2;i;--i){
		for(j=logo_w/2;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				*dst = AUCtoC((CtoAUC(*dst)*LOGO_MAX_DP - lgp->c*dp
							   +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	//V
	dst_pitch = frame->GetPitch(PLANAR_V);
	dst_pitch_r = dst_pitch - logo_w/2;
	dst = frame->GetWritePtr(PLANAR_V) + (dst_x + dst_y * dst_pitch)/2;
	lgp = logodata.get() + (lgh.w*lgh.h)/4*5 + (logo_x + logo_y * lgh.w)/2;
	for(i=logo_h/2;i;--i){
		for(j=logo_w/2;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				*dst = AUCtoC((CtoAUC(*dst)*LOGO_MAX_DP - lgp->c*dp
							   +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}

	return frame;
}

/*---------------------------------------------------------------------------*/
template<>
PVideoFrame __stdcall deLOGO<Add,YV12p>::GetFrame(int n,IScriptEnvironment *env)
{
	PVideoFrame frame(child->GetFrame(n,env));
	int fade = CalcFade(n);
	if(fade==0) return frame;

	env->MakeWritable(&frame);

	// ロゴのxywh, frameのwhは全部偶数
	int logo_w = min(lgh.w, frame->GetRowSize()-lgh.x);
	int logo_h = min(lgh.h, frame->GetHeight()-lgh.y);
	int dst_x = lgh.x;
	int dst_y = lgh.y;
	int logo_x = 0;
	int logo_y = 0;
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

	//Y
	int dst_pitch = frame->GetPitch(PLANAR_Y);
	int dst_pitch_r = dst_pitch - logo_w;
	BYTE *dst = frame->GetWritePtr(PLANAR_Y) + dst_x + dst_y * dst_pitch;
	int logo_pitch = lgh.w;
	int logo_pitch_r = logo_pitch - logo_w;
	YV12p::LOGO_PIXEL *lgp = logodata.get() + logo_x + logo_y * lgh.w;
	int i,j;
	for(i=logo_h;i;--i){
		for(j=logo_w;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				*dst = AUYtoY((YtoAUY(*dst)*(LOGO_MAX_DP-dp) + lgp->c*dp
							   + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	//U
	dst_pitch = frame->GetPitch(PLANAR_U);
	dst_pitch_r = dst_pitch - logo_w/2;
	dst = frame->GetWritePtr(PLANAR_U) + (dst_x + dst_y * dst_pitch)/2;
	logo_pitch = lgh.w /2;
	logo_pitch_r = logo_pitch - logo_w/2;
	lgp = logodata.get() + (lgh.w * lgh.h) + (logo_x + logo_y * lgh.w)/2;
	for(i=logo_h/2;i;--i){
		for(j=logo_w/2;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				*dst = AUCtoC((CtoAUC(*dst)*(LOGO_MAX_DP-dp) + lgp->c*dp
							   + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	//V
	dst_pitch = frame->GetPitch(PLANAR_V);
	dst_pitch_r = dst_pitch - logo_w/2;
	dst = frame->GetWritePtr(PLANAR_V) + (dst_x + dst_y * dst_pitch)/2;
	lgp = logodata.get() + (lgh.w*lgh.h)*5/4 + (logo_x + logo_y * lgh.w)/2;
	for(i=logo_h/2;i;--i){
		for(j=logo_w/2;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				*dst = AUCtoC((CtoAUC(*dst)*(LOGO_MAX_DP-dp) + lgp->c*dp
							   + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}

	return frame;
}

/*---------------------------------------------------------------------------*/
template <>
PVideoFrame __stdcall deLOGO<Erase,YV12i>::GetFrame(int n,IScriptEnvironment *env)
{
	PVideoFrame frame(child->GetFrame(n,env));
	int fade = CalcFade(n);
	if(fade==0) return frame;

	env->MakeWritable(&frame);

	// ロゴのxywh, frameのwhは全部偶数
	int logo_w = min(lgh.w, frame->GetRowSize()-lgh.x);
	int logo_h = min(lgh.h, frame->GetHeight()-lgh.y);
	int dst_x = lgh.x;
	int dst_y = lgh.y;
	int logo_x = 0;
	int logo_y = 0;
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

	//Y
	int dst_pitch = frame->GetPitch(PLANAR_Y);
	int dst_pitch_r = dst_pitch - logo_w;
	BYTE *dst = frame->GetWritePtr(PLANAR_Y) + dst_x + dst_y * dst_pitch;
	int logo_pitch = lgh.w;
	int logo_pitch_r = logo_pitch - logo_w;
	YV12i::LOGO_PIXEL *lgp = logodata.get() + logo_x + logo_y * lgh.w;
	int i,j;
	for(i=logo_h;i;--i){
		for(j=logo_w;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				*dst = AUYtoY((YtoAUY(*dst)*LOGO_MAX_DP - lgp->c*dp
							   +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	//U
	dst_pitch = frame->GetPitch(PLANAR_U);
	dst_pitch_r = dst_pitch - logo_w/2;
	dst = frame->GetWritePtr(PLANAR_U) + (dst_x + dst_y * dst_pitch)/2;
	logo_pitch = lgh.w /2;
	logo_pitch_r = logo_pitch - logo_w/2;
	lgp = logodata.get() + (lgh.w*lgh.h) + (logo_x + logo_y * lgh.w)/2;
	for(i=logo_h/2;i;--i){
		for(j=logo_w/2;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				*dst = AUCtoC((CtoAUC(*dst)*LOGO_MAX_DP - lgp->c*dp
							   +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	//V
	dst_pitch = frame->GetPitch(PLANAR_V);
	dst_pitch_r = dst_pitch - logo_w/2;
	dst = frame->GetWritePtr(PLANAR_V) + (dst_x + dst_y * dst_pitch)/2;
	lgp = logodata.get() + (lgh.w*lgh.h)/4*5 + (logo_x + logo_y * lgh.w)/2;
	for(i=logo_h/2;i;--i){
		for(j=logo_w/2;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				if(dp==LOGO_MAX_DP) --dp;
				*dst = AUCtoC((CtoAUC(*dst)*LOGO_MAX_DP - lgp->c*dp
							   +(LOGO_MAX_DP-dp)/2) / (LOGO_MAX_DP-dp));
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}

	return frame;
}

/*---------------------------------------------------------------------------*/
template<>
PVideoFrame __stdcall deLOGO<Add,YV12i>::GetFrame(int n,IScriptEnvironment *env)
{
	PVideoFrame frame(child->GetFrame(n,env));
	int fade = CalcFade(n);
	if(fade==0) return frame;

	env->MakeWritable(&frame);

	// ロゴのxywh, frameのwhは全部偶数
	int logo_w = min(lgh.w, frame->GetRowSize()-lgh.x);
	int logo_h = min(lgh.h, frame->GetHeight()-lgh.y);
	int dst_x = lgh.x;
	int dst_y = lgh.y;
	int logo_x = 0;
	int logo_y = 0;
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

	//Y
	int dst_pitch = frame->GetPitch(PLANAR_Y);
	int dst_pitch_r = dst_pitch - logo_w;
	BYTE *dst = frame->GetWritePtr(PLANAR_Y) + dst_x + dst_y * dst_pitch;
	int logo_pitch = lgh.w;
	int logo_pitch_r = logo_pitch - logo_w;
	YV12i::LOGO_PIXEL *lgp = logodata.get() + logo_x + logo_y * lgh.w;
	int i,j;
	for(i=logo_h;i;--i){
		for(j=logo_w;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				*dst = AUYtoY((YtoAUY(*dst)*(LOGO_MAX_DP-dp) + lgp->c*dp
							   + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	//U
	dst_pitch = frame->GetPitch(PLANAR_U);
	dst_pitch_r = dst_pitch - logo_w/2;
	dst = frame->GetWritePtr(PLANAR_U) + (dst_x + dst_y * dst_pitch)/2;
	logo_pitch = lgh.w /2;
	logo_pitch_r = logo_pitch - logo_w/2;
	lgp = logodata.get() + (lgh.w * lgh.h) + (logo_x + logo_y * lgh.w)/2;
	for(i=logo_h/2;i;--i){
		for(j=logo_w/2;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				*dst = AUCtoC((CtoAUC(*dst)*(LOGO_MAX_DP-dp) + lgp->c*dp
							   + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}
	//V
	dst_pitch = frame->GetPitch(PLANAR_V);
	dst_pitch_r = dst_pitch - logo_w/2;
	dst = frame->GetWritePtr(PLANAR_V) + (dst_x + dst_y * dst_pitch)/2;
	lgp = logodata.get() + (lgh.w*lgh.h)*5/4 + (logo_x + logo_y * lgh.w)/2;
	for(i=logo_h/2;i;--i){
		for(j=logo_w/2;j;--j){
			int dp = (lgp->dp * fade + LOGO_FADE_MAX/2)/LOGO_FADE_MAX;
			if(dp){
				*dst = AUCtoC((CtoAUC(*dst)*(LOGO_MAX_DP-dp) + lgp->c*dp
							   + LOGO_MAX_DP/2) / LOGO_MAX_DP);
			}
			++dst;
			++lgp;
		}
		dst += dst_pitch_r;
		lgp += logo_pitch_r;
	}

	return frame;
}


#endif
