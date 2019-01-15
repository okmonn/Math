//------------------------------------------------------------
// Textures_5_3.cpp
// 連続発生する球形爆発
// 
//------------------------------------------------------------

#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <xnamath.h>

#define VIEW_WIDTH			800					// 画面幅
#define VIEW_HEIGHT			600					// 画面高さ

#define PI					3.1415927f			// 円周率
#define ROT_SPEED			( PI / 100.0f )		// 回転速度
#define R					1.0f				// 図形の大きさ
#define CORNER_NUM			20					// 角数
#define MAX_EXPLOSION_NUM	10					// 最大爆発数
#define EXPLOSION_INTERVAL	200					// 爆発の発生間隔(ミリ秒単位)
#define EXPLOSION_AREA		4.0f				// 爆発の発生範囲
#define EXPLOSION_SPEED		0.05f				// 爆発の拡大速度
#define EXPLOSION_R1		1.0f				// 明るさが落ち始める半径
#define EXPLOSION_R2		2.0f				// 消滅する半径


// 頂点構造体
struct CUSTOMVERTEX {
    XMFLOAT4	v4Pos;
	XMFLOAT2	v2UV;
};

// 爆発構造体
struct Explosion {
	int			bActive;						// 有効フラグ
	int			nTime;							// 発生してからの時間
	XMFLOAT3	v3CenterPos;					// 中心位置
	float		fRadius;						// 半径
	float		fAngle;							// 角度
	XMMATRIX	matMatrix;						// 変換行列
	float		fBright;						// 明るさ
};


Explosion		exExplode[MAX_EXPLOSION_NUM];	// 爆発バッファ


int MakeSphereIndexed( CUSTOMVERTEX *pVertices, int *pVertexNum,
					   WORD *pIndices, int *pIndexNum )		// 球の作成(インデックス付き)
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
	for ( i = 0; i < CORNER_NUM / 4 + 1; i++ ) {
		fPhi = 0.0f;
		for ( j = 0; j < CORNER_NUM + 1; j++ ) {
			pVertices[nIndex].v4Pos  = XMFLOAT4( R * sinf( fTheta ) * cosf( fPhi ),
												 R * cosf( fTheta ),
												 R * sinf( fTheta ) * sinf( fPhi ), 1.0f );
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
		for ( j = 0; j < CORNER_NUM / 4; j++ ) {
			nIndexY = j * ( CORNER_NUM + 1 );
			pIndices[nIndex    ] = nIndexY + i;
			pIndices[nIndex + 1] = nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 2] = nIndexY + i + 1;
			nIndex += 3;
			pIndices[nIndex    ] = nIndexY + i + 1;
			pIndices[nIndex + 1] = nIndexY + ( CORNER_NUM + 1 ) + i;
			pIndices[nIndex + 2] = nIndexY + ( CORNER_NUM + 1 ) + i + 1;
			nIndex += 3;
		}
	}
	*pIndexNum = nIndex;

	return 0;
}


// 爆発の動作
int MoveExplosion( void )
{
	int					i;
	int					nNowTime;				// 今の時刻(爆発間隔ベース)
	static int			nLastTime = 0;			// 前の時刻(爆発間隔ベース)
	float				x, z;					// x, z座標

	// 爆発の発生
	nNowTime = timeGetTime() / EXPLOSION_INTERVAL;
	if ( nNowTime != nLastTime ) {
		for ( i = 0; i < MAX_EXPLOSION_NUM; i++ ) {
			if ( !exExplode[i].bActive ) {
				exExplode[i].bActive = true;
				exExplode[i].nTime = 0;
				x = ( rand() * EXPLOSION_AREA / RAND_MAX - EXPLOSION_AREA / 2.0f );
				z = ( rand() * EXPLOSION_AREA / RAND_MAX - EXPLOSION_AREA / 2.0f );
				exExplode[i].v3CenterPos = XMFLOAT3( x, 0.0f, z );
				exExplode[i].fAngle = 0.0f;
				exExplode[i].fBright = 1.0f;
				exExplode[i].fRadius = 0.1f;
				exExplode[i].matMatrix = XMMatrixIdentity();
				break;
			}
		}
	}
	nLastTime = nNowTime;

	// 爆発の制御
	for ( i = 0; i < MAX_EXPLOSION_NUM; i++ ) {
		if ( exExplode[i].bActive ) {
			exExplode[i].fRadius += EXPLOSION_SPEED;
			if ( exExplode[i].fRadius > EXPLOSION_R2 ) {
				exExplode[i].bActive = false;
			}
			else {
				if ( exExplode[i].fRadius > EXPLOSION_R1 ) {
					exExplode[i].fBright = ( EXPLOSION_R2 - exExplode[i].fRadius ) /
										   ( EXPLOSION_R2 - EXPLOSION_R1 );
				}
				//ここに色々と書く





			}
		}
	}

	return 0;
}


