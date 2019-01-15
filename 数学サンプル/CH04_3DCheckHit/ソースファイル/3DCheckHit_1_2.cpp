//------------------------------------------------------------
// 3DCheckHit_1_2.cpp
// 球と線分の当たり判定
// 
//------------------------------------------------------------

#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>

#define VIEW_WIDTH					800					// 画面幅
#define VIEW_HEIGHT					600					// 画面高さ

#define PI							3.1415927f			// 円周率
#define ROT_SPEED					( PI / 100.0f )		// 回転速度
#define CORNER_NUM					20					// 角数
#define PLAYER_SPEED				0.08f				// プレイヤーの移動速度

#define SPHERE_R					1.0f				// 球の半径
#define CYLINDER_SP_R				0.5f				// 当たり判定領域の半径
#define CYLINDER_SP_LEN				2.0f				// 当たり判定領域の長さ


// 頂点構造体
struct CUSTOMVERTEX {
    XMFLOAT4	v4Pos;
	XMFLOAT2	v2UV;
};


// 当たり判定領域構造体
struct CYLINDER_SP {
	XMFLOAT3			v3Pos;					// 始点位置
	XMFLOAT3			v3Vec;					// 線分ベクトル
	float				fLen;					// 長さ
	float				r;						// 半径
	float				fRot_z;					// z軸中心回転角
	float				fRot_y;					// y軸中心回転角
	XMMATRIX			matTransform;			// 姿勢変換行列
};


struct MY_PLAYER {
	XMFLOAT3			v3Pos;					// 位置
	float				r;						// 半径
};


CYLINDER_SP	HitArea;							// 当たり判定領域
MY_PLAYER	Player_1;							// プレイヤーデータ


// ベクトルの引き算
XMFLOAT3 Subtract( XMFLOAT3 *pv3Vec1, XMFLOAT3 *pv3Vec2 )
{
	return XMFLOAT3( pv3Vec1->x - pv3Vec2->x,
					 pv3Vec1->y - pv3Vec2->y,
					 pv3Vec1->z - pv3Vec2->z );
}


//
/// 線分と球の当たり判定
///
int CheckHit( XMFLOAT3 *pv3LineStart, XMFLOAT3 *pv3LineVec, float fLine_r,
			  XMFLOAT3 *pv3SphereCenter, float fSphere_r )
{
	int				nResult = false;
	float			dx, dy, dz;						// 位置の差分
	float			t;
	float			mx, my, mz;						// 最小の距離を与える座標
	float			ar;								// 2半径を足したもの

	float			fDistSqr;
	
	//線分の起点から円の中心までのベクトルを(dx,dy,dz)とする
	//ヒント：終点から始点を引く
	dx = 0;
	dy = 0;
	dz = 0;

	//内積÷「線分の長さ」＝射影長
	//変数tに射影長を代入してください。
	//tは線分の長さを1.0としたときの射影の長さの割合
	t =0;

	//線分の外に射影点が出てしまった時に線分の端っこに丸めてる
	if ( t < 0.0f ) t = 0.0f;			// tの下限
	if ( t > 1.0f ) t = 1.0f;			// tの上限

	//射影ベクトルの作成(mx,my,mz)
	//射影長×線分のベクトル＝球体がいる位置までのベクトル
	mx =0;//
	my =0;//
	mz =0;//

	//球体から線分への垂線ベクトル=射影ベクトル-球体へのベクトル
	//これを二乗することで球の中心からの最短距離を計算できる
	fDistSqr = 0;

	ar = fLine_r + fSphere_r;//線分の太さと球の半径足す
	if ( fDistSqr < ar * ar ) {						// ２乗のまま比較
		nResult = true;
	}

	return nResult;
}


// エリア初期化
int InitArea( void )
{
	XMVECTOR			vTrans;

	HitArea.v3Pos = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	HitArea.v3Vec = XMFLOAT3( CYLINDER_SP_LEN, 0.0f, 0.0f );
	HitArea.fLen = CYLINDER_SP_LEN;
	HitArea.r = CYLINDER_SP_R;
	HitArea.fRot_z = PI / 6.0f;
	HitArea.fRot_y = PI / 4.0f;
	HitArea.matTransform = XMMatrixRotationZ( HitArea.fRot_z ) * XMMatrixRotationY( HitArea.fRot_y );
	vTrans = XMLoadFloat3( &( HitArea.v3Vec ) );
	vTrans = XMVector3Transform( vTrans, HitArea.matTransform );
	XMStoreFloat3( &( HitArea.v3Vec ), vTrans );

	return 0;
}


int MoveArea( void )
{
	return 0;
}


int InitPlayer( void )									// プレイヤーの初期化
{
	// プレイヤー1
	Player_1.v3Pos = XMFLOAT3( 0.0f, 0.0f, -4.0f );
	Player_1.r = SPHERE_R;

	return 0;
}


