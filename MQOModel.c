//
//  MQOModel.c
//  cc3MQO
//
//  Created by T.Takabayashi on 11/08/04.
//  Copyright 2011 T.Takabayashi.
// 
#include "MQOModel.h"


/*=========================================================================
 【関数宣言】
 =========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
    
    
    int	mqoLoadFile(MQOModel *model, const char *filename, double scale, unsigned char alpha);

    void mqoReadScene(FILE *fp, MQOModel *model, double scale);

    int mqoReadMaterial(FILE *fp, MQOMaterial M[]);
    int mqoReadObject(FILE *fp, MQOObjectChunk *obj);
    int mqoReadVertex(FILE *fp, glPOINT3f V[]);
    int	mqoReadBVertex(FILE *fp,glPOINT3f V[]);
    int mqoReadFace(FILE *fp, MQOFace F[]);
    
    glPOINT3f *mqoCreateVertexNormals(MQOObjectChunk *obj);
    void mqoSnormal(glPOINT3f A, glPOINT3f B, glPOINT3f C, glPOINT3f *normal);

    
    void mqoMakePolygon(MQOObjectChunk *obj_chunk, 
                        MQOModel *model,
                        glPOINT3f N[], 
                        double scale, 
                        unsigned char alpha);
    
    void mqoGetDirectory(const char *path_file, char *path_dir);
    void endianConverter(void *addr,unsigned int size);

    void mqoMakeVertexArray(MQOMesh *mesh, 
                            MQOFace face_arr[], 
                            int face_num, 
                            glPOINT3f vertex_arr[],
                            glPOINT3f normals_arr[], 
                            double facet, 
                            double scale, 
                            unsigned char alpha);

    void mqoTransformCoordinates (glPOINT3f *v);
    

    
#ifdef __cplusplus
}
#endif




/*=========================================================================
 【関数】MQOModelRefCreate
 【用途】MQOファイルからMQOモデルを作成する
 【引数】
 filename	MQOファイル
 scale		拡大率（1.0でそのまま）
 
 【戻値】MQOModelRef（MQOモデルのポインタ）
 =========================================================================*/
MQOModelRef MQOModelRefCreate(const char *filename, double scale) 
{
	MQOModelRef ret;
    
        
	// 領域確保と初期化
	ret = (MQOModelRef)malloc(sizeof(MQOModel));
    
    if (ret == NULL) {
        return NULL;
    }
    
	memset(ret, 0, sizeof(MQOModel));
        
    if (!mqoLoadFile(ret, filename, scale, (unsigned char)255)) {
        MQOModelRefRelease(ret);
        ret = NULL;
    }
    
	return ret;
}

/*=========================================================================
 【関数】MQOModelRefRelease
 【用途】MQOモデルを削除する
 【引数】
 model	MQOモデル
 
 【戻値】なし
 =========================================================================*/
void MQOModelRefRelease(MQOModelRef model)
{
    if (model == NULL) {
        return;
    }
	int o, m;
	MQOObject *obj;
	MQOMesh *mesh;
    for (o = 0; o < model->object_num; o++) {
        
        obj = &(model->obj[o]);
        for ( m = 0; m < obj->mesh_num; m++ ) {
            mesh = &obj->mesh_arr[m];
            if ( mesh->vertex_num <= 0 ) continue;
            
            // 頂点配列の削除
            if (mesh->locations != NULL) {
                free(mesh->locations);
                mesh->locations = NULL;
            }
            if (mesh->normals != NULL) {
                free(mesh->normals);
                mesh->normals = NULL;
            }
            if (mesh->texture_cordinates != NULL) {
                free(mesh->texture_cordinates);
                mesh->texture_cordinates = NULL;
            }
            
            
        }
        if ( obj->mesh_arr != NULL ) {
            free(obj->mesh_arr);
            obj->mesh_arr = NULL;
        }
        obj->mesh_num = 0;
    }
    
	// マテリアルの開放
	free(model->material_arr);

	
    free(model);
    
}








/*=========================================================================
 【関数】mqoLoadFile
 【用途】メタセコイアファイル(*.mqo)からデータを読み込む
 【引数】
 mqoobj		MQOオブジェクト
 filename	ファイルのパス
 scale		拡大率
 alpha		アルファ
 
 【戻値】成功：1 ／ 失敗：0
 =========================================================================*/

