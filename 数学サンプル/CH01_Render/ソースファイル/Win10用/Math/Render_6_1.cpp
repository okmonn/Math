//------------------------------------------------------------
// Render_6_1.cpp
// バイリニアフィルタなし
// 
//------------------------------------------------------------

#include <windows.h>
#include <math.h>

#define PI					3.1415927f			// 円周率
#define VIEW_WIDTH			640					// 画面幅
#define VIEW_HEIGHT			480					// 画面高さ


// ビットマップ構造体
struct MemoryBMP {									// ＢＭＰデータ構造体
	int					nDataSize;					// データサイズ
	BITMAPFILEHEADER	*pbfFileH;					// ファイルヘッダ
	BITMAPINFOHEADER	*pbiInfoH;					// インフォヘッダ
	unsigned char		*pcPixels;					// ピクセルデータ
	int					nPixelOffset;				// ピクセルまでのオフセット
	int					nPaletteNum;				// パレット数
	int					nPitch;						// ピッチ幅
};

MemoryBMP			mbPicture;						// ビットマップの絵

int GetBMPPixel( int x, int y, MemoryBMP *pmbSrc,
				 int *pnRed, int *pnGreen, int *pnBlue );
													// ピクセルの取得
int FlushDrawingPictures( void );				// 絵の描画待ち行列フラッシュ
int DrawPoints( int x, int y, int nRed, int nGreen, int nBlue );
												// 1点の描画

// スキャンラインアルゴリズムによるレンダリング
//ひとまず実行ファイルのRender_6_1.exeとRender_6_2.exeを比較していただければわかりますが
//Render_6_1.exeを実行するとモザイク状になっているのがわかるでしょうか。
//今回は元の画像を20倍して30°傾けています。
//このプログラムでは改造のしようがないので、6_2を開いてください。
int RenderScanLine( void )
{
	int				x, y;
	float				bx, by;				// ローカルベース座標
	float				cx, cy;				// ローカル座標
	float				vBase1_x, vBase1_y;		// 基底ベクトル1
	float				vBase2_x, vBase2_y;		// 基底ベクトル2
	int				nRed, nGreen, nBlue;		// ピクセル色
	float				fAngle = PI / 6.0f;		// 回転角

	vBase1_x = 0.05f * cosf( fAngle );				// 基底ベクトル1
	vBase1_y = 0.05f * sinf( fAngle );
	vBase2_x = 0.05f * cosf( fAngle + PI / 2.0f );	// 基底ベクトル2
	vBase2_y = 0.05f * sinf( fAngle + PI / 2.0f );
	bx = -VIEW_WIDTH  / 2.0f * vBase1_x + -VIEW_HEIGHT / 4.0f * vBase2_x; 
	by = -VIEW_WIDTH  / 2.0f * vBase1_y + -VIEW_HEIGHT / 4.0f * vBase2_y; 
	for ( y = 0; y < VIEW_HEIGHT; y++ ) {		// y方向ループ
		cx = bx; 
		cy = by; 
		for ( x = 0; x < VIEW_WIDTH; x++ ) {	// x方向ループ
			GetBMPPixel( ( int )( cx + VIEW_WIDTH  / 2.0f ),
						 ( int )( cy + VIEW_HEIGHT / 4.0f ),
						 &mbPicture,
						 &nRed, &nGreen, &nBlue );		// 点取得
			DrawPoints( x, y, nRed, nGreen, nBlue );	// 点描画
			cx += vBase1_x;
			cy += vBase1_y;
		}
		FlushDrawingPictures();
		bx += vBase2_x;
		by += vBase2_y;
	}

	return 0;
}


//------------------------------------------------------------
// 以下、DirectXによる表示プログラム

#include <stdio.h>
#include <tchar.h>								// Unicode・マルチバイト文字関係

#include <D3D11.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>


#define MAX_BUFFER_VERTEX				20000	// 最大バッファ頂点数


// リンクライブラリ
#pragma comment( lib, "d3d11.lib" )   // D3D11ライブラリ
#pragma comment( lib, "d3dcompiler.lib" )


// セーフリリースマクロ
#ifndef SAFE_RELEASE
#define SAFE_RELEASE( p )      { if ( p ) { ( p )->Release(); ( p )=NULL; } }
#endif

using namespace DirectX;

// 頂点構造体
struct CUSTOMVERTEX {
    XMFLOAT4	v4Pos;
    XMFLOAT4	v4Color;
};

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
D3D_FEATURE_LEVEL       g_FeatureLevel;			// フィーチャーレベル