int MovePlayer( void )									// 球の移動
{
	// 左
	if ( GetAsyncKeyState( VK_LEFT ) ) {
		Player_1.v3Pos.x -= PLAYER_SPEED;
	}
	// 右
	if ( GetAsyncKeyState( VK_RIGHT ) ) {
		Player_1.v3Pos.x += PLAYER_SPEED;
	}
	// 奥
	if ( GetAsyncKeyState( VK_UP ) ) {
		Player_1.v3Pos.z += PLAYER_SPEED;
	}
	// 手前
	if ( GetAsyncKeyState( VK_DOWN ) ) {
		Player_1.v3Pos.z -= PLAYER_SPEED;
	}
	// 上
	if ( GetAsyncKeyState( 'Z' ) ) {
		Player_1.v3Pos.y += PLAYER_SPEED;
	}
	// 下
	if ( GetAsyncKeyState( 'X' ) ) {
		Player_1.v3Pos.y -= PLAYER_SPEED;
	}

	return 0;
}



XMMATRIX CreateWorldMatrix( float x, float y, float z, float fSize )	// ワールド行列の生成
{
	float			fAngleY;							// y軸周り回転角
	static float	fAngleX = 0.0f;						// x軸周り回転角
	XMMATRIX		matRot_Y;							// y軸周り回転行列
	XMMATRIX		matRot_X;							// x軸周り回転行列
	XMMATRIX		matScaleTrans;						// 拡大縮小平行移動行列

	// 強制回転
	fAngleY = 2.0f * PI * ( float )( timeGetTime() % 3000 ) / 3000.0f;

	// 行列作成
	matRot_Y = XMMatrixRotationY( fAngleY );
	matRot_X = XMMatrixRotationX( fAngleX );

	matScaleTrans= XMMatrixIdentity();
	matScaleTrans._11 = fSize;
	matScaleTrans._22 = fSize;
	matScaleTrans._33 = fSize;
	matScaleTrans._41 = x;
	matScaleTrans._42 = y;
	matScaleTrans._43 = z;

	return matRot_Y * matRot_X * matScaleTrans;		// 変換の合成
}


int MakeSphereIndexed( float x, float y, float z, float r,
					   CUSTOMVERTEX *pVertices, int *pVertexNum,
					   WORD *pIndices, int *pIndexNum,
					   int nIndexOffset )			// 球の作成(中心位置とインデックス付き)
{
	int					i, j;
	float				fTheta;
	float				fPhi;
	float				fAngleDelta;
	int					nIndex;						// データのインデックス
	int					nIndexY;					// x方向インデックス

	// 頂点データ作成
	fAngleDelta = 2.0f * PI / CORNER_NUM;
	nIndex = 0;
	fTheta = 0.0f;
	for ( i = 0; i < CORNER_NUM / 2 + 1; i++ ) {
		fPhi = 0.0f;
		for ( j = 0; j < CORNER_NUM + 1; j++ ) {
			pVertices[nIndex].v4Pos  = XMFLOAT4( x + r * sinf( fTheta ) * cosf( fPhi ),
												 y + r * cosf( fTheta ),
												 z + r * sinf( fTheta ) * sinf( fPhi ), 1.0f );
			pVertices[nIndex].v2UV = XMFLOAT2( fPhi / ( 2.0f * PI ), fTheta / PI );
			nIndex++;
			fPhi += fAngleDelta;
		}
		fTheta += fAngleDelta;
	}
	*pVertexNum = nIndex;

	// インデックスデータ作成
	nIndex = 0;
	for ( i = 0; i < CORNER_NUM; i++ ) {
		for ( j = 0; j < CORNER_NUM / 2; j++ ) {
			nIndexY = j * ( CORNER_NUM + 1 );
			pIndices[nIndex    ] = nIndexOffset + nIndexY + i;
			pIndices[nIndex + 1] = nIndexOffset + nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 2] = nIndexOffset + nIndexY + i + 1;
			nIndex += 3;
			pIndices[nIndex    ] = nIndexOffset + nIndexY + i + 1;
			pIndices[nIndex + 1] = nIndexOffset + nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 2] = nIndexOffset + nIndexY + ( CORNER_NUM + 1 ) + i + 1;
			nIndex += 3;
		}
	}
	*pIndexNum = nIndex;

	return 0;
}


int MakeConeIndexed( float fHeight, float r,
					 CUSTOMVERTEX *pVertices, int *pVertexNum,
					 WORD *pIndices, int *pIndexNum,
					 int nIndexOffset )				// 円錐の作成(インデックス付き)
{
	int					i, j;
	float				fTheta;
	float				fAngleDelta;
	int					nIndex;						// データのインデックス

	// 頂点データ作成
	fAngleDelta = 2.0f * PI / CORNER_NUM;
	nIndex = 0;
	pVertices[nIndex].v4Pos  = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	pVertices[nIndex].v2UV = XMFLOAT2( 0.5f, 1.0f );
	nIndex++;
	fTheta = 0.0f;
	for ( j = 0; j < CORNER_NUM + 1; j++ ) {
		pVertices[nIndex].v4Pos  = XMFLOAT4( r * cosf( fTheta ),
											 fHeight,
											 r * sinf( fTheta ), 1.0f );
		pVertices[nIndex].v2UV = XMFLOAT2( fTheta / ( 2.0f * PI ), 0.5f );
		nIndex++;
		fTheta += fAngleDelta;
	}
	pVertices[nIndex].v4Pos  = XMFLOAT4( 0.0f, fHeight, 0.0f, 1.0f );
	pVertices[nIndex].v2UV = XMFLOAT2( 0.5f, 0.0f );
	nIndex++;
	*pVertexNum = nIndex;

	// インデックスデータ作成
	nIndex = 0;
	for ( i = 0; i < CORNER_NUM; i++ ) {
		pIndices[nIndex    ] = nIndexOffset + 0;
		pIndices[nIndex + 1] = nIndexOffset + i + 1 + 1;
		pIndices[nIndex + 2] = nIndexOffset + i + 1;
		nIndex += 3;
		pIndices[nIndex    ] = nIndexOffset + CORNER_NUM + 2;
		pIndices[nIndex + 1] = nIndexOffset + i + 1;
		pIndices[nIndex + 2] = nIndexOffset + i + 1 + 1;
		nIndex += 3;
	}
	*pIndexNum = nIndex;

	return 0;
}