int mqoLoadFile(MQOModel *model, const char *filename, double scale, unsigned char alpha)
{
	FILE *fp;
	MQOObjectChunk obj[MAX_OBJECT];
    
	char buf[SIZE_STR];		// 文字列読み込みバッファ
	int	n_obj = 0;			// オブジェクト数
	int i;
    int res = 1;
    
	// MaterialとObjectの読み込み
	fp = fopen(filename,"rb");
	if (fp==NULL) return 0;
    
	model->alpha = alpha;
	memset(obj, 0, sizeof(obj));
    
	i = 0;
	while (!feof(fp)) {
		fgets(buf, SIZE_STR, fp);
        
		// Scene
		if (strstr(buf, "Scene")) {
            
            mqoReadScene(fp, model, scale);
		}
        
		// Material
		if (strstr(buf, "Material")) {
			sscanf(buf, "Material %d", &model->material_num);
            
			model->material_arr = (MQOMaterial *) calloc(model->material_num, sizeof(MQOMaterial));
            if (mqoReadMaterial(fp, model->material_arr) != model->material_num) {
                res = 0;
                break;
            }
		}
        
		// Object
		if (strstr(buf,"Object")) {
            if (i < MAX_OBJECT) {
                sscanf(buf,"Object %s", obj[i].objname);
                if (mqoReadObject(fp, &obj[i]) != 1) {
                    res = 0;
                    break;
                }
            } else {
                printf("MQOファイル読み込み：　最大オブジェクト数を超えました[%d]\n", i);
            }
            i++;
		}
	}
	n_obj = i;
	fclose(fp);
    
	// パスの取得
    strcpy(model->filename, filename);

    glPOINT3f *normals;
    for (i = 0; i < n_obj; i ++) {
        normals = mqoCreateVertexNormals(&obj[i]);
        mqoMakePolygon(&obj[i],
                       model,
                       normals,
                       scale,
                       alpha);
        free(normals);
    }

    
    
	// オブジェクトのデータの開放
	for (i = 0; i < n_obj; i ++) {
		free(obj[i].V);
		free(obj[i].F);
	}
    
    
	return res;
}


void mqoReadScene(FILE *fp, MQOModel *model, double scale) {
    char buf[SIZE_STR];
    glPOINT3f pos;
	while (!feof(fp)) {
		fgets(buf, SIZE_STR, fp);	// 行読み込み        
		// pos
		if (strstr(buf,"pos ")) {
			sscanf(buf," pos %f %f %f", &pos.x, &pos.y, &pos.z);
            mqoTransformCoordinates(&pos);
            
            model->camera_pos.x = scale * pos.x;
            model->camera_pos.y = scale * pos.y;
            model->camera_pos.z = scale * pos.z;
		}        
		// lookat
        else if (strstr(buf,"lookat ")) {
			sscanf(buf," lookat %f %f %f", &pos.x, &pos.y, &pos.z);
            mqoTransformCoordinates(&pos);

            model->camera_lookat.x = scale * pos.x;
            model->camera_lookat.y = scale * pos.y;
            model->camera_lookat.z = scale * pos.z;
		}
		// head
		else if (strstr(buf,"head ")) {
			sscanf(buf," head %f", &model->camera_head);
		}
		// pich
		else if (strstr(buf,"pich ")) {
			sscanf(buf," pich %f", &model->camera_pich);
		}
		// head
		else if (strstr(buf,"head ")) {
			sscanf(buf," head %f", &model->camera_head);
		}
        // ortho
		else if (strstr(buf,"ortho ")) {
			sscanf(buf," ortho %d", &model->ortho);
		}
        // zoom2
		else if (strstr(buf,"zoom2 ")) {
			sscanf(buf," zoom2 %f", &model->camera_zoom2);
		}
        // amb
		else if (strstr(buf,"amb ")) {
			sscanf(buf," amb %f %f %f", &model->amb[0], &model->amb[1], &model->amb[2]);
		}
        else if (strstr(buf,"}")) break;
	}
    
}