ID3D11Buffer			*g_pD3D11VertexBuffer;	// 頂点バッファ
ID3D11BlendState		*g_pbsAlphaBlend;		// アルファブレンド
ID3D11VertexShader		*g_pVertexShader;		// 頂点シェーダ
ID3D11PixelShader		*g_pPixelShader;		// ピクセルシェーダ
ID3D11InputLayout		*g_pInputLayout;		// シェーダ入力レイアウト
ID3D11SamplerState		*g_pSamplerState;		// サンプラステート

ID3D11Buffer			*g_pCBNeverChanges = NULL;

//TEX_PICTURE				g_tBall, g_tBack;

// 描画頂点バッファ
CUSTOMVERTEX g_cvVertices[MAX_BUFFER_VERTEX];
int							g_nVertexNum = 0;
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

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRTV, NULL );

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
    hr = D3DCompileFromFile( _T( "Basic_2D_Geom.fx" ), nullptr, nullptr, "VS", "vs_4_0_level_9_1",
								dwShaderFlags, 0,  &pVertexShaderBuffer, &pError );
    if ( FAILED( hr ) ) {
		MessageBox( NULL, _T( "Can't open Basic_2D_Geom.fx" ), _T( "Error" ), MB_OK );
        SAFE_RELEASE( pError );
        return hr;
    }
    hr = D3DCompileFromFile( _T( "Basic_2D_Geom.fx" ), nullptr, nullptr, "PS", "ps_4_0_level_9_1",
								dwShaderFlags, 0,  &pPixelShaderBuffer, &pError );
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
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//        { "TEXTURE",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
	mScreen.r[0].m128_f32[0] =  2.0f / g_nClientWidth;
	mScreen.r[1].m128_f32[1] = -2.0f / g_nClientHeight;
	mScreen.r[3].m128_f32[0] = -1.0f;
	mScreen.r[3].m128_f32[1] =  1.0f;
	cbNeverChanges.mView = XMMatrixTranspose( mScreen );
	g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

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
    BlendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    BlendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = g_pd3dDevice->CreateBlendState( &BlendDesc, &g_pbsAlphaBlend );
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


int LoadBMPData( char *szFileName, MemoryBMP *pmbDest )		// ＢＭＰファイルロード
{
	FILE				*fp;						// ファイルポインタ
	unsigned char		*pcBMPBuf;					// ＢＭＰデータバッファ

	// ファイルオープン
	if ( fopen_s( &fp, szFileName, "rb" ) != 0 ) {
//	if ( ( fp = fopen( szFileName, "rb" ) ) == NULL ) {
		return -1;
	}

	// ファイルサイズ取得
	fseek( fp, 0, SEEK_END );						// 最後までシーク
	pmbDest->nDataSize = ftell( fp );				// 位置を調べる
	fseek( fp, 0, SEEK_SET );						// 最初に戻す

	// メモリ確保・ファイルロード
	pcBMPBuf = ( unsigned char * )malloc( pmbDest->nDataSize );	// メモリ確保
	fread( pcBMPBuf, 1, pmbDest->nDataSize, fp );	// ファイルロード

	fclose( fp );									// ファイルクローズ
	
	pmbDest->pbfFileH = ( BITMAPFILEHEADER * )pcBMPBuf;	// ファイルヘッダ
	pmbDest->pbiInfoH = ( BITMAPINFOHEADER * )( pcBMPBuf + sizeof( BITMAPFILEHEADER ) );	// インフォヘッダ

	if ( pmbDest->pbiInfoH->biBitCount != 24 ) return -1;
	// パレットデータ数の決定
	if ( pmbDest->pbiInfoH->biBitCount <= 8 ) {			// ２５６色以下ならパレットデータあり
		if ( pmbDest->pbiInfoH->biClrUsed == 0 ) {		// デフォルトの場合
			pmbDest->nPaletteNum = 1 << pmbDest->pbiInfoH->biBitCount;
		}
		else {
			pmbDest->nPaletteNum = pmbDest->pbiInfoH->biClrUsed;
		}
	}

	// ピクセル位置
	pmbDest->nPixelOffset = sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER )
						  + pmbDest->nPaletteNum * sizeof( RGBQUAD );
	pmbDest->pcPixels = pcBMPBuf + pmbDest->nPixelOffset;

	// ピッチ計算
	pmbDest->nPitch = ( pmbDest->pbiInfoH->biBitCount * pmbDest->pbiInfoH->biWidth / 8 + 3 ) & 0xfffc;

	return 0;
}


int ReleaseBMPData( MemoryBMP *pmbRelease )			// ＢＭＰデータ開放
{
	if ( pmbRelease->pbfFileH ) {					// メモリが残っているか
		free( pmbRelease->pbfFileH );				// 開放
		pmbRelease->pbfFileH = NULL;
	}

	return 0;
}