int MakeCylinderSpIndexed( float x, float y, float z, float fLen, float r,
						   CUSTOMVERTEX *pVertices, int *pVertexNum,
						   WORD *pIndices, int *pIndexNum )		// 両端に球が付いた円筒の作成
{
	int					i, j;
	float				fTheta;
	float				fPhi;
	float				fAngleDelta;
	int					nIndex;						// データのインデックス
	int					nIndex2Base;				// データのインデックス2基底
	int					nIndex3Base;				// データのインデックス3基底
	int					nIndexY;					// x方向インデックス

	// 頂点データ作成
	// 半球部
	fAngleDelta = 2.0f * PI / CORNER_NUM;
	nIndex = 0;
	nIndex2Base = ( CORNER_NUM + 1 ) * ( CORNER_NUM / 4 + 1 );
	fTheta = 0.0f;
	for ( i = 0; i < CORNER_NUM / 4 + 1; i++ ) {
		fPhi = 0.0f;
		for ( j = 0; j < CORNER_NUM + 1; j++ ) {
			pVertices[nIndex].v4Pos  = XMFLOAT4( x + fLen + r * cosf( fTheta ),
												 y + r * sinf( fTheta ) * cosf( fPhi ),
												 z + r * sinf( fTheta ) * sinf( fPhi ), 1.0f );
			pVertices[nIndex].v2UV = XMFLOAT2( fPhi / ( 2.0f * PI ), fTheta / PI );
			pVertices[nIndex + nIndex2Base].v4Pos  = XMFLOAT4( x - r * cosf( fTheta ),
															   y + r * sinf( fTheta ) * cosf( fPhi ),
															   z + r * sinf( fTheta ) * sinf( fPhi ), 1.0f );
			pVertices[nIndex + nIndex2Base].v2UV = XMFLOAT2( fPhi / ( 2.0f * PI ), fTheta / PI );
			nIndex++;
			fPhi += fAngleDelta;
		}
		fTheta += fAngleDelta;
	}
	nIndex3Base = nIndex * 2;

	// 円筒部
	nIndex = nIndex3Base;
	fAngleDelta = 2.0f * PI / CORNER_NUM;
	fTheta = 0.0f;
	for ( i = 0; i < CORNER_NUM + 1; i++ ) {
		pVertices[nIndex].v4Pos  = XMFLOAT4( x,
											 y + r * cosf( fTheta ),
											 z + r * sinf( fTheta ), 1.0f );
		pVertices[nIndex].v2UV = XMFLOAT2( 0.0f, fTheta / ( 2.0f * PI ) );
		nIndex++;
		pVertices[nIndex].v4Pos  = XMFLOAT4( x + fLen,
											 y + r * cosf( fTheta ),
											 z + r * sinf( fTheta ), 1.0f );
		pVertices[nIndex].v2UV = XMFLOAT2( 1.0f, fTheta / ( 2.0f * PI ) );
		nIndex++;
		fTheta += fAngleDelta;
	}
	*pVertexNum = nIndex;

	// インデックスデータ作成
	nIndex = 0;
	// 半球部
	for ( i = 0; i < CORNER_NUM; i++ ) {
		for ( j = 0; j < CORNER_NUM / 4; j++ ) {
			nIndexY = j * ( CORNER_NUM + 1 );
			pIndices[nIndex    ] = nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 1] = nIndexY + i;
			pIndices[nIndex + 2] = nIndexY + i + 1;
			nIndex += 3;
			pIndices[nIndex    ] = nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 1] = nIndexY + i + 1;
			pIndices[nIndex + 2] = nIndexY + ( CORNER_NUM + 1 ) + i + 1;
			nIndex += 3;
		}
	}
	for ( i = 0; i < CORNER_NUM; i++ ) {
		for ( j = 0; j < CORNER_NUM / 4; j++ ) {
			nIndexY = j * ( CORNER_NUM + 1 );
			pIndices[nIndex    ] = nIndex2Base + nIndexY + i;
			pIndices[nIndex + 1] = nIndex2Base + nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 2] = nIndex2Base + nIndexY + i + 1;
			nIndex += 3;
			pIndices[nIndex    ] = nIndex2Base + nIndexY + i + 1;
			pIndices[nIndex + 1] = nIndex2Base + nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 2] = nIndex2Base + nIndexY + ( CORNER_NUM + 1 ) + i + 1;
			nIndex += 3;
		}
	}
	// 円筒部
	for ( i = 0; i < CORNER_NUM; i++ ) {
		pIndices[nIndex    ] = nIndex3Base + ( i * 2 );
		pIndices[nIndex + 1] = nIndex3Base + ( ( i + 1 ) * 2 ) + 1;
		pIndices[nIndex + 2] = nIndex3Base + ( ( i + 1 ) * 2 );
		nIndex += 3;
		pIndices[nIndex    ] = nIndex3Base + ( ( i + 1 ) * 2 ) + 1;
		pIndices[nIndex + 1] = nIndex3Base + ( i * 2 );
		pIndices[nIndex + 2] = nIndex3Base + ( i * 2 ) + 1;
		nIndex += 3;
	}
	*pIndexNum = nIndex;

	return 0;
}