/*=========================================================================
 【関数】mqoReadMaterial
 【用途】マテリアル情報の読み込み
 【引数】
 fp		ファイルポインタ
 M		マテリアル配列
 
 【戻値】読み込んだマテリアル数
 【仕様】mqoCreateModel(), mqoCreateSequence()のサブ関数．
 =========================================================================*/

int mqoReadMaterial(FILE *fp, MQOMaterial M[])
{
	float dif, amb, emi, spc;
	glCOLOR4f c;
	char buf[SIZE_STR];
	char *pStrEnd, *pStr;
	int len;
	int	i = 0;
    
	while (!feof(fp)) {
		fgets(buf,SIZE_STR,fp);	// 行読み込み
		if (strstr(buf,"col(")) {
            
            
            pStr = strstr(buf,"col(");	// 材質名読み飛ばし
            sscanf( pStr,
                   "col(%f %f %f %f) dif (%f) amb(%f) emi(%f) spc(%f) power(%f)",
                   &c.r, &c.g, &c.b, &c.a, &dif, &amb, &emi, &spc, &M[i].power );
            
            // 頂点カラー
            M[i].col = c;
            
            // 拡散光
            M[i].dif[0] = dif * c.r;
            M[i].dif[1] = dif * c.g;
            M[i].dif[2] = dif * c.b;
            M[i].dif[3] = c.a;
            
            // 周囲光
            M[i].amb[0] = amb * c.r;
            M[i].amb[1] = amb * c.g;
            M[i].amb[2] = amb * c.b;
            M[i].amb[3] = c.a;
            
            // 自己照明
            M[i].emi[0] = emi * c.r;
            M[i].emi[1] = emi * c.g;
            M[i].emi[2] = emi * c.b;
            M[i].emi[3] = c.a;
            
            // 反射光
            M[i].spc[0] = spc * c.r;
            M[i].spc[1] = spc * c.g;
            M[i].spc[2] = spc * c.b;
            M[i].spc[3] = c.a;
            
            // tex：模様マッピング名
            if ( (pStr = strstr(buf,"tex(")) != NULL ) {
                M[i].useTex = TRUE;
                
                pStrEnd = strstr(pStr,")")-1;
                len = pStrEnd - (pStr+5);
                strncpy(M[i].texFile,pStr+5,len);
                M[i].texFile[len] = (char)0;
            } else {
                M[i].useTex = FALSE;
                M[i].texFile[0] = (char)0;
            }
            // aplane：アルファマッピング名
            if ( (pStr = strstr(buf,"aplane(")) != NULL ) {
                pStrEnd = strstr(pStr,")")-1;
                len = pStrEnd - (pStr+8);
                strncpy(M[i].alpFile,pStr+8,len);
                M[i].alpFile[len] = (char)0;
            } else {
                M[i].alpFile[0] = (char)0;
            }
            // bump：バンプマッピング名
            if ( (pStr = strstr(buf,"bump(")) != NULL ) {
                pStrEnd = strstr(pStr,")")-1;
                len = pStrEnd - (pStr+6);
                strncpy(M[i].bmpFile,pStr+6,len);
                M[i].bmpFile[len] = (char)0;
            } else {
                M[i].bmpFile[0] = (char)0;
            }
        
            i++;
        } else if (strstr(buf,"}")) break;
	}
    
    return i;
}



/*=========================================================================
 【関数】mqoReadObject
 【用途】オブジェクト情報の読み込み
 【引数】
 fp		ファイルポインタ
 obj		オブジェクト情報
 
 【戻値】なし
 【仕様】この関数で１個のオブジェクト情報が読み込まれる．
 =========================================================================*/