//------------------------------------------------------------
// 以下、DirectXによる表示プログラム

#include <stdio.h>
#include <windows.h>
#include <tchar.h>								// Unicode・マルチバイト文字関係


#define MAX_BUFFER_VERTEX				10000	// 最大バッファ頂点数
#define MAX_BUFFER_INDEX				20000	// 最大バッファインデックス数


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
};

// テクスチャ絵構造体
struct TEX_PICTURE {
	ID3D11ShaderResourceView	*pSRViewTexture;
	D3D11_TEXTURE2D_DESC		tdDesc;
	int							nWidth, nHeight;
};


// グローバル変数
UINT  g_nClientWidth;							// 描画領域の横幅
UINT  g_nClientHeight;							// 描画領域の高さ

HWND        g_hWnd;         // ウィンドウハンドル


ID3D11Device			*g_pd3dDevice;			// デバイス
IDXGISwapChain			*g_pSwapChain;			// DXGIスワップチェイン
ID3D11DeviceContext		*g_pImmediateContext;	// デバイスコンテキスト
ID3D11RasterizerState	*g_pRS;					// ラスタライザ
ID3D11RenderTargetView	*g_pRTV;				// レンダリングターゲット
ID3D11Texture2D*        g_pDepthStencil = NULL;	// Zバッファ
ID3D11DepthStencilView* g_pDepthStencilView = NULL;	// Zバッファのビュー
ID3D11DepthStencilState *g_pDSStencilState = NULL;	// Zバッファのステート
D3D_FEATURE_LEVEL       g_FeatureLevel;			// フィーチャーレベル

ID3D11Buffer			*g_pVertexBuffer;		// 頂点バッファ
ID3D11Buffer			*g_pIndexBuffer;		// インデックスバッファ
ID3D11BlendState		*g_pbsAddBlend;		// 加算ブレンド
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

TEX_PICTURE				g_tTexture;

// 現在のテクスチャ
ID3D11ShaderResourceView	*g_pNowTexture = NULL;


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
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;//D3D11_DEPTH_WRITE_MASK_ALL;
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
	hr = g_pd3dDevice->CreateDepthStencilState( &dsDesc, &g_pDSStencilState );

	g_pImmediateContext->OMSetDepthStencilState( g_pDSStencilState, 1 );

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
    hr = D3DX11CompileFromFile( _T( "Basic_3D_Tex.fx" ), NULL, NULL, "VS", "vs_4_0_level_9_1",
								dwShaderFlags, 0, NULL, &pVertexShaderBuffer, &pError, NULL );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't open Basic_3D_Tex.fx" ), _T( "Error" ), MB_OK );
        SAFE_RELEASE( pError );
        return hr;
    }
    hr = D3DX11CompileFromFile( _T( "Basic_3D_Tex.fx" ), NULL, NULL, "PS", "ps_4_0_level_9_1",
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

	// 変換行列
    CBNeverChanges	cbNeverChanges;
	XMMATRIX		mScreen;
    mScreen = XMMatrixIdentity();
	mScreen._11 =  2.0f / g_nClientWidth;
	mScreen._22 = -2.0f / g_nClientHeight;
	mScreen._41 = -1.0f;
	mScreen._42 =  1.0f;
	cbNeverChanges.mView = XMMatrixTranspose( mScreen );
	g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

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
    BlendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_BLEND_FACTOR;//D3D11_BLEND_ONE;
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

	// モデルデータ作成
	D3D11_MAPPED_SUBRESOURCE mappedVertices, mappedIndices;
	hr = g_pImmediateContext->Map( g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertices );
    if( FAILED( hr ) )
        return hr;
	hr = g_pImmediateContext->Map( g_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIndices );
    if( FAILED( hr ) ) {
		g_pImmediateContext->Unmap( g_pVertexBuffer, 0 );
        return hr;
	}
	MakeSphereIndexed( ( CUSTOMVERTEX * )mappedVertices.pData, &g_nVertexNum,
					   ( WORD * )mappedIndices.pData, &g_nIndexNum );
	g_pImmediateContext->Unmap( g_pVertexBuffer, 0 );
	g_pImmediateContext->Unmap( g_pIndexBuffer, 0 );

	// テクスチャ作成
	g_tTexture.pSRViewTexture =  NULL;
	hr = LoadTexture( _T( "6.bmp" ), &g_tTexture, 512, 512, 512, 512 );
    if ( FAILED( hr ) ) {
 		MessageBox( NULL, _T( "Can't open 6.bmp" ), _T( "Error" ), MB_OK );
       return hr;
    }

	return S_OK;
}