//------------------------------------------------------------
// 以下、DirectXによる表示プログラム

#include <stdio.h>
#include <windows.h>
#include <tchar.h>								// Unicode・マルチバイト文字関係


#define MAX_BUFFER_VERTEX				10000	// 最大バッファ頂点数
#define MAX_BUFFER_INDEX				20000	// 最大バッファインデックス数
#define MAX_MODEL_NUM					100		// 最大モデル数


// リンクライブラリ
#pragma comment( lib, "d3d11.lib" )   // D3D11ライブラリ
#pragma comment( lib, "d3dx11.lib" )
#pragma comment( lib, "winmm.lib" )


// セーフリリースマクロ
#ifndef SAFE_RELEASE
#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release(); ( p )=NULL; } }
#endif

// シェーダ定数構造体
struct CBNeverChanges
{
    XMMATRIX mView;
	XMFLOAT4 v4AddColor;
};

// テクスチャ絵構造体
struct TEX_PICTURE {
	ID3D11ShaderResourceView	*pSRViewTexture;
	D3D11_TEXTURE2D_DESC		tdDesc;
	int							nWidth, nHeight;
};

// モデル構造体
struct MY_MODEL {
	int					nVertexPos;						// 頂点位置
	int					nVertexNum;						// 頂点数
	int					nIndexPos;						// インデックス位置
	int					nIndexNum;						// インデックス数
	TEX_PICTURE			*ptpTexture;					// テクスチャ
	XMMATRIX			mMatrix;						// 変換行列
	XMFLOAT4			v4AddColor;						// 加算色
};


// グローバル変数
UINT  g_nClientWidth;							// 描画領域の横幅
UINT  g_nClientHeight;							// 描画領域の高さ

HWND        g_hWnd;         // ウィンドウハンドル


ID3D11Device			*g_pd3dDevice;			// デバイス
IDXGISwapChain			*g_pSwapChain;			// DXGIスワップチェイン
ID3D11DeviceContext		*g_pImmediateContext;	// デバイスコンテキスト
ID3D11RasterizerState	*g_pRS;					// ラスタライザ
ID3D11RasterizerState	*g_pRS_Cull_CW;			// ラスタライザ(時計回りカリング)
ID3D11RasterizerState	*g_pRS_Cull_CCW;		// ラスタライザ(反時計回りカリング)
ID3D11RenderTargetView	*g_pRTV;				// レンダリングターゲット
ID3D11Texture2D*        g_pDepthStencil = NULL;	// Zバッファ
ID3D11DepthStencilView* g_pDepthStencilView = NULL;	// Zバッファのビュー
ID3D11DepthStencilState *g_pDSDepthState = NULL;	// Zバッファのステート
ID3D11DepthStencilState *g_pDSDepthState_NoWrite = NULL;	// Zバッファのステート(Zバッファへ書き込みなし)
D3D_FEATURE_LEVEL       g_FeatureLevel;			// フィーチャーレベル

ID3D11Buffer			*g_pVertexBuffer;		// 頂点バッファ
ID3D11Buffer			*g_pIndexBuffer;		// インデックスバッファ
ID3D11BlendState		*g_pbsAddBlend;			// 加算ブレンド
ID3D11VertexShader		*g_pVertexShader;		// 頂点シェーダ
ID3D11PixelShader		*g_pPixelShader;		// ピクセルシェーダ
ID3D11InputLayout		*g_pInputLayout;		// シェーダ入力レイアウト
ID3D11SamplerState		*g_pSamplerState;		// サンプラステート

ID3D11Buffer			*g_pCBNeverChanges = NULL;

// 描画頂点バッファ
CUSTOMVERTEX g_cvVertices[MAX_BUFFER_VERTEX];
int							g_nVertexNum = 0;

WORD		g_wIndices[MAX_BUFFER_INDEX];
int							g_nIndexNum = 0;

TEX_PICTURE				g_tSphere1Texture, g_tSphere2Texture;
MY_MODEL					g_mmPlayer;
MY_MODEL					g_mmHit;