int mqoReadObject(FILE *fp, MQOObjectChunk *obj)
{
	char buf[SIZE_STR];
    int res = 0;
	while (!feof(fp)) {
		fgets(buf,SIZE_STR,fp);
		// visible
		if (strstr(buf,"visible ")) {
			sscanf(buf," visible %d", &obj->visible);
		}
		// shading
		else if (strstr(buf,"shading ")) {
			sscanf(buf," shading %d", &obj->shading);
		}
		// facet
		else if (strstr(buf,"facet ")) {
			sscanf(buf," facet %f", &obj->facet);
		}
		// vertex
		else if (strstr(buf,"vertex ")) {
			sscanf(buf," vertex %d", &obj->n_vertex);
			obj->V = (glPOINT3f*) calloc( obj->n_vertex, sizeof(glPOINT3f) );
			mqoReadVertex(fp, obj->V);
		}
		// BVertex
		else if (strstr(buf,"BVertex"))	{
			sscanf(buf," BVertex %d", &obj->n_vertex);
			obj->V = (glPOINT3f*) calloc( obj->n_vertex, sizeof(glPOINT3f) );
			mqoReadBVertex(fp,obj->V);
		}
		// face
		else if (strstr(buf,"face ")) {
			sscanf(buf," face %d", &obj->n_face);
			obj->F = (MQOFace*) calloc( obj->n_face, sizeof(MQOFace) );
			mqoReadFace(fp, obj->F);
		}
		else if (strstr(buf,"}")) { 
            res = 1;
            break;
        }
        
	}
    
    return res;
}


/*=========================================================================
 【関数】mqoReadVertex
 【用途】頂点情報の読み込み
 【引数】
 fp		現在オープンしているメタセコイアファイルのファイルポインタ
 V		頂点を格納する配列
 
 【戻値】なし
 【仕様】mqoReadObject()のサブ関数
 =========================================================================*/

int mqoReadVertex(FILE *fp, glPOINT3f V[])
{
	char buf[SIZE_STR];
	int i = 0;
    int res = 0;
	while (1) {
		fgets(buf,SIZE_STR,fp);
		if (strstr(buf,"}")) {
            res = 1;
            break;
        }
		sscanf(buf,"%f %f %f",&V[i].x,&V[i].y,&V[i].z);
        mqoTransformCoordinates(&V[i]);
		i++;
	}
    return res;
}


/*=========================================================================
 【関数】mqoReadBVertex
 【用途】バイナリ形式の頂点情報を読み込む
 【引数】
 fp		現在オープンしているメタセコイアファイルのファイルポインタ
 V		頂点を格納する配列
 
 【戻値】頂点数
 【仕様】mqoReadObject()のサブ関数
 =========================================================================*/

int mqoReadBVertex(FILE *fp, glPOINT3f V[])
{
	int n_vertex,i;
	float *wf;
	int size;
	char cw[256];
	char *pStr;
    
	fgets(cw,sizeof(cw),fp);
	if ( (pStr = strstr(cw,"Vector")) != NULL ) {
		sscanf(pStr,"Vector %d [%d]",&n_vertex,&size);	// 頂点数、データサイズを読み込む
	}
	else {
		return -1;
	}
	//MQOファイルのバイナリ頂点データはintel形式（リトルエディアン）
	wf = (float *)malloc(size);
	fread(wf,size,1,fp);
	for ( i = 0; i < n_vertex; i++ ) {
		V[i].x = wf[i*3+0];
		V[i].y = wf[i*3+1];
		V[i].z = wf[i*3+2];
#if DEF_IS_LITTLE_ENDIAN
#else
		endianConverter((void *)&V[i].x,sizeof(V[i].x));
		endianConverter(&V[i].y,sizeof(V[i].y));
		endianConverter(&V[i].z,sizeof(V[i].z));
#endif

        mqoTransformCoordinates(&V[i]);
	
    }
	free(wf);
    
	// "}"まで読み飛ばし
	{
		char buf[SIZE_STR];
		while (1) {
			fgets(buf,SIZE_STR,fp);
			if (strstr(buf,"}")) break;
		}
	}
    
	return n_vertex;
}


/*=========================================================================
 【関数】mqoReadFace
 【用途】面情報の読み込み
 【引数】
 fp		ファイルポインタ
 F		面配列
 
 【戻値】なし
 【仕様】mqoReadObject()のサブ関数
 =========================================================================*/

