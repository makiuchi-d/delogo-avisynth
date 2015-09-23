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

#include <windows.h>
#include <math.h>
#include "avisynth.h"
#include "../logo.h"
#include "delogo.h"
#include "yuy2.h"
#include "yv12.h"


/*****************************************************************************/
/** フィルタ作成
 */
template <class TYPE>
class deLOGO_Create {
	enum {
		LOGOFILE =1, LOGONAME,
		POS_X, POS_Y, DEPTH,
		YC_Y , YC_U , YC_V ,
		START, F_IN , F_OUT, END,
		INTERLACED,
	};

public:
	static AVSValue __cdecl Create(AVSValue args,void *user_data,IScriptEnvironment *env)
	{
		PClip clip(args[0].AsClip());
		const char *logofile = args[LOGOFILE].AsString(NULL);
		const char *logoname = args[LOGONAME].AsString(NULL);

		int  pos_x = args[POS_X].AsInt(0);
		int  pos_y = args[POS_Y].AsInt(0);
		int  depth = args[DEPTH].AsInt(LOGO_DEFAULT_DEPTH);
		int  yc_y  = args[YC_Y].AsInt(0);
		int  yc_u  = args[YC_U].AsInt(0);
		int  yc_v  = args[YC_V].AsInt(0);
		int  start   = args[START].AsInt(0);
		int  fadein  = args[F_IN].AsInt(0);
		int  fadeout = args[F_OUT].AsInt(0);
		int  end     = args[END].AsInt(-1);
		bool interlaced = args[INTERLACED].AsBool(false);

		const VideoInfo& vi = clip->GetVideoInfo();

		if(vi.IsYUY2()){
			return new deLOGO<TYPE,YUY2>(clip,logofile,logoname,pos_x,pos_y,depth,yc_y,yc_u,yc_v,start,fadein,fadeout,end,env);
		}
		else if(vi.IsYV12()){
			if(interlaced){
				return new deLOGO<TYPE,YV12i>(clip,logofile,logoname,pos_x,pos_y,depth,yc_y,yc_u,yc_v,start,fadein,fadeout,end,env);
			}
			else{
				return new deLOGO<TYPE,YV12p>(clip,logofile,logoname,pos_x,pos_y,depth,yc_y,yc_u,yc_v,start,fadein,fadeout,end,env);
			}
		}
		else{
			env->ThrowError("%s : Supprot only YUY2 or YV12. - YUY2,YV12専用",Name());
		}

		return NULL;
	}

	static const char *Name(void){ return TYPE::Name(); };
	static const char *Params(void){
		return "c[logofile]s[logoname]s[pos_x]i[pos_y]i[depth]i[yc_y]i[yc_u]i[yc_v]i[start]i[fadein]i[fadeout]i[end]i[interlaced]b";
	}
};

/*****************************************************************************/
/** エクスポート関数
 */
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit2(IScriptEnvironment* env)
{
	typedef deLOGO_Create<Add> AddLOGO;
	typedef deLOGO_Create<Erase> EraseLOGO;

	env->AddFunction(EraseLOGO::Name(),EraseLOGO::Params(),EraseLOGO::Create,0);
	env->AddFunction(AddLOGO::Name(),AddLOGO::Params(),AddLOGO::Create,0);

	return "DeLOGO plugin"; 
}