// Direct3Dの初期化
HRESULT InitD3D( void )
{
    HRESULT hr = S_OK;
	D3D_FEATURE_LEVEL  FeatureLevelsRequested[6] = { D3D_FEATURE_LEVEL_11_0,
													 D3D_FEATURE_LEVEL_10_1,
													 D3D_FEATURE_LEVEL_10_0,
													 D3D_FEATURE_LEVEL_9_3,
													 D3D_FEATURE_LEVEL_9_2,
													 D3D_FEATURE_LEVEL_9_1 };
	UINT               numLevelsRequested = 6;
	D3D_FEATURE_LEVEL  FeatureLevelsSupported;

	// デバイス作成
	hr = D3D11CreateDevice( NULL,
					D3D_DRIVER_TYPE_HARDWARE, 
					NULL, 
					0,
					FeatureLevelsRequested, 
					numLevelsRequested,
					D3D11_SDK_VERSION, 
					&g_pd3dDevice,
					&FeatureLevelsSupported,
					&g_pImmediateContext );
	if( FAILED ( hr ) ) {
		return hr;
	}

	// ファクトリの取得
	IDXGIDevice * pDXGIDevice;
	hr = g_pd3dDevice->QueryInterface( __uuidof( IDXGIDevice ), ( void ** )&pDXGIDevice );
	IDXGIAdapter * pDXGIAdapter;
	hr = pDXGIDevice->GetParent( __uuidof( IDXGIAdapter ), ( void ** )&pDXGIAdapter );
	IDXGIFactory * pIDXGIFactory;
	pDXGIAdapter->GetParent( __uuidof( IDXGIFactory ), ( void ** )&pIDXGIFactory);

	// スワップチェインの作成
    DXGI_SWAP_CHAIN_DESC	sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = g_nClientWidth;
	sd.BufferDesc.Height = g_nClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	hr = pIDXGIFactory->CreateSwapChain( g_pd3dDevice, &sd, &g_pSwapChain );

	pDXGIDevice->Release();
	pDXGIAdapter->Release();
	pIDXGIFactory->Release();

	if( FAILED ( hr ) ) {
		return hr;
	}

    // レンダリングターゲットの生成
    ID3D11Texture2D			*pBackBuffer = NULL;
    D3D11_TEXTURE2D_DESC BackBufferSurfaceDesc;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't get backbuffer." ), _T( "Error" ), MB_OK );
        return hr;
    }
    pBackBuffer->GetDesc( &BackBufferSurfaceDesc );
    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRTV );
    SAFE_RELEASE( pBackBuffer );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create render target view." ), _T( "Error" ), MB_OK );
        return hr;
    }

    // *** Create depth stencil texture ***
    D3D11_TEXTURE2D_DESC descDepth;
	RECT rc;
    GetClientRect( g_hWnd, &rc );
	ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = rc.right - rc.left;
    descDepth.Height = rc.bottom - rc.top;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // *** Create the depth stencil view ***
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

	// *** レンダリングターゲット設定 ***
    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRTV, g_pDepthStencilView );

	// ステンシルステートの作成
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	hr = g_pd3dDevice->CreateDepthStencilState( &dsDesc, &g_pDSDepthState );

	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = g_pd3dDevice->CreateDepthStencilState( &dsDesc, &g_pDSDepthState_NoWrite );

//	g_pImmediateContext->OMSetDepthStencilState( g_pDSDepthState, 1 );

    // ラスタライザの設定
    D3D11_RASTERIZER_DESC drd;
	ZeroMemory( &drd, sizeof( drd ) );
	drd.FillMode				= D3D11_FILL_SOLID;
	drd.CullMode				= D3D11_CULL_NONE;
	drd.FrontCounterClockwise	= FALSE;
	drd.DepthClipEnable			= TRUE;
    hr = g_pd3dDevice->CreateRasterizerState( &drd, &g_pRS );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create rasterizer state." ), _T( "Error" ), MB_OK );
        return hr;
    }
	g_pImmediateContext->RSSetState( g_pRS );

    // ラスタライザの設定(時計回りカリング)
	ZeroMemory( &drd, sizeof( drd ) );
	drd.FillMode				= D3D11_FILL_SOLID;
	drd.CullMode				= D3D11_CULL_BACK;
	drd.FrontCounterClockwise	= TRUE;
	drd.DepthClipEnable			= TRUE;
    hr = g_pd3dDevice->CreateRasterizerState( &drd, &g_pRS_Cull_CW );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create rasterizer state." ), _T( "Error" ), MB_OK );
        return hr;
    }
//    g_pImmediateContext->RSSetState( g_pRS_Cull_CW );

    // ラスタライザの設定(反時計回りカリング)
	ZeroMemory( &drd, sizeof( drd ) );
	drd.FillMode				= D3D11_FILL_SOLID;
	drd.CullMode				= D3D11_CULL_BACK;
	drd.FrontCounterClockwise	= FALSE;
	drd.DepthClipEnable			= TRUE;
    hr = g_pd3dDevice->CreateRasterizerState( &drd, &g_pRS_Cull_CCW );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't create rasterizer state." ), _T( "Error" ), MB_OK );
        return hr;
    }
//    g_pImmediateContext->RSSetState( g_pRS_Cull_CCW );

    // ビューポートの設定
    D3D11_VIEWPORT vp;
    vp.Width    = ( FLOAT )g_nClientWidth;
    vp.Height   = ( FLOAT )g_nClientHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    return S_OK;
}