int mqoReadFace(FILE *fp, MQOFace F[])
{
	char buf[SIZE_STR];
	char *pStr;
	int  i=0;
    
	while (1) {
		fgets(buf,SIZE_STR,fp);
		if (strstr(buf,"}")) break;
        
		// 面を構成する頂点数
		sscanf(buf,"%d",&F[i].n);
        
		// 頂点(V)の読み込み
		if ( (pStr = strstr(buf,"V(")) != NULL ) {
			switch (F[i].n) {
				case 3:
                    //メタセコは頂点の並びが表面からみて右回り
					//sscanf(pStr,"V(%d %d %d)",&F[i].v[0],&F[i].v[1],&F[i].v[2]);
                    sscanf(pStr,"V(%d %d %d)",&F[i].v[2],&F[i].v[1],&F[i].v[0]);
					break;
				case 4:
					//sscanf(pStr,"V(%d %d %d %d)",&F[i].v[0],&F[i].v[1],&F[i].v[2],&F[i].v[3]);
                    sscanf(pStr,"V(%d %d %d %d)",&F[i].v[3],&F[i].v[2],&F[i].v[1],&F[i].v[0]);
					break;
				default:
					break;
			}		
		}
        
		// マテリアル(M)の読み込み
		F[i].material_index = 0;
		if ( (pStr = strstr(buf,"M(")) != NULL ) {
			sscanf(pStr, "M(%d)", &F[i].material_index);
		}
		else { // マテリアルが設定されていない面
			F[i].material_index = -1;
		}
        
		// UVマップ(UV)の読み込み
		if ( (pStr = strstr(buf,"UV(")) != NULL ) {
			switch (F[i].n) {
				case 3:	// 頂点数3
//					sscanf(pStr,"UV(%f %f %f %f %f %f)",
//                           &F[i].uv[0].x, &F[i].uv[0].y,
//                           &F[i].uv[1].x, &F[i].uv[1].y,
//                           &F[i].uv[2].x, &F[i].uv[2].y
//                           );
					sscanf(pStr,"UV(%f %f %f %f %f %f)",
                           &F[i].uv[2].x, &F[i].uv[2].y,
                           &F[i].uv[1].x, &F[i].uv[1].y,
                           &F[i].uv[0].x, &F[i].uv[0].y
                           );
					break;
                    
				case 4:	// 頂点数4
//					sscanf(pStr,"UV(%f %f %f %f %f %f %f %f)",
//                           &F[i].uv[0].x, &F[i].uv[0].y,
//                           &F[i].uv[1].x, &F[i].uv[1].y,
//                           &F[i].uv[2].x, &F[i].uv[2].y,
//                           &F[i].uv[3].x, &F[i].uv[3].y
//                           );
					sscanf(pStr,"UV(%f %f %f %f %f %f %f %f)",
                           &F[i].uv[3].x, &F[i].uv[3].y,
                           &F[i].uv[2].x, &F[i].uv[2].y,
                           &F[i].uv[1].x, &F[i].uv[1].y,
                           &F[i].uv[0].x, &F[i].uv[0].y
                           );
					break;
				default:
					break;
			}		
		}
        
		i++;
	}
    return 1;
}


/*=========================================================================
 【関数】mqoCreateVertexNormal
 【用途】頂点法線の計算
 【引数】
 obj			オブジェクト情報
 
 【戻値】法線配列
 【仕様】４頂点の面は三角形に分割して計算
 戻り値は必ず呼び出し元で解放（free）すること！
 =========================================================================*/

