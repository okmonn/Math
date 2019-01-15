// ---------------------------------------------------------
// Basic_3D_Light.fx
// 3D光源処理付きシェーダ
// ---------------------------------------------------------

// 定数バッファ
cbuffer cbNeverChanges : register( b0 )
{
    matrix View;
    matrix World;
	float3 v3Light;
	float  Ambient;
	float  Directional;
};


// VertexShader入力形式
struct VS_INPUT {
    float4 v4Position	: POSITION;		// 位置
    float3 v3Normal		: NORMAL;		// 法線ベクトル
};

// VertexShader出力形式
struct VS_OUTPUT {
    float4 v4Position	: SV_POSITION;	// 位置
    nointerpolation float3 v3Normal		: NORMAL;		// 法線
    float4 v4Color : COLOR;//色
    
};

// 頂点シェーダ
VS_OUTPUT VS( VS_INPUT Input )
{
    VS_OUTPUT	Output;
	float3		v3WorldNormal;
	float		fPlusDot;
	float		fBright;

    Output.v4Position = mul( Input.v4Position, View );
	v3WorldNormal = mul( Input.v3Normal, World );//法線にワールド行列をかけている
	Output.v3Normal = v3WorldNormal;
	fPlusDot = max( 0, 0.0 );
    fBright = Directional * fPlusDot + Ambient;
    return Output;
}

// ピクセルシェーダ
float4 PS( VS_OUTPUT Input ) : SV_TARGET {
    return float4(Input.v4Color.xyz,1.0f);
}