// プログラマブルシェーダ作成
HRESULT MakeShaders( void )
{
    HRESULT hr;
    ID3DBlob* pVertexShaderBuffer = NULL;
    ID3DBlob* pPixelShaderBuffer = NULL;
    ID3DBlob* pError = NULL;

    DWORD dwShaderFlags = 0;
#ifdef _DEBUG
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
    // コンパイル
    hr = D3DX11CompileFromFile( _T( "Basic_3D_TexMark.fx" ), NULL, NULL, "VS", "vs_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pVertexShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't open Basic_3D_TexMark.fx" ), _T( "Error" ), MB_OK );
        SAFE_RELEASE( pError );
        return hr;
    }
    hr = D3DX11CompileFromFile( _T( "Basic_3D_TexMark.fx" ), NULL, NULL, "PS", "ps_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pPixelShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pError );
        return hr;
    }
    SAFE_RELEASE( pError );
    
    // VertexShader作成
    hr = g_pd3dDevice->CreateVertexShader( pVertexShaderBuffer->GetBufferPointer(),
										   pVertexShaderBuffer->GetBufferSize(),
										   NULL, &g_pVertexShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }
    // PixelShader作成
    hr = g_pd3dDevice->CreatePixelShader( pPixelShaderBuffer->GetBufferPointer(),
										  pPixelShaderBuffer->GetBufferSize(),
										  NULL, &g_pPixelShader );
    if ( FAILED( hr ) ) {
        SAFE_RELEASE( pVertexShaderBuffer );
        SAFE_RELEASE( pPixelShaderBuffer );
        return hr;
    }

    // 入力バッファの入力形式
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );
	// 入力バッファの入力形式作成
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements,
										  pVertexShaderBuffer->GetBufferPointer(),
										  pVertexShaderBuffer->GetBufferSize(),
										  &g_pInputLayout );
    SAFE_RELEASE( pVertexShaderBuffer );
    SAFE_RELEASE( pPixelShaderBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // シェーダ定数バッファ作成
    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof( bd ) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( CBNeverChanges );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;

    return S_OK;
}


// テクスチャロード
int LoadTexture( TCHAR *szFileName, TEX_PICTURE *pTexPic, int nWidth, int nHeight,
				 int nTexWidth, int nTexHeight )
{
    HRESULT						hr;
	D3DX11_IMAGE_LOAD_INFO		liLoadInfo;
	ID3D11Texture2D				*pTexture;

	ZeroMemory( &liLoadInfo, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
	liLoadInfo.Width = nTexWidth;
	liLoadInfo.Height = nTexHeight;
	liLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	liLoadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, szFileName, &liLoadInfo,
												 NULL, &( pTexPic->pSRViewTexture ), NULL );
    if ( FAILED( hr ) ) {
        return hr;
    }
	pTexPic->pSRViewTexture->GetResource( ( ID3D11Resource ** )&( pTexture ) );
	pTexture->GetDesc( &( pTexPic->tdDesc ) );
	pTexture->Release();

	pTexPic->nWidth = nWidth;
	pTexPic->nHeight = nHeight;

	return S_OK;
}


// 描画モードオブジェクト初期化
int InitDrawModes( void )
{
    HRESULT				hr;

	// ブレンドステート
    D3D11_BLEND_DESC BlendDesc;
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;
    BlendDesc.RenderTarget[0].BlendEnable           = TRUE;
    BlendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = g_pd3dDevice->CreateBlendState( &BlendDesc, &g_pbsAddBlend );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // サンプラ
    D3D11_SAMPLER_DESC samDesc;
    ZeroMemory( &samDesc, sizeof( samDesc ) );
    samDesc.Filter          = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressV        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressW        = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.ComparisonFunc  = D3D11_COMPARISON_ALWAYS;
    samDesc.MaxLOD          = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &samDesc, &g_pSamplerState );
    if ( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}


// ジオメトリの初期化
HRESULT InitGeometry( void )
{
    HRESULT hr = S_OK;

    // 頂点バッファ作成
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth            = sizeof( CUSTOMVERTEX ) * MAX_BUFFER_VERTEX;
    BufferDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags            = 0;

    D3D11_SUBRESOURCE_DATA SubResourceData;
    SubResourceData.pSysMem             = g_cvVertices;
    SubResourceData.SysMemPitch         = 0;
    SubResourceData.SysMemSlicePitch    = 0;
    hr = g_pd3dDevice->CreateBuffer( &BufferDesc, &SubResourceData, &g_pVertexBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

    // インデックスバッファ作成
    BufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth            = sizeof( WORD ) * MAX_BUFFER_INDEX;
    BufferDesc.BindFlags            = D3D11_BIND_INDEX_BUFFER;
    BufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags            = 0;

	SubResourceData.pSysMem         = g_wIndices;
    hr = g_pd3dDevice->CreateBuffer( &BufferDesc, &SubResourceData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;

	// テクスチャ作成
	g_tSphere1Texture.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "4.bmp" ), &g_tSphere1Texture, 1152, 576, 1024, 512 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 4.bmp" ), _T( "Error" ), MB_OK );
       return hr;
    }
	g_tSphere2Texture.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "7.bmp" ), &g_tSphere2Texture, 691, 691, 1024, 1024 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 7.bmp" ), _T( "Error" ), MB_OK );
       return hr;
    }

	// モデル作成
	int						nVertexNum1, nIndexNum1;