glPOINT3f *mqoCreateVertexNormals(MQOObjectChunk *obj)
{
	int f;
	int v;
	int i;
	double len;
	glPOINT3f fnormal;	// 面法線ベクトル
	MQOFace *F;
	glPOINT3f *V;
	glPOINT3f *ret;
	F = obj->F;
	V = obj->V;
	ret = (glPOINT3f *)calloc(obj->n_vertex,sizeof(glPOINT3f));
	//面の法線を頂点に足し込み
	for ( f = 0; f < obj->n_face; f++ ) {
		if ( obj->F[f].n == 3 ) {
			mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&fnormal);
			for ( i = 0; i < 3; i++ ) {
				ret[F[f].v[i]].x += fnormal.x;
				ret[F[f].v[i]].y += fnormal.y;
				ret[F[f].v[i]].z += fnormal.z;
			}
		}
		if ( obj->F[f].n == 4 ) {
			mqoSnormal(V[F[f].v[0]],V[F[f].v[1]],V[F[f].v[2]],&fnormal);
			for ( i = 0; i < 4; i++ ) {
				if ( i == 3 ) continue;
				ret[F[f].v[i]].x += fnormal.x;
				ret[F[f].v[i]].y += fnormal.y;
				ret[F[f].v[i]].z += fnormal.z;
			}
			mqoSnormal(V[F[f].v[0]],V[F[f].v[2]],V[F[f].v[3]],&fnormal);
			for ( i = 0; i < 4; i++ ) {
				if ( i == 1 ) continue;
				ret[F[f].v[i]].x += fnormal.x;
				ret[F[f].v[i]].y += fnormal.y;
				ret[F[f].v[i]].z += fnormal.z;
			}
		}
	}
	//正規化
	for ( v = 0; v < obj->n_vertex; v++ ) {
		if ( ret[v].x == 0 && ret[v].y == 0 && ret[v].z == 0 ) {
			//面に使われてない点
			continue;
		}
		len = sqrt(ret[v].x*ret[v].x + ret[v].y*ret[v].y + ret[v].z*ret[v].z);
		if ( len != 0 ) {
			ret[v].x = ret[v].x/len;
			ret[v].y = ret[v].y/len;
			ret[v].z = ret[v].z/len;
		}
	}
    
	return ret;
}

/*=========================================================================
 【関数】mqoSnormal
 【用途】法線ベクトルを求める
 【引数】
 A		3次元座標上の点A
 B		3次元座標上の点B
 C		3次元座標上の点C
 *normal	ベクトルBAとベクトルBCの法線ベクトル（右ねじ方向）
 
 【戻値】なし
 【仕様】メタセコイアにおいて面を構成する頂点の番号は，表示面から見て
 時計回りに記述してある．つまり，頂点A,B,C があったとき，
 求めるべき法線はBAとBCの外積によって求められる
 =========================================================================*/

void mqoSnormal(glPOINT3f A, glPOINT3f B, glPOINT3f C, glPOINT3f *normal)
{
	double norm;
	glPOINT3f vec0,vec1;
    
	// ベクトルBA
	vec0.x = A.x - B.x; 
	vec0.y = A.y - B.y;
	vec0.z = A.z - B.z;
    
	// ベクトルBC
	vec1.x = C.x - B.x;
	vec1.y = C.y - B.y;
	vec1.z = C.z - B.z;
        
	// 法線ベクトル
	normal->x = vec0.y * vec1.z - vec0.z * vec1.y;
	normal->y = vec0.z * vec1.x - vec0.x * vec1.z;
	normal->z = vec0.x * vec1.y - vec0.y * vec1.x;
    
	// 正規化
	norm = normal->x * normal->x + normal->y * normal->y + normal->z * normal->z;
	norm = sqrt ( norm );
    
	normal->x /= norm;
	normal->y /= norm;
	normal->z /= norm;
}


/*=========================================================================
 【関数】mqoMakePolygon
 【用途】ポリゴンの生成
 【引数】
 readObj		読み込んだオブジェクト情報
 mqoobj		MQOオブジェクト 
 N[]			法線配列
 M[]			マテリアル配列
 n_mat		マテリアル数
 scale		スケール
 alpha		アルファ
 
 【戻値】なし
 =========================================================================*/