// 終了処理
int Cleanup( void )
{
    SAFE_RELEASE( g_tTexture.pSRViewTexture );
    SAFE_RELEASE( g_pVertexBuffer );
    SAFE_RELEASE( g_pIndexBuffer );

    SAFE_RELEASE( g_pSamplerState );
    SAFE_RELEASE( g_pbsAddBlend );
    SAFE_RELEASE( g_pInputLayout );
    SAFE_RELEASE( g_pPixelShader );
    SAFE_RELEASE( g_pVertexShader );
    SAFE_RELEASE( g_pCBNeverChanges );

    SAFE_RELEASE( g_pRS );									// ラスタライザ

	// ステータスをクリア
	if ( g_pImmediateContext ) {
		g_pImmediateContext->ClearState();
		g_pImmediateContext->Flush();
	}

    SAFE_RELEASE( g_pRTV );									// レンダリングターゲット
    SAFE_RELEASE( g_pDepthStencil );						// Zバッファ
    SAFE_RELEASE( g_pDepthStencilView );					// Zバッファのビュー
    SAFE_RELEASE( g_pDSStencilState );						// Zバッファのステート

    // スワップチェーン
    if ( g_pSwapChain != NULL ) {
        g_pSwapChain->SetFullscreenState( FALSE, 0 );
    }
    SAFE_RELEASE( g_pSwapChain );

    SAFE_RELEASE( g_pImmediateContext );					// デバイスコンテキスト
    SAFE_RELEASE( g_pd3dDevice );							// デバイス

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
	int				i, j;

    // 画面クリア
	XMFLOAT4	v4Color = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
    g_pImmediateContext->ClearRenderTargetView( g_pRTV, ( float * )&v4Color );
	// *** Zバッファクリア ***
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    // サンプラ・ラスタライザセット
    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerState );
    g_pImmediateContext->RSSetState( g_pRS );
    
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

	// 変換行列
    CBNeverChanges	cbNeverChanges;
	XMMATRIX		mWorld;
	XMMATRIX		mView;
	XMMATRIX		mProjection;

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0.0f, 3.0f, -5.0f, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	mView = XMMatrixLookAtLH( Eye, At, Up );

	// Initialize the projection matrix
	mProjection = XMMatrixPerspectiveFovLH( XM_PIDIV4, VIEW_WIDTH / ( FLOAT )VIEW_HEIGHT, 0.01f, 100.0f );

    // 描画
	float		fFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
//    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );		// 爆発の制御
	for ( i = 0; i < MAX_EXPLOSION_NUM; i++ ) {
		if ( exExplode[i].bActive ) {
			mWorld = exExplode[i].matMatrix;

			cbNeverChanges.mView = XMMatrixTranspose( mWorld * mView * mProjection );
			g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

			for ( j = 0; j < 3; j++ ) fFactor[j] = exExplode[i].fBright;
			g_pImmediateContext->OMSetBlendState( g_pbsAddBlend, fFactor, 0xFFFFFFFF );
			g_pImmediateContext->PSSetShaderResources( 0, 1, &( g_tTexture.pSRViewTexture ) );
			g_pImmediateContext->DrawIndexed( g_nIndexNum, 0, 0 );
		}
	}

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
    g_hWnd = CreateWindow( _T( "D3D Sample" ), _T( "Textures_5_3" ),
						   WS_OVERLAPPEDWINDOW, 100, 20, rcRect.right - rcRect.left, rcRect.bottom - rcRect.top,
						   GetDesktopWindow(), NULL, wc.hInstance, NULL );

    // Initialize Direct3D
    if( SUCCEEDED( InitD3D() ) && SUCCEEDED( MakeShaders() ) )
    {
        // Create the shaders
        if( SUCCEEDED( InitDrawModes() ) )
        {
			if ( SUCCEEDED( InitGeometry() ) ) {					// ジオメトリ作成

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
					MoveExplosion();
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

