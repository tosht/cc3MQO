//
//  MQOModel.h
//  cc3MQO
//
//  Created by T.Takabayashi on 11/08/04.
//  Copyright (c) 2011 T.Takabayashi.
//

/*
 GLMetaseq
 The MIT License
 Copyright (c) 2009 Sunao Hashimoto and Keisuke Konishi
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 
 GLMetaseq
 MITライセンス
 Copyright (c) 2009 Sunao Hashimoto and Keisuke Konishi
 
 以下に定める条件に従い、本ソフトウェアおよび関連文書のファイル（以下「ソフト
 ウェア」）の複製を取得するすべての人に対し、ソフトウェアを無制限に扱うことを
 無償で許可します。これには、ソフトウェアの複製を使用、複写、変更、結合、掲載、
 頒布、サブライセンス、および/または販売する権利、およびソフトウェアを提供する
 相手に同じことを許可する権利も無制限に含まれます。 
 
 上記の著作権表示および本許諾表示を、ソフトウェアのすべての複製または重要な部分
 に記載するものとします。 
 
 ソフトウェアは「現状のまま」で、明示であるか暗黙であるかを問わず、何らの保証
 もなく提供されます。ここでいう保証とは、商品性、特定の目的への適合性、および
 権利非侵害についての保証も含みますが、それに限定されるものではありません。 
 作者または著作権者は、契約行為、不法行為、またはそれ以外であろうと、ソフト
 ウェアに起因または関連し、あるいはソフトウェアの使用またはその他の扱いに
 よって生じる一切の請求、損害、その他の義務について何らの責任も負わないもの
 とします。 
 */



#ifndef __CC3_MQO_H__
#define __CC3_MQO_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


#define MAX_TEXTURE				100			// テクスチャの最大取り扱い数
#define MAX_OBJECT				128			// 1個のMQOファイル内の最大オブジェクト数
#define SIZE_STR				256			// 文字列バッファのサイズ
#define DEF_IS_LITTLE_ENDIAN	1			// エンディアン指定（intel系=1）

#ifndef MAX_PATH
#define MAX_PATH    256
#endif

#ifndef TRUE
#define TRUE    (1==1)
#endif

#ifndef FALSE
#define FALSE   (1!=1)
#endif



/*=========================================================================
 【マクロ定義】 最大値マクロ
 =========================================================================*/
#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif




/*=========================================================================
 【型定義】 OpenGL用色構造体 (4色float)
 =========================================================================*/
typedef struct {
	float r;
	float g;
	float b;
	float a;
} glCOLOR4f;


/*=========================================================================
 【型定義】 OpenGL用２次元座標構造体 (float)
 =========================================================================*/
typedef struct {
	float x;
	float y;
} glPOINT2f;


/*=========================================================================
 【型定義】 OpenGL用３次元座標構造体 (float)
 =========================================================================*/
typedef struct tag_glPOINT3f {
	float x;
	float y;
	float z;
} glPOINT3f;


/*=========================================================================
 【型定義】 面情報構造体
 =========================================================================*/
typedef struct {
	int	n;              // 1つの面を構成する頂点の数（3〜4）
	int material_index; // 面の材質番号
	int v[4];           // 頂点番号を格納した配列
	glPOINT2f uv[4];	// UVマップ
    int color[4];
} MQOFace;

typedef MQOFace *MQOFaceRef;

/*=========================================================================
 【型定義】 材質情報構造体（ファイルから情報を読み込む際に使用）
 =========================================================================*/
typedef struct {
	glCOLOR4f col;			// 色
	float dif[4];			// 拡散光
	float amb[4];			// 周囲光
	float emi[4];			// 自己照明
	float spc[4];			// 反射光
	float power;			// 反射光の強さ
	int useTex;				// テクスチャの有無
	char texFile[SIZE_STR];	// テクスチャファイル
	char alpFile[SIZE_STR];	// アルファマップ　テクスチャファイル
	char bmpFile[SIZE_STR];	// バンプマップ　テクスチャファイル
} MQOMaterial;
typedef MQOMaterial* MQOMaterialRef;

/*=========================================================================
 【型定義】 オブジェクト構造体（パーツ１個のデータ）
 =========================================================================*/
typedef struct {
	char objname[SIZE_STR];	// パーツ名
	int visible;			// 可視状態
	int	shading;			// シェーディング（0:フラット／1:グロー）
	float facet;			// スムージング角
	int n_face;				// 面数
	int	n_vertex;			// 頂点数
	MQOFace *F;			// 面
	glPOINT3f *V;			// 頂点
} MQOObjectChunk;

typedef MQOObjectChunk *MQOObjectChunkRef;


/*=========================================================================
 【型定義】 メッシュ情報（マテリアル別に頂点配列を持つ）
 =========================================================================*/
typedef struct {
	int	isValidMaterialInfo;// マテリアル情報の有効/無効
    
    
	int	vertex_num;			// 頂点数
    float *locations;
    float *normals;
    float *texture_cordinates;
    
    int material_index;
    
} MQOMesh;

typedef MQOMesh *MQOMeshRef;

/*=========================================================================
 【型定義】 オブジェクト（1つのパーツを管理）
 =========================================================================*/
typedef struct {
	char objname[SIZE_STR]; // オブジェクト名
	int	isVisible;			// 0：非表示　その他：表示
	int	isShadingFlat;      // シェーディングモード
	int	mesh_num;           // メッシュ数
	MQOMesh *mesh_arr;     // メッシュ配列
} MQOObject;

typedef MQOObject *MQOObjectRef;

/*=========================================================================
 【型定義】 MQOモデル（1つのモデルを管理）
 =========================================================================*/
typedef struct {
	unsigned char alpha;		// 頂点配列作成時に指定されたアルファ値（参照用）
    
    // カメラ情報
    glPOINT3f camera_pos;
	glPOINT3f camera_lookat;
	float camera_head;
	float camera_pich;
	int ortho;
	float camera_zoom2;
    
    float amb[3];
            
	int object_num;				// オブジェクト数
	MQOObject obj[MAX_OBJECT];	// オブジェクト配列
    
    char filename[SIZE_STR];    // パス；
    
    int material_num;           // 材質数
    MQOMaterial	*material_arr;  // 材質配列
} MQOModel;

typedef MQOModel* MQOModelRef;

/*=========================================================================
 【関数宣言】
 =========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
    
    
    // モデル生成
    MQOModelRef MQOModelRefCreate(const char *filename, double scale);
        
    // モデルの削除
    void MQOModelRefRelease(MQOModelRef model);
    
    
    
#ifdef __cplusplus
}
#endif






#endif