void mqoMakePolygon(MQOObjectChunk *obj_chunk, 
                    MQOModel *model,
					glPOINT3f N[], 
                    double scale, 
                    unsigned char alpha)
{
    
	MQOObject *obj;
	MQOMesh *mesh;
	int i, f, m, *mat_vnum;
	int	face_num, mat_index;
	MQOFace *face_arr;
	glPOINT3f *V;
	double facet;
    
    
	obj = &model->obj[model->object_num];
	strcpy(obj->objname, obj_chunk->objname);
	obj->isVisible = obj_chunk->visible;
	obj->isShadingFlat = (obj_chunk->shading == 0);
	face_arr = obj_chunk->F;
	face_num = obj_chunk->n_face;
	V = obj_chunk->V;
	facet = obj_chunk->facet;
    
	// faceの中でのマテリアル毎の頂点の数
	// M=NULLのとき、F[].m = 0 が入ってくる
    
    mat_vnum = (int *)calloc(model->material_num + 1, sizeof(int));
    
    
	for ( f = 0; f < face_num; f++ ){
        
		if( face_arr[f].material_index < 0 ) {
            mat_index = model->material_num;
        } else {
            mat_index = face_arr[f].material_index;
        }
		if ( face_arr[f].n == 3 ) {
			mat_vnum[mat_index] += 3;
		}
		if ( face_arr[f].n == 4 ) {
			//４頂点（四角）は３頂点（三角）ｘ２に分割
			//  0  3      0    0  3
			//   □   →　△　　▽
			//  1  2     1  2   2
			// ４頂点の平面データは
			// ３頂点の平面データｘ２個
			mat_vnum[mat_index] += 3 * 2;
		}
	}
    
    obj->mesh_num = 0;
    for (i = 0; i < model->material_num + 1; i ++) {
        if (mat_vnum[i] > 0) {
            obj->mesh_num ++;
        }
    }
    
    
	// マテリアル別に頂点配列を作成する
	obj->mesh_arr = (MQOMesh *)malloc(sizeof(MQOMesh) * obj->mesh_num);
	memset(obj->mesh_arr, 0, sizeof(MQOMesh) * obj->mesh_num);
    
    int mesh_index = 0;
	for (m = 0; m < model->material_num + 1; m ++ ) {
        if (mat_vnum[m] < 1) {
            continue;
        }
		mesh = &obj->mesh_arr[mesh_index];
        mesh_index ++;
        
        mesh->vertex_num = mat_vnum[m];
        mesh->material_index = m;
        
        
        mesh->isValidMaterialInfo = (model->material_arr != NULL);
        

		mqoMakeVertexArray(mesh, face_arr, face_num, V, N, facet, scale, alpha);

	}
	model->object_num ++;
	if ( MAX_OBJECT <= model->object_num ) {
		printf("MQOファイル読み込み：　最大オブジェクト数を超えました[%d]\n",model->object_num);
		model->object_num = MAX_OBJECT-1;
	}
    
	free(mat_vnum);
    
}



void mqoSetMeshVertex(MQOMesh *mesh, 
                      glPOINT3f vertex, 
                      glPOINT3f vnormal, 
                      glPOINT3f normal, 
                      glPOINT2f uv,
                      int dpos, 
                      double facet, 
                      double scale) 
{
	double s;

    // locations
    mesh->locations[dpos * 3] = vertex.x * scale;
    mesh->locations[dpos * 3 + 1] = vertex.y * scale;
    mesh->locations[dpos * 3 + 2] = vertex.z * scale;
    
    
    // UV
    mesh->texture_cordinates[dpos * 2] = uv.x;
    mesh->texture_cordinates[dpos * 2 + 1] = uv.y;
    s = acos(normal.x * vnormal.x 
             + normal.y * vnormal.y 
             + normal.z * vnormal.z);
    if ( facet < s) {
        // スムージング角　＜（頂点法線と面法線の角度）のときは面法線を頂点法線とする
        mesh->normals[dpos * 3] = normal.x;
        mesh->normals[dpos * 3 + 1] = normal.y;
        mesh->normals[dpos * 3 + 2] = normal.z;
    }
    else {
        mesh->normals[dpos * 3] = vnormal.x;
        mesh->normals[dpos * 3 + 1] = vnormal.y;
        mesh->normals[dpos * 3 + 2] = vnormal.z;
    }
    
    
}


/*=========================================================================
 【関数】mqoMakeVertexArray
 【用途】頂点配列の作成
 【引数】
 mesh		メッシュ（この中に頂点データを含む）
 face_arr		面
 face_num	面数
 vertex_arr		頂点
 normals_arr		法線
 facet	スムージング角
 mcol	色
 scale	スケール
 alpha	アルファ
 
 【戻値】なし
 【仕様】頂点配列はすべて三角にするので、四角は三角ｘ２に分割
 0  3      0    0  3
 □   →　△　　▽
 1  2     1  2   2  
 =========================================================================*/