//	int						nVertexNum2, nIndexNum2;
	// プレイヤー
	MakeSphereIndexed( 0.0f, 0.0f, 0.0f, SPHERE_R,
					   &( g_cvVertices[g_nVertexNum] ), &nVertexNum1,
					   &( g_wIndices[g_nIndexNum] ),    &nIndexNum1, 0 );
	g_mmPlayer.nVertexPos = g_nVertexNum;
	g_mmPlayer.nVertexNum = nVertexNum1;
	g_mmPlayer.nIndexPos = g_nIndexNum;
	g_mmPlayer.nIndexNum = nIndexNum1;
	g_nVertexNum += nVertexNum1;
	g_nIndexNum += nIndexNum1;
	g_mmPlayer.ptpTexture = &g_tSphere2Texture;
	g_mmPlayer.mMatrix = XMMatrixIdentity();
	g_mmPlayer.v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );

	// 当たり判定領域
	MakeCylinderSpIndexed( 0.0f, 0.0f, 0.0f, CYLINDER_SP_LEN, CYLINDER_SP_R,
						   &( g_cvVertices[g_nVertexNum] ), &nVertexNum1,
						   &( g_wIndices[g_nIndexNum] ),    &nIndexNum1 );
	g_mmHit.nVertexPos = g_nVertexNum;
	g_mmHit.nVertexNum = nVertexNum1;
	g_mmHit.nIndexPos = g_nIndexNum;
	g_mmHit.nIndexNum = nIndexNum1;
	g_nVertexNum += nVertexNum1;
	g_nIndexNum += nIndexNum1;
	g_mmHit.ptpTexture = &g_tSphere2Texture;
	g_mmHit.mMatrix = XMMatrixIdentity();
	g_mmHit.v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );

	// 頂点バッファ・インデックスバッファ作成
	D3D11_MAPPED_SUBRESOURCE mappedVertices, mappedIndices;
	hr = g_pImmediateContext->Map( g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertices );
    if( FAILED( hr ) )
        return hr;
	hr = g_pImmediateContext->Map( g_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIndices );
    if( FAILED( hr ) ) {
		g_pImmediateContext->Unmap( g_pVertexBuffer, 0 );
        return hr;
	}
	CopyMemory( mappedVertices.pData,  g_cvVertices, sizeof( CUSTOMVERTEX ) * g_nVertexNum );
	CopyMemory( mappedIndices.pData,  g_wIndices, sizeof( WORD ) * g_nIndexNum );
	g_pImmediateContext->Unmap( g_pVertexBuffer, 0 );
	g_pImmediateContext->Unmap( g_pIndexBuffer, 0 );

	return S_OK;
}


// 終了処理
int Cleanup( void )
{
    SAFE_RELEASE( g_tSphere1Texture.pSRViewTexture );
    SAFE_RELEASE( g_tSphere2Texture.pSRViewTexture );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );

    SAFE_RELEASE( g_pSamplerState );
    SAFE_RELEASE( g_pbsAddBlend );
    SAFE_RELEASE( g_pInputLayout );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pCBNeverChanges );

    SAFE_RELEASE( g_pRS );									// ラスタライザ
    SAFE_RELEASE( g_pRS_Cull_CW );
    SAFE_RELEASE( g_pRS_Cull_CCW );

	// ステータスをクリア
	if ( g_pImmediateContext ) {
		g_pImmediateContext->ClearState();
		g_pImmediateContext->Flush();
	}

    SAFE_RELEASE( g_pRTV );									// レンダリングターゲット
    SAFE_RELEASE( g_pDepthStencil );						// Zバッファ
    SAFE_RELEASE( g_pDepthStencilView );					// Zバッファのビュー
    SAFE_RELEASE( g_pDSDepthState );						// Zバッファのステート
    SAFE_RELEASE( g_pDSDepthState_NoWrite );

    // スワップチェーン
    if ( g_pSwapChain != NULL ) {
        g_pSwapChain->SetFullscreenState( FALSE, 0 );
    }
    SAFE_RELEASE( g_pSwapChain );

    SAFE_RELEASE( g_pImmediateContext );					// デバイスコンテキスト
    SAFE_RELEASE( g_pd3dDevice );							// デバイス

	return 0;
}


// モデルの描画
int DrawMyModel( MY_MODEL *pmmDrawModel, XMMATRIX *pmViewProjection )
{
    CBNeverChanges	cbNeverChanges;

	cbNeverChanges.mView = XMMatrixTranspose( pmmDrawModel->mMatrix * *pmViewProjection );
	cbNeverChanges.v4AddColor = pmmDrawModel->v4AddColor;
	g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &( pmmDrawModel->ptpTexture->pSRViewTexture ) );
	g_pImmediateContext->DrawIndexed( pmmDrawModel->nIndexNum, pmmDrawModel->nIndexPos, pmmDrawModel->nVertexPos );

	return 0;
}


// ウィンドウプロシージャ
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


