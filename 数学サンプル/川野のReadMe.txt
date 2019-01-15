ここの数学サンプルは2010年ごろの
DX11(OSがWin7だったころのDirectX2010June)を想定して作られています。
このため、そのままでコンパイルして動かすことは出来ません。

色々とプログラム的な手間が必要になります。

これを書いてる時点で、例の書籍のソースコードは更新されていないみたいですし
自前で何とかするしか無いです。
ホンマは、皆さんゲームプログラマ３年生なんですから？
自分の力のみで何とかして欲しいところですが、まぁ大半が無理でしょうし。

どういう変更かというとD3DX系統が尽く使用不可となっており、それの代替物を
探して、それを使用するようにプログラムを書き換える必要が出てきます。

CH02_Polygonsまでは
①インクルード文の変更
②リンクするライブラリの変更
③using namespace DirectX;の追加
④CreateShaderFromFileの変更
⑤行列の仕様が変更されているので、その変更

でなんとかなります。
①インクルード文の変更
#include <D3D11.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

②リンクするライブラリの変更
#pragma comment( lib, "d3d11.lib" )   // D3D11ライブラリ
#pragma comment( lib, "d3dcompiler.lib" )

③using namespace DirectX;の追加


④CreateShaderFromFileの変更
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


⑤行列の仕様が変更されているので、その変更
mScreen = XMMatrixIdentity();
mScreen.r[0].m128_f32[0] =  2.0f / g_nClientWidth;
mScreen.r[1].m128_f32[1] = -2.0f / g_nClientHeight;
mScreen.r[3].m128_f32[0] = -1.0f;
mScreen.r[3].m128_f32[1] =  1.0f;


で済んでました。人によってはこれでも大変でしょうが、この程度で済むだけマシです。

ところがCH03_Texturesからはさらに面倒なことになります。
テクスチャロード関連がなくなってるので、基本的には自前でやらなければなりません。
とはいえ、トップレベルなら兎も角、全員には無理でしょう( ＾∀＾)ﾉｼ

ということで、DirectXTexというライブラリを使用します。
https://github.com/Microsoft/DirectXTex
Microsoftのライブラリなので、まぁ信用していいでしょう。
Clone Or Downloadをクリックしてダウンロードしてください。

で、DirectXTex_Desktop_2015_Win10.slnをコンパイル。出来上がったライブラリへのパスを通してリンク
インクルードファイルとしてDirectXTexの中にあるhファイルとinlファイルへのパスを通しておきます。

ただ、これだけではpngやbmpのロードができません。このためWICTextureLoaderを使用します。
WICTextureLoaderフォルダの中に行って、WICTextureLoader.hをインクルードし、また、cppファイルの方を
プロジェクトに追加します。


そしてこれで終わらない。
テクスチャロード部分にバグが発生しているため、
hr=CreateWICTextureFromFile(g_pd3dDevice,g_pImmediateContext , szFileName, &resource, &(pTexPic->pSRViewTexture));

に書き換えます。
ただ、これでも実行時エラーが発生します。実はこれらのライブラリはCOMを基盤につくられているため
ComInitializeしなければなりません。

WinMainにてCreateWindowの直後くらいに
HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
と記述してください。

あーーーー！！！！
くっっっっっっっっっそ！！！めんどくせえーーーーー！！！
とりあえずここまでやれば表示させることができます。めんどくさいけど必須なのでお願いします。