void mqoMakeVertexArray(MQOMesh *mesh, 
                        MQOFace face_arr[], 
                        int face_num, 
                        glPOINT3f vertex_arr[],
                        glPOINT3f normals_arr[], 
                        double facet, 
                        double scale, 
                        unsigned char alpha)
{
	int f;
	int i;
	int dpos;
	glPOINT3f normal;	// 法線ベクトル
	MQOFace face;
	dpos = 0;

    
    mesh->locations = (float *)calloc(mesh->vertex_num * 3, sizeof(float));
    mesh->normals = (float *)calloc(mesh->vertex_num * 3, sizeof(float));
    mesh->texture_cordinates = (float *)calloc(mesh->vertex_num * 2, sizeof(float));
    
    
    
	for (f = 0; f < face_num; f++){
        face = face_arr[f];
        if ( face.material_index != mesh->material_index ) continue;
        if ( face.n == 3 ) {
            // 法線ベクトルを計算
            mqoSnormal(vertex_arr[face.v[0]],
                       vertex_arr[face.v[1]],
                       vertex_arr[face.v[2]],
                       &normal);	
			for ( i = 0; i < 3; i++ ) {
                mqoSetMeshVertex(mesh, 
                                 vertex_arr[face.v[i]], 
                                 normals_arr[face.v[i]], 
                                 normal, 
                                 face.uv[i],
                                 dpos, 
                                 facet, 
                                 scale);
                dpos++;
            }
        }
        //４頂点（四角）は３頂点（三角）ｘ２に分割
        if ( face.n == 4 ) {
            mqoSnormal(vertex_arr[face.v[0]],
                       vertex_arr[face.v[1]],
                       vertex_arr[face.v[2]],
                       &normal);	// 法線ベクトルを計算
            for ( i = 0; i < 4; i++ ) {
                if ( i == 3 ) continue;
                mqoSetMeshVertex(mesh, 
                                 vertex_arr[face.v[i]], 
                                 normals_arr[face.v[i]], 
                                 normal, 
                                 face.uv[i],
                                 dpos, 
                                 facet, 
                                 scale);
                dpos++;
            }
            // 法線ベクトルを計算
            mqoSnormal(vertex_arr[face.v[0]],
                       vertex_arr[face.v[2]],
                       vertex_arr[face.v[3]],
                       &normal);
            for ( i = 0; i < 4; i++ ) {
                if ( i == 1 ) continue;
                mqoSetMeshVertex(mesh, 
                                 vertex_arr[face.v[i]], 
                                 normals_arr[face.v[i]], 
                                 normal, 
                                 face.uv[i],
                                 dpos, 
                                 facet, 
                                 scale);
                dpos++;
            }
        }
    }
}




/*=========================================================================
 【関数】mqoGetDirectory
 【用途】ファイル名を含むパス文字列からディレクトリのパスのみを抽出する
 【引数】
 *path_file	ファイル名を含むパス文字列（入力）
 *path_dir	ファイル名を除いたパス文字列（出力）
 
 【戻値】なし
 【仕様】例：
 "C:/data/file.bmp" → "C:/data/"
 "data/file.mqo"    → "data/"
 =========================================================================*/

void mqoGetDirectory(const char *path_file, char *path_dir)
{
	char *pStr;
	int len;
    
	pStr = MAX( strrchr(path_file,'\\'), strrchr(path_file,'/') );
	len = MAX((int)(pStr-path_file)+1,0);
	strncpy(path_dir,path_file,len);
	path_dir[len] = (char)0;
}


/*=========================================================================
 【関数】endianConverter
 【用途】エンディアン変換
 【引数】
 addr	アドレス
 size	サイズ
 
 【戻値】なし
 =========================================================================*/

void endianConverter(void *addr,unsigned int size)
{
	unsigned int pos;
	char c;
	if ( size <= 1 ) return;
	for ( pos = 0; pos < size/2; pos++ ) {
		c = *(((char *)addr)+pos);
		*(((char *)addr)+pos) = *(((char *)addr)+(size-1 - pos));
		*(((char *)addr)+(size-1 - pos)) = c;
	}
}



//座標変換
void mqoTransformCoordinates (glPOINT3f *v){
//    v->x = v->x * (-1) ;
//    *y = *y * (-1) ;
//    v->z = v->z * (-1) ;
}