// ピクセルの取得
int GetBMPPixel( int x, int y, MemoryBMP *pmbSrc,
				 int *pnRed, int *pnGreen, int *pnBlue )
{
	unsigned char		*pLineHead, *pPixelLoc;			// bmp読み出し用

	if ( ( x >= 0 ) && ( x < pmbSrc->pbiInfoH->biWidth ) &&
		 ( y >= 0 ) && ( y < pmbSrc->pbiInfoH->biHeight ) )
	{
		pLineHead = pmbSrc->pcPixels + pmbSrc->nPitch * ( pmbSrc->pbiInfoH->biHeight - 1 );
		pPixelLoc = pLineHead - pmbSrc->nPitch * y + x * 3;
		*pnRed   = ( int )*( pPixelLoc + 2 );
		*pnGreen = ( int )*( pPixelLoc + 1 );
		*pnBlue  = ( int )*pPixelLoc;
	}
	else {
		*pnRed   = 0;
		*pnGreen = 0;
		*pnBlue  = 255;
	}

	return 0;
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
    hr = g_pd3dDevice->CreateBuffer( &BufferDesc, &SubResourceData, &g_pD3D11VertexBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

	// 絵のロード
	if ( LoadBMPData( "1.bmp", &mbPicture ) < 0 ) {
 		MessageBox( NULL, _T( "Can't open 1.BMP" ), _T( "Error" ), MB_OK );
		return -1;
	}

	return S_OK;
}


// 終了処理
int Cleanup( void )
{
	ReleaseBMPData( &mbPicture );
	SAFE_RELEASE( g_pD3D11VertexBuffer );

    SAFE_RELEASE( g_pSamplerState );
    SAFE_RELEASE( g_pbsAlphaBlend );
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


// 絵の描画待ち行列フラッシュ
int FlushDrawingPictures( void )
{
	HRESULT			hr;

	if ( g_nVertexNum > 0 ) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		hr = g_pImmediateContext->Map( g_pD3D11VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		if ( SUCCEEDED( hr ) ) {
			CopyMemory( mappedResource.pData, &( g_cvVertices[0] ), sizeof( CUSTOMVERTEX ) * g_nVertexNum );
			g_pImmediateContext->Unmap( g_pD3D11VertexBuffer, 0 );
		}
		g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pNowTexture );
		g_pImmediateContext->Draw( g_nVertexNum, 0 );
	}
	g_nVertexNum = 0;
	g_pNowTexture = NULL;

	return 0;
}


// 1点の描画
int DrawPoints( int x, int y, int nRed, int nGreen, int nBlue )
{
	g_cvVertices[g_nVertexNum].v4Pos   = XMFLOAT4( ( float )x, ( float )y, 0.0f, 1.0f );
	g_cvVertices[g_nVertexNum].v4Color = XMFLOAT4( nRed / 255.0f, nGreen / 255.0f, nBlue / 255.0f, 1.0f );
	g_nVertexNum++;

	return 0;
}


// レンダリング
HRESULT Render( void )
{
//	int				i, j;
    // 画面クリア
	XMFLOAT4	v4Color = XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f );
    g_pImmediateContext->ClearRenderTargetView( g_pRTV, ( float * )&v4Color );

    // サンプラ・ラスタライザセット
    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerState );
    g_pImmediateContext->RSSetState( g_pRS );
    
    // 描画設定
    UINT nStrides = sizeof( CUSTOMVERTEX );
    UINT nOffsets = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pD3D11VertexBuffer, &nStrides, &nOffsets );
//    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
    g_pImmediateContext->IASetInputLayout( g_pInputLayout );

    // シェーダ設定
    g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
    g_pImmediateContext->PSSetShader( g_pPixelShader, NULL, 0 );

    // 描画
    g_pImmediateContext->OMSetBlendState( NULL, NULL, 0xFFFFFFFF );
	RenderScanLine();
//	DrawPicture( 0.0f, 0.0f, &g_tBack );

    // 表示
	FlushDrawingPictures();

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
    g_hWnd = CreateWindow( _T( "D3D Sample" ), _T( "Render_6_1" ),
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

//				InitCharacter();									// キャラクタ初期化
				
				QueryPerformanceFrequency( &nTimeFreq );			// 時間単位
				QueryPerformanceCounter( &nLastTime );				// 1フレーム前時刻初期化

				// Enter the message loop
				MSG msg;
				ZeroMemory( &msg, sizeof( msg ) );
				while( msg.message != WM_QUIT )
				{
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