// レンダリング
HRESULT Render( void )
{
	//int						i;
	//int						bHitResult[3];			// 当たり判定結果

    // 画面クリア
	XMFLOAT4	v4Color = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
    g_pImmediateContext->ClearRenderTargetView( g_pRTV, ( float * )&v4Color );
	// *** Zバッファクリア ***
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    // サンプラセット
    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerState );
    
    // 描画設定
    UINT nStrides = sizeof( CUSTOMVERTEX );
    UINT nOffsets = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &nStrides, &nOffsets );
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    g_pImmediateContext->IASetInputLayout( g_pInputLayout );

    // シェーダ設定
    g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
    g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );
    g_pImmediateContext->PSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );

	//// 三角形との当たり判定
	//for ( i = 0; i < CHECK_TRIANGLE_NUM; i++ ) {
	//	bHitResult[i] = CheckHit( &( g_TriangleVertices[i * 3] ), &( Player_1.v3Pos ) );
	//}
		
	// 変換行列
    CBNeverChanges	cbNeverChanges;
	XMMATRIX		mWorld;
	XMMATRIX		mView;
	XMMATRIX		mProjection;
	XMMATRIX		mViewProjection;

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( Player_1.v3Pos.x, Player_1.v3Pos.y + 3.0f, Player_1.v3Pos.z - 5.0f, 0.0f );
	XMVECTOR At = XMVectorSet( Player_1.v3Pos.x, Player_1.v3Pos.y, Player_1.v3Pos.z, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	mView = XMMatrixLookAtLH( Eye, At, Up );

    // Initialize the projection matrix
	mProjection = XMMatrixPerspectiveFovLH( XM_PIDIV4, VIEW_WIDTH / ( FLOAT )VIEW_HEIGHT, 0.01f, 100.0f );

	mViewProjection = mView * mProjection;

    // 描画
	g_pImmediateContext->OMSetDepthStencilState( g_pDSDepthState, 1 );
    g_pImmediateContext->RSSetState( g_pRS_Cull_CW );				// カリングあり

	// 当たり判定領域
    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
	g_mmHit.mMatrix = HitArea.matTransform;
	DrawMyModel( &g_mmHit, &mViewProjection );

	// プレイヤー
    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
	if ( CheckHit( &( HitArea.v3Pos ), &( HitArea.v3Vec ), HitArea.r,
				   &( Player_1.v3Pos ), Player_1.r ) )
	{
		g_mmPlayer.v4AddColor = XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f );
	}
	else {
		g_mmPlayer.v4AddColor = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	}
	g_mmPlayer.mMatrix = CreateWorldMatrix( Player_1.v3Pos.x, Player_1.v3Pos.y, Player_1.v3Pos.z, 1.0f );
	DrawMyModel( &g_mmPlayer, &mViewProjection );

    return S_OK;
}


// エントリポイント
int WINAPI _tWinMain( HINSTANCE hInst, HINSTANCE, LPTSTR, int )
{
	LARGE_INTEGER			nNowTime, nLastTime;		// 現在とひとつ前の時刻
	LARGE_INTEGER			nTimeFreq;					// 時間単位

    // 画面サイズ
    g_nClientWidth  = VIEW_WIDTH;						// 幅
    g_nClientHeight = VIEW_HEIGHT;						// 高さ

	// Register the window class
    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
                      GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
                      _T( "D3D Sample" ), NULL };
    RegisterClassEx( &wc );

	RECT rcRect;
	SetRect( &rcRect, 0, 0, g_nClientWidth, g_nClientHeight );
	AdjustWindowRect( &rcRect, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( _T( "D3D Sample" ), _T( "3DCheckHit_1_2" ),
						   WS_OVERLAPPEDWINDOW, 100, 20, rcRect.right - rcRect.left, rcRect.bottom - rcRect.top,
						   GetDesktopWindow(), NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D() ) && SUCCEEDED( MakeShaders() ) )
    {
        // Create the shaders
        if( SUCCEEDED( InitDrawModes() ) )
        {
			if ( SUCCEEDED( InitGeometry() ) ) {					// ジオメトリ作成
				InitArea();											// 当たり判定領域の初期化
				InitPlayer();										// プレイヤーの初期化
				// Show the window
				ShowWindow( g_hWnd, SW_SHOWDEFAULT );
				UpdateWindow( g_hWnd );
				
				QueryPerformanceFrequency( &nTimeFreq );			// 時間単位
				QueryPerformanceCounter( &nLastTime );				// 1フレーム前時刻初期化

				// Enter the message loop
				MSG msg;
				ZeroMemory( &msg, sizeof( msg ) );
				while( msg.message != WM_QUIT )
				{
					MoveArea();
					MovePlayer();
					Render();
					do {
						if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
						{
							TranslateMessage( &msg );
							DispatchMessage( &msg );
						}
						QueryPerformanceCounter( &nNowTime );
					} while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 90 ) ) &&
							 ( msg.message != WM_QUIT ) );
					while( ( ( nNowTime.QuadPart - nLastTime.QuadPart ) < ( nTimeFreq.QuadPart / 60 ) ) &&
						   ( msg.message != WM_QUIT ) )
					{
						QueryPerformanceCounter( &nNowTime );
					}
					nLastTime = nNowTime;
					g_pSwapChain->Present( 0, 0 );					// 表示
				}
			}
        }
    }

    // Clean up everything and exit the app
    Cleanup();
    UnregisterClass( _T( "D3D Sample" ), wc.hInstance );
    return 0;
}